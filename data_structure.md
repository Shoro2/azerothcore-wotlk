# File and directory structure — azerothcore-wotlk (Fork)

> This repo is a fork of `azerothcore/azerothcore-wotlk`. The paths documented here are the **most important** for custom work — a complete listing of the huge tree would not be helpful. For detailed navigation: use GitHub search or run `Bash`/`Read` against specific paths.

## Top level (excerpt)

```
azerothcore-wotlk/
├── .github/                                # PR templates, workflows, CODEOWNERS
│   ├── workflows/                          # CI: codestyle, build (clang/gcc, macOS, Windows), modules
│   ├── actions/                            # Composite actions
│   └── README.md, SECURITY.md, ...         # GitHub standard files
├── apps/                                   # Helper scripts (codestyle-cpp.py, codestyle-sql.py, db_assembler/, ci/)
├── conf/                                   # Config templates (worldserver.conf.dist, authserver.conf.dist)
├── data/
│   ├── sql/
│   │   ├── base/db_world/                  # Initial world schema (DO NOT change in PRs!)
│   │   ├── base/db_characters/             # Initial characters schema
│   │   ├── base/db_auth/                   # Initial auth schema
│   │   ├── updates/db_world/               # Merged SQL updates
│   │   ├── updates/pending_db_world/       # **Pending updates** — new SQL files go here
│   │   ├── updates/pending_db_characters/  # ...
│   │   ├── updates/pending_db_auth/        # ...
│   │   └── archive/                        # Historical archived updates
├── deps/                                   # Bundled dependencies: boost, MySQL client, OpenSSL, zlib, recastnav, fmt, argon2, jemalloc, ...
├── doc/
│   ├── ConfigPolicy.md                     # Config policy doc
│   ├── Logging.md                          # Logging framework doc
│   └── changelog/                          # Upstream changelog
├── env/                                    # Environment-specific files (Docker, etc.)
├── modules/                                # **Custom module slot** — mod-paragon, mod-paragon-itemgen, etc. live here (symlinks or clones)
├── src/
│   ├── common/                             # Shared libs (networking, crypto, logging, threading, collision)
│   ├── server/
│   │   ├── apps/
│   │   │   ├── authserver/                 # Auth executable
│   │   │   └── worldserver/                # World executable
│   │   ├── database/                       # DB abstraction + schema updater
│   │   ├── game/                           # Core game logic (~52 subsystems — see below)
│   │   ├── scripts/                        # Content (bosses, spells, commands, instances)
│   │   └── shared/                         # Auth↔World shared code
│   └── test/                               # GTest unit tests (linked against `game` lib)
├── var/                                    # Runtime data (created at runtime)
├── bin/                                    # Optional helper binaries
├── tools/                                  # Optional map extractor or similar
├── CMakeLists.txt                          # Top-level build
├── PreLoad.cmake                           # CMake preload
├── docker-compose.yml                      # Docker setup
├── flake.nix / flake.lock                  # Nix setup (optional)
├── install.sh, acore.sh, acore.json        # Helper scripts
├── pull_request_template.md                # GitHub PR template
├── CLAUDE.md                               # Project-specific AI doc
├── log.md                                  # Custom commit log (modular)
├── data_structure.md                       # This file
└── functions.md                            # Mechanics reference (custom hooks etc.)
```

## `src/server/game/` — Core subsystems

Most important directories for custom module work:

| Path | Contents |
|------|--------|
| `Entities/` | `Player`, `Creature`, `Unit`, `Item`, `GameObject` (core classes) |
| `Spells/` | Spell mechanics, `SpellMgr.h` (ProcFlags defines!), aura system |
| `Scripting/ScriptDefines/` | Hook headers: `PlayerScript.h`, `WorldScript.h`, `UnitScript.h`, ... |
| `Scripting/ScriptMgr.{h,cpp}` | Global `ScriptMgr` — lists all hook types |
| `Handlers/` | Client packet handlers (methods on `WorldSession`) |
| `Maps/` | Map management, grid, instancing |
| `AI/` | Creature AI framework |
| `DataStores/` | DBC loading + storage templates |
| `Conditions/` | Condition system for DB logic |
| `Loot/` | Loot generation |
| `Globals/` | `ObjectMgr` and global caches |
| `Server/` | `WorldSession`, `World`, opcodes |

## `src/server/scripts/` — Content

| Path | Contents |
|------|--------|
| `Commands/cs_*.cpp` | GM commands |
| `Spells/spell_*.cpp` | Class-specific SpellScripts (`spell_dk.cpp`, `spell_mage.cpp`, ...) |
| `EasternKingdoms/`, `Kalimdor/`, `Northrend/`, `Outland/` | Zone/dungeon/raid scripts |
| `Custom/` | Custom user scripts (gitignored) |

## `data/sql/` — SQL file layout

| Path | Use |
|------|-----------|
| `data/sql/base/<db>/` | Initial schema — **never edit in PRs** (CI warns + maintainer approval required) |
| `data/sql/updates/<db>/` | Merged updates |
| `data/sql/updates/pending_<db>/` | **New SQL files go here** with a random filename |
| `data/sql/archive/` | Archived old updates |

## Custom code in this fork

Beyond the regular upstream codebase, this fork has:

- **2 additional PlayerScript hooks** — `OnPlayerCheckReagent` / `OnPlayerConsumeReagent` (for crafting reagent routing). See `functions.md`.
- **Custom Spell.dbc** at `share/dbc/Spell.dbc` with custom spell IDs (100xxx auras, 900xxx custom effects, 950xxx passives).
- **Custom modules** in `modules/` (mod-paragon, mod-paragon-itemgen, mod-loot-filter, mod-auto-loot, mod-endless-storage; possibly more like mod-custom-spells, mod-dungeon-challenge).

## Size notes

- **Huge tree** — never list the full `src/` tree or read it as a whole. Grep specifically or read by path.
- Individual source files may be >50 KB (`Player.cpp`, `Spell.cpp`, `Unit.cpp`). Chunk with `Read offset/limit`.
- DBC files in `share/dbc/` are **binary** — never read directly.

## CI workflows (in `.github/workflows/`)

- `codestyle.yml` — C++ codestyle + cppcheck (triggered on `src/`)
- `sql-codestyle.yml` — SQL codestyle (triggered on `data/`)
- `core-build-pch.yml` — Linux build with PCH (clang-15 Ubuntu 22.04, clang-18 Ubuntu 24.04)
- `core-build-nopch.yml` — without PCH (clang-15, clang-18, gcc-14)
- `macos_build.yml`, `windows_build.yml` — OS compatibility
- `core_modules_build.yml` — module compilation

All builds with `-Werror`. Warnings are errors.

## What is not where?

- **Eluna is not part of this core** — included as a separate module (`mod-eluna`), not contained here.
- **AIO Framework is not server code** — lives in `share-public/AIO_Server/` and is copied into the Eluna `lua_scripts/` directory.
