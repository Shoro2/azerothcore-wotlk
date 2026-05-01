# INDEX — azerothcore-wotlk (Fork)

Einstiegspunkt für KI-Tools.

## Files in diesem Repo

| Datei | Größe | Zweck |
|-------|------:|-------|
| `INDEX.md` | <1 KB | diese Datei — Navigation |
| `CLAUDE.md` | ~6 KB | **Was** ist dieser Fork, welche Custom-Erweiterungen, welche IDs/DBC-Patches |
| `data_structure.md` | ~7 KB | Folder/File-Auflistung des Cores |
| `functions.md` | ~7 KB | **Wie**: Hooks-Liste, Build, DB-Setup, Custom-Hook-Implementations |
| `log.md` | ~2 KB | Commit-Log |
| `todo.md` | ~1 KB | offene Aufgaben (fork-spezifisch) |

## Cross-Repo

- Projekt-Übersicht: [`share-public/AI_GUIDE.md`](https://github.com/Shoro2/share-public/blob/main/AI_GUIDE.md)
- Cross-Repo-Historie: [`share-public/claude_log.md`](https://github.com/Shoro2/share-public/blob/main/claude_log.md)
- Architektur (deep dive): [`share-public/docs/02-architecture.md`](https://github.com/Shoro2/share-public/blob/main/docs/02-architecture.md)
- Spell-System: [`share-public/docs/03-spell-system.md`](https://github.com/Shoro2/share-public/blob/main/docs/03-spell-system.md)
- DB-Tabellen-Referenz: [`share-public/mysqldbextracts/mysql_column_list_all.txt`](https://github.com/Shoro2/share-public/blob/main/mysqldbextracts/mysql_column_list_all.txt)

## Quick Facts

- AzerothCore-Fork für **WoW 3.3.5a (WotLK)** — C++17, CMake, MySQL/MariaDB
- **Custom-Erweiterungen über Upstream hinaus**:
  - `OnPlayerCheckReagent` / `OnPlayerConsumeReagent` PlayerScript-Hooks (für Crafting-Reagenz-Integration)
  - Custom Spell.dbc mit Custom-Spells (100xxx, 900xxx)
- Doku zur Doku: dieses Repo ist die **Tiefenreferenz**; Daily-Use-Guide liegt in `share-public`.
- **Wichtig für KI**: nicht ungefragt builden, nicht in `data/sql/base/` editieren, kein `--no-verify`.
