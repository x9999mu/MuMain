#include "doctest.h"

#include "MUHelper/MuHelperData.h"

TEST_CASE("MU Helper client-local options survive configuration round trips [core][muhelper]")
{
    MUHelper::ConfigData source;
    source.bUseSelfDefense = true;
    source.bAutoAcceptFriend = false;
    source.bAutoAcceptGuild = true;
    source.bFallbackBasicAttack = false;
    source.bRandomMoveWhenIdle = true;
    source.iAttackDelayMs = 500;

    PRECEIVE_MUHELPER_DATA packet{};
    MUHelper::ConfigDataSerDe::Serialize(source, packet);

    CHECK(static_cast<int>(packet.bUseSelfDefense) == 1);
    CHECK(static_cast<int>(packet.bAutoAcceptFriend) == 0);
    CHECK(static_cast<int>(packet.bAutoAcceptGuild) == 1);
    CHECK(static_cast<int>(packet.bFallbackBasicAttack) == 0);
    CHECK(static_cast<int>(packet.bRandomMoveWhenIdle) == 1);
    CHECK(packet.AttackDelayMs == 500);

    MUHelper::ConfigData restored;
    MUHelper::ConfigDataSerDe::Deserialize(packet, restored);

    CHECK(restored.bUseSelfDefense);
    CHECK_FALSE(restored.bAutoAcceptFriend);
    CHECK(restored.bAutoAcceptGuild);
    CHECK_FALSE(restored.bFallbackBasicAttack);
    CHECK(restored.bRandomMoveWhenIdle);
    CHECK(restored.iAttackDelayMs == 500);

    source.bRandomMoveWhenIdle = false;
    MUHelper::ConfigDataSerDe::Serialize(source, packet);
    CHECK(static_cast<int>(packet.bRandomMoveWhenIdle) == 0);

    MUHelper::ConfigDataSerDe::Deserialize(packet, restored);
    CHECK_FALSE(restored.bRandomMoveWhenIdle);
    source.iAttackDelayMs = 1;
    MUHelper::ConfigDataSerDe::Serialize(source, packet);
    CHECK(packet.AttackDelayMs == 1);

    MUHelper::ConfigDataSerDe::Deserialize(packet, restored);
    CHECK(restored.iAttackDelayMs == 1);

    packet.AttackDelayMs = 0;
    MUHelper::ConfigDataSerDe::Deserialize(packet, restored);
    CHECK(restored.iAttackDelayMs == MUHelper::DEFAULT_ATTACK_DELAY_MS);
}
