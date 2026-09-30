/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "ScriptDefines/AllSpellScript.h"
#include "ScriptDefines/PlayerScript.h"
#include "ScriptDefines/UnitScript.h"
#include "gtest/gtest.h"

/**
 * The hooks the Chapters-of-Azeroth catch-up adds to the core reach the scripts that enable them:
 *   UNITHOOK_ON_AFTER_AURA_EFFECT_CALCULATE_AMOUNT (the Barbarian's Barbaric Rage),
 *   PLAYERHOOK_ON_REFRESH_QUEST_GIVER (the Hero Call Board),
 *   ALLSPELLHOOK_ON_SPELL_FOCUS_ANSWERED (Woodworking's sawmills).
 * Each ScriptMgr entry point must call a script that enabled its hook, hand the script's answer back (or its
 * change of the amount), and leave alone a script that overrides the virtual without enabling the hook.
 * Each test script is registered once, like a module script; its behaviour is switched per test.
 */
namespace
{
struct HookCounters
{
    uint32 afterAmount = 0;
    uint32 afterAmountNotEnabled = 0;
    uint32 refreshQuestGiver = 0;
    uint32 refreshQuestGiverNotEnabled = 0;
    uint32 spellFocus = 0;
    uint32 spellFocusNotEnabled = 0;
};

HookCounters counters;
int32 amountDelta = 0;
bool refreshAnswer = false;
bool focusAnswer = false;

class HookDispatchAfterAmountScript : public UnitScript
{
public:
    HookDispatchAfterAmountScript() : UnitScript("HookDispatchAfterAmountScript", true,
        { UNITHOOK_ON_AFTER_AURA_EFFECT_CALCULATE_AMOUNT }) { }

    void OnAfterAuraEffectCalculateAmount(AuraEffect const* /*effect*/, Unit* /*caster*/, int32& amount) override
    {
        ++counters.afterAmount;
        amount += amountDelta;
    }
};

class HookDispatchAfterAmountNotEnabledScript : public UnitScript
{
public:
    HookDispatchAfterAmountNotEnabledScript() : UnitScript("HookDispatchAfterAmountNotEnabledScript", true,
        { UNITHOOK_ON_HEAL }) { }

    void OnAfterAuraEffectCalculateAmount(AuraEffect const* /*effect*/, Unit* /*caster*/, int32& amount) override
    {
        ++counters.afterAmountNotEnabled;
        amount = -1;
    }
};

class HookDispatchRefreshQuestGiverScript : public PlayerScript
{
public:
    HookDispatchRefreshQuestGiverScript() : PlayerScript("HookDispatchRefreshQuestGiverScript",
        { PLAYERHOOK_ON_REFRESH_QUEST_GIVER }) { }

    bool OnPlayerRefreshQuestGiver(Player* /*player*/, Object* /*questGiver*/, Quest const* /*quest*/) override
    {
        ++counters.refreshQuestGiver;
        return refreshAnswer;
    }
};

class HookDispatchRefreshQuestGiverNotEnabledScript : public PlayerScript
{
public:
    HookDispatchRefreshQuestGiverNotEnabledScript() : PlayerScript("HookDispatchRefreshQuestGiverNotEnabledScript",
        { PLAYERHOOK_ON_QUEST_ABANDON }) { }

    bool OnPlayerRefreshQuestGiver(Player* /*player*/, Object* /*questGiver*/, Quest const* /*quest*/) override
    {
        ++counters.refreshQuestGiverNotEnabled;
        return true;
    }
};

class HookDispatchSpellFocusScript : public AllSpellScript
{
public:
    HookDispatchSpellFocusScript() : AllSpellScript("HookDispatchSpellFocusScript",
        { ALLSPELLHOOK_ON_SPELL_FOCUS_ANSWERED }) { }

    bool OnSpellFocusAnswered(Spell* /*spell*/) override
    {
        ++counters.spellFocus;
        return focusAnswer;
    }
};

class HookDispatchSpellFocusNotEnabledScript : public AllSpellScript
{
public:
    HookDispatchSpellFocusNotEnabledScript() : AllSpellScript("HookDispatchSpellFocusNotEnabledScript",
        { ALLSPELLHOOK_ON_SPELL_CHECK_CAST }) { }

    bool OnSpellFocusAnswered(Spell* /*spell*/) override
    {
        ++counters.spellFocusNotEnabled;
        return true;
    }
};

class CoACatchUpHookDispatchTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        static bool registered = false;
        if (!registered)
        {
            ScriptRegistry<UnitScript>::InitEnabledHooksIfNeeded(UNITHOOK_END);
            ScriptRegistry<PlayerScript>::InitEnabledHooksIfNeeded(PLAYERHOOK_END);
            ScriptRegistry<AllSpellScript>::InitEnabledHooksIfNeeded(ALLSPELLHOOK_END);
            new HookDispatchAfterAmountScript();
            new HookDispatchAfterAmountNotEnabledScript();
            new HookDispatchRefreshQuestGiverScript();
            new HookDispatchRefreshQuestGiverNotEnabledScript();
            new HookDispatchSpellFocusScript();
            new HookDispatchSpellFocusNotEnabledScript();
            registered = true;
        }

        counters = HookCounters();
        amountDelta = 0;
        refreshAnswer = false;
        focusAnswer = false;
    }
};
}

TEST_F(CoACatchUpHookDispatchTest, AfterAuraEffectCalculateAmountReachesTheEnabledScript)
{
    amountDelta = -500;
    int32 amount = 500;
    sScriptMgr->OnAfterAuraEffectCalculateAmount(nullptr, nullptr, amount);

    EXPECT_EQ(amount, 0);
    EXPECT_EQ(counters.afterAmount, 1u);
    EXPECT_EQ(counters.afterAmountNotEnabled, 0u);
}

TEST_F(CoACatchUpHookDispatchTest, RefreshQuestGiverReturnsTheScriptsAnswer)
{
    EXPECT_FALSE(sScriptMgr->OnPlayerRefreshQuestGiver(nullptr, nullptr, nullptr));
    refreshAnswer = true;
    EXPECT_TRUE(sScriptMgr->OnPlayerRefreshQuestGiver(nullptr, nullptr, nullptr));

    EXPECT_EQ(counters.refreshQuestGiver, 2u);
    EXPECT_EQ(counters.refreshQuestGiverNotEnabled, 0u);
}

TEST_F(CoACatchUpHookDispatchTest, SpellFocusAnsweredReturnsTheScriptsAnswer)
{
    EXPECT_FALSE(sScriptMgr->OnSpellFocusAnswered(nullptr));
    focusAnswer = true;
    EXPECT_TRUE(sScriptMgr->OnSpellFocusAnswered(nullptr));

    EXPECT_EQ(counters.spellFocus, 2u);
    EXPECT_EQ(counters.spellFocusNotEnabled, 0u);
}
