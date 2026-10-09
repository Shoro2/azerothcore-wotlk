# Change Log — azerothcore-wotlk (Fork)

> This repo is a **fork** of `azerothcore/azerothcore-wotlk`. The majority of commits are upstream sync merges.
> Only **project-specific / custom changes** are noted here (hooks, DBC patches). Standard upstream fixes can be extracted from the GitHub UI or via `git log master..HEAD`.

## Custom changes (project-specific)

- 2026-10-09 — CoA catch-up round 4, the core picks (CU4-WP5, branch `claude/coa-round4-ad93e862`, 22 code commits on `f793ad439` up to `ee8b8a3ec`, 20 files +288/−47, then this entry; plan `ForgottenLand2.0/tools/coa-program/cu4-plan-report.md` §3) — the `UnitScript` hook `OnBeforeHealAbsorb` plus 21 CoA commits of `10fa1627f6..fc359be9` that the kept class code needs or that fix our classes, each with the CoA commit and PR in its message. Lines marked `// FL:` limit a CoA rule to the Ascension classes (or the units they own or charm), so the ten stock classes keep stock behaviour; CoA's `m_caster`-as-Unit reads `unitCaster` (AzerothCore #27627). Deliberate stock fixes: `d1b95282`, `f212136d`, `c34555c4` (Pet.cpp), `cbf82dc0` (summon decline). Not taken: the optional `b71e5b58`, every Hero/Wildcard/Paths part, CoA's silent-swap machinery. Host: vault MIG entry of round 4's core pull (WP-10).
  - `cbf82dc0` (#6949): Pulverize Reset 802895 also clears Ram's category; a declined summon always clears the pending request (stock fix)
  - `44409572` (#6884, hand port): a Sun Cleric's off-hand two-hander is unequipped when Valkyr's Grip 707072 goes; the Hero Titan's Grip parts cut
  - `feaef178` (#6919, SpellInfo.cpp only): the Artificer's Wand 561284/561354-561357 keeps its data cast time, united with our NPC Volley limit
  - `340294d3` (#6773): a mechanical under Control Mechanical 807846 uses pet AI
  - `80361ef4` (#6808, lock hunk only): Smash Lock 570122 and Melt Lock 804662 keep their lock bonus after Lockpicking is learned
  - `c34555c4` (#6822, Pet.cpp only): an abandoned pet is deleted from the database even while it loads (stock fix)
  - `58323f05` (#6811, Spell.cpp only): the open-lock bonus scales from the caster - FL: only for an Ascension class
  - `20451dd9` (#6793, hand port): Hellknight 800703 movement per stack in `Unit::UpdateSpeed`
  - `36a3d8b5` (#6608, core half): a charged spell (`MaxCharges`, families 18-38 only) recovers through its charges, not its DBC category cooldown
  - `341e49df` (#6526, core half): Dark Frenzy's GCD rule, deferred extra attacks at a named victim, `RuntimeExcludeCasterAuraSpell` - FL: the GCD and extra-attack rules only for the Ascension classes
  - `9bf73a95` (#6436): Wind Surge 805750 adds jump targets to Air Engraving's copy 653226
  - `eb9959e6` (#6413): a friendly target that only receives positive effects cannot miss - FL: only for Ascension-class casters
  - `4b0adf33` (#6015, helper only): `Unit::IsImmuneToForcedMovement`; `KnockbackFrom` stays stock
  - `f212136d` (#6157): `RemoveAppliedAuras` removes each applied aura once (stock safety fix)
  - `ae10c347` (#6209), `8493cbf7` (#5986): the periodic-heal multiplier exempts 680693 and 561231
  - `d1b95282` (#6148): a dead player keeps 0 health through power resets and health auras (stock fix, AzerothCore #27885)
  - `7498750f` (#5964): a two-hander in the off-hand satisfies an off-hand weapon spell (Valkyr's Grip)
  - `8037cdbf` (#5908, core part): `MoveJump` with an optional final orientation, `Spell::SetJumpFinalOrientation`
  - `57410f0e` (#5901): Gatling Gun shoots the gun inside the Mechsuit; `FORM_TINKER_MECHSUIT`
  - `0540151f` (#5891): the Mechsuit helper 803451 passes a no-mount instance
  - `70f8774c` (#4542, hook only): `UNITHOOK_ON_BEFORE_HEAL_ABSORB` / `OnBeforeHealAbsorb` at the top of `Unit::CalcHealAbsorb` (Infuse in the kept Bloodmage Talents)
  - affected files: `Entities/Item/Item.cpp`, `Entities/Pet/Pet.cpp`, `Entities/Player/Player.cpp`, `Entities/Unit/StatSystem.cpp`, `Entities/Unit/Unit.cpp`, `Entities/Unit/Unit.h`, `Entities/Unit/UnitDefines.h`, `Handlers/MovementHandler.cpp`, `Movement/MotionMaster.cpp`, `Movement/MotionMaster.h`, `Scripting/ScriptDefines/UnitScript.cpp`, `Scripting/ScriptDefines/UnitScript.h`, `Scripting/ScriptMgr.h`, `Spells/AscensionPooledVitality.h`, `Spells/Auras/SpellAuraEffects.cpp`, `Spells/Spell.cpp`, `Spells/Spell.h`, `Spells/SpellEffects.cpp`, `Spells/SpellInfo.cpp`, `Spells/SpellInfoCorrections.cpp` (all under `src/server/game/`)

- 2026-10-09 — fix(Core/Vmaps): a BIH ray traversal stops at the size of its node stack — taken from the CoA fork (`7c283aa5`, CoA #6687), the bug is stock AzerothCore. A ray with a zero direction component whose origin lies on a split plane gives NaN split distances; `BIH::intersectRay` then pushed the same node forever and wrote past its 64-entry stack array until the thread stack ended. Host: vault MIG entry of the CoA crash fixes.
  - affected files: `src/common/Collision/BoundingIntervalHierarchy.h`

- 2026-10-08 — refactor(DB/Tickets): Player reports no longer write GM tickets (operator decision). Removed the 2026-10-04 TicketMgr pending-write lease and every guard it needed (ticket handlers, `.ticket` commands, `Player::DeleteFromDB`, character delete opcode, `.character erase`, `AccountMgr::DeleteAccount` order): those files match upstream again, except the `.ticket delete` fix (`7dab1a844`). `CHAR_INS_FL_PLAYER_REPORT` writes the ticket link as 0 (24 parameters); `CHAR_SEL_FL_PLAYER_REPORT_RECEIPT` returns only the unsigned report Id. The checked START/COMMIT boundaries stay. Companion `mod-fl-player-reports` change required (vault MIG-095).

- 2026-10-08 — fix(Core/Unit): `Unit::Kill` runs only once per victim at a time (`3559bb45e`). The victim keeps its health until `setDeathState`, so a KILL/KILLED/DEATH proc that damages the dying unit re-entered `Kill` through `DealDamage` - loot, rewards and procs again, without end when the proc cannot miss. Forgotten Talents 120830 → 120831 did this: the production crash of 2026-10-04 (stack overflow on the map thread). A thread-local set of victims in `Kill` returns the nested call. T1: bot runs 383 (unfixed: 7 re-entries until a miss), 384 (unfixed: crash at the killing blow), 385 (fixed: PASS) - share-public `python_scripts/fl_content/tests/core_kill_reentry.tbs`. Host: vault MIG-088.

- 2026-10-08 — fix(Core/Tickets): `.ticket delete` read the ticket after `TicketMgr::RemoveTicket` deleted it (`UpdateLastChange` → `OnTicketUpdateLastChange`, which mod-ale pushes into Lua, and `GetPlayer`). The player is read and the hook runs first, then the ticket is removed (`7dab1a844`, `cs_ticket.cpp`). Still in upstream. T1: bot run 370 `core_ticket_delete` (share-public `python_scripts/fl_content/tests`). Host: vault MIG-088.

- 2026-10-04 — fix(DB/Tickets): Explicit unsigned binary integer results for all four player-report receipt fields. MySQL's mixed unsigned BIGINT/signed COALESCE returns NEWDECIMAL; interpreting its prepared ASCII zero as uint64 falsely acknowledges an absent report. Actual native run338 failed6/3 with no new report/ticket rows; preserve that result. Companion module uses uint64 getters and validates stored ticket identity before narrowing. Isolated real MySQL prepared-result metadata/value proof; corrected native acceptance remains pending.

- 2026-10-04 — fix(Core/Accounts): Refuse account deletion before hooks/kicks/character or auth-row removal if any account character has a pending linked report; guard explicit character erase before kick/success message. Reuse the existing character query/result, no added queries/API. Prevent account deletion from continuing after central void DeleteFromDB deferral. Source-derived isolated lifecycle proof and syntax checks; no existing test account/character deletion fixture.

- 2026-10-04 — feat(Core/Tickets): Provide asynchronous linked player-report persistence: world-thread per-character pending leases guard native ticket/character mutations; committed tickets publish in place; two characters-DB statements support the additive module schema. Check transaction START/COMMIT results and suppress single-statement reconnect/replay within ExecuteTransaction. Companion mod-fl-player-reports required; isolated fault/protocol/UI checks pass, shared build/native/client acceptance pending (MIG-079).

- 2026-10-03 — fix(DB): Add Paragon allocation deletion statement — async `CHAR_DEL_PARAGON_POINTS` for mod-paragon's permanent-deletion hook; T1 isolated Windows worldserver build and native deletion fixture; companion mod-paragon change required, full-fleet integration/deployment pending (vault MIG-068)
> As of: 2026-09-17. Add a line here for every new custom commit.

### Custom classes (CoA port)

- 2026-09-19 — feat(Core): CoA's core delta ported onto the synced base (branch `claude/coa-core-port-753fde25`: `765f90c3d` mechanical port of CoA's 83-file delta against upstream 084c9e2 with 8 conflict resolutions, `6b048046e` Ascension protocol branches removed, `440b4c178` module-owned racial test out, `8505ea911` stock behaviour kept for quest-level packets (behind `LocalLevelScaling::QuestEnabled`), aura amount broadcasts, projectile display ids and Mithril Spurs). Excluded: the Ascension launcher login (`AuthSession`, `SRP6`), the `WorldSocket` protocol, the class-10 mapping, the 11-byte spell-modifier layout, the Manastorm statements. Plan and hunk inventory: `share-public` `docs/superpowers/plans/2026-09-19-coa-core-port.md`, `docs/World of Warcraft/coa-classes/evidence/core-port-hunk-inventory.md`. Not on master: the workbench blockers are listed there (FT spell 120856 on aura 313, a host-toolchain build, T2 of the general changes).
  - affected files: 74 net (CoA's 83 minus E1/E2's 6, `enuminfo_SharedDefines.cpp` already equal, E7's test and G15's `spell_item.cpp` back to stock) - `Spells/*`, `Entities/Unit|Player|Creature|Item|Totem`, `Scripting/ScriptDefines/*`, `Handlers/*`, `Maps/*`, `SharedDefines.h`, `DBCStore(s)`, `CharacterDatabase`, `LoginDatabase`, `WorldSession.h`, 5 tests
- 2026-09-19 — feat(Core/Spells): spell effect and aura enums widened for the CoA spell package, Phase 3 "must widen to load" (spike branch `claude/coa-spell-enums-753fde25`; spec: `share-public` `docs/World of Warcraft/coa-classes/evidence/spell-package-spec.md` §2) — `TOTAL_SPELL_EFFECTS` 165→199 and `TOTAL_AURAS` 317→367 with CoA's names plus `SPELL_EFFECT_ASCENSION_<id>` / `SPELL_AURA_ASCENSION_<id>` placeholder names; every new effect is `EffectNULL`, every new aura `HandleNULL`, every new `SpellEffectInfo::_data` row a no-implicit-target placeholder, so such spells load and do nothing on those effects until the handlers are ported. Both handler tables and `_data` are length-checked at compile time (a short list no longer leaves a nullptr tail); the aura dispatch falls back to `HandleNoImmediateEffect` on a nullptr slot.
  - affected files: `SharedDefines.h`, `SpellAuraDefines.h`, `SpellEffects.cpp`, `SpellInfo.cpp`, `SpellAuraEffects.cpp`, `Spell.cpp`
- 2026-09-14 — feat(Core): class ids 12-32 foundation, CoA Tier 0 port (spike branch `claude/coa-dummy-class-753fde25`; documented in `share-public` PR #65, `docs/World of Warcraft/coa-classes/04-dummy-class-spike.md`) — `MAX_CLASSES` 12→33, the 21 CoA class enum members + `IsAscensionClass()` / `GetLegacyClassForCustomClass()` / `ExpandLegacyClassMask()`, per-class dodge/miss/parry arrays indexed through the legacy class, who-list shift guard.
  - affected files: `SharedDefines.h`, `enuminfo_SharedDefines.cpp`, `Player.cpp`, `StatSystem.cpp`, `MiscHandler.cpp`

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
