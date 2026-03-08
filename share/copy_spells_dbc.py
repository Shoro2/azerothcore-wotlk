#!/usr/bin/env python3
"""
Copy specific spells by ID from a source Spell.dbc into a new target Spell.dbc.

Usage:
    python3 copy_spells_dbc.py <source.dbc> <target.dbc> <spell_id> [spell_id ...]
    python3 copy_spells_dbc.py <source.dbc> <target.dbc> --file <id_list.txt>

Examples:
    python3 copy_spells_dbc.py Spell.dbc SpellOut.dbc 133 116 5143
    python3 copy_spells_dbc.py Spell.dbc SpellOut.dbc --file spell_ids.txt

The id list file should contain one spell ID per line (blank lines and
lines starting with # are ignored).
"""

import argparse
import struct
import sys
from pathlib import Path

DBC_HEADER_FORMAT = "<4s4I"  # magic(4) + record_count, field_count, record_size, string_table_size
DBC_HEADER_SIZE = 20
DBC_MAGIC = b"WDBC"


def read_dbc(path):
    """Read a Spell.dbc and return header info, raw records keyed by spell ID,
    and the full string table."""
    data = Path(path).read_bytes()

    magic, record_count, field_count, record_size, string_table_size = struct.unpack_from(
        DBC_HEADER_FORMAT, data, 0
    )
    if magic != DBC_MAGIC:
        sys.exit(f"Error: {path} is not a valid DBC file (magic: {magic!r})")

    records_start = DBC_HEADER_SIZE
    string_table_start = records_start + record_count * record_size
    string_table = data[string_table_start : string_table_start + string_table_size]

    records = {}
    for i in range(record_count):
        offset = records_start + i * record_size
        record_data = data[offset : offset + record_size]
        spell_id = struct.unpack_from("<I", record_data, 0)[0]
        records[spell_id] = record_data

    return {
        "field_count": field_count,
        "record_size": record_size,
        "records": records,
        "string_table": string_table,
    }


def collect_string_offsets(record_data, fmt):
    """Return the set of string-table offsets referenced by a record."""
    offsets = set()
    pos = 0
    for ch in fmt:
        if ch == "s":
            off = struct.unpack_from("<I", record_data, pos)[0]
            offsets.add(off)
            pos += 4
        elif ch in ("n", "i", "f", "x"):
            pos += 4
        elif ch in ("X", "b"):
            pos += 1
    return offsets


def extract_string(string_table, offset):
    """Extract a null-terminated string from the string table."""
    end = string_table.index(b"\x00", offset)
    return string_table[offset:end]


# Spell.dbc format string from DBCfmt.h (234 chars, 936 bytes per record)
SPELL_FMT = (
    "niiiiiiiiiiiixixiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiifx"
    "iiiiiiiiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiiifff"
    "iiiiiiiiiiiiiissssssssssssssssxssssssssssssssss"
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxiiiiiiiiiiixfffxxxiiiiixxfffxx"
)


def write_dbc(path, field_count, record_size, records, source_string_table):
    """Write selected records into a fresh DBC file, rebuilding the string table
    to only contain strings referenced by the copied spells."""

    # Collect all referenced string offsets from selected records
    all_offsets = set()
    for rec in records:
        all_offsets |= collect_string_offsets(rec, SPELL_FMT)
    all_offsets.discard(0)  # offset 0 is always the empty string

    # Build a new compact string table and offset mapping
    new_string_table = bytearray(b"\x00")  # offset 0 = empty string
    offset_map = {0: 0}
    for old_off in sorted(all_offsets):
        s = extract_string(source_string_table, old_off)
        new_off = len(new_string_table)
        offset_map[old_off] = new_off
        new_string_table.extend(s)
        new_string_table.append(0)

    # Rewrite records with remapped string offsets
    new_records = []
    for rec in records:
        rec = bytearray(rec)
        pos = 0
        for ch in SPELL_FMT:
            if ch == "s":
                old_off = struct.unpack_from("<I", rec, pos)[0]
                struct.pack_into("<I", rec, pos, offset_map.get(old_off, 0))
                pos += 4
            elif ch in ("n", "i", "f", "x"):
                pos += 4
            elif ch in ("X", "b"):
                pos += 1
        new_records.append(bytes(rec))

    # Write the file
    header = struct.pack(
        DBC_HEADER_FORMAT,
        DBC_MAGIC,
        len(new_records),
        field_count,
        record_size,
        len(new_string_table),
    )

    with open(path, "wb") as f:
        f.write(header)
        for rec in new_records:
            f.write(rec)
        f.write(new_string_table)

    print(f"Wrote {len(new_records)} spell(s) to {path} "
          f"({DBC_HEADER_SIZE + len(new_records) * record_size + len(new_string_table)} bytes)")


def main():
    parser = argparse.ArgumentParser(
        description="Copy specific spells from a source Spell.dbc into a new Spell.dbc."
    )
    parser.add_argument("source", help="Path to source Spell.dbc")
    parser.add_argument("target", help="Path to output Spell.dbc")
    parser.add_argument("spell_ids", nargs="*", type=int, help="Spell IDs to copy")
    parser.add_argument(
        "--file", "-f", dest="id_file",
        help="Text file with spell IDs (one per line, # comments allowed)",
    )
    args = parser.parse_args()

    # Collect spell IDs
    wanted = set(args.spell_ids or [])
    if args.id_file:
        for line in Path(args.id_file).read_text().splitlines():
            line = line.strip()
            if line and not line.startswith("#"):
                try:
                    wanted.add(int(line))
                except ValueError:
                    print(f"Warning: skipping invalid ID '{line}'", file=sys.stderr)

    if not wanted:
        parser.error("No spell IDs specified. Provide IDs as arguments or via --file.")

    print(f"Reading {args.source} ...")
    dbc = read_dbc(args.source)
    print(f"  {len(dbc['records'])} spells in source, record size {dbc['record_size']} bytes")

    # Find requested spells
    found = []
    missing = []
    for sid in sorted(wanted):
        if sid in dbc["records"]:
            found.append(dbc["records"][sid])
        else:
            missing.append(sid)

    if missing:
        print(f"Warning: {len(missing)} spell(s) not found: {missing}", file=sys.stderr)

    if not found:
        sys.exit("Error: none of the requested spells were found in the source DBC.")

    write_dbc(args.target, dbc["field_count"], dbc["record_size"], found, dbc["string_table"])
    print("Done.")


if __name__ == "__main__":
    main()
