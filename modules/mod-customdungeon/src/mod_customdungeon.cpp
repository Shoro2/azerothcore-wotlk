#include "Creature.h"
#include "MapManager.h"
#include "World.h"
#include "Map.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "Configuration/Config.h"

class Customdungeon_P : public PlayerScript{
public:

    Customdungeon_P() : PlayerScript("Customdungeon_P") { }

    void OnLogin(Player* player) override {
        if (sConfigMgr->GetBoolDefault("customdungeon.announceModule", true)) {
            ChatHandler(player->GetSession()).SendSysMessage("This Server is using the customdungeon module.");
        }
    }
};

class Customdungeon_M : public AllMapScript{
public:
    Customdungeon_M() : AllMapScript("Customdungeon_M") {}

    void OnPlayerEnterAll(Map* map, Player* player){

    }

    void OnPlayerLeaveAll(Map* map, Player* player){

    }
};

class Customdungeon_C : public CreatureScript{
public:
    Customdungeon_C() : CreatureScript("Customdungeon_C") {}

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (sConfigMgr->GetBoolDefault("customdungeon.enabled", true)) {
            ModifyCreatureAttributes(creature);
        }
        
    }

    void ModifyCreatureAttributes(Creature* creature){

    }
};

void AddCustomdungeonScripts() {
    new Customdungeon_P();
    new Customdungeon_M();
    new Customdungeon_C();
}

