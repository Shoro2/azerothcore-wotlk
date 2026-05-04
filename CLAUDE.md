# azerothcore-wotlk (Fork)

> Read [`INDEX.md`](./INDEX.md) first. Build, hooks, custom implementations: [`functions.md`](./functions.md). Folder layout: [`data_structure.md`](./data_structure.md). Open items: [`todo.md`](./todo.md). Commit trail: [`log.md`](./log.md).
>
> The **deep reference** for AzerothCore lives in the official wiki: [azerothcore.org/wiki](https://www.azerothcore.org/wiki). This documentation is limited to what is specific to **this fork**.

## What is this fork?

This fork is based on the upstream repo [`azerothcore/azerothcore-wotlk`](https://github.com/azerothcore/azerothcore-wotlk) — an open-source server emulator for World of Warcraft 3.3.5a (Wrath of the Lich King). C++17, CMake, MySQL/MariaDB, GPL v2. Current upstream version line: 15.0.0-dev.

The fork exists because the custom module set (`mod-paragon`, `mod-paragon-itemgen`, `mod-loot-filter`, `mod-auto-loot`, `mod-endless-storage`, …) requires a small number of project-specific extensions to the core that are not (or not yet) available upstream.

## Role in the overall project

```
azerothcore-wotlk (this fork)
   ├── contains custom hooks (see below)
   ├── contains patched Spell.dbc with custom spell IDs (100xxx, 900xxx)
   ├── modules/ ← all custom modules are symlinked or cloned here
   └── delivers worldserver + authserver binaries
```

All custom modules attach to hooks provided by `ScriptMgr`. The actual module loading runs via auto-detection through CMake.

## Custom extensions beyond upstream

| Area | Extension | Consumer |
|---------|-------------|-----------|
| **Hook** | `OnPlayerCheckReagent(Player*, uint32 itemId, uint32 count, bool& consumed)` | `mod-endless-storage` (historical; currently unused) |
| **Hook** | `OnPlayerConsumeReagent(Player*, uint32 itemId, uint32 count)` | `mod-endless-storage` (historical; currently unused) |
| **DBC** | `share/dbc/Spell.dbc` with custom spell entries 100000-100027, 100201-100227, 900100-900116, 920920, … | `mod-paragon`, `mod-paragon-itemgen`, `mod-custom-spells` |
| **Tools** | `share/copy_spells_dbc.py` (with 6 safeguards against DBC corruption) | DBC maintenance |

Implementation details of the hooks: [`functions.md`](./functions.md#custom-hooks).

## Custom data

This fork brings **no** own DB schema (beyond upstream). All custom tables live in their respective modules.

| Area | What lives here |
|---------|----------------|
| `data/sql/base/db_world/` | Upstream schema — **do not edit** (CI warning, maintainer approval) |
| `data/sql/updates/pending_*` | New SQL files for custom hook migrations land here if needed |
| `share/dbc/` | DBC files including the project-specifically patched `Spell.dbc` |
| `share/copy_spells_dbc.py`, `add_paragon_spell.py` | Maintenance tools |

## Configuration

`conf/dist/` contains the templates (`worldserver.conf.dist`, `authserver.conf.dist`). Runtime configs go to `conf/` (gitignored). The custom modules bring their own `.conf.dist` files — see the respective module repo.

## What this fork does **not** change (as of 2026-05)

- **no** custom class/race system
- **no** changes to combat formulas (crit tables, resilience, etc.)
- **no** merge conflict override at the database layer
- **no** patch to the AIO framework itself (that lives in `share-public/AIO_Server/` as an external Lua library and is loaded via Eluna)
- **no** intervention in the module loader system — that is upstream vanilla.

## Architecture notes

- **Server apps**: `authserver` (port 3724) and `worldserver` (port 8085).
- **Three databases**: `acore_auth`, `acore_characters`, `acore_world`.
- **Build default**: `cmake .. -DSCRIPTS=static -DMODULES=static && make -j`. Because of `-Werror`, warnings become errors — custom patches must be warning-clean.
- **Code style**: AzerothCore standard (`auto const&`, `Type const*`, 4 spaces, max 80 chars). Pre-commit checks via `apps/codestyle/codestyle-cpp.py` + `codestyle-sql.py`.
- **CI**: macOS, Ubuntu (clang-15, clang-18, gcc-14), Windows. `-Werror` active. Module build separate.
- **Spell.dbc corruption**: occurred in the past via `copy_spells_dbc.py` — the tool has had 6 safeguards since then. If corruption is suspected: restore the backup from `ac-share/data/dbc/Spell.dbc`, then merge custom spells back in via the tool.

## License

GPL v2 (upstream). All custom patches in this fork are released under the same license.
