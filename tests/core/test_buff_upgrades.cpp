#include "App/stdafx.h"

#include <doctest.h>

#include "GameLogic/Buffs/w_Buff.h"

namespace
{
// Every master skill tree upgrade the server can send in place of its base buff.
constexpr eBuffState UPGRADE_VARIANTS[] = {
    EFFECT_GREATER_LIFE_ENHANCED,
    EFFECT_GREATER_LIFE_MASTERED,
    EFFECT_MAGIC_CIRCLE_IMPROVED,
    EFFECT_MAGIC_CIRCLE_ENHANCED,
    EFFECT_GREATER_CRITICAL_DAMAGE_EXTENDED,
    EFFECT_GREATER_CRITICAL_DAMAGE_MASTERED,
    EFFECT_INFINITY_ARROW_IMPROVED,
    EFFECT_BLIND_IMPROVED,
    EFFECT_POISON_ARROW_IMPROVED,
    EFFECT_BLESS_IMPROVED,
    EFFECT_IRON_DEFENSE_IMPROVED,
    EFFECT_BLOOD_HOWLING_IMPROVED,
};
} // namespace

TEST_CASE("Master skill upgrades resolve to the buff they replace [core][buffs]")
{
    for (eBuffState variant : UPGRADE_VARIANTS)
    {
        CHECK(GetBaseBuffState(variant) != variant);
        CHECK(GetBuffUpgradeTier(variant) >= 1);
    }

    CHECK(GetBaseBuffState(EFFECT_GREATER_LIFE_ENHANCED) == eBuff_Life);
    CHECK(GetBaseBuffState(EFFECT_GREATER_LIFE_MASTERED) == eBuff_Life);
    CHECK(GetBuffUpgradeTier(EFFECT_GREATER_LIFE_ENHANCED) < GetBuffUpgradeTier(EFFECT_GREATER_LIFE_MASTERED));

    CHECK(GetBaseBuffState(EFFECT_MAGIC_CIRCLE_ENHANCED) == eBuff_SwellOfMagicPower);
    CHECK(GetBaseBuffState(EFFECT_GREATER_CRITICAL_DAMAGE_EXTENDED) == eBuff_AddCriticalDamage);
    CHECK(GetBaseBuffState(EFFECT_INFINITY_ARROW_IMPROVED) == eBuff_InfinityArrow);
    CHECK(GetBaseBuffState(EFFECT_BLIND_IMPROVED) == eDeBuff_Blind);

    // A buff which has no upgrade maps to itself and is tier 0, so callers can
    // ask about any buff id without knowing whether it is part of a chain.
    CHECK(GetBaseBuffState(eBuff_Life) == eBuff_Life);
    CHECK(GetBuffUpgradeTier(eBuff_Life) == 0);
    CHECK(GetBaseBuffState(eBuff_Defense) == eBuff_Defense);
    CHECK(GetBuffUpgradeTier(eBuff_Defense) == 0);
    CHECK(GetBaseBuffState(eBuffNone) == eBuffNone);
}

TEST_CASE("A buff counts as active while only its upgrade is registered [core][buffs]")
{
    auto buff = Buff::Make();

    CHECK_FALSE(buff->isBuffActive(eBuff_Life));

    // A master level Swell Life arrives as the upgraded effect id only - the base
    // id never shows up in a server buff status.
    buff->RegisterBuff(EFFECT_GREATER_LIFE_MASTERED);

    CHECK_FALSE(buff->isBuff(eBuff_Life));
    CHECK(buff->isBuffActive(eBuff_Life));
    CHECK(buff->isBuffActive(EFFECT_GREATER_LIFE_MASTERED));
    CHECK_FALSE(buff->isBuffActive(eBuff_SwellOfMagicPower));

    buff->UnRegisterBuff(EFFECT_GREATER_LIFE_MASTERED);

    CHECK_FALSE(buff->isBuffActive(eBuff_Life));
    CHECK_FALSE(buff->isBuffActive(EFFECT_GREATER_LIFE_MASTERED));

    buff->RegisterBuff(EFFECT_GREATER_LIFE_ENHANCED);

    CHECK(buff->isBuffActive(eBuff_Life));
}

TEST_CASE("Clearing an upgrade also clears the base buff it replaced [core][buffs]")
{
    auto buff = Buff::Make();

    // The cast registers the base id as a stand-in (ReceiveMagic plays the skill
    // effect), then the server reports the upgraded effect id - for a master level
    // Swell Life the base id never arrives from the server at all.
    buff->RegisterBuff(eBuff_Life);
    buff->RegisterBuff(EFFECT_GREATER_LIFE_MASTERED);

    // The server clears the upgrade id when the buff runs out.
    buff->UnRegisterBuff(EFFECT_GREATER_LIFE_MASTERED);

    CHECK_FALSE(buff->isBuff(eBuff_Life));
    CHECK_FALSE(buff->isBuffActive(eBuff_Life));

    // Clearing a base id on its own leaves an upgrade which is still running.
    buff->RegisterBuff(EFFECT_GREATER_LIFE_MASTERED);
    buff->RegisterBuff(eBuff_Life);
    buff->UnRegisterBuff(eBuff_Life);

    CHECK(buff->isBuffActive(eBuff_Life));
    CHECK(buff->isBuff(eBuff_Life) == false);
}
