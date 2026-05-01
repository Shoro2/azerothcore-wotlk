# TODOs — azerothcore-wotlk (Fork)

> Offene Aufgaben für diesen Fork (Custom-Erweiterungen über Upstream hinaus). Erledigte TODOs in `log.md` festhalten und hier entfernen.
> **Wichtig**: hier nur **projekt-spezifische** TODOs — Upstream-AzerothCore-Issues gehören nicht hierher.

## Custom-Hooks

- [ ] **(niedrig)** `CanCreateAuction`-Hook für mod-paragon-itemgen ergänzen: aktuell nicht verfügbar in AzerothCore. Wenn das Restriction-Feature wichtig wird, einen Player-Hook nach Pattern von `OnPlayerCheckReagent` einbauen (siehe `functions.md` für Vorgehen).
- [ ] **(niedrig)** `OnPlayerCheckReagent` / `OnPlayerConsumeReagent` werden derzeit von keinem aktiven Modul mehr genutzt (mod-endless-storage ist auf reines Lua/AIO migriert). Code im Core lassen, aber als "verfügbar für künftige Nutzung" markieren — nicht entfernen.

## DBC

- [ ] **(niedrig)** Custom-Spell-DBC-Tooling: `share-public/python_scripts/copy_spells_dbc.py` und `add_paragon_spell.py` haben ihre Schutzmaßnahmen. Eine zusätzliche CI/Pre-Push-Validierung der `share/dbc/Spell.dbc` (Header-Größe, Duplikat-Check) wäre wertvoll, damit man nicht wieder eine korrupte DBC ins Repo bekommt.

## Upstream-Sync-Hygiene

- [ ] **(niedrig)** Periodische Upstream-Merges (alle 4-6 Wochen) — nur dokumentieren, keine Aufgabe für KI ohne Userwunsch. Letzter Sync: 2026-04-06.

## Konvention

Erledigte Items NICHT durchstreichen — entfernen und in `log.md` dokumentieren. Upstream-Issues gehören NICHT in diese Datei.
