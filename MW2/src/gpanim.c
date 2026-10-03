#include "gpanim.h"

#include "decomp.h"
#include "eyepoint.h"
#include "mech.h"
#include "players.h"
#include "playersteering.h"
#include "polydraw.h"
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
		p_player->m_pendingSound = 0x103;
	}

	p_player->m_speedLevel = 0;
	p_player->m_nextMotionState = -1;
}

// FUNCTION: MW2 0x1000369e
void FUN_1000369e(Player* p_player)
{
	p_player->m_speedLevel = 0;
	p_player->m_nextMotionState = -1;
}

// FUNCTION: MW2 0x100036c3
MechS32* FUN_100036c3(Player* p_player, MechS32* p_offset)
{
	p_offset[0] = g_eyepoint->m_unk0x00 - p_player->m_position.m_x;
	p_offset[1] = g_eyepoint->m_unk0x04 - p_player->m_position.m_y;
	p_offset[2] = g_eyepoint->m_unk0x08 - p_player->m_position.m_z;
	return p_offset;
}

// Picks the player's motion state from its mech's throttle (m_speedLevel, m_nextMotionState; 2
// while its steering's m_reverse is set), plays its pending sound m_pendingSound once (bit 8 of
// m_unk0x80), and for the local player updates the looping sounds (FUN_100038c2).
// Stack-slot permutation: offset and position.
// FUNCTION: MW2 0x10003710
void FUN_10003710(Player* p_player)
{
	MechS32* offset;
	MechS32 position[3];
	MechS32 height;

	offset = NULL;
	if (p_player->m_mech->m_throttle.m_value <= 0x420) {
		FUN_1000369e(p_player);
	}
	else {
		height = p_player->m_mech->m_throttle.m_value - 0x400;
		if (height < 0) {
			height = 0;
		}

		p_player->m_nextMotionState = 0;
		if (height < 0x100) {
			p_player->m_speedLevel = 1;
		}
		else if (height < 0x300) {
			p_player->m_speedLevel = 2;
		}
		else {
			p_player->m_speedLevel = 3;
			p_player->m_nextMotionState = 1;
		}

		if (p_player->m_motionState == 2 && p_player->m_speedLevel > 2) {
			p_player->m_speedLevel = 2;
		}

		if (p_player->m_steering->m_reverse) {
			p_player->m_nextMotionState = 2;
		}

		if (p_player->m_type == c_playerTypeTank) {
			p_player->m_motionState = p_player->m_nextMotionState;
		}
	}

	if (p_player->m_pendingSound != -1 && p_player->m_unk0x80 & 8) {
		p_player->m_unk0x80 &= ~8;
		offset = FUN_100036c3(p_player, position);
		FUN_1007ebd1(position[0], position[1], position[2], p_player->m_pendingSound, g_unk0x100a2420);
	}

	if (p_player->m_index == g_localPlayerId) {
		FUN_100038c2(p_player, g_unk0x100a0110, offset);
	}
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
	if ((p_player->m_motionState != p_player->m_nextMotionState && g_unk0x10181970[id] != p_player->m_motionState) ||
		(p_player->m_motionState == p_player->m_nextMotionState && g_unk0x10181970[id] != -2)) {
		if (p_player->m_motionState == 0 && g_unk0x10181970[id] == 2) {
			sound = p_sounds[0][1];
		}
		else {
			sound = p_sounds[p_player->m_motionState + 1][p_player->m_nextMotionState + 1];
		}

		if (sound != -1) {
			if (p_offset == NULL) {
				p_offset = FUN_100036c3(p_player, offset);
			}

			FUN_1007ebd1(p_offset[0], p_offset[1], p_offset[2], sound, g_unk0x100a2420);
		}

		if (p_player->m_motionState != p_player->m_nextMotionState) {
			g_unk0x10181970[id] = p_player->m_motionState;
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
	p_player->m_speedLevel = 0;
	p_player->m_nextMotionState = -1;
	p_player->m_motionState = -1;
	p_player->m_unk0x80 |= 0x10;
	g_unk0x10181970[id] = -2;
}
