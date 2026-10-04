#include "unk1000f0f0.h"

#include "bwdkeywords.h"
#include "bwdwriter.h"
#include "decomp.h"
#include "difficultycfg.h"
#include "resourcename.h"
#include "types.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk10007c30.h"
#include "unk1000aa90.h"

#include <stdio.h>
#include <string.h>

#pragma pack(1)

// A GPS node of the BWD streams the lobby writes: a player's mech, pilot and name.
// SIZE 0x62
struct GpSpecNode {
	MechU32 m_type;                   // 0x00
	MechS32 m_size;                   // 0x04
	MechS16 m_plrId;                  // 0x08: the PLR resource of m_plr, or -2
	MechS16 m_mechId;                 // 0x0a: the BWD resource of m_mechFile
	MechU8 m_unk0x0c;                 // 0x0c
	MechU8 m_unk0x0d;                 // 0x0d
	MechU8 m_remote;                  // 0x0e: not the local player
	undefined m_unk0x0f;              // 0x0f
	MechS16 m_unk0x10;                // 0x10
	MechS16 m_unk0x12;                // 0x12
	MechS16 m_unk0x14;                // 0x14
	MechS16 m_unk0x16;                // 0x16
	MechS16 m_unk0x18;                // 0x18
	MechS16 m_unk0x1a;                // 0x1a
	MechS16 m_unk0x1c;                // 0x1c
	MechS16 m_unk0x1e;                // 0x1e
	MechS16 m_unk0x20;                // 0x20
	MechU16 m_unk0x22;                // 0x22
	MechChar m_mechFile[9];           // 0x24
	MechChar m_plr[9];                // 0x2d
	MechChar m_name[0x16];            // 0x36
	undefined m_unk0x4c[0x62 - 0x4c]; // 0x4c
};

#pragma pack()

DECOMP_SIZE_ASSERT(GpSpecNode, 0x62)
DECOMP_SIZE_ASSERT(DifficultyCfg, 0x17)

void FUN_1000f27f(MechS32 p_index, MechU8 p_unk0x0c, MechU8 p_unk0x0d);
MechS32 FUN_1000f487(MechChar* p_code);

// Writes a BWD stream for each occupied player slot, en<slot>star.bwd, with the slot's player.
// FUNCTION: NETMECHW 0x1000f0f0
void FUN_1000f0f0()
{
	MechChar name[16];
	MechS32 i;

	for (i = 0; i < 8; i++) {
		BeginBwdStream();
		if (IS_PLAYER_SLOT_USED(i)) {
			FUN_1000f27f(i, i, 1);
		}

		sprintf(name, "en%02dstar.bwd", i);
		WriteBwdStream(name);
	}
}

// Writes a BWD stream for each team, en01star.bwd with the players of team 0 and en00star.bwd
// with those of team 1.
// FUNCTION: NETMECHW 0x1000f172
void FUN_1000f172()
{
	MechS32 first;
	MechS32 i;

	BeginBwdStream();
	first = TRUE;
	for (i = 0; i < 8; i++) {
		if (IS_PLAYER_SLOT_USED(i) && g_players[i].m_team == 0) {
			FUN_1000f27f(i, 1, first);
			first = FALSE;
		}
	}

	WriteBwdStream("en01star.bwd");

	BeginBwdStream();
	first = TRUE;
	for (i = 0; i < 8; i++) {
		if (IS_PLAYER_SLOT_USED(i) && g_players[i].m_team == 1) {
			FUN_1000f27f(i, 0, first);
			first = FALSE;
		}
	}

	WriteBwdStream("en00star.bwd");
}

// Appends the GPS node of the player in slot p_index to the BWD stream.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000f27f
void FUN_1000f27f(MechS32 p_index, MechU8 p_unk0x0c, MechU8 p_unk0x0d)
{
	CopperField0x4d::Options flags;
	GpSpecNode node;
	MechS16 plrId;
	MechChar code[4];
	MechS32 index;

	memset(&node, 0, sizeof(node));
	node.m_type = g_bwdTypeCodes[c_bwdGpSpec];
	node.m_size = 100;

	if (_strnicmp(&g_unk0x1001ca90.m_settings.m_mechs[p_index][5], "PLR", 3)) {
		strncpy(node.m_plr, g_unk0x1001ca90.m_settings.m_mechs[p_index], 8);
		node.m_plr[8] = '\0';
	}
	else {
		sprintf(node.m_plr, "%3.3s%02dPLR", g_unk0x1001ca90.m_settings.m_mechs[p_index], p_index);
	}

	strncpy(code, g_unk0x1001ca90.m_settings.m_mechs[p_index], 3);
	code[3] = '\0';
	index = FUN_1000f487(code);
	strncpy(node.m_mechFile, g_unk0x1001c318[index].m_mechFile, sizeof(node.m_mechFile));
	strncpy(node.m_name, g_players[p_index].m_name, sizeof(node.m_name) - 1);
	node.m_name[sizeof(node.m_name) - 1] = '\0';

	flags = g_unk0x1001ca90.m_settings.m_options.m_bits;
	if (flags.m_option5) {
		node.m_unk0x22 = 0x400;
	}
	else {
		node.m_unk0x22 = 0;
	}

	if (_strnicmp(&node.m_plr[5], "std", 3)) {
		plrId = -2;
	}
	else {
		plrId = FindResourceIdByName(6, node.m_plr);
	}

	node.m_plrId = plrId;
	node.m_mechId = FindResourceIdByName(0xe, node.m_mechFile);
	node.m_unk0x0c = p_unk0x0c;
	node.m_unk0x0d = p_unk0x0d;
	node.m_remote = FUN_1000b02b(g_unk0x1001ca90.m_playerId) == p_index ? FALSE : TRUE;
	node.m_unk0x10 = 0;
	node.m_unk0x12 = 0;
	node.m_unk0x14 = 0;
	node.m_unk0x16 = 0;
	node.m_unk0x18 = 0;
	node.m_unk0x1a = 0;
	node.m_unk0x1c = 0;
	node.m_unk0x1e = 0;
	node.m_unk0x20 = 6;
	AppendBwdNode(&node, sizeof(node));
}

// Returns the index in g_unk0x1001c318 of the chassis p_code, or -1.
// Matches except for the comparison i < g_unk0x1001fe70, whose operands VC++ 2.2 swaps.
// FUNCTION: NETMECHW 0x1000f487
MechS32 FUN_1000f487(MechChar* p_code)
{
	MechS32 i;

	for (i = 0; i < g_unk0x1001fe70; i++) {
		if (!strcmp(p_code, g_unk0x1001c318[i].m_code)) {
			break;
		}
	}

	if (i == g_unk0x1001fe70) {
		return -1;
	}

	return i;
}

// Writes the game options into the difficulty settings for the simulator, MW2NET.CFG, starting
// from MW2NET.CFG or else MW2DIF.CFG.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000f52c
void WriteNetDifficultyCfg(
	CopperField0x4d::Options p_options,
	MechU8 p_temperature,
	MechU8 p_gravity,
	MechU8 p_timeOfDay,
	MechU8 p_teamGame
)
{
	DifficultyCfg cfg;
	FILE* file;

	file = fopen("MW2NET.CFG", "rb");
	if (file == NULL) {
		file = fopen("MW2DIF.CFG", "rb");
		if (file == NULL) {
			return;
		}
	}

	fread(&cfg, sizeof(cfg), 1, file);
	fclose(file);

	cfg.m_unlimitedAmmo = p_options.m_option1 ? TRUE : FALSE;
	cfg.m_regenerate = p_options.m_option0 ? TRUE : FALSE;
	cfg.m_splashDamage = p_options.m_option3 ? TRUE : FALSE;
	cfg.m_collisionDamage = p_options.m_option4 ? TRUE : FALSE;
	cfg.m_heatTracking = p_options.m_option2 ? TRUE : FALSE;
	cfg.m_radar = p_options.m_option5 ? TRUE : FALSE;

	switch (p_temperature) {
	case 1:
		cfg.m_temperature = -50;
		break;
	case 2:
		cfg.m_temperature = 10;
		break;
	case 3:
		cfg.m_temperature = 70;
		break;
	}

	cfg.m_gravity = p_gravity << 12;
	cfg.m_timeOfDay = p_timeOfDay;
	cfg.m_teamGame = p_teamGame;

	file = fopen("MW2NET.CFG", "wb");
	if (file) {
		fwrite(&cfg, sizeof(cfg), 1, file);
		fclose(file);
	}
}
