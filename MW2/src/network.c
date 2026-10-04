/* The network game over DirectPlay: electing the master, which keeps the clock, and the
   messages the players exchange. Each message starts with a two-letter tag:
   DA  a player's state (NetStateMsg), sent every g_stateInterval ticks
   WE  the weapons a player fired since the last one
   CO  a collision push against a player
   GO  a slave is ready; the master's GO starts the game
   SN  the game things destroyed so far, a bit each
   SU  a player's objectives succeeded
   SS  a player left the game
   CH  a chat line */
#include "network.h"

#include "approxlen.h"
#include "carcfg.h"
#include "clock.h"
#include "collision.h"
#include "debugprint.h"
#include "decomp.h"
#include "gamekeys.h"
#include "keyboard.h"
#include "mech.h"
#include "mechcollision.h"
#include "mechdamage.h"
#include "mechreload.h"
#include "mechsection.h"
#include "muldiv.h"
#include "netio.h"
#include "netlaunchinfo.h"
#include "object.h"
#include "players.h"
#include "shots.h"
#include "simmain.h"
#include "soundfx.h"
#include "team.h"
#include "timedoverlays.h"
#include "types.h"
#include "weapons.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

#pragma pack(push, 1)

// DA: a player's state, filled from the local player every g_stateInterval ticks.
// SIZE 0x85
typedef struct NetStateMsg {
	MechChar m_tag[2];     // 0x00
	MechU8 m_player;       // 0x02
	MechS16 m_target;      // 0x03 — the player's target (PlayerTargetInfo::m_target)
	MechS16 m_killer;      // 0x05
	MechU16 m_flags;       // 0x07 — the power state in the low nibble
	MechS32 m_unk0x09;     // 0x09 — section 3's m_unk0x04
	MechS32 m_unk0x0d;     // 0x0d — section 2's
	MechS32 m_unk0x11;     // 0x11 — section 1's
	MechS32 m_unk0x15[8];  // 0x15 — each section's m_unk0x00
	MechS32 m_unk0x35[8];  // 0x35 — each section's m_unk0x08
	MechS32 m_clock;       // 0x55
	MechS32 m_x;           // 0x59
	MechS32 m_y;           // 0x5d
	MechS32 m_z;           // 0x61
	MechS32 m_unk0x65;     // 0x65
	MechS32 m_heading;     // 0x69
	MechS32 m_unk0x6d;     // 0x6d
	MechS32 m_mechUnk0x2c; // 0x71
	MechS32 m_mechUnk0x4c; // 0x75
	MechS32 m_mechUnk0x38; // 0x79
	MechS32 m_mechUnk0x08; // 0x7d
	MechS32 m_mechUnk0x18; // 0x81
} NetStateMsg;

// WE: the weapons a player fired since the last one, a bit each.
// SIZE 0x05
typedef struct NetWeaponsMsg {
	MechChar m_tag[2]; // 0x00
	MechU8 m_player;   // 0x02
	MechU16 m_weapons; // 0x03
} NetWeaponsMsg;

// CO: a collision push, sent to the player pushed.
// SIZE 0x1a
typedef struct NetCollisionMsg {
	MechChar m_tag[2]; // 0x00
	MechS32 m_normalX; // 0x02
	MechS32 m_normalY; // 0x06
	MechS32 m_normalZ; // 0x0a
	MechS32 m_unk0x0e; // 0x0e — the pusher's Mech::m_unk0x100
	MechS32 m_unk0x12; // 0x12
	MechS32 m_unk0x16; // 0x16
} NetCollisionMsg;

// CH: a chat line.
// SIZE 0x52
typedef struct NetChatMsg {
	MechChar m_tag[2];     // 0x00
	MechChar m_text[0x50]; // 0x02
} NetChatMsg;

#pragma pack(pop)

void FUN_1000ebad(void);
void FUN_1000f171(NetChatMsg* p_msg, MechS32 p_slot);
void FUN_1000f1ee(NetStateMsg* p_msg, MechS32 p_slot);
void FUN_1000f989(void);
void FUN_1000fa58(NetWeaponsMsg* p_msg, MechS32 p_slot);
void FUN_1000fb8f(NetCollisionMsg* p_msg, MechS32 p_slot);
void FUN_1000fca8(void);
void FUN_1000fcf5(MechChar* p_msg, MechS32 p_slot);
void FUN_1000fda6(void);
void FUN_1000fe61(MechU8* p_msg, MechS32 p_slot);
void FUN_1000ff70(MechChar* p_msg, MechS32 p_slot);
BOOL PASCAL FUN_1000ffe6(DPID p_id, LPSTR p_friendlyName, LPSTR p_formalName, DWORD p_flags, LPVOID p_context);
BOOL PASCAL FUN_1001001a(LPDPSESSIONDESC p_desc, LPVOID p_context, LPDWORD p_timeout, DWORD p_flags);
BOOL PASCAL FUN_10010098(LPGUID p_guid, LPSTR p_name, DWORD p_major, DWORD p_minor, LPVOID p_context);
MechS32 FUN_100100cc(LPDPSESSIONDESC p_desc);

// GLOBAL: MW2 0x100a1758
MechS32 g_stateInterval = 36;

// The number of players of a network game, or 0.
// GLOBAL: MW2 0x100a175c
MechS32 g_isNetworkGame = 0;

// GLOBAL: MW2 0x100a1760
MechChar* g_sessionName = NULL;

// Set when this tick's messages brought the master's clock.
// GLOBAL: MW2 0x100a1764
MechS16 g_clockSynced = 0;

// Set when the shell launched the game with its own DirectPlay session.
// GLOBAL: MW2 0x100a1768
MechS16 g_launchedByShell = 0;

// GLOBAL: MW2 0x100a1770
MechS32 g_stateCount = 0;

// GLOBAL: MW2 0x100a1774
MechS32 g_messagesReceived = 0;

// GLOBAL: MW2 0x100a1778
MechS32 g_unrecognizedMessages = 0;

// GLOBAL: MW2 0x100a177c
MechS32 g_successSent = 0;

// GLOBAL: MW2 0x100a1780
MechS16 g_gameStarted = 0;

// GLOBAL: MW2 0x100a1788
NetLaunchInfo* g_netLaunch = NULL;

// The send buffers. The weapons message shares the state message's.

// GLOBAL: MW2 0x100a178c
NetStateMsg* g_stateMsg = NULL;

// GLOBAL: MW2 0x100a1790
NetWeaponsMsg* g_weaponsMsg = NULL;

// SN: the game things destroyed so far. After the tag, the sender's g_stateCount (an older
// message is ignored) and a bit for each thing, the first in the high bit of the first byte.
// GLOBAL: MW2 0x100a1794
MechU8* g_thingsMsg = NULL;

// GLOBAL: MW2 0x100a1798
NetChatMsg* g_chatMsg = NULL;

// 1 while the network runs, 2 once it has stopped.
// GLOBAL: MW2 0x100a179c
MechS32 g_netState = 0;

// 0 alone, 1 the master (it keeps the clock), 2 a slave.
// GLOBAL: MW2 0x100a17a0
MechS32 g_netRole = 0;

// GLOBAL: MW2 0x100a17a8
GUID g_sessionGuid = {0x5a237e00, 0xea03, 0x11ce, {0x97, 0xdc, 0x00, 0x20, 0xaf, 0x24, 0xc6, 0x4a}};

// GLOBAL: MW2 0x100a17b8
LPDIRECTPLAY g_directPlay = NULL;

// GLOBAL: MW2 0x100a17bc
DPID g_localDpid = 99;

// The lowest id among the players: the master's.
// GLOBAL: MW2 0x100a17c0
DPID g_masterDpid = 0;

// GLOBAL: MW2 0x100a17c4
LPGUID g_serviceProvider = NULL;

// GLOBAL: MW2 0x100a17c8
MechS32 g_nextStateTime = 0;

// GLOBAL: MW2 0x101770a0
MechChar* g_netRecvBuffer;

// GLOBAL: MW2 0x101770cc
void* g_unk0x101770cc;

// GLOBAL: MW2 0x101770d0
MechU32 g_stateMsgSize;

// When each player was last heard from.
// GLOBAL: MW2 0x101770e0
MechS32 g_lastHeard[8];

// The clock of each player's last state message.
// GLOBAL: MW2 0x10177100
MechS32 g_lastStateClock[8];

// GLOBAL: MW2 0x10177120
MechS8 g_playerReady[8];

// GLOBAL: MW2 0x10177128
MechS32 g_playersFound;

// GLOBAL: MW2 0x1017712c
MechU32 g_thingsMsgSize;

// GLOBAL: MW2 0x10177130
MechS8 g_playerDestroyed[8];

// The count of each player's last things message.
// GLOBAL: MW2 0x10177140
MechS32 g_lastThingsCount[8];

// The only diff is the order the m_playerIds index loads its base and index in.
// FUNCTION: MW2 0x1000e410
MechS32 GetPlayerSlotFromNetId(DPID p_id)
{
	MechS32 i;

	if (g_netLaunch) {
		for (i = 0; i < 8; i++) {
			if (g_netLaunch->m_playerIds[i] == p_id) {
				return i;
			}
		}

		return 0;
	}
	else {
		return p_id - 1;
	}
}

// FUNCTION: MW2 0x1000e47d
void FUN_1000e47d(void)
{
}

// FUNCTION: MW2 0x1000e488
void FUN_1000e488(void)
{
}

// FUNCTION: MW2 0x1000e493
void FirstNetwork(NetLaunchInfo* p_netLaunch)
{
	MechS32 i;

	if (!g_isNetworkGame) {
		g_goLaunch |= 1;
	}
	else {
		g_netLaunch = p_netLaunch;
		for (i = 0; i < 8; i++) {
			g_lastHeard[i] = 0;
			g_playerDestroyed[i] = 0;
			g_playerReady[i] = 0;
		}

		g_carCfg.m_unk0x50 = g_playerCount;
		g_stateMsgSize = sizeof(NetStateMsg);
		g_stateMsg = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, g_stateMsgSize);
		memset(g_stateMsg, 0, g_stateMsgSize);
		g_weaponsMsg = (NetWeaponsMsg*) g_stateMsg;
		g_thingsMsgSize = 0x27;
		g_thingsMsg = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, g_thingsMsgSize);
		memset(g_thingsMsg, 0, g_thingsMsgSize);
		g_chatMsg = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(NetChatMsg));
		memset(g_chatMsg, 0, sizeof(NetChatMsg));
		g_unk0x101770cc = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, 0x100);
		g_netRecvBuffer = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, 0x100);
		for (i = 0; i < 8; i++) {
			g_lastStateClock[i] = 0;
			g_lastThingsCount[i] = 0;
		}

		if (StartExternalIO(p_netLaunch)) {
			if (g_localDpid > 0) {
				g_localPlayerId = GetPlayerSlotFromNetId(g_localDpid);
			}
		}
		else {
			g_shouldQuit = 1;
			g_quitStage += 666;
		}

		return;
	}
}

// FUNCTION: MW2 0x1000e677
MechS32 FirstExternalCtrl(void)
{
	return 1;
}

// Matches except for a stack-slot permutation of from, player, i, tag and text, and the
// operand order of the comparisons of slot and i with g_localPlayerId and of the state timer.
// FUNCTION: MW2 0x1000e68c
MechS32 UpdateNetwork(void)
{
	MechChar text[80];
	Player* player;
	DPID from;
	MechS32 i;
	MechChar tag[3];
	MechS32 slot;

	if (g_netState != 1) {
		return 1;
	}

	if (g_shouldQuit) {
		return 1;
	}

	g_clockSynced = 0;
	g_unk0x100ba54c = FUN_1007d0fb();
	while ((from = NetReceive()) != -1) {
		g_messagesReceived++;
		slot = GetPlayerSlotFromNetId(from);
		if (slot == g_localPlayerId || slot < 0) {
			continue;
		}

		player = g_players[slot];
		if (!player) {
			continue;
		}

		tag[0] = g_netRecvBuffer[0];
		tag[1] = g_netRecvBuffer[1];
		tag[2] = 0;
		g_lastHeard[slot] = g_unk0x100ba54c;
		if (player->m_flags & 0x4000) {
			ShowObjTree(player->m_obj);
			EnableObjTreeCollision(player->m_obj);
			UpdateObj(player->m_obj);
			player->m_flags &= ~0x4800;
			ElectMaster();
		}

		if (_strcmpi(tag, "DA") == 0) {
			FUN_1000f1ee((NetStateMsg*) g_netRecvBuffer, slot);
		}
		else if (_strcmpi(tag, "WE") == 0) {
			FUN_1000fa58((NetWeaponsMsg*) g_netRecvBuffer, slot);
		}
		else if (_strcmpi(tag, "CO") == 0) {
			FUN_1000fb8f((NetCollisionMsg*) g_netRecvBuffer, slot);
		}
		else if (_strcmpi(tag, "GO") == 0) {
			FUN_1000fcf5(g_netRecvBuffer, slot);
		}
		else if (_strcmpi(tag, "SN") == 0) {
			FUN_1000fe61((MechU8*) g_netRecvBuffer, slot);
		}
		else if (_strcmpi(tag, "SU") == 0) {
			FUN_1000ff70(g_netRecvBuffer, slot);
		}
		else if (_strcmpi(tag, "SS") == 0) {
			if (!g_unk0x100aa2bc) {
				FUN_1000832b(-2, player->m_mech);
			}

			ElectMaster();
			sprintf(text, "'%s' exited the game.", player->m_name);
			ShowInGameMessage(text, 1, 0x712, 0x50);
			break;
		}
		else if (_strcmpi(tag, "CH") == 0) {
			FUN_1000f171((NetChatMsg*) g_netRecvBuffer, slot);
		}
		else {
			g_unrecognizedMessages++;
		}
	}

	if (g_nextStateTime <= g_unk0x100ba54c) {
		g_nextStateTime = g_stateInterval + g_unk0x100ba54c;
		for (i = 0; i < 8; i++) {
			if (i != g_localPlayerId && g_players[i] != NULL && !(g_players[i]->m_flags & 0x4000) &&
				g_unk0x100ba54c - g_lastHeard[i] > 0x38e) {
				HideObjTree(g_players[i]->m_obj);
				DisableObjTreeCollision(g_players[i]->m_obj);
				g_players[i]->m_flags |= 0x4800;
				ElectMaster();
			}
		}

		if (g_gameStarted) {
			FUN_1000ebad();
			FUN_1000f989();
			g_stateCount++;
			if (g_stateCount & 1) {
				FUN_1000fda6();
			}
		}
		else if (g_netRole == 2) {
			FUN_1000fca8();
		}
		else {
			g_playerReady[g_localPlayerId] = 1;
		}
	}

	if (!g_clockSynced && g_netRole == 2) {
		g_currentClock += FUN_1007d07b();
	}

	if (g_netRole == 2) {
		FUN_1007d099();
	}

	return 1;
}

// FUNCTION: MW2 0x1000eb05
void ShutdownNetwork(void)
{
	StopExternalIO();
	if (g_netState) {
		if (g_stateMsg) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_stateMsg);
		}

		if (g_thingsMsg) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_thingsMsg);
		}

		if (g_chatMsg) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_chatMsg);
		}

		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_unk0x101770cc);
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_netRecvBuffer);
	}
}

// Sends the local player's state.
// The only diff is a stack-slot permutation of player, thing, mech, i, section and steering.
// FUNCTION: MW2 0x1000ebad
void FUN_1000ebad(void)
{
	Player* player;
	MechU32 thing;
	Mech* mech;
	MechS32 i;
	MechSection* section;
	PlayerSteering* steering;

	thing = 0;
	memcpy(g_stateMsg->m_tag, "DA", 2);
	player = g_players[g_localPlayerId];
	mech = player->m_mech;
	g_stateMsg->m_flags = 0;
	g_stateMsg->m_player = player->m_index;
	g_stateMsg->m_target = player->m_targetInfo.m_target;
	if ((player->m_targetInfo.m_target & 0xf00) == 0x400) {
		thing = player->m_targetInfo.m_target & 0xff;
		if ((g_gameThings[thing].m_unk0x00 & 0x20) && (g_gameThings[thing].m_unk0x02 & (1 << player->m_team))) {
			g_stateMsg->m_flags |= 0x400;
		}
	}

	g_stateMsg->m_killer = player->m_killer;
	g_stateMsg->m_x = player->m_position.m_x;
	g_stateMsg->m_y = player->m_position.m_y;
	g_stateMsg->m_z = player->m_position.m_z;
	g_stateMsg->m_unk0x65 = player->m_pitch;
	g_stateMsg->m_heading = player->m_heading;
	g_stateMsg->m_unk0x6d = player->m_roll;
	g_stateMsg->m_mechUnk0x2c = mech->m_speed.m_value;
	g_stateMsg->m_mechUnk0x4c = mech->m_throttle.m_value;
	g_stateMsg->m_mechUnk0x38 = mech->m_turnRate.m_target;
	g_stateMsg->m_mechUnk0x08 = mech->m_torsoTwist.m_target;
	g_stateMsg->m_mechUnk0x18 = mech->m_torsoPitch.m_target;
	g_stateMsg->m_flags |= mech->m_powerState & 0xf;
	if (mech->m_flags & 0x80) {
		g_stateMsg->m_flags |= 0x800;
	}

	if (mech->m_player->m_steering->m_reverse) {
		g_stateMsg->m_flags |= 0x10;
	}

	if (mech->m_jumpFuel > 0 && mech->m_powerState == 2) {
		steering = mech->m_player->m_steering;
		if (steering->m_jumpJetEnabled) {
			g_stateMsg->m_flags |= 0x20;
		}

		if (steering->m_jumpJetFireLeft) {
			g_stateMsg->m_flags |= 0x40;
		}

		if (steering->m_jumpJetFireRight) {
			g_stateMsg->m_flags |= 0x80;
		}

		if (steering->m_jumpJetFireForward) {
			g_stateMsg->m_flags |= 0x100;
		}

		if (steering->m_jumpJetFireBackward) {
			g_stateMsg->m_flags |= 0x200;
		}
	}

	g_stateMsg->m_clock = g_currentClock;
	for (i = 0; i < 8; i++) {
		section = &mech->m_sections[i];
		switch (i + 1) {
		case 4:
			g_stateMsg->m_unk0x09 = section->m_armor[1];
			break;
		case 3:
			g_stateMsg->m_unk0x0d = section->m_armor[1];
			break;
		case 2:
			g_stateMsg->m_unk0x11 = section->m_armor[1];
			break;
		default:
			break;
		}

		g_stateMsg->m_unk0x15[i] = section->m_armor[0];
		g_stateMsg->m_unk0x35[i] = section->m_unk0x08;
	}

	NetSend((MechU8*) g_stateMsg, g_stateMsgSize);
}

// Sends a chat line to player p_to, or to the players of side 1 (-3), those of side 0 (-2) or
// everybody (-1). Fails on a line of 80 characters or more.
// Matches except for a stack-slot permutation of i and size, the operand order of the
// comparison of i with g_localPlayerId and the order the m_playerIds[p_to] index loads in.
// FUNCTION: MW2 0x1000efa4
MechS32 FUN_1000efa4(MechS32 p_to, MechChar* p_text)
{
	MechS32 i;
	MechU32 size;

	size = strlen(p_text);
	if (size >= 80) {
		return 0;
	}

	size += 3;
	memcpy(g_chatMsg->m_tag, "CH", 2);
	strcpy(g_chatMsg->m_text, p_text);
	switch (p_to) {
	case -1:
		NetSend((MechU8*) g_chatMsg, size);
		break;
	case -2:
		for (i = 0; i < 8; i++) {
			if (g_players[i] != NULL && !GetPlayerSide(i) && i != g_localPlayerId) {
				NetSendTo(g_netLaunch->m_playerIds[i], g_chatMsg, size);
			}
		}
		break;
	case -3:
		for (i = 0; i < 8; i++) {
			if (g_players[i] != NULL && GetPlayerSide(i) == 1) {
				NetSendTo(g_netLaunch->m_playerIds[i], g_chatMsg, size);
			}
		}
		break;
	default:
		NetSendTo(g_netLaunch->m_playerIds[p_to], g_chatMsg, size);
		break;
	}

	return 1;
}

// FUNCTION: MW2 0x1000f171
void FUN_1000f171(NetChatMsg* p_msg, MechS32 p_slot)
{
	MechChar text[100];

	if (g_players[p_slot] == NULL) {
		return;
	}

	p_msg->m_text[0x50] = 0; // one past the line: the buffer is the 0x100-byte receive buffer
	sprintf(text, "%s: %s", g_players[p_slot]->m_name, p_msg->m_text);
	ShowInGameMessage(text, 1, 0x43e, 0x32);
	FUN_1007eb23(0xdc, 100, 0x40, 5, 0x32);
}

// Applies a player's state message.
// The only diff is a stack-slot permutation of its locals.
// FUNCTION: MW2 0x1000f1ee
void FUN_1000f1ee(NetStateMsg* p_msg, MechS32 p_slot)
{
	MechChar text2[40];
	MechChar text[40];
	MechSection* section;
	MechS32 i;
	Mech* mech;
	MechU32 levels;
	MechU32 thing;
	Player* player;
	MechS32 damage2;
	MechS32 damage1;
	NetStateMsg* msg;

	damage1 = 0;
	damage2 = 0;
	thing = 0;
	msg = p_msg;
	if (msg->m_player == g_localPlayerId) {
		return;
	}

	if (g_lastStateClock[p_slot] >= msg->m_clock) {
		return;
	}

	g_lastStateClock[p_slot] = msg->m_clock;
	player = g_players[p_slot];
	mech = player->m_mech;
	player->m_targetInfo.m_target = msg->m_target;
	if ((msg->m_target & 0xf00) == 0x400 && (msg->m_flags & 0x400)) {
		thing = player->m_targetInfo.m_target & 0xff;
		g_gameThings[thing].m_unk0x00 |= 0x20;
		g_gameThings[thing].m_unk0x02 |= 1 << player->m_team;
	}

	player->m_killer = msg->m_killer;
	player->m_flags |= 1;
	if (GetPlayerSlotFromNetId(g_masterDpid) == player->m_index) {
		g_clockSynced = 1;
		if (g_netRole == 2) {
			g_currentClock = msg->m_clock;
		}
	}

	player->m_position.m_x = msg->m_x;
	player->m_position.m_y = msg->m_y;
	player->m_position.m_z = msg->m_z;
	player->m_pitch = msg->m_unk0x65;
	player->m_heading = msg->m_heading;
	player->m_roll = msg->m_unk0x6d;
	SetObjPosition(player->m_obj, player->m_position.m_x, player->m_position.m_y, player->m_position.m_z);
	SetObjRotation(player->m_obj, player->m_pitch, player->m_heading, player->m_roll, 0);
	UpdateObj(player->m_obj);
	mech->m_speed.m_value = msg->m_mechUnk0x2c;
	mech->m_speed.m_target = msg->m_mechUnk0x2c;
	mech->m_throttle.m_value = msg->m_mechUnk0x4c;
	mech->m_throttle.m_target = msg->m_mechUnk0x4c;
	mech->m_turnRate.m_target = msg->m_mechUnk0x38;
	mech->m_torsoTwist.m_target = msg->m_mechUnk0x08;
	mech->m_torsoPitch.m_target = msg->m_mechUnk0x18;
	mech->m_powerState = msg->m_flags & 0xf;
	if (msg->m_flags & 0x800) {
		mech->m_flags |= 0x80;
	}
	else {
		mech->m_flags &= ~0x80;
	}

	if (msg->m_flags & 0x10) {
		mech->m_player->m_steering->m_reverse = 1;
	}
	else {
		mech->m_player->m_steering->m_reverse = 0;
	}

	if (msg->m_flags & 0x20) {
		mech->m_player->m_steering->m_jumpJetEnabled = 1;
	}
	else {
		mech->m_player->m_steering->m_jumpJetEnabled = 0;
	}

	if (msg->m_flags & 0x40) {
		mech->m_player->m_steering->m_jumpJetFireLeft = 1;
	}
	else {
		mech->m_player->m_steering->m_jumpJetFireLeft = 0;
	}

	if (msg->m_flags & 0x80) {
		mech->m_player->m_steering->m_jumpJetFireRight = 1;
	}
	else {
		mech->m_player->m_steering->m_jumpJetFireRight = 0;
	}

	if (msg->m_flags & 0x100) {
		mech->m_player->m_steering->m_jumpJetFireForward = 1;
	}
	else {
		mech->m_player->m_steering->m_jumpJetFireForward = 0;
	}

	if (msg->m_flags & 0x200) {
		mech->m_player->m_steering->m_jumpJetFireBackward = 1;
	}
	else {
		mech->m_player->m_steering->m_jumpJetFireBackward = 0;
	}

	for (i = 0; i < 8; i++) {
		damage1 = damage2 = 0;
		section = &mech->m_sections[i];
		section->m_armor[0] = msg->m_unk0x15[i];
		section->m_unk0x08 = msg->m_unk0x35[i];
		switch (i + 1) {
		case 4:
			section->m_armor[1] = msg->m_unk0x09;
			levels = (section->m_unk0x26 & 0xf0) >> 4;
			if (levels) {
				damage2 =
					15 - ((section->m_unk0x08 + section->m_armor[1] / g_unk0x100a1598) * 3) / (MechS32) (levels << 16);
			}
			break;
		case 3:
			section->m_armor[1] = msg->m_unk0x0d;
			levels = (section->m_unk0x26 & 0xf0) >> 4;
			if (levels) {
				damage2 =
					15 - ((section->m_unk0x08 + section->m_armor[1] / g_unk0x100a1598) * 3) / (MechS32) (levels << 16);
			}
			break;
		case 2:
			section->m_armor[1] = msg->m_unk0x11;
			levels = (section->m_unk0x26 & 0xf0) >> 4;
			if (levels) {
				damage2 =
					15 - ((section->m_unk0x08 + section->m_armor[1] / g_unk0x100a1598) * 3) / (MechS32) (levels << 16);
			}
			break;
		default:
			break;
		}

		levels = section->m_unk0x26 & 0xf;
		if (levels) {
			damage1 =
				15 - ((section->m_unk0x08 + section->m_armor[0] / g_unk0x100a1598) * 3) / (MechS32) (levels << 16);
		}

		RaisePartDamageLevel(mech->m_player->m_obj, damage2 > damage1 ? damage2 : damage1, i + 1);
		if (section->m_unk0x08 <= 0 && !(section->m_unk0x26 & 0x2000)) {
			FUN_1000899d(player->m_killer, mech, i + 1);
		}
	}

	if (mech->m_powerState == 4 || mech->m_powerState == 5) {
		if (g_playerDestroyed[player->m_index] != 1) {
			FUN_1000832b(player->m_killer, mech);
			if (player->m_killer >= 0 && player->m_killer < 8) {
				if (player->m_killer == player->m_index) {
					sprintf(text, "'%s' destroyed.", player->m_name);
				}
				else {
					sprintf(text, "'%s' destroyed by '%s'.", player->m_name, g_players[player->m_killer]->m_name);
				}
			}
			else {
				sprintf(text, "'%s' destroyed.", player->m_name);
			}

			ShowInGameMessage(text, 1, 0x712, 0x50);
			g_playerDestroyed[player->m_index] = 1;
		}
	}
	else if (g_playerDestroyed[player->m_index] == 1) {
		FUN_1007fbe0(player->m_index, 0, 0, 0);
		sprintf(text2, "'%s' resurrected.", player->m_name);
		ShowInGameMessage(text2, 1, 0x712, 0x50);
		g_playerDestroyed[player->m_index] = 0;
	}
}

// Sends the weapons the local player fired since the last time, if any.
// FUNCTION: MW2 0x1000f989
void FUN_1000f989(void)
{
	Player* player;
	Mech* mech;
	MechS32 i;

	memcpy(g_weaponsMsg->m_tag, "WE", 2);
	player = g_players[g_localPlayerId];
	mech = player->m_mech;
	g_weaponsMsg->m_player = player->m_index;
	g_weaponsMsg->m_weapons = 0;
	for (i = 0; i < 10; i++) {
		if (g_localWeaponsFired[i]) {
			g_weaponsMsg->m_weapons |= 1 << i;
			g_localWeaponsFired[i] = 0;
		}
	}

	if (g_weaponsMsg->m_weapons) {
		NetSend((MechU8*) g_weaponsMsg, sizeof(NetWeaponsMsg));
	}
}

// Fires the weapons of a player's weapons message.
// The only diff is a stack-slot permutation of player, msg, bit, mech and i.
// FUNCTION: MW2 0x1000fa58
void FUN_1000fa58(NetWeaponsMsg* p_msg, MechS32 p_slot)
{
	Player* player;
	NetWeaponsMsg* msg;
	MechU32 bit;
	Mech* mech;
	MechS32 i;

	msg = p_msg;
	if (msg->m_player == g_localPlayerId) {
		return;
	}

	player = g_players[p_slot];
	mech = player->m_mech;
	bit = 1;
	for (i = 0; i < 10; i++) {
		if (msg->m_weapons & bit) {
			g_remoteWeaponsFired[i] = 1;
		}

		bit <<= 1;
	}

	FireRemoteWeapons(mech);
}

// Sends player p_slot a collision push along the normal (p_x, p_y, p_z).
// FUNCTION: MW2 0x1000faef
void FUN_1000faef(MechS32 p_slot, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	Mech* mech;
	NetCollisionMsg* msg;

	msg = (NetCollisionMsg*) g_stateMsg;
	memcpy(msg->m_tag, "CO", 2);
	msg->m_normalX = -p_x;
	msg->m_normalY = -p_y;
	msg->m_normalZ = -p_z;
	mech = g_players[g_localPlayerId]->m_mech;
	msg->m_unk0x0e = mech->m_newVelocityX;
	msg->m_unk0x12 = mech->m_newVelocityY;
	msg->m_unk0x16 = mech->m_newVelocityZ;
	NetSendTo(g_netLaunch->m_playerIds[p_slot], msg, sizeof(NetCollisionMsg));
}

// Applies a collision push from player p_slot to the local mech.
// FUNCTION: MW2 0x1000fb8f
void FUN_1000fb8f(NetCollisionMsg* p_msg, MechS32 p_slot)
{
	MechS32 volume;
	Mech* mech;
	NetCollisionMsg* msg;

	if (g_localPlayerId == p_slot) {
		return;
	}

	msg = p_msg;
	mech = g_players[p_slot]->m_mech;
	g_segmentNormalX = msg->m_normalX;
	g_segmentNormalY = msg->m_normalY;
	g_segmentNormalZ = msg->m_normalZ;
	mech->m_newVelocityX = msg->m_unk0x0e;
	mech->m_newVelocityY = msg->m_unk0x12;
	mech->m_newVelocityZ = msg->m_unk0x16;
	volume = ApproximateVectorLength(mech->m_newVelocityX, mech->m_newVelocityY, mech->m_newVelocityZ);
	if (volume > 1500000) {
		volume = 1500000;
	}

	volume = MulDiv64(200, volume, 1500000);
	FUN_1007eb23(0xf0, volume, 0x40, 5, 0x32);
	FUN_100765f8(g_players[g_localPlayerId]->m_mech, g_players[p_slot]->m_mech);
}

// Sends GO: a slave is ready, or the master starts the game.
// FUNCTION: MW2 0x1000fca8
void FUN_1000fca8(void)
{
	MechChar msg[] = "GO";

	NetSend((MechU8*) msg, 2);
	if (g_netRole == 1) {
		g_goLaunch |= 1;
		g_gameStarted = 1;
	}
}

// FUNCTION: MW2 0x1000fcf5
void FUN_1000fcf5(MechChar* p_msg, MechS32 p_slot)
{
	MechS32 i;

	if (p_slot == g_localPlayerId) {
		return;
	}

	if (g_netRole == 1) {
		g_playerReady[p_slot] = 1;
		for (i = 0; i < g_playersFound; i++) {
			if (g_playerReady[i] != 1) {
				break;
			}
		}

		if (i == g_playersFound) {
			FUN_1000fca8();
		}
	}
	else if (GetPlayerSlotFromNetId(g_masterDpid) == p_slot) {
		g_goLaunch |= 1;
		g_gameStarted = 1;
	}
}

// Sends the game things destroyed so far.
// The only diff is a stack-slot permutation of out, i, count and bits.
// FUNCTION: MW2 0x1000fda6
void FUN_1000fda6(void)
{
	MechU8* out;
	MechS32 i;
	MechS32 count;
	MechU8 bits;

	count = 0;
	bits = 0;
	out = g_thingsMsg + 2;
	memcpy(g_thingsMsg, "SN", 2);
	*(MechS32*) out = g_stateCount;
	out += 4;
	for (i = 0; i < 254; i++) {
		bits <<= 1;
		if (g_gameThings[i].m_unk0x00 & 4) {
			bits |= 1;
		}

		count++;
		if (count == 8) {
			count = 0;
			*out = bits;
			out++;
			bits = 0;
		}
	}

	NetSend(g_thingsMsg, g_thingsMsgSize);
}

// Destroys the game things a player's things message has destroyed.
// The only diff is a stack-slot permutation of in, msgCount, i, count and bits.
// FUNCTION: MW2 0x1000fe61
void FUN_1000fe61(MechU8* p_msg, MechS32 p_slot)
{
	MechU8* in;
	MechS32 msgCount;
	MechS32 i;
	MechS32 count;
	MechU8 bits;

	count = 0;
	in = p_msg + 2;
	msgCount = *(MechS32*) in;
	in += 4;
	if (g_lastThingsCount[p_slot] >= msgCount) {
		return;
	}

	g_lastThingsCount[p_slot] = msgCount;
	bits = *in;
	for (i = 0; i < 254; i++) {
		if ((bits & 0x80) && !(g_gameThings[i].m_unk0x00 & 4)) {
			KillGameThing(i);
		}

		bits <<= 1;
		count++;
		if (count == 8) {
			count = 0;
			in++;
			bits = *in;
		}
	}
}

// Sends SU once: the local player's objectives succeeded.
// FUNCTION: MW2 0x1000ff29
void FUN_1000ff29(void)
{
	MechChar msg[] = "SU";

	if (!g_successSent) {
		NetSend((MechU8*) msg, 2);
		g_successSent = 1;
	}
}

// FUNCTION: MW2 0x1000ff70
void FUN_1000ff70(MechChar* p_msg, MechS32 p_slot)
{
	MechChar text[80];

	if (p_slot == g_localPlayerId) {
		return;
	}

	sprintf(text, "'%s' successful.", g_players[p_slot]->m_name);
	ShowInGameMessage(text, 1, 0x712, 0x50);
	g_carCfg.m_unk0xd2 = p_slot;
	if (!g_successSent) {
		g_successSent = 1;
	}
}

// Counts the players and keeps the lowest id as the master's.
// The only diff is the operand order of the comparison of p_id with g_masterDpid.
// FUNCTION: MW2 0x1000ffe6
BOOL PASCAL FUN_1000ffe6(DPID p_id, LPSTR p_friendlyName, LPSTR p_formalName, DWORD p_flags, LPVOID p_context)
{
	if (p_id < g_masterDpid) {
		g_masterDpid = p_id;
	}

	g_playersFound++;
	return TRUE;
}

// Copies the session named g_sessionName into p_context.
// FUNCTION: MW2 0x1001001a
BOOL PASCAL FUN_1001001a(LPDPSESSIONDESC p_desc, LPVOID p_context, LPDWORD p_timeout, DWORD p_flags)
{
	if (p_flags & DPESC_TIMEDOUT) {
		return FALSE;
	}

	if (strcmp(p_desc->szSessionName, g_sessionName) == 0) {
		*(DPSESSIONDESC*) p_context = *p_desc;
	}

	return FALSE;
}

// Picks the IPX service provider.
// FUNCTION: MW2 0x10010098
BOOL PASCAL FUN_10010098(LPGUID p_guid, LPSTR p_name, DWORD p_major, DWORD p_minor, LPVOID p_context)
{
	if (memcmp(p_name, "WinSock IPX Connection For DirectPlay", 38) == 0) {
		g_serviceProvider = p_guid;
	}

	return TRUE;
}

// Looks for the session named g_sessionName, into p_desc.
// The only diff is a stack-slot permutation of result and desc.
// FUNCTION: MW2 0x100100cc
MechS32 FUN_100100cc(LPDPSESSIONDESC p_desc)
{
	HRESULT result;
	DPSESSIONDESC desc;

	desc = *p_desc;
	result =
		g_directPlay->lpVtbl->EnumSessions(g_directPlay, &desc, 500, FUN_1001001a, p_desc, DPENUMSESSIONS_AVAILABLE);
	if (result) {
		return 0;
	}

	if (strcmp(p_desc->szSessionName, g_sessionName) == 0) {
		return 1;
	}
}

// The only diff is a stack-slot permutation of result, desc, guid and unk0x04.
// FUNCTION: MW2 0x10010178
MechS32 StartExternalIO(NetLaunchInfo* p_netLaunch)
{
	HRESULT result;
	DPSESSIONDESC desc;
	GUID guid;
	MechS32 unk0x04;

	guid = g_sessionGuid;
	unk0x04 = 1;
	if (p_netLaunch) {
		g_launchedByShell = 1;
		g_directPlay = p_netLaunch->m_directPlay;
		if (!g_directPlay) {
			DebugPrint("StartExternalIO(): No DirectPlay object.");
			return 0;
		}

		g_localDpid = p_netLaunch->m_localPlayerId;
		g_masterDpid = g_localDpid;
		g_netState = 1;
		ElectMaster();
		g_currentClock = FUN_1007d05d();
		g_unk0x100ba54c = FUN_1007d0fb();
		return 1;
	}
	else {
		DirectPlayEnumerate(FUN_10010098, NULL);
		if (!g_serviceProvider) {
			DebugPrint("StartExternalIO() No DPlay service provider.");
			return 0;
		}

		result = DirectPlayCreate(g_serviceProvider, &g_directPlay, NULL);
		if (!g_directPlay) {
			DebugPrint("StartExternalIO() DirectPlayCreate() failed.");
			return 0;
		}

		memset(&desc, 0, sizeof(desc));
		desc.dwSize = sizeof(desc);
		desc.dwMaxPlayers = 8;
		desc.guidSession = g_sessionGuid;
		strcpy(desc.szSessionName, g_sessionName);
		if (g_netRole == 1) {
			desc.dwFlags = DPOPEN_CREATESESSION;
			result = g_directPlay->lpVtbl->Open(g_directPlay, &desc);
			if (result) {
				DebugPrint("StartExternalIO() DPlay:Open(CREATESESSION) failed.");
				return 0;
			}

			result = g_directPlay->lpVtbl
						 ->CreatePlayer(g_directPlay, &g_localDpid, g_sessionName, "Master Mech Player", NULL);
		}
		else {
			DebugPrint("StartExternalIO() Searching for session...");
			for (;;) {
				if (FUN_100100cc(&desc)) {
					break;
				}

				if (KeyboardPollKeyCode() == 0x1b) {
					return 0;
				}
			}

			desc.dwFlags = DPOPEN_OPENSESSION;
			result = g_directPlay->lpVtbl->Open(g_directPlay, &desc);
			if (result) {
				DebugPrint("StartExternalIO() DPlay:Open(OPENSESSION) failed.");
				return 0;
			}

			result = g_directPlay->lpVtbl
						 ->CreatePlayer(g_directPlay, &g_localDpid, g_sessionName, "Slave Mech2 Player", NULL);
		}

		if (result) {
			DebugPrint("StartExternalIO() DPlay:CreatePlayer() failed.");
			return 0;
		}

		while (g_playersFound < g_isNetworkGame) {
			if (g_directPlay) {
				g_masterDpid = g_localDpid;
				g_playersFound = 0;
				result = g_directPlay->lpVtbl->EnumPlayers(g_directPlay, 500, FUN_1000ffe6, NULL, DPENUMPLAYERS_ALL);
			}

			DebugPrint("StartExternalIO() Located %d of %d players.", g_playersFound, g_isNetworkGame);
			if (KeyboardPollKeyCode() == 0x1b) {
				return 0;
			}
		}

		g_netState = 1;
		ElectMaster();
		g_currentClock = FUN_1007d05d();
		g_unk0x100ba54c = FUN_1007d0fb();
		return 1;
	}

	// Unreachable: both arms return.
	if (p_netLaunch && (p_netLaunch->m_unk0x14 & 1)) {
		g_stateInterval = 0x2d;
	}

	return 1;
}

// Sends SS and leaves the session, then stops the network.
// FUNCTION: MW2 0x10010539
MechS32 StopExternalIO(void)
{
	MechChar msg[] = "SS";

	if (!g_launchedByShell) {
		if (g_directPlay && g_localDpid) {
			if (g_netRole) {
				NetSend((MechU8*) msg, 2);
			}

			g_directPlay->lpVtbl->DestroyPlayer(g_directPlay, g_localDpid);
			g_localDpid = 0;
		}

		if (g_directPlay && !g_launchedByShell) {
			g_directPlay->lpVtbl->Close(g_directPlay);
			g_directPlay = NULL;
		}
	}
	else if (g_directPlay && g_localDpid && g_netRole) {
		NetSend((MechU8*) msg, 2);
	}

	if (g_netState != 1) {
		return 1;
	}
	else {
		g_netState = 2;
	}

	g_netRole = 0;
	g_unk0x100ba554 = 0;
}

// Counts the players and elects the one with the lowest id master. Alone, the network stops.
// FUNCTION: MW2 0x10010669
void ElectMaster(void)
{
	HRESULT result;

	if (g_directPlay) {
		g_masterDpid = g_localDpid;
		g_playersFound = 0;
		result = g_directPlay->lpVtbl->EnumPlayers(g_directPlay, 0, FUN_1000ffe6, NULL, DPENUMPLAYERS_GROUP);
	}

	DebugPrint("ElectMaster() found %d players.  MasterDPID is %d\n", g_playersFound, g_masterDpid);
	if (g_playersFound == 1) {
		g_netState = 2;
		g_netRole = 0;
		g_unk0x100ba554 = 0;
		g_goLaunch |= 1;
	}
	else if (g_localDpid == g_masterDpid) {
		g_netRole = 1;
		g_unk0x100ba554 = 0;
	}
	else {
		g_netRole = 2;
		g_unk0x100ba554 = 2;
	}
}
