# Functions & mechanics — azerothcore-wotlk (Fork)

> Detailed function and mechanics reference for **custom extensions beyond upstream AzerothCore**. For pure content docs see `CLAUDE.md`. Standard AC concepts (SpellScript lifecycle, proc system, DBC) are documented centrally in `share-public/docs/03-spell-system.md`.

## Reagent guard (hooks reverted)

The fork has **no custom reagent hooks**. `OnPlayerCheckReagent` / `OnPlayerConsumeReagent` were added in `e99877bce` for mod-endless-storage and reverted the same day in `0cb0773a7` (2026-03-22), after the module moved to its Lua path. The revert removed them from `PlayerScript.h/.cpp`, `ScriptMgr.h` and both call sites in `Spell.cpp`.

What remains is one guard in `Spell::TakeReagents()` (`src/server/game/Spells/Spell.cpp:5587`):

```cpp
if (itemcount > 0)
    p_caster->DestroyItemCount(itemid, itemcount, true);
```

`DestroyItemCount` is skipped when a reagent slot's count is 0. The guard was added with the consume hook, which could lower the count to 0; without the hook it is harmless. Total scope: 2 lines.

## Custom Spell.dbc

The server loads a custom variant of `Spell.dbc` that adds custom spell IDs. **This repo tracks no DBC file** (there is no `share/` directory); the copies live outside it, documented in share-public:

| Copy | Location | Reference |
|------|----------|-----------|
| Server | `Data/dbc/Spell.dbc` of the installed server: workbench `C:\wowstuff\dcore\Data\dbc\Spell.dbc`, mirrored to the host `/wowserver/acore-server/data/dbc/` through the migration ledger; record-level patches by the Forgotten Land workspace scripts 35, 36, 39, 40 (`C:\wowstuff\ForgottenLand2.0\scripts\`) | [FL/15 MIG-002](https://github.com/Shoro2/share-public/blob/main/docs/World%20of%20Warcraft/forgotten-land/15-host-migration-log.md) |
| Client | `DBFilesClient\Spell.dbc` in `patch-9.MPQ`, built from the hot-DBC staging (patched by scripts 33, 35, 36, 40) | [FL/07](https://github.com/Shoro2/share-public/blob/main/docs/World%20of%20Warcraft/forgotten-land/07-client-packaging-and-rendering.md) |
| DB override | `acore_world.spell_dbc` rows, loaded over the file at server start | [03-spell-system](https://github.com/Shoro2/share-public/blob/main/docs/World%20of%20Warcraft/03-spell-system.md) |

share-public `dbc/Spell.dbc` is a non-stock reference extract that differs from the deployed server copy — never deploy it. If the server copy looks corrupt, restore it from the nightly host backup (`data/dbc/`, share-public [Production-Host/04](https://github.com/Shoro2/share-public/blob/main/docs/General/Production-Host/04-backup-and-restore.md)).

Custom spell ID ranges:

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
| `PlayerScript` | `OnPlayerLogin`, `OnPlayerLogout`, `OnPlayerLootItem`, `OnPlayerLevelChanged`, `OnPlayerMapChanged`, `OnCreatureKill`, `OnPlayerCanSetTradeItem`, `OnPlayerCanSendMail`, `OnPlayerCanSendErrorAlreadyLooted`, `OnPlayerUpdate`, `OnPlayerCreateItem`, `OnPlayerQuestRewardItem`, `OnPlayerAfterStoreOrEquipNewItem` |
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
