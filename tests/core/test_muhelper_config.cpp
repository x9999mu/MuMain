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

    PRECEIVE_MUHELPER_DATA packet{};
    MUHelper::ConfigDataSerDe::Serialize(source, packet);

    CHECK(static_cast<int>(packet.bUseSelfDefense) == 1);
    CHECK(static_cast<int>(packet.bAutoAcceptFriend) == 0);
    CHECK(static_cast<int>(packet.bAutoAcceptGuild) == 1);
    CHECK(static_cast<int>(packet.bFallbackBasicAttack) == 0);
    CHECK(static_cast<int>(packet.bRandomMoveWhenIdle) == 1);

    MUHelper::ConfigData restored;
    MUHelper::ConfigDataSerDe::Deserialize(packet, restored);

    CHECK(restored.bUseSelfDefense);
    CHECK_FALSE(restored.bAutoAcceptFriend);
    CHECK(restored.bAutoAcceptGuild);
    CHECK_FALSE(restored.bFallbackBasicAttack);
    CHECK(restored.bRandomMoveWhenIdle);

    source.bRandomMoveWhenIdle = false;
    MUHelper::ConfigDataSerDe::Serialize(source, packet);
    CHECK(static_cast<int>(packet.bRandomMoveWhenIdle) == 0);

    MUHelper::ConfigDataSerDe::Deserialize(packet, restored);
    CHECK_FALSE(restored.bRandomMoveWhenIdle);
}
