#pragma once

#include "Core/Globals/_define.h"

#include <cstddef>
#include <vector>

namespace GameLogic::Social
{
    /// <summary>
    /// The maximum number of players which are expected in a single chunk of the
    /// server player list. The server sends at most 16 entries per packet; the
    /// headroom avoids dropping a chunk if that constant ever grows.
    /// </summary>
    constexpr int MaxServerPlayersPerChunk = 32;

    /// <summary>
    /// A player which is currently online on the same game server.
    /// </summary>
    struct ServerPlayerInfo
    {
        wchar_t Name[MAX_USERNAME_SIZE + 1] = { 0, };
        unsigned short Level = 0;
        unsigned char ClassId = 0;
        unsigned short Map = 0;
    };

    /// <summary>
    /// Removes all received players, e.g. when the character leaves the game.
    /// </summary>
    void ClearServerPlayerList();

    /// <summary>
    /// Applies one received chunk of the server player list. The first chunk replaces
    /// the previous list, the following chunks append to it.
    /// </summary>
    /// <param name="chunkIndex">The zero-based index of the chunk.</param>
    /// <param name="totalChunks">The number of chunks which belong to the list.</param>
    /// <param name="players">The players of the chunk, or <c>nullptr</c> for an empty chunk.</param>
    /// <param name="count">The number of players of the chunk.</param>
    void ApplyServerPlayerListChunk(unsigned char chunkIndex, unsigned char totalChunks, const ServerPlayerInfo* players, int count);

    /// <summary>
    /// Gets the number of players which have been received so far.
    /// </summary>
    int GetServerPlayerCount();

    /// <summary>
    /// Gets a received player.
    /// </summary>
    /// <param name="index">The zero-based index of the player.</param>
    const ServerPlayerInfo& GetServerPlayer(int index);

    /// <summary>
    /// Gets a counter which is incremented whenever the list changed. Callers can
    /// use it to skip recalculating derived state while nothing changed.
    /// </summary>
    unsigned int GetServerPlayerListRevision();
}
