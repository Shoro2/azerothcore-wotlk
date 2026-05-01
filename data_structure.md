# Datei- und Verzeichnisstruktur — azerothcore-wotlk (Fork)

> Dieser Repo ist ein Fork von `azerothcore/azerothcore-wotlk`. Die hier dokumentierten Pfade sind die **wichtigsten** für Custom-Arbeit — vollständige Auflistung des riesigen Trees wäre nicht hilfreich. Für Detail-Navigation: GitHub-Suche nutzen oder gezielt `Bash`/`Read` auf konkrete Pfade.

## Top-Level (Auszug)

```
azerothcore-wotlk/
├── .github/                                # PR-Templates, Workflows, CODEOWNERS
│   ├── workflows/                          # CI: codestyle, build (clang/gcc, macOS, Windows), modules
│   ├── actions/                            # Composite-Actions
│   └── README.md, SECURITY.md, ...         # GitHub-Standardfiles
├── apps/                                   # Helper-Skripte (codestyle-cpp.py, codestyle-sql.py, db_assembler/, ci/)
├── conf/                                   # Konfig-Templates (worldserver.conf.dist, authserver.conf.dist)
├── data/
│   ├── sql/
│   │   ├── base/db_world/                  # Initiales World-Schema (NICHT in PRs ändern!)
│   │   ├── base/db_characters/             # Initiales Characters-Schema
│   │   ├── base/db_auth/                   # Initiales Auth-Schema
│   │   ├── updates/db_world/               # Gemergte SQL-Updates
│   │   ├── updates/pending_db_world/       # **Pending Updates** — neue SQL-Files hier
│   │   ├── updates/pending_db_characters/  # ...
│   │   ├── updates/pending_db_auth/        # ...
│   │   └── archive/                        # Historische archivierte Updates
├── deps/                                   # Bundled Dependencies: boost, MySQL-client, OpenSSL, zlib, recastnav, fmt, argon2, jemalloc, ...
├── doc/
│   ├── ConfigPolicy.md                     # Konfig-Policy-Doku
│   ├── Logging.md                          # Logging-Framework-Doku
│   └── changelog/                          # Upstream-Changelog
├── env/                                    # Environment-spezifische Files (Docker, etc.)
├── modules/                                # **Custom-Module-Slot** — hier liegen mod-paragon, mod-paragon-itemgen, etc. (Symlinks oder Clones)
├── src/
│   ├── common/                             # Shared Libs (Networking, Crypto, Logging, Threading, Collision)
│   ├── server/
│   │   ├── apps/
│   │   │   ├── authserver/                 # Auth-Executable
│   │   │   └── worldserver/                # World-Executable
│   │   ├── database/                       # DB-Abstraktion + Schema-Updater
│   │   ├── game/                           # Kern-Spiellogik (~52 Subsysteme — siehe unten)
│   │   ├── scripts/                        # Content (Bosse, Spells, Commands, Instanzen)
│   │   └── shared/                         # Auth↔World-gemeinsamer Code
│   └── test/                               # GTest-Unit-Tests (links gegen `game`-Lib)
├── var/                                    # Runtime-Daten (zur Laufzeit erzeugt)
├── bin/                                    # ggf. Helper-Binaries
├── tools/                                  # ggf. Map-Extractor o.ä.
├── CMakeLists.txt                          # Top-Level-Build
├── PreLoad.cmake                           # CMake-Vorladung
├── docker-compose.yml                      # Docker-Setup
├── flake.nix / flake.lock                  # Nix-Setup (optional)
├── install.sh, acore.sh, acore.json        # Helper-Scripts
├── pull_request_template.md                # GitHub PR-Template
├── CLAUDE.md                               # Projekt-spezifische KI-Doku
├── log.md                                  # Custom-Commit-Log (modular)
├── data_structure.md                       # Diese Datei
└── functions.md                            # Mechanik-Referenz (Custom-Hooks etc.)
```

## `src/server/game/` — Kern-Subsysteme

Wichtigste Verzeichnisse für Custom-Module-Arbeit:

| Pfad | Inhalt |
|------|--------|
| `Entities/` | `Player`, `Creature`, `Unit`, `Item`, `GameObject` (Kern-Klassen) |
| `Spells/` | Spell-Mechaniken, `SpellMgr.h` (ProcFlags-Defines!), Aura-System |
| `Scripting/ScriptDefines/` | Hook-Header: `PlayerScript.h`, `WorldScript.h`, `UnitScript.h`, ... |
| `Scripting/ScriptMgr.{h,cpp}` | Globaler `ScriptMgr` — listet alle Hook-Typen |
| `Handlers/` | Client-Packet-Handler (Methoden auf `WorldSession`) |
| `Maps/` | Map-Management, Grid, Instancing |
| `AI/` | Creature-AI-Framework |
| `DataStores/` | DBC-Loading + Storage-Templates |
| `Conditions/` | Condition-System für DB-Logic |
| `Loot/` | Loot-Generierung |
| `Globals/` | `ObjectMgr` und globale Caches |
| `Server/` | `WorldSession`, `World`, Opcodes |

## `src/server/scripts/` — Content

| Pfad | Inhalt |
|------|--------|
| `Commands/cs_*.cpp` | GM-Commands |
| `Spells/spell_*.cpp` | Klassen-spezifische SpellScripts (`spell_dk.cpp`, `spell_mage.cpp`, ...) |
| `EasternKingdoms/`, `Kalimdor/`, `Northrend/`, `Outland/` | Zone/Dungeon/Raid-Scripts |
| `Custom/` | Custom-User-Scripts (gitignored) |

## `data/sql/` — SQL-Datei-Ablage

| Pfad | Verwendung |
|------|-----------|
| `data/sql/base/<db>/` | Initiales Schema — **niemals in PRs editieren** (CI warnt + Maintainer-Approval nötig) |
| `data/sql/updates/<db>/` | Gemergte Updates |
| `data/sql/updates/pending_<db>/` | **Hier kommen neue SQL-Files rein** mit Random-Filename |
| `data/sql/archive/` | Archivierte alte Updates |

## Custom-Code in diesem Fork

Über die normale Upstream-Codebase hinaus hat dieser Fork:

- **2 zusätzliche PlayerScript-Hooks** — `OnPlayerCheckReagent` / `OnPlayerConsumeReagent` (für Crafting-Reagenz-Routing). Siehe `functions.md`.
- **Custom Spell.dbc** unter `share/dbc/Spell.dbc` mit Custom-Spell-IDs (100xxx Auras, 900xxx Custom-Effekte, 950xxx Passives).
- **Custom-Module** in `modules/` (mod-paragon, mod-paragon-itemgen, mod-loot-filter, mod-auto-loot, mod-endless-storage; ggf. weitere wie mod-custom-spells, mod-dungeon-challenge).

## Größenhinweise

- **Riesiger Tree** — niemals den ganzen `src/`-Baum auflisten oder am Stück lesen. Gezielt grepen oder per Pfad lesen.
- Einzelne Source-Files können >50 KB sein (`Player.cpp`, `Spell.cpp`, `Unit.cpp`). Mit `Read offset/limit` chunken.
- DBC-Files in `share/dbc/` sind **binär** — niemals direkt lesen.

## CI-Workflows (im `.github/workflows/`)

- `codestyle.yml` — C++-Codestyle + cppcheck (Trigger bei `src/`)
- `sql-codestyle.yml` — SQL-Codestyle (Trigger bei `data/`)
- `core-build-pch.yml` — Linux-Build mit PCH (clang-15 Ubuntu 22.04, clang-18 Ubuntu 24.04)
- `core-build-nopch.yml` — ohne PCH (clang-15, clang-18, gcc-14)
- `macos_build.yml`, `windows_build.yml` — OS-Kompatibilität
- `core_modules_build.yml` — Modul-Compilation

Alle Builds mit `-Werror`. Warnings sind Errors.

## Wo ist was nicht?

- **Eluna ist kein Bestandteil dieses Cores** — wird als separates Modul (`mod-eluna`) eingebunden, hier nicht enthalten.
- **AIO Framework ist kein Server-Code** — liegt in `share-public/AIO_Server/` und wird in das Eluna `lua_scripts/`-Verzeichnis kopiert.
