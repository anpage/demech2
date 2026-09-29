#include "gpanim.h"

#include "decomp.h"
#include "players.h"
#include "simmain.h"
#include "soundfx.h"
#include "types.h"

// GLOBAL: MW2 0x100a0110
MechS32 g_unk0x100a0110[4][4] =
	{{-1, 0x12e, 0x12e, -1}, {0x14f, -1, -1, 0x14f}, {0x11a, 0x11a, 0x148, 0x11a}, {0x119, 0x119, 0x119, 0x12d}};

// GLOBAL: MW2 0x10181970
MechS32 g_unk0x10181970[60];

// FUNCTION: MW2 0x10003620
void FirstGPAnim(void)
{
	MechS32 i;

	for (i = 0; i < 60; i++) {
		g_unk0x10181970[i] = -2;
	}
}

// FUNCTION: MW2 0x1000365a
void FUN_1000365a(Player* p_player)
{
	if (p_player->m_index != g_localPlayerId) {
		p_player->m_unk0x90 = 0x103;
	}

	p_player->m_unk0x8c = 0;
	p_player->m_unk0x88 = -1;
}

// FUNCTION: MW2 0x1000369e
void FUN_1000369e(Player* p_player)
{
	p_player->m_unk0x8c = 0;
	p_player->m_unk0x88 = -1;
}

// FUNCTION: MW2 0x100036c3
MechS32* FUN_100036c3(Player* p_player, MechS32* p_offset)
{
	p_offset[0] = g_eyepoint->m_unk0x00 - p_player->m_position.m_x;
	p_offset[1] = g_eyepoint->m_unk0x04 - p_player->m_position.m_y;
	p_offset[2] = g_eyepoint->m_unk0x08 - p_player->m_position.m_z;
	return p_offset;
}

// Matches except for the stack slots of offset, sound and id (a consistent permutation) and
// the order the p_sounds index loads its row and column in.
// FUNCTION: MW2 0x100038c2
void FUN_100038c2(Player* p_player, MechS32 (*p_sounds)[4], MechS32* p_offset)
{
	MechS32 offset[3];
	MechS32 sound;
	MechS32 id;

	id = p_player->m_index;
	if ((p_player->m_unk0x84 != p_player->m_unk0x88 && g_unk0x10181970[id] != p_player->m_unk0x84) ||
		(p_player->m_unk0x84 == p_player->m_unk0x88 && g_unk0x10181970[id] != -2)) {
		if (p_player->m_unk0x84 == 0 && g_unk0x10181970[id] == 2) {
			sound = p_sounds[0][1];
		}
		else {
			sound = p_sounds[p_player->m_unk0x84 + 1][p_player->m_unk0x88 + 1];
		}

		if (sound != -1) {
			if (p_offset == NULL) {
				p_offset = FUN_100036c3(p_player, offset);
			}

			FUN_1007ebd1(p_offset[0], p_offset[1], p_offset[2], sound, g_unk0x100a2420);
		}

		if (p_player->m_unk0x84 != p_player->m_unk0x88) {
			g_unk0x10181970[id] = p_player->m_unk0x84;
		}
		else {
			g_unk0x10181970[id] = -2;
		}
	}
}

// FUNCTION: MW2 0x10003a10
void FUN_10003a10(Player* p_player)
{
	MechS32 id;

	id = p_player->m_index;
	p_player->m_unk0x8c = 0;
	p_player->m_unk0x88 = -1;
	p_player->m_unk0x84 = -1;
	p_player->m_unk0x80 |= 0x10;
	g_unk0x10181970[id] = -2;
}
