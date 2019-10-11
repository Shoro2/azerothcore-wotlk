#include "Creature.h"
#include "MapManager.h"
#include "World.h"
#include "Map.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"

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

class Customdungeon_C : public AllCreatureScript{
public:
    Customdungeon_C() : AllCreatureScript("Customdungeon_C") {}
    uint32 counter = 0;
	list<uint64> creatureList;
	bool done=false;
    void OnAllCreatureUpdate(Creature* creature, uint32 diff) override
    {
        if (sConfigMgr->GetBoolDefault("customdungeon.enabled", true)) {
			Map* dungeon = creature->GetMap();
			if (dungeon->IsDungeon()) ModifyCreatureAttributes(creature);
        }
        
    }

	

    void ModifyCreatureAttributes(Creature* creature, bool resetSelLevel = false){
		if(!done){
			if (!creature->IsAlive())
				return;
			if (!creature || !creature->GetMap())
				return;
			if (((creature->IsHunterPet() || creature->IsPet() || creature->IsSummon()) && creature->IsControlledByPlayer()))
			{
				return;
			}
			
			
			if (!creature->IsDungeonBoss()) {
				uint64 creature_guid = creature->GetGUID();
				list<uint64>::iterator temp;
				temp = find(creatureList.begin(), creatureList.end(), creature_guid);
				if (temp != creatureList.end()) {
					if (counter % 100 == 0) {
						//apply buff +300%
						creature->SetLevel(creature->getLevel() + 3);
						creature->AddAura(100000, creature);
						counter++;
						
					}
					else {
						creatureList.assign(1, creature_guid);
						counter++;
					}
				}
				else {
					done=true;
				}
			}
		}
		
    }
};

class Customdungeon_W : public WorldScript {
public:
	Customdungeon_W() : WorldScript("Customdungeon_W") {}
	void OnBeforeConfigLoad(bool reload) override
	{
		if (!reload) {
			std::string conf_path = _CONF_DIR;
			std::string cfg_file = conf_path + "Settings/modules/mod_customdungeon.conf";
#ifdef WIN32
			cfg_file = "Settings/modules/mod_customdungeon.conf";
#endif
			std::string cfg_def_file = cfg_file + ".dist";
			sConfigMgr->LoadMore(cfg_def_file.c_str());

			sConfigMgr->LoadMore(cfg_file.c_str());
		}
	}
};

void Addmod_customdungeonScripts() {
    new Customdungeon_P();
    new Customdungeon_M();
    new Customdungeon_C();
	new Customdungeon_W();
}

