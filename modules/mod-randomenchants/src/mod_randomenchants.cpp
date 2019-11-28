/*
# Modifications by StygianTheBest
- v2017.07.29 - Added module announce, Updated strings

---

* Converted from the original LUA script to a module for Azerothcore(Sunwell) :D
*/

#include "ScriptMgr.h"
#include "Player.h"
#include "Configuration/Config.h"

class RandomEnchantsPlayer : public PlayerScript {
public:

    RandomEnchantsPlayer() : PlayerScript("RandomEnchantsPlayer") { }

    // StygianTheBest - v2017.07.29
    void OnLogin(Player* player) {
        if (sConfigMgr->GetBoolDefault("RandomEnchants.Announce", true))
            ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00RandomEnchants |rmodule. Edited by GvR Mr Mister");
    }

    void OnLootItem(Player* player, Item* item, uint32 count, uint64 /*lootguid*/) override
    {
		if (sConfigMgr->GetBoolDefault("RandomEnchants.OnLoot", true)) {
			RollPossibleEnchant(player, item);
		}
            
    }
    void OnCreateItem(Player* player, Item* item, uint32 count) override
    {
		if (sConfigMgr->GetBoolDefault("RandomEnchants.OnCreate", true)) {
			RollPossibleEnchant(player, item);
		}
            
    }
    void OnQuestRewardItem(Player* player, Item* item, uint32 count) override
    {
		if (sConfigMgr->GetBoolDefault("RandomEnchants.OnQuestReward", true)) {
			RollPossibleEnchant(player, item);
		}
            
    }
    void RollPossibleEnchant(Player* player, Item* item)
    {
        uint32 Quality = item->GetTemplate()->Quality;
        uint32 Class = item->GetTemplate()->Class;
		uint32 Level = item->GetTemplate()->RequiredLevel;
        if ((Quality > 5 && Quality < 0)/*eliminates enchanting anything that isn't a recognized quality*/ || (Class != 2 && Class != 4)/*eliminates enchanting anything but weapons/armor*/)
            return;
        int slotRand[3] = { -1, -1, -1 };
        // überprüfen
        uint32 slotEnch[3] = { 0, 1, 5 };
		ChatHandler chathandle = ChatHandler(player->GetSession());
		chathandle.PSendSysMessage("req lvl: %d", Level);

		slotRand[0] = getRandEnchantment(item, player);
		slotRand[1] = getRandEnchantment(item, player);
		slotRand[2] = getRandEnchantment(item, player);
		
        for (int i = 0; i < 3; i++)
        {
            if (slotRand[i] != -1)
            {
                if (sSpellItemEnchantmentStore.LookupEntry(slotRand[i]))//Make sure enchantment id exists
                {
                    player->ApplyEnchantment(item, EnchantmentSlot(slotEnch[i]), false);
                    item->SetEnchantment(EnchantmentSlot(slotEnch[i]), slotRand[i], 0, 0);
                    player->ApplyEnchantment(item, EnchantmentSlot(slotEnch[i]), true);
                }
            }
        }
        
       
    }
    int getRandEnchantment(Item* item, Player* player)
    {
		ChatHandler cHandler = ChatHandler(player->GetSession());
        uint32 Class = item->GetTemplate()->Class;
		uint32 RequiredLevel = item->GetTemplate()->RequiredLevel;
		uint32 Level = player->getLevel();
		cHandler.PSendSysMessage("plvl: %d", Level);
		cHandler.PSendSysMessage("req lvl: %d", RequiredLevel);
        std::string ClassQueryString = "";
        switch (Class)
        {
        case 2:
            ClassQueryString = "WEAPON";
            break;
        case 4:
            ClassQueryString = "ARMOR";
            break;
        }
        if (ClassQueryString == "")
            return -1;

        int rarityRoll = -1;
		int tier = 0;

		//< lvl 20
		if (RequiredLevel >= 100) {
			tier = 10;
		}
		else if (RequiredLevel >= 90) {
			tier = 9;
		}
		else if (RequiredLevel >= 80) {
			tier = 8;
		}
		else if (RequiredLevel >= 70) {
			tier = 7;
		}
		else if (RequiredLevel >= 60) {
			tier = 6;
		}
		else if (RequiredLevel >= 50) {
			tier = 5;
		}
		else if (RequiredLevel >= 40) {
			tier = 4;
		}
		else if (RequiredLevel >= 30) {
			tier = 3;
		}
		else if (RequiredLevel >= 20) {
			tier = 2;
		}
		else if (RequiredLevel >= 10) {
			tier = 1;
		}
		if (item->GetTemplate()->Quality == 7) tier = 2;

		if((rand() % 10) + 1 == 1) tier = tier +1;
		if (tier > 10) tier = 10;
		cHandler.PSendSysMessage("tier: %d", tier);
		
		cHandler.PSendSysMessage("SELECT enchantID FROM item_enchantment_random_stats WHERE tier= %d order by rand() LIMIT 1", tier);
		QueryResult qr = WorldDatabase.PQuery("SELECT enchantID FROM item_enchantment_random_stats WHERE tier= %d order by rand() LIMIT 1", tier);
		cHandler.PSendSysMessage("query: %d", qr->Fetch()[0].GetUInt32());
		return qr->Fetch()[0].GetUInt32();
    }
};
class RandomEnchantsWorld : public WorldScript
{
public:
    RandomEnchantsWorld() : WorldScript("RandomEnchantsWorld") { }

    void OnBeforeConfigLoad(bool reload) override
    {
        if (!reload) {
            std::string conf_path = _CONF_DIR;
            std::string cfg_file = conf_path + "Settings/modules/mod_randomenchants.conf";
#ifdef WIN32
            cfg_file = "Settings/modules/mod_randomenchants.conf";
#endif
            std::string cfg_def_file = cfg_file + ".dist";
            sConfigMgr->LoadMore(cfg_def_file.c_str());

            sConfigMgr->LoadMore(cfg_file.c_str());
        }
    }
};

void AddRandomEnchantsScripts() {
    new RandomEnchantsWorld();
    new RandomEnchantsPlayer();
}
