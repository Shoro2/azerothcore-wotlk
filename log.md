# Change Log — azerothcore-wotlk (Fork)

> This repo is a **fork** of `azerothcore/azerothcore-wotlk`. The majority of commits are upstream sync merges.
> Only **project-specific / custom changes** are noted here (hooks, DBC patches). Standard upstream fixes can be extracted from the GitHub UI or via `git log master..HEAD`.

## Custom changes (project-specific)

> As of: 2026-09-17. Add a line here for every new custom commit.

### Core hardening

- 2026-09-17 — fix(Core/DataStores): size overlay by index field (`0d7371ffd`, branch `claude/core-upstream-sync-753fde25`; documented in `share-public/docs/World of Warcraft/forgotten-land/18-core-upstream-sync-084c9e2.md`) — since upstream `5af99dc39` the DB overlay of `currencytypes_dbc`, `scalingstatvalues_dbc` and `worldmaparea_dbc` is indexed by the format's index field, but `DBCDatabaseLoader` sized the index table from the first row of `ORDER BY ID DESC`; a later row with a higher index value wrote out of bounds at boot. Now the loader tracks the maximum over all rows, sizes once after the loop, and logs and skips an out-of-range row. Next sync: upstream `90bccf4fa` edits the same loop — read its old entry only when `indexValue < records`.
  - affected files: `src/server/shared/DataStores/DBCDatabaseLoader.cpp`
- 2026-07-11 — fix(Core/Globals): no crash on missing display id (documented in `share-public/claude_log.md`) — `ObjectMgr::LoadCreatureModelInfo` dereferenced a null `CreatureDisplayInfoEntry` when `creature_model_info` references a display id absent from `CreatureDisplayInfo.dbc`, killing the worldserver at startup; now guarded warn-and-continue.
  - affected files: `src/server/game/Globals/ObjectMgr.cpp`

### Hooks for external modules

- 2026-03-22 — revert: reagent hooks removed again (`0cb0773a7`) — `OnPlayerCheckReagent` / `OnPlayerConsumeReagent` and both call sites are gone; only the `Spell::TakeReagents()` `itemcount > 0` guard remains. The entry below is history. (Logged 2026-09-17; the fork docs described the hooks until then.)
- 2026-03-22 — feat: reagent hooks for External Storage (documented in `share-public/claude_log.md`) — added `OnPlayerCheckReagent` and `OnPlayerConsumeReagent` PlayerScript hooks. Call sites: `Spell::CheckItems()` and `Spell::TakeReagents()`. Used by **mod-endless-storage** (now replaced via the Lua path — but hooks remain in the core in case they are needed again in the future).
  - affected files: `src/server/game/Scripting/ScriptDefines/PlayerScript.{h,cpp}`, `src/server/game/Scripting/ScriptMgr.h`, `src/server/game/Spells/Spell.cpp`
  - ~45 lines of custom code

### DBC

- 2026-03-18 — fix: Spell.dbc corruption resolved + validation built in (in `share-public/claude_log.md`) — `share/dbc/Spell.dbc` and `share/copy_spells_dbc.py` with 6 safeguards (size check, string table, duplicate detection, format consistency, source≠target, post-write verify).

## Latest upstream syncs (for orientation)

- 2026-09-14 — Merge upstream `084c9e2` (CoA base, 2026-08-24; merge `acc41b16`, branch `claude/core-upstream-sync-753fde25`, not yet on `master`; rehearsal, T2 runbook and deploy plan in `share-public/docs/World of Warcraft/forgotten-land/18-core-upstream-sync-084c9e2.md`)
- Conflicts: `CharacterDatabase.h/.cpp` (kept both: upstream `CHAR_NO_OP_PROVIDE_REALM_CONTEXT`, then our 39 statements), `AGENTS.md` / `CLAUDE.md` (kept ours). All fork customs intact.
- Upstream highlights from this merge:
  - mmap format v20 (`MMAP_VERSION`, `src/common/Collision/Maps/MapDefines.h:29`): every map's mmaps must be regenerated
  - 208 SQL updates (auth 6, characters 4, world 198); 2024/2025 update files moved to `data/sql/archive`
  - DBC DB overlays indexed by the format's index field (`5af99dc39`)
  - clustering groundwork (`deps/libsidecar` stub, `Cluster.*` config keys, off by default)
  - creature text options (`creature_text_option_sets`, `creature_text_options`)
- 2026-04-06 — Merge upstream/master ([67375a7](https://github.com/Shoro2/azerothcore-wotlk/commit/67375a7ca6c583f339d7f628c4f1f2ee91b76fbf))
- Upstream highlights from this merge:
  - fix(Core/Movement): prevent PvP flag and backwards movement on taxi login (#25153)
  - fix(Scripts/Magtheridon): scheduler update before UpdateVictim (#25379)
  - fix(Core/Unit): prevent creature evade when on threat list (#25328)
  - fix(Core/OutdoorPvP): use-after-free in DelCapturePoint (#25229)
  - fix(Core/Groups): pass actual loot count to OnPlayerGroupRollRewardItem (#25312)
  - fix(Core/Loot): restore hide quest starter item conditions (#25355)

## Convention

Append new custom entries at the top under "Custom changes". Bundle upstream sync merges into a single entry per sync date, without listing every individual upstream PR.

Detailed descriptions of custom changes belong alongside in `share-public/claude_log.md`.
