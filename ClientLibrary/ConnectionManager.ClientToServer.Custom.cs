// <copyright file="ConnectionManager.ClientToServer.Custom.cs" company="MUnique">
// Licensed under the MIT License. See LICENSE file in the project root for full license information.
// </copyright>

namespace MUnique.Client.Library;

using System;
using System.Runtime.InteropServices;
using System.Text;
using MUnique.OpenMU.Network;
using MUnique.OpenMU.Network.Packets.ClientToServer;
using MUnique.OpenMU.Network.Xor;

/// <summary>
/// Extension methods to start writing messages of this namespace on a <see cref="IConnection"/>.
/// </summary>
public unsafe partial class ConnectionManager
{
    private const byte ServerPlayerListRequestHeaderType = 0xC1;
    private const byte ServerPlayerListRequestCode = 0xF3;
    private const byte ServerPlayerListRequestSubCode = 0x60;
    private const int ServerPlayerListRequestLength = 4;

    private static readonly Xor3Encryptor Xor3Encryptor = new(0);

    /// <summary>
    /// Sends a server player list request (0xF3 / 0x60) to this connection. The packet has
    /// no payload; the server answers with the players which are online on the same game
    /// server. It is hand-written because the packet is not part of the packet definitions
    /// of the referenced package yet.
    /// </summary>
    /// <param name="handle">The handle of the connection.</param>
    [UnmanagedCallersOnly(EntryPoint = "ConnectionManager_SendServerPlayerListRequest")]
    public static void SendServerPlayerListRequest(int handle)
    {
        if (!Connections.TryGetValue(handle, out var connection))
        {
            ManagedLog.Write(ManagedLog.Level.Error, $"NET: Server player list request skipped; connection handle={handle} not found");
            return;
        }

        try
        {
            connection.CreateAndSend(pipeWriter =>
            {
                var packet = pipeWriter.GetSpan(ServerPlayerListRequestLength)[..ServerPlayerListRequestLength];
                packet[0] = ServerPlayerListRequestHeaderType;
                packet[1] = ServerPlayerListRequestLength;
                packet[2] = ServerPlayerListRequestCode;
                packet[3] = ServerPlayerListRequestSubCode;
                return ServerPlayerListRequestLength;
            });
        }
        catch (Exception ex)
        {
            ManagedLog.Write(ManagedLog.Level.Error, $"NET: Server player list request staging failed, handle={handle}: {ex}");
        }
    }

    /// <summary>
    /// Sends a <see cref="LoginLongPassword" /> to this connection.
    /// </summary>
    /// <param name="handle">The handle of the connection.</param>
    /// <param name="username">The user name, "encrypted" with Xor3.</param>
    /// <param name="password">The password, "encrypted" with Xor3.</param>
    /// <param name="tickCount">The tick count.</param>
    /// <param name="clientVersion">The client version.</param>
    /// <param name="clientSerial">The client serial.</param>
    /// <remarks>
    /// Is sent by the client when: The player tries to log into the game.
    /// Causes reaction on server side: The server is authenticating the sent login name and password. If it's correct, the state of the player is proceeding to be logged in.
    /// </remarks>
    [UnmanagedCallersOnly(EntryPoint = "ConnectionManager_SendLogin")]
    public static void SendLogin(int handle, IntPtr username, IntPtr password, uint @tickCount, byte* @clientVersion, byte* @clientSerial)
    {
        if (!Connections.TryGetValue(handle, out var connection))
        {
            ManagedLog.Write(ManagedLog.Level.Error, $"NET: Login send skipped; connection handle={handle} not found");
            return;
        }

        try
        {
            var usernameStr = NativeInterop.PtrToWideString(@username)
                ?? throw new ArgumentNullException(nameof(username));
            var passwordStr = NativeInterop.PtrToWideString(@password)
                ?? throw new ArgumentNullException(nameof(password));
            ArgumentNullException.ThrowIfNull(@clientVersion);
            ArgumentNullException.ThrowIfNull(@clientSerial);

            const int usernameLength = 10;
            const int passwordLength = 20;
            if (Encoding.UTF8.GetByteCount(usernameStr) > usernameLength
                || Encoding.UTF8.GetByteCount(passwordStr) > passwordLength)
            {
                throw new ArgumentException("Login credentials exceed packet field length.");
            }

            connection.CreateAndSend(pipeWriter =>
            {
                Span<byte> usernameBytes = stackalloc byte[usernameLength];
                Span<byte> passwordBytes = stackalloc byte[passwordLength];
                usernameBytes.Clear();
                passwordBytes.Clear();
                Encoding.UTF8.GetBytes(usernameStr, usernameBytes);
                Encoding.UTF8.GetBytes(passwordStr, passwordBytes);
                Xor3Encryptor.Encrypt(usernameBytes);
                Xor3Encryptor.Encrypt(passwordBytes);

                var length = LoginLongPasswordRef.Length;
                var packet = new LoginLongPasswordRef(pipeWriter.GetSpan(length)[..length]);
                usernameBytes.CopyTo(packet.Username);
                passwordBytes.CopyTo(packet.Password);
                packet.TickCount = @tickCount;
                new Span<byte>(@clientVersion, packet.ClientVersion.Length).CopyTo(packet.ClientVersion);
                new Span<byte>(@clientSerial, packet.ClientSerial.Length).CopyTo(packet.ClientSerial);

                return length;
            });
            ManagedLog.Write(ManagedLog.Level.Info, $"NET: Login packet staged, handle={handle}, bytes={LoginLongPasswordRef.Length}");
        }
        catch (Exception ex)
        {
            ManagedLog.Write(ManagedLog.Level.Error, $"NET: Login packet staging failed, handle={handle}: {ex}");
        }
    }
}
