# TODOs — azerothcore-wotlk (Fork)

> Open tasks for this fork (custom extensions beyond upstream). Record completed TODOs in `log.md` and remove them here.
> **Important**: only **project-specific** TODOs here — upstream AzerothCore issues do not belong here.

## Custom hooks

None open. The `OnPlayerCheckReagent` / `OnPlayerConsumeReagent` hooks were reverted in `0cb0773a7` (2026-03-22) and do not exist in `src/`; only the `Spell::TakeReagents()` `itemcount > 0` guard remains (see `functions.md`).

## Upstream sync hygiene

- [ ] **(low)** Periodic upstream merges (every 4-6 weeks) — only document, no task for AI without user request. Last sync on `master`: upstream `0f107f2` (2026-07-08). Pending: upstream `084c9e2` (2026-08-24) on `claude/core-upstream-sync-753fde25`, rehearsed T1 on isolated copies; operator T2, merge and deploy owed (`share-public`: `docs/World of Warcraft/forgotten-land/18-core-upstream-sync-084c9e2.md`).

## Convention

Do NOT cross out completed items — remove them and document in `log.md`. Upstream issues do NOT belong in this file.
