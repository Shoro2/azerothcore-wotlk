#!/usr/bin/env python3
"""
Copy specific spells by ID from a source Spell.dbc and append them into an
existing target Spell.dbc. The target keeps all its existing spells and gets
the imported ones added. If a spell ID already exists in the target, it is
skipped (use --overwrite to replace it).

Usage:
    python3 copy_spells_dbc.py <source.dbc> <target.dbc> <spell_id> [spell_id ...]
    python3 copy_spells_dbc.py <source.dbc> <target.dbc> --file <id_list.txt>

Examples:
    python3 copy_spells_dbc.py Spell.dbc SpellOut.dbc 133 116 5143
    python3 copy_spells_dbc.py Spell.dbc SpellOut.dbc --file spell_ids.txt
    python3 copy_spells_dbc.py Spell.dbc SpellOut.dbc --overwrite --file spell_ids.txt

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
    and the full string table.  Validates file integrity before returning."""
    data = Path(path).read_bytes()

    if len(data) < DBC_HEADER_SIZE:
        sys.exit(f"Error: {path} is too small to be a valid DBC file ({len(data)} bytes)")

    magic, record_count, field_count, record_size, string_table_size = struct.unpack_from(
        DBC_HEADER_FORMAT, data, 0
    )
    if magic != DBC_MAGIC:
        sys.exit(f"Error: {path} is not a valid DBC file (magic: {magic!r})")

    expected_size = DBC_HEADER_SIZE + record_count * record_size + string_table_size
    if len(data) != expected_size:
        sys.exit(
            f"Error: {path} is corrupt — file size {len(data)} does not match "
            f"expected {expected_size} (header claims {record_count} records × "
            f"{record_size} bytes + {string_table_size} string table)"
        )

    records_start = DBC_HEADER_SIZE
    string_table_start = records_start + record_count * record_size
    string_table = data[string_table_start : string_table_start + string_table_size]

    if string_table_size > 0 and string_table[0:1] != b"\x00":
        sys.exit(f"Error: {path} is corrupt — string table does not start with a null byte")

    records = {}
    duplicates = 0
    for i in range(record_count):
        offset = records_start + i * record_size
        record_data = data[offset : offset + record_size]
        spell_id = struct.unpack_from("<I", record_data, 0)[0]
        if spell_id in records:
            duplicates += 1
        records[spell_id] = record_data

    if duplicates > 0:
        sys.exit(
            f"Error: {path} is corrupt — found {duplicates} duplicate spell ID(s) "
            f"across {record_count} records (only {len(records)} unique). "
            f"Restore from a clean backup before proceeding."
        )

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


# Spell.dbc format string — must exactly match SpellEntryfmt from DBCfmt.h
# 234 fields, 936 bytes per record (all fields are 4 bytes: n/i/f/s/x)
SPELL_FMT = (
    "niiiiiiiiiiiixixiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiifx"
    "iiiiiiiiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiiifff"
    "iiiiiiiiiiiiiissssssssssssssssxssssssssssssssss"
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxiiiiiiiiiiixfffxxxiiiiixxfffxx"
)


def remap_record_strings(record_data, old_string_table, new_string_table, offset_map):
    """Remap all string offsets in a record into a new string table.
    Strings not yet in the new table are appended automatically."""
    rec = bytearray(record_data)
    pos = 0
    for ch in SPELL_FMT:
        if ch == "s":
            old_off = struct.unpack_from("<I", rec, pos)[0]
            if old_off not in offset_map:
                s = extract_string(old_string_table, old_off)
                new_off = len(new_string_table)
                offset_map[old_off] = new_off
                new_string_table.extend(s)
                new_string_table.append(0)
            struct.pack_into("<I", rec, pos, offset_map[old_off])
            pos += 4
        elif ch in ("n", "i", "f", "x"):
            pos += 4
        elif ch in ("X", "b"):
            pos += 1
    return bytes(rec)


def write_dbc(path, field_count, record_size, all_records, string_table):
    """Write a complete DBC file from a list of (record_bytes) and a string table."""
    header = struct.pack(
        DBC_HEADER_FORMAT,
        DBC_MAGIC,
        len(all_records),
        field_count,
        record_size,
        len(string_table),
    )

    with open(path, "wb") as f:
        f.write(header)
        for rec in all_records:
            f.write(rec)
        f.write(string_table)

    total = DBC_HEADER_SIZE + len(all_records) * record_size + len(string_table)
    print(f"Wrote {len(all_records)} spell(s) to {path} ({total} bytes)")

    # Verify written file integrity
    actual_size = Path(path).stat().st_size
    if actual_size != total:
        sys.exit(f"CRITICAL: Written file size {actual_size} != expected {total} — write error!")


def main():
    parser = argparse.ArgumentParser(
        description="Copy specific spells from a source Spell.dbc into a target Spell.dbc "
        "(existing spells in the target are preserved)."
    )
    parser.add_argument("source", help="Path to source Spell.dbc")
    parser.add_argument("target", help="Path to target Spell.dbc (will be updated in-place)")
    parser.add_argument("spell_ids", nargs="*", type=int,
                        help="Spell IDs to import (can appear anywhere on the command line)")
    parser.add_argument(
        "--file", "-f", dest="id_file",
        help="Text file with spell IDs (one per line, # comments allowed)",
    )
    parser.add_argument(
        "--overwrite", action="store_true",
        help="Overwrite spells that already exist in the target",
    )
    args, extra = parser.parse_known_args()

    # Collect spell IDs (from positional args + any trailing numbers)
    wanted = set(args.spell_ids or [])
    for val in extra:
        try:
            wanted.add(int(val))
        except ValueError:
            parser.error(f"unrecognized argument: {val}")
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

    # Safety: prevent accidental self-overwrite
    if Path(args.source).resolve() == Path(args.target).resolve():
        sys.exit("Error: source and target resolve to the same file. Use different paths.")

    # Read source DBC
    print(f"Reading source: {args.source} ...")
    source = read_dbc(args.source)
    print(f"  {len(source['records'])} spells, record size {source['record_size']} bytes")

    # Read target DBC
    target_path = Path(args.target)
    if not target_path.exists():
        sys.exit(f"Error: target file {args.target} does not exist.")

    print(f"Reading target: {args.target} ...")
    target = read_dbc(args.target)
    print(f"  {len(target['records'])} spells, record size {target['record_size']} bytes")

    if source["record_size"] != target["record_size"]:
        sys.exit("Error: source and target have different record sizes — incompatible DBC versions.")

    # Verify SPELL_FMT matches the actual record size
    fmt_bytes = sum(4 for c in SPELL_FMT if c in "nifsx") + sum(1 for c in SPELL_FMT if c in "Xb")
    if fmt_bytes != source["record_size"]:
        sys.exit(
            f"Error: SPELL_FMT calculates to {fmt_bytes} bytes but DBC has "
            f"{source['record_size']}-byte records. Update SPELL_FMT to match DBCfmt.h."
        )

    # Find requested spells in source
    missing = [sid for sid in sorted(wanted) if sid not in source["records"]]
    if missing:
        print(f"Warning: {len(missing)} spell(s) not found in source: {missing}", file=sys.stderr)

    to_import = {sid: source["records"][sid] for sid in sorted(wanted) if sid in source["records"]}
    if not to_import:
        sys.exit("Error: none of the requested spells were found in the source DBC.")

    # Merge: start with the target's string table and remap imported records into it
    merged_string_table = bytearray(target["string_table"])
    # Build an initial offset_map with identity mapping for target strings (they stay as-is)
    source_offset_map = {}  # old source offset -> new offset in merged table

    skipped = []
    added = []
    overwritten = []

    # Start with all target records (keyed by spell ID for easy replacement)
    merged_records = dict(target["records"])

    for sid, rec in to_import.items():
        if sid in merged_records and not args.overwrite:
            skipped.append(sid)
            continue

        # Remap source record strings into the merged string table
        remapped = remap_record_strings(
            rec, source["string_table"], merged_string_table, source_offset_map
        )

        if sid in merged_records:
            overwritten.append(sid)
        else:
            added.append(sid)
        merged_records[sid] = remapped

    if skipped:
        print(f"Skipped {len(skipped)} spell(s) already in target (use --overwrite): {skipped}")
    if overwritten:
        print(f"Overwritten {len(overwritten)} spell(s): {overwritten}")
    if added:
        print(f"Added {len(added)} spell(s): {added}")

    if not added and not overwritten:
        print("Nothing to do — all requested spells already exist in target.")
        return

    # Write merged DBC sorted by spell ID
    final_records = [merged_records[sid] for sid in sorted(merged_records)]
    write_dbc(args.target, target["field_count"], target["record_size"],
              final_records, bytes(merged_string_table))
    print("Done.")


if __name__ == "__main__":
    main()
