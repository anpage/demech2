#include "unk1000aa90.h"

#include "decomp.h"
#include "types.h"
#include "unk10003660.h"
#include "unk10006b20.h"

#include <dplay.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(NetPlayer, 0x24)

// The players in the session, kept sorted by DirectPlay ID. Guarded by g_unk0x1001ca78.
// GLOBAL: NETMECHW 0x1001efd0
NetPlayer g_players[8];

// The number of slots in use in g_players.
// GLOBAL: NETMECHW 0x10023590
MechS32 g_playerCount = 0;

// Adds the player p_id to the table, in ID order: the players after it move up a slot, and
// their mech copies (MEK\<xxx><slot>PLR.MEK, CopyPlayerMech) are renamed after their new slots.
// The new player joins team 1 if team 0 has two more players, team 0 otherwise. Returns
// whether the table had room.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000aa90
MechS32 FUN_1000aa90(MechChar* p_name, DPID p_id)
{
	MechS32 i;
	MechS32 index;
	MechChar path[276];
	HANDLE find;
	WIN32_FIND_DATA findData;
	MechChar newPath[276];

	index = 0;
	if (FUN_1000b0b3() == 8) {
		return FALSE;
	}

	EnterCriticalSection(&g_unk0x1001ca78);
	while (!(g_players[index].m_flags & NetPlayer::c_flagFree) && index < 8 && g_players[index].m_id < p_id) {
		index++;
	}

	if (index == 8) {
		LeaveCriticalSection(&g_unk0x1001ca78);
		return FALSE;
	}

	for (i = FUN_1000b0b3(); i > index; i--) {
		g_players[i] = g_players[i - 1];
		sprintf(path, "MEK\\???%02dPLR.MEK", i - 1);
		find = FindFirstFile(path, &findData);
		sprintf(path, "MEK\\%s", findData.cFileName);
		sprintf(newPath, "MEK\\%3.3s%02dPLR.MEK", findData.cFileName, i);
		rename(path, newPath);
		FindClose(find);
	}

	strcpy(g_players[index].m_name, p_name);
	g_players[index].m_id = p_id;
	g_players[index].m_flags = 0;
	g_players[index].m_unk0x20 = 0;
	g_players[index].m_team = 0;

	if (FUN_1000b295(0) - 1 > FUN_1000b295(1)) {
		g_players[index].m_team = 1;
	}
	else {
		g_players[index].m_team = 0;
	}

	g_playerCount++;
	LeaveCriticalSection(&g_unk0x1001ca78);
	return TRUE;
}

// Removes the player p_id from the table: deletes its mech copy, and the players after it move
// down a slot, their mech copies renamed after their new slots. Returns whether the player was
// in the table.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000acc9
MechS32 FUN_1000acc9(DPID p_id)
{
	MechS32 index;
	MechChar path[276];
	HANDLE find;
	WIN32_FIND_DATA findData;
	MechChar newPath[276];

	index = 0;
	EnterCriticalSection(&g_unk0x1001ca78);
	while (index < 8) {
		if (!(g_players[index].m_flags & NetPlayer::c_flagFree) && g_players[index].m_id == p_id) {
			break;
		}

		index++;
	}

	if (index == 8) {
		LeaveCriticalSection(&g_unk0x1001ca78);
		return FALSE;
	}

	sprintf(path, "MEK\\???%02dPLR.MEK", index);
	find = FindFirstFile(path, &findData);
	sprintf(path, "MEK\\%s", findData.cFileName);
	remove(path);
	FindClose(find);

	while (!(g_players[index + 1].m_flags & NetPlayer::c_flagFree) && index < 7) {
		g_players[index] = g_players[index + 1];
		sprintf(path, "MEK\\???%02dPLR.MEK", index + 1);
		find = FindFirstFile(path, &findData);
		sprintf(path, "MEK\\%s", findData.cFileName);
		sprintf(newPath, "MEK\\%3.3s%02dPLR.MEK", findData.cFileName, index);
		rename(path, newPath);
		FindClose(find);
		index++;
	}

	strcpy(g_players[index].m_name, "");
	g_players[index].m_flags = NetPlayer::c_flagFree;
	g_players[index].m_unk0x20 = 0;
	g_playerCount--;
	LeaveCriticalSection(&g_unk0x1001ca78);
	return TRUE;
}

// Copies the player p_id's entry to p_player; returns whether the player is in the table.
// FUNCTION: NETMECHW 0x1000aeff
MechS32 FUN_1000aeff(DPID p_id, NetPlayer* p_player)
{
	MechS32 index;

	index = 0;
	EnterCriticalSection(&g_unk0x1001ca78);
	while (index < 8) {
		if (!(g_players[index].m_flags & NetPlayer::c_flagFree) && g_players[index].m_id == p_id) {
			break;
		}

		index++;
	}

	if (index == 8) {
		LeaveCriticalSection(&g_unk0x1001ca78);
		return FALSE;
	}

	*p_player = g_players[index];
	LeaveCriticalSection(&g_unk0x1001ca78);
	return TRUE;
}

// Copies the entry of slot p_index to p_player; returns whether the slot is in use.
// FUNCTION: NETMECHW 0x1000afa8
MechS32 FUN_1000afa8(MechS32 p_index, NetPlayer* p_player)
{
	MechS32 index;

	// A leftover of the search FUN_1000aeff makes: the index is never anything but 0.
	index = 0;
	if (index >= 8) {
		return FALSE;
	}

	EnterCriticalSection(&g_unk0x1001ca78);
	if (g_players[p_index].m_flags & NetPlayer::c_flagFree) {
		LeaveCriticalSection(&g_unk0x1001ca78);
		return FALSE;
	}

	*p_player = g_players[p_index];
	LeaveCriticalSection(&g_unk0x1001ca78);
	return TRUE;
}

// The slot of the player p_id in the player table, -1 if none.
// FUNCTION: NETMECHW 0x1000b02b
MechS32 FUN_1000b02b(DPID p_id)
{
	MechS32 index;

	index = 0;
	EnterCriticalSection(&g_unk0x1001ca78);
	while (index < 8) {
		if (!(g_players[index].m_flags & NetPlayer::c_flagFree) && g_players[index].m_id == p_id) {
			break;
		}

		index++;
	}

	LeaveCriticalSection(&g_unk0x1001ca78);
	if (index == 8) {
		return -1;
	}

	return index;
}

// FUNCTION: NETMECHW 0x1000b0b3
MechS32 FUN_1000b0b3()
{
	return g_playerCount;
}

// Empties the player table.
// FUNCTION: NETMECHW 0x1000b0c8
void FUN_1000b0c8()
{
	MechS32 i;

	for (i = 0; i < 8; i++) {
		strcpy(g_players[i].m_name, "");
		g_players[i].m_id = 0;
		g_players[i].m_flags = NetPlayer::c_flagFree;
		g_players[i].m_team = 0;
		g_players[i].m_unk0x20 = 0;
	}

	g_playerCount = 0;
}

// The highest player ID in the table.
// FUNCTION: NETMECHW 0x1000b178
DPID FUN_1000b178()
{
	MechS32 i;
	DPID max;

	i = 0;
	max = 0;
	EnterCriticalSection(&g_unk0x1001ca78);
	while (!(g_players[i].m_flags & NetPlayer::c_flagFree) && i < 8) {
		if (g_players[i].m_id > max) {
			max = g_players[i].m_id;
		}
		i++;
	}

	LeaveCriticalSection(&g_unk0x1001ca78);
	return max;
}

// Sets the players' teams from p_teams, one bit per slot: set for team 1.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000b1fe
void FUN_1000b1fe(MechU8 p_teams)
{
	MechS32 i;
	MechU8 bit;

	i = 0;
	bit = 1;
	EnterCriticalSection(&g_unk0x1001ca78);
	while (!(g_players[i].m_flags & NetPlayer::c_flagFree) && i < 8) {
		if (p_teams & bit) {
			g_players[i].m_team = 1;
		}
		else {
			g_players[i].m_team = 0;
		}

		i++;
		bit <<= 1;
	}

	LeaveCriticalSection(&g_unk0x1001ca78);
}

// The number of players on team p_team.
// FUNCTION: NETMECHW 0x1000b295
MechS32 FUN_1000b295(MechS32 p_team)
{
	MechS32 i;
	MechS32 count;

	i = 0;
	count = 0;
	EnterCriticalSection(&g_unk0x1001ca78);
	while (!(g_players[i].m_flags & NetPlayer::c_flagFree) && i < 8) {
		if (g_players[i].m_team == p_team) {
			count++;
		}
		i++;
	}

	LeaveCriticalSection(&g_unk0x1001ca78);
	return count;
}

// Counts another period without a message from the player in slot p_index (FUN_1000b3c5
// resets the count): -2 after 50 periods, -1 after 20, 0 otherwise and for the local player.
// FUNCTION: NETMECHW 0x1000b30e
MechS32 FUN_1000b30e(MechS32 p_index)
{
	EnterCriticalSection(&g_unk0x1001ca78);
	if (g_players[p_index].m_id == g_unk0x1001ca90.m_playerId) {
		LeaveCriticalSection(&g_unk0x1001ca78);
		return 0;
	}

	g_players[p_index].m_unk0x20++;
	if (g_players[p_index].m_unk0x20 >= 50) {
		LeaveCriticalSection(&g_unk0x1001ca78);
		return -2;
	}
	else if (g_players[p_index].m_unk0x20 >= 20) {
		LeaveCriticalSection(&g_unk0x1001ca78);
		return -1;
	}

	LeaveCriticalSection(&g_unk0x1001ca78);
	return 0;
}

// FUNCTION: NETMECHW 0x1000b3c5
void FUN_1000b3c5(MechS32 p_index)
{
	EnterCriticalSection(&g_unk0x1001ca78);
	g_players[p_index].m_unk0x20 = 0;
	LeaveCriticalSection(&g_unk0x1001ca78);
}
