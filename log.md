# Change Log — azerothcore-wotlk (Fork)

> Dieser Repo ist ein **Fork** von `azerothcore/azerothcore-wotlk`. Der Großteil der Commits sind Upstream-Sync-Merges.
> Hier werden nur **projektspezifische / Custom-Änderungen** notiert (Hooks, DBC-Patches). Standard-Upstream-Fixes lassen sich aus der GitHub-Oberfläche oder via `git log master..HEAD` extrahieren.

## Custom-Änderungen (projekt-spezifisch)

> Stand: 2026-05-01. Bei jedem neuen Custom-Commit hier eine Zeile ergänzen.

### Hooks für externe Module

- 2026-03-22 — feat: Reagent-Hooks für External Storage (in `share-public/claude_log.md` dokumentiert) — `OnPlayerCheckReagent` und `OnPlayerConsumeReagent` PlayerScript-Hooks ergänzt. Aufrufstellen: `Spell::CheckItems()` und `Spell::TakeReagents()`. Genutzt von **mod-endless-storage** (mittlerweile via Lua-Pfad ersetzt — Hooks bleiben aber im Core, falls künftig wieder gebraucht).
  - betroffene Dateien: `src/server/game/Scripting/ScriptDefines/PlayerScript.{h,cpp}`, `src/server/game/Scripting/ScriptMgr.h`, `src/server/game/Spells/Spell.cpp`
  - ~45 Zeilen Custom-Code

### DBC

- 2026-03-18 — fix: Spell.dbc Korruption behoben + Validierung eingebaut (in `share-public/claude_log.md`) — `share/dbc/Spell.dbc` und `share/copy_spells_dbc.py` mit 6 Schutzmaßnahmen (Größencheck, String-Table, Duplikat-Erkennung, Format-Konsistenz, Source≠Target, Post-Write-Verify).

## Letzte Upstream-Syncs (zur Orientierung)

- 2026-04-06 — Merge upstream/master ([67375a7](https://github.com/Shoro2/azerothcore-wotlk/commit/67375a7ca6c583f339d7f628c4f1f2ee91b76fbf))
- Upstream-Highlights aus diesem Merge:
  - fix(Core/Movement): prevent PvP flag and backwards movement on taxi login (#25153)
  - fix(Scripts/Magtheridon): scheduler update before UpdateVictim (#25379)
  - fix(Core/Unit): prevent creature evade when on threat list (#25328)
  - fix(Core/OutdoorPvP): use-after-free in DelCapturePoint (#25229)
  - fix(Core/Groups): pass actual loot count to OnPlayerGroupRollRewardItem (#25312)
  - fix(Core/Loot): restore hide quest starter item conditions (#25355)

## Konvention

Neue Custom-Einträge oben unter "Custom-Änderungen" anhängen. Upstream-Sync-Merges in einem Sammeleintrag pro Sync-Datum, ohne jeden einzelnen Upstream-PR aufzulisten.

Detaillierte Beschreibungen Custom-Änderungen gehören parallel in `share-public/claude_log.md`.
