# INDEX — azerothcore-wotlk (Fork)

Entry point for AI tools.

## Files in this repo

| File | Size | Purpose |
|-------|------:|-------|
| `INDEX.md` | <1 KB | this file — navigation |
| `CLAUDE.md` | ~6 KB | **What** this fork is, which custom extensions, which IDs/DBC patches |
| `data_structure.md` | ~7 KB | Folder/file listing of the core |
| `functions.md` | ~7 KB | **How**: hooks list, build, DB setup, custom hook implementations |
| `log.md` | ~2 KB | Commit log |
| `todo.md` | ~1 KB | open tasks (fork-specific) |

## Cross-Repo

- Project overview: [`share-public/AI_GUIDE.md`](https://github.com/Shoro2/share-public/blob/main/AI_GUIDE.md)
- Cross-repo history: [`share-public/claude_log.md`](https://github.com/Shoro2/share-public/blob/main/claude_log.md)
- Architecture (deep dive): [`share-public/docs/02-architecture.md`](https://github.com/Shoro2/share-public/blob/main/docs/02-architecture.md)
- Spell system: [`share-public/docs/03-spell-system.md`](https://github.com/Shoro2/share-public/blob/main/docs/03-spell-system.md)
- DB tables reference: [`share-public/mysqldbextracts/mysql_column_list_all.txt`](https://github.com/Shoro2/share-public/blob/main/mysqldbextracts/mysql_column_list_all.txt)

## Quick Facts

- AzerothCore fork for **WoW 3.3.5a (WotLK)** — C++17, CMake, MySQL/MariaDB
- **Custom extensions beyond upstream**:
  - `OnPlayerCheckReagent` / `OnPlayerConsumeReagent` PlayerScript hooks (for crafting reagent integration)
  - Custom Spell.dbc with custom spells (100xxx, 900xxx)
- Docs about docs: this repo is the **deep reference**; daily-use guide lives in `share-public`.
- **Important for AI**: do not build unprompted, do not edit in `data/sql/base/`, no `--no-verify`.
