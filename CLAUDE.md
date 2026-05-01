# azerothcore-wotlk (Fork)

> Lies zuerst [`INDEX.md`](./INDEX.md). Build, Hooks, Custom-Implementations: [`functions.md`](./functions.md). Folder-Layout: [`data_structure.md`](./data_structure.md). Offenes: [`todo.md`](./todo.md). Commit-Spur: [`log.md`](./log.md).
>
> **Tiefenreferenz** für AzerothCore liegt im offiziellen Wiki: [azerothcore.org/wiki](https://www.azerothcore.org/wiki). Diese Doku beschränkt sich auf das, was in **diesem Fork** speziell ist.

## Was ist dieser Fork?

Dieser Fork basiert auf dem Upstream-Repo [`azerothcore/azerothcore-wotlk`](https://github.com/azerothcore/azerothcore-wotlk) — einem Open-Source-Server-Emulator für World of Warcraft 3.3.5a (Wrath of the Lich King). C++17, CMake, MySQL/MariaDB, GPL v2. Aktuelle Upstream-Version-Linie: 15.0.0-dev.

Der Fork existiert, weil das Custom-Module-Set (`mod-paragon`, `mod-paragon-itemgen`, `mod-loot-filter`, `mod-auto-loot`, `mod-endless-storage`, …) eine kleine Anzahl projekt-spezifischer Erweiterungen am Core benötigt, die im Upstream nicht (oder noch nicht) verfügbar sind.

## Rolle im Gesamtprojekt

```
azerothcore-wotlk (dieser Fork)
   ├── enthält Custom-Hooks (siehe unten)
   ├── enthält gepatchte Spell.dbc mit Custom-Spell-IDs (100xxx, 900xxx)
   ├── modules/ ← hier werden alle Custom-Module symlinked oder geclont
   └── liefert worldserver + authserver Binaries
```

Alle Custom-Module hängen sich an Hooks an, die von `ScriptMgr` bereitgestellt werden. Das eigentliche Modul-Loading läuft via Auto-Detection durch CMake.

## Custom-Erweiterungen über Upstream hinaus

| Bereich | Erweiterung | Konsument |
|---------|-------------|-----------|
| **Hook** | `OnPlayerCheckReagent(Player*, uint32 itemId, uint32 count, bool& consumed)` | `mod-endless-storage` (historisch; aktuell ungenutzt) |
| **Hook** | `OnPlayerConsumeReagent(Player*, uint32 itemId, uint32 count)` | `mod-endless-storage` (historisch; aktuell ungenutzt) |
| **DBC** | `share/dbc/Spell.dbc` mit Custom-Spell-Einträgen 100000-100027, 100201-100227, 900100-900116, 920920, … | `mod-paragon`, `mod-paragon-itemgen`, `mod-custom-spells` |
| **Tools** | `share/copy_spells_dbc.py` (mit 6 Schutzmaßnahmen gegen DBC-Korruption) | DBC-Wartung |

Implementierungs-Details der Hooks: [`functions.md`](./functions.md#custom-hooks).

## Custom-Daten

Dieser Fork bringt **kein** eigenes DB-Schema mit (über Upstream hinaus). Alle Custom-Tabellen leben in den jeweiligen Modulen.

| Bereich | Was hier liegt |
|---------|----------------|
| `data/sql/base/db_world/` | Upstream-Schema — **nicht editieren** (CI-Warnung, Maintainer-Approval) |
| `data/sql/updates/pending_*` | Hier landen neue SQL-Files für Custom-Hook-Migrations falls nötig |
| `share/dbc/` | DBC-Files inkl. der projektspezifisch gepatchten `Spell.dbc` |
| `share/copy_spells_dbc.py`, `add_paragon_spell.py` | Wartungs-Tools |

## Konfiguration

`conf/dist/` enthält die Templates (`worldserver.conf.dist`, `authserver.conf.dist`). Runtime-Configs kommen nach `conf/` (gitignored). Die Custom-Module bringen ihre eigenen `.conf.dist`-Files mit — siehe das jeweilige Modul-Repo.

## Was dieser Fork **nicht** ändert (Stand 2026-05)

- **kein** Custom-Class/Race-System
- **keine** Veränderungen an Combat-Formulas (Crit-Tables, Resilience etc.)
- **kein** Merge-Konflikt-Override am Datenbank-Layer
- **kein** Patch am AIO-Framework selbst (das liegt in `share-public/AIO_Server/` als externe Lua-Library und wird per Eluna geladen)
- **kein** Eingriff in das Modul-Loader-System — das ist Upstream-vanilla.

## Hinweise zur Architektur

- **Server-Apps**: `authserver` (Port 3724) und `worldserver` (Port 8085).
- **Drei Datenbanken**: `acore_auth`, `acore_characters`, `acore_world`.
- **Build-Default**: `cmake .. -DSCRIPTS=static -DMODULES=static && make -j`. Wegen `-Werror` werden Warnungen zu Errors — Custom-Patches müssen warning-clean sein.
- **Codestyle**: AzerothCore-Standard (`auto const&`, `Type const*`, 4 Spaces, max 80 chars). Pre-Commit-Checks via `apps/codestyle/codestyle-cpp.py` + `codestyle-sql.py`.
- **CI**: macOS, Ubuntu (clang-15, clang-18, gcc-14), Windows. `-Werror` aktiv. Module-Build separat.
- **Spell.dbc-Korruption**: in der Vergangenheit durch `copy_spells_dbc.py` aufgetreten — Tool hat seitdem 6 Schutzmaßnahmen. Bei Verdacht auf Korruption: Backup aus `ac-share/data/dbc/Spell.dbc` zurückspielen, Custom-Spells via Tool wieder dazu mergen.

## Lizenz

GPL v2 (Upstream). Alle Custom-Patches in diesem Fork werden unter der gleichen Lizenz veröffentlicht.
