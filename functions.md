# Functions & mechanics — azerothcore-wotlk (Fork)

> Detailed function and mechanics reference for **custom extensions beyond upstream AzerothCore**. For pure content docs see `CLAUDE.md`. Standard AC concepts (SpellScript lifecycle, proc system, DBC) are documented centrally in `share-public/docs/03-spell-system.md`.

## Custom hooks compared to upstream

This fork extends `PlayerScript` with two hooks for external storage solutions.

### `OnPlayerCheckReagent`

```cpp
// src/server/game/Scripting/ScriptDefines/PlayerScript.h
virtual bool OnPlayerCheckReagent(Player* player, Spell* spell,
                                   uint32 itemId, uint32 itemCount, uint32& foundCount);
```

- **Call site**: `Spell::CheckItems()` — when the inventory does not have enough reagents for a cast.
- **Contract**: hook implementer can increase `foundCount` (additively from an external source) to indicate reagents available there. Returns `true` if the cast may now proceed.
- **Used by**: originally mod-endless-storage (now replaced via the Lua path; the hook remains available in the core).

### `OnPlayerConsumeReagent`

```cpp
virtual bool OnPlayerConsumeReagent(Player* player, Spell* spell,
                                     uint32 itemId, uint32& itemCount);
```

- **Call site**: `Spell::TakeReagents()` — before `DestroyItemCount`. Called for every reagent.
- **Contract**: hook implementer can reduce `itemCount` (the amount still to be consumed from the inventory) after a portion was deducted from an external source. Returns `true` if consumed externally.
- **Default behavior** (not implemented): no external consumption, everything from the inventory.

### Implementation details

| File | Change |
|-------|----------|
| `src/server/game/Scripting/ScriptDefines/PlayerScript.h` | Enum extension (`PLAYERHOOK_ON_CHECK_REAGENT`, `..._CONSUME_REAGENT`), virtual methods |
| `src/server/game/Scripting/ScriptDefines/PlayerScript.cpp` | `ScriptMgr::OnPlayerCheckReagent` / `..._ConsumeReagent` dispatcher |
| `src/server/game/Scripting/ScriptMgr.h` | Method declarations |
| `src/server/game/Spells/Spell.cpp` | Call sites in `CheckItems()` and `TakeReagents()` |

Total scope: ~45 lines of custom code.

## Custom Spell.dbc

The server loads a custom variant of `Spell.dbc` from `share/dbc/` that adds custom spell IDs:

| ID range | Use |
|----------|-----------|
| `100000` | Paragon level counter aura |
| `100001-100027` | Paragon stat auras (small) |
| `100201-100227` | Paragon stat auras (big — 100x stack value) |
| `900100-900116` | Custom spells (Paragon Strike, Bladestorm CD reduce, Bloody Whirlwind, ...) |
| `900168-901108` | mod-custom-spells marker auras |

Toolchain for Spell.dbc:
- `share-public/python_scripts/copy_spells_dbc.py` — extracts custom spells from a source DBC, merges them into target. **With 6 safeguards** against corruption (size check, string table, duplicates, format, source≠target, post-verify).
- `share-public/python_scripts/add_paragon_spell.py` — generates SQL for new Paragon passive spells (IDs 950001-950099).
- `share-public/python_scripts/patch_dbc.py` — patches `SpellItemEnchantment.dbc` for ~11,323 custom enchantments.

## Build with custom modules

```bash
# Modules live as symlinks or clones under modules/
ls modules/
# mod-paragon  mod-paragon-itemgen  mod-loot-filter  mod-auto-loot  mod-endless-storage  ...

mkdir -p build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/azeroth-server \
         -DCMAKE_BUILD_TYPE=RelWithDebInfo \
         -DSCRIPTS=static -DMODULES=static
make -j$(nproc)
make install
```

Disable single modules:
```bash
cmake .. -DDISABLED_AC_MODULES="mod-foo;mod-bar"
```

## Module loader system

```
Build:
  modules/CMakeLists.txt
    → GetModuleSourceList()
    → for each module collect sources + RegisterModuleScript()
    → ConfigureScriptLoader() generates ModulesLoader.cpp:
       - Forward declarations: Addmod_paragonScripts(), Addmod_paragon_itemgenScripts(), ...
       - AddModulesScripts() calls all in order

Server start:
  ScriptMgr::Initialize()
    → AddModulesScripts()
       → Addmod_paragonScripts()
       → Addmod_paragon_itemgenScripts()
       → ...
```

Loader naming convention: `<module-name>` with `-` replaced by `_`. Special case: mod-auto-loot uses `AddSC_AutoLoot()` — that is also recognized by the loader generator.

## Hook class reference (excerpt)

| Class | Important hooks for custom modules |
|--------|----------------------------------|
| `WorldScript` | `OnAfterConfigLoad`, `OnStartup`, `OnUpdate`, `OnShutdown` |
| `PlayerScript` | `OnPlayerLogin`, `OnPlayerLogout`, `OnPlayerLootItem`, `OnPlayerLevelChanged`, `OnPlayerMapChanged`, `OnCreatureKill`, `OnPlayerCheckReagent` (custom), `OnPlayerConsumeReagent` (custom), `OnPlayerCanSetTradeItem`, `OnPlayerCanSendMail`, `OnPlayerCanSendErrorAlreadyLooted`, `OnPlayerUpdate`, `OnPlayerCreateItem`, `OnPlayerQuestRewardItem`, `OnPlayerAfterStoreOrEquipNewItem` |
| `UnitScript` | `OnDamage`, `OnHeal`, `OnAuraApply`, `OnAuraRemove` |
| `CreatureScript` | `OnGossipHello`, `OnGossipSelect`, AI factory |
| `SpellScript`/`AuraScript` | Cast/Hit/Effect/Proc hooks |
| `CommandScript` | `GetCommands()` → `ChatCommandTable` |

Full list: `src/server/game/Scripting/ScriptMgr.h` and `src/server/game/Scripting/ScriptDefines/`.

## DB layer conventions

All custom modules should use prepared statements:

```cpp
// Statement enum (per module):
enum ModFooDatabaseStatements {
    MOD_FOO_SEL_THING = MAX_CHARACTERDATABASE_STATEMENTS, // shift to avoid colliding with upstream
    MOD_FOO_INS_THING,
    ...
};

// Registration in the loader:
CharacterDatabase.PrepareStatement(MOD_FOO_SEL_THING,
    "SELECT col FROM table WHERE id = ?", CONNECTION_SYNCH);
```

Async calls: `CharacterDatabase.Execute(stmt)`. Sync reads: `CharacterDatabase.Query(stmt)`. Transactions: `CharacterDatabase.BeginTransaction() / CommitTransaction(trans)`.

## SQL pending workflow

1. New SQL file with random filename in `data/sql/updates/pending_<db>/<random>.sql`.
2. CI codestyle checks.
3. PR merge.
4. GitHub action `chore(DB): import pending files` automatically moves it to `data/sql/updates/<db>/`.

## Code style (CI-enforced)

Checked at PR time via `apps/codestyle/codestyle-cpp.py` and `codestyle-sql.py`. Detailed rules in `share-public/docs/07-codestyle.md`. Most important pitfalls:
- `auto const&` (not `const auto&`)
- `Type const*` (not `const Type*`)
- `IsPlayer()`/`IsCreature()` instead of `GetTypeId() == TYPEID_*`
- `ObjectGuid::ToString().c_str()` instead of `GetCounter()` in logs
- 4-space indent, LF, UTF-8, max 80 chars

## Logging

```cpp
LOG_ERROR("category", "Msg with {} formatting", var);
LOG_INFO("module-name", "...");
LOG_DEBUG("...", "...");
```

Category convention: for custom modules use `mod-<name>` as the category.

## Known custom pain points

- **`SpellMgr.h` ProcFlag values**: some online sources (wowhead, wowdb) have wrong values. Always verify against `SpellMgr.h`. Corrected table in `share-public/docs/03-spell-system.md`.
- **DBC override layer**: server reads `.dbc` files first, then optional DB override tables (`spell_dbc`, `spellitemenchantment_dbc`). When editing, keep both in sync.
- **`-Werror`**: any unused-variable, missing-override, similar issues block CI immediately.
