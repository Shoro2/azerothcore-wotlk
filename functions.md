# Funktionen & Mechaniken — azerothcore-wotlk (Fork)

> Detaillierte Funktions- und Mechanik-Referenz für **Custom-Erweiterungen über den Upstream-AzerothCore hinaus**. Reine Inhalts-Doku siehe `CLAUDE.md`. Standard-AC-Konzepte (SpellScript-Lifecycle, Proc-System, DBC) sind in `share-public/docs/03-spell-system.md` zentral dokumentiert.

## Custom-Hooks gegenüber Upstream

Dieser Fork erweitert `PlayerScript` um zwei Hooks für externe Storage-Lösungen.

### `OnPlayerCheckReagent`

```cpp
// src/server/game/Scripting/ScriptDefines/PlayerScript.h
virtual bool OnPlayerCheckReagent(Player* player, Spell* spell,
                                   uint32 itemId, uint32 itemCount, uint32& foundCount);
```

- **Aufrufstelle**: `Spell::CheckItems()` — wenn das Inventar nicht genug Reagenzien für einen Cast hat.
- **Vertrag**: Hook-Implementer kann `foundCount` erhöhen (additiv aus externer Quelle), um anzuzeigen, dass Reagenzien dort zu finden sind. Returnt `true`, wenn der Cast nun durchgehen darf.
- **Genutzt von**: ursprünglich mod-endless-storage (mittlerweile via Lua-Pfad ersetzt; Hook bleibt im Core verfügbar).

### `OnPlayerConsumeReagent`

```cpp
virtual bool OnPlayerConsumeReagent(Player* player, Spell* spell,
                                     uint32 itemId, uint32& itemCount);
```

- **Aufrufstelle**: `Spell::TakeReagents()` — vor `DestroyItemCount`. Wird für jedes Reagenz aufgerufen.
- **Vertrag**: Hook-Implementer kann `itemCount` reduzieren (Anzahl, die noch aus dem Inventar zu konsumieren ist), nachdem ein Teil aus externer Quelle abgezogen wurde. Returnt `true`, wenn extern konsumiert wurde.
- **Default-Verhalten** (nicht-implementiert): kein extern-Konsum, alles aus Inventar.

### Implementierungsdetails

| Datei | Änderung |
|-------|----------|
| `src/server/game/Scripting/ScriptDefines/PlayerScript.h` | Enum-Erweiterung (`PLAYERHOOK_ON_CHECK_REAGENT`, `..._CONSUME_REAGENT`), virtuelle Methoden |
| `src/server/game/Scripting/ScriptDefines/PlayerScript.cpp` | `ScriptMgr::OnPlayerCheckReagent` / `..._ConsumeReagent` Dispatcher |
| `src/server/game/Scripting/ScriptMgr.h` | Methoden-Deklarationen |
| `src/server/game/Spells/Spell.cpp` | Aufrufstellen in `CheckItems()` und `TakeReagents()` |

Gesamtumfang: ~45 Zeilen Custom-Code.

## Custom Spell.dbc

Der Server lädt eine Custom-Variante von `Spell.dbc` aus `share/dbc/`, die Custom-Spell-IDs ergänzt:

| ID-Range | Verwendung |
|----------|-----------|
| `100000` | Paragon Level-Counter Aura |
| `100001-100027` | Paragon Stat-Auras (Small) |
| `100201-100227` | Paragon Stat-Auras (Big — 100× Stack-Wert) |
| `900100-900116` | Custom-Spells (Paragon Strike, Bladestorm CD Reduce, Bloody Whirlwind, ...) |
| `900168-901108` | mod-custom-spells Marker-Auras |

Toolchain für Spell.dbc:
- `share-public/python_scripts/copy_spells_dbc.py` — extrahiert Custom-Spells aus Source-DBC, merged in Target. **Mit 6 Schutzmaßnahmen** gegen Korruption (Größencheck, String-Table, Duplikate, Format, Source≠Target, Post-Verify).
- `share-public/python_scripts/add_paragon_spell.py` — generiert SQL für neue Paragon-Passive-Spells (IDs 950001-950099).
- `share-public/python_scripts/patch_dbc.py` — patcht `SpellItemEnchantment.dbc` für ~11.323 Custom-Enchantments.

## Build mit Custom-Modulen

```bash
# Module liegen als Symlinks oder Clones unter modules/
ls modules/
# mod-paragon  mod-paragon-itemgen  mod-loot-filter  mod-auto-loot  mod-endless-storage  ...

mkdir -p build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/azeroth-server \
         -DCMAKE_BUILD_TYPE=RelWithDebInfo \
         -DSCRIPTS=static -DMODULES=static
make -j$(nproc)
make install
```

Single-Module deaktivieren:
```bash
cmake .. -DDISABLED_AC_MODULES="mod-foo;mod-bar"
```

## Modul-Loader-System

```
Build:
  modules/CMakeLists.txt
    → GetModuleSourceList()
    → für jedes Modul Quellen sammeln + RegisterModuleScript()
    → ConfigureScriptLoader() generiert ModulesLoader.cpp:
       - Forward-Declarations: Addmod_paragonScripts(), Addmod_paragon_itemgenScripts(), ...
       - AddModulesScripts() ruft alle in Reihenfolge auf

Server-Start:
  ScriptMgr::Initialize()
    → AddModulesScripts()
       → Addmod_paragonScripts()
       → Addmod_paragon_itemgenScripts()
       → ...
```

Loader-Naming-Konvention: `<modul-name>` mit `-` durch `_` ersetzt. Sonderfall: mod-auto-loot nutzt `AddSC_AutoLoot()` — das wird ebenfalls vom Loader-Generator erkannt.

## Hook-Klassen-Referenz (Auszug)

| Klasse | Wichtige Hooks für Custom-Module |
|--------|----------------------------------|
| `WorldScript` | `OnAfterConfigLoad`, `OnStartup`, `OnUpdate`, `OnShutdown` |
| `PlayerScript` | `OnPlayerLogin`, `OnPlayerLogout`, `OnPlayerLootItem`, `OnPlayerLevelChanged`, `OnPlayerMapChanged`, `OnCreatureKill`, `OnPlayerCheckReagent` (custom), `OnPlayerConsumeReagent` (custom), `OnPlayerCanSetTradeItem`, `OnPlayerCanSendMail`, `OnPlayerCanSendErrorAlreadyLooted`, `OnPlayerUpdate`, `OnPlayerCreateItem`, `OnPlayerQuestRewardItem`, `OnPlayerAfterStoreOrEquipNewItem` |
| `UnitScript` | `OnDamage`, `OnHeal`, `OnAuraApply`, `OnAuraRemove` |
| `CreatureScript` | `OnGossipHello`, `OnGossipSelect`, AI-Factory |
| `SpellScript`/`AuraScript` | Cast/Hit/Effect/Proc-Hooks |
| `CommandScript` | `GetCommands()` → `ChatCommandTable` |

Vollständige Liste: `src/server/game/Scripting/ScriptMgr.h` und `src/server/game/Scripting/ScriptDefines/`.

## DB-Layer-Konventionen

Alle Custom-Module sollen Prepared-Statements nutzen:

```cpp
// Statement-Enum (per Modul):
enum ModFooDatabaseStatements {
    MOD_FOO_SEL_THING = MAX_CHARACTERDATABASE_STATEMENTS, // shift, um upstream nicht zu kollidieren
    MOD_FOO_INS_THING,
    ...
};

// Registrierung im Loader:
CharacterDatabase.PrepareStatement(MOD_FOO_SEL_THING,
    "SELECT col FROM table WHERE id = ?", CONNECTION_SYNCH);
```

Async-Calls: `CharacterDatabase.Execute(stmt)`. Sync-Reads: `CharacterDatabase.Query(stmt)`. Transaktionen: `CharacterDatabase.BeginTransaction() / CommitTransaction(trans)`.

## SQL-Pending-Workflow

1. Neue SQL-Datei mit Random-Filename in `data/sql/updates/pending_<db>/<random>.sql`.
2. CI-Codestyle prüft.
3. PR-Merge.
4. GitHub-Action `chore(DB): import pending files` verschiebt automatisch in `data/sql/updates/<db>/`.

## Codestyle (CI-enforced)

Wird beim PR geprüft via `apps/codestyle/codestyle-cpp.py` und `codestyle-sql.py`. Regeln im Detail siehe `share-public/docs/07-codestyle.md`. Wichtigste Stolpersteine:
- `auto const&` (nicht `const auto&`)
- `Type const*` (nicht `const Type*`)
- `IsPlayer()`/`IsCreature()` statt `GetTypeId() == TYPEID_*`
- `ObjectGuid::ToString().c_str()` statt `GetCounter()` in Logs
- 4-Space Einrückung, LF, UTF-8, max 80 Zeichen

## Logging

```cpp
LOG_ERROR("category", "Msg with {} formatting", var);
LOG_INFO("module-name", "...");
LOG_DEBUG("...", "...");
```

Kategorien-Konvention: für Custom-Module `mod-<name>` als Kategorie.

## Bekannte Custom-Pain-Points

- **`SpellMgr.h` ProcFlag-Werte**: Manche Online-Quellen (wowhead, wowdb) haben falsche Werte. Immer gegen `SpellMgr.h` verifizieren. Korrigierte Tabelle in `share-public/docs/03-spell-system.md`.
- **DBC-Override-Layer**: Server liest erst `.dbc`-Files, dann ggf. DB-Override-Tabellen (`spell_dbc`, `spellitemenchantment_dbc`). Beim Editieren beide synchron halten.
- **`-Werror`**: jeder unused-variable, missing-override, ähnliches blockt CI sofort.
