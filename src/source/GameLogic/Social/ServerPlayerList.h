#pragma once

#include <string>
#include <vector>

namespace GameLogic::Social
{
    /// <summary>
    /// A player which is currently online on the same game server.
    /// </summary>
    struct ServerPlayerInfo
    {
        std::wstring Name;
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
    /// <param name="players">The players of the chunk.</param>
    void ApplyServerPlayerListChunk(unsigned char chunkIndex, unsigned char totalChunks, std::vector<ServerPlayerInfo> players);

    /// <summary>
    /// Gets the players which have been received so far.
    /// </summary>
    const std::vector<ServerPlayerInfo>& GetServerPlayerList();
}
