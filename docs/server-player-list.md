# Server player list

The server player list shows every character which is currently online on the
same game server, together with its level, class and the map it is on. Unlike
the party window it does not require a party - it is meant to answer "who is
online right now, and where?".

## Opening and closing it

Press **O** to open and close the window. **Esc** closes it as well.

The window requests the list from the server when it is opened and then
refreshes it every five seconds while it stays open, so level ups and map
changes of other players show up without reopening it.

## What is shown

| Column | Meaning |
|---|---|
| Name | The character name of the player |
| Level | The current level of the character |
| Class | The character class |
| Map | The map the character is currently on |

The own character is highlighted in green so it can be found quickly in a long
list. The entries are sorted by map and then by name.

## What is not shown

Characters which are hidden from other players are not listed:

* Game masters which used the hide command
* Duel spectators
* Offline sessions, e.g. offline leveling or personal stores

## Requirements

The list is provided by the game server (packet `0xF3`/`0x60`). A server which
does not implement it simply never sends an answer, so the window stays empty.
Players only see the characters of the game server they are connected to; other
game servers of the same installation are not included.
