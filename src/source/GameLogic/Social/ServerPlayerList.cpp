#include "stdafx.h"

#include "GameLogic/Social/ServerPlayerList.h"

namespace
{
    std::vector<GameLogic::Social::ServerPlayerInfo> s_players;
    unsigned char s_expectedChunks = 0;
    unsigned char s_receivedChunks = 0;
}

namespace GameLogic::Social
{
    void ClearServerPlayerList()
    {
        s_players.clear();
        s_expectedChunks = 0;
        s_receivedChunks = 0;
    }

    void ApplyServerPlayerListChunk(unsigned char chunkIndex, unsigned char totalChunks, std::vector<ServerPlayerInfo> players)
    {
        if (chunkIndex == 0 || s_expectedChunks != totalChunks)
        {
            s_players.clear();
            s_expectedChunks = totalChunks;
            s_receivedChunks = 0;
        }

        if (chunkIndex != s_receivedChunks)
        {
            // A chunk got lost; drop it and wait for the next complete list.
            return;
        }

        s_receivedChunks = static_cast<unsigned char>(chunkIndex + 1);
        s_players.insert(s_players.end(), players.begin(), players.end());
    }

    const std::vector<ServerPlayerInfo>& GetServerPlayerList()
    {
        return s_players;
    }
}
