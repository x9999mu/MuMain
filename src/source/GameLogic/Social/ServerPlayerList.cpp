#include "stdafx.h"

#include "GameLogic/Social/ServerPlayerList.h"

namespace
{
    std::vector<GameLogic::Social::ServerPlayerInfo> s_players;
    unsigned char s_expectedChunks = 0;
    unsigned char s_receivedChunks = 0;
    unsigned int s_revision = 0;
}

namespace GameLogic::Social
{
    void ClearServerPlayerList()
    {
        if (!s_players.empty())
        {
            s_players.clear();
            s_revision++;
        }

        s_expectedChunks = 0;
        s_receivedChunks = 0;
    }

    void ApplyServerPlayerListChunk(unsigned char chunkIndex, unsigned char totalChunks, const ServerPlayerInfo* players, int count)
    {
        if (chunkIndex == 0 || s_expectedChunks != totalChunks)
        {
            s_players.clear();
            s_players.reserve(static_cast<std::size_t>(totalChunks) * MaxServerPlayersPerChunk);
            s_expectedChunks = totalChunks;
            s_receivedChunks = 0;
            s_revision++;
        }

        if (chunkIndex != s_receivedChunks)
        {
            // A chunk got lost; drop it and wait for the next complete list.
            return;
        }

        s_receivedChunks = static_cast<unsigned char>(chunkIndex + 1);
        if (players != nullptr && count > 0)
        {
            s_players.insert(s_players.end(), players, players + count);
            s_revision++;
        }
    }

    int GetServerPlayerCount()
    {
        return static_cast<int>(s_players.size());
    }

    const ServerPlayerInfo& GetServerPlayer(int index)
    {
        return s_players[static_cast<std::size_t>(index)];
    }

    unsigned int GetServerPlayerListRevision()
    {
        return s_revision;
    }
}
