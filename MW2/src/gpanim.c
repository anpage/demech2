#include "gpanim.h"

#include "decomp.h"
#include "eyepoint.h"
#include "fixedfloat.h"
#include "mech.h"
#include "players.h"
#include "playersteering.h"
#include "polydraw.h"
#include "simmain.h"
#include "soundfx.h"
#include "types.h"

// GLOBAL: MW2 0x100a0110
// GLOBAL: MW2GBL 0x100a0d18
// GLOBAL: MW2MATROX 0x100bca68
// GLOBAL: MW23DFX 0x1024b2b8
// GLOBAL: MW2ATI 0x1017f638
// GLOBAL: MW2S3 0x10065c18
// GLOBAL: MW2POWERVR 0x101aa380
MechS32 g_motionSounds[4][4] =
	{{-1, 0x12e, 0x12e, -1}, {0x14f, -1, -1, 0x14f}, {0x11a, 0x11a, 0x148, 0x11a}, {0x119, 0x119, 0x119, 0x12d}};

// GLOBAL: MW2 0x10181970
// GLOBAL: MW2GBL 0x10134f50
// GLOBAL: MW2MATROX 0x1012c0f0
// GLOBAL: MW23DFX 0x10207f00
// GLOBAL: MW2ATI 0x1013bbc0
// GLOBAL: MW2S3 0x100adb90
// GLOBAL: MW2POWERVR 0x10166880
MechS32 g_lastMotionSound[60];

// FUNCTION: MW2 0x10003620
// FUNCTION: MW2GBL 0x10021790
// FUNCTION: MW2MATROX 0x1008c3e0
// FUNCTION: MW23DFX 0x1000b300
// FUNCTION: MW2ATI 0x1000ba70
// FUNCTION: MW2S3 0x100236b0
// FUNCTION: MW2POWERVR 0x1000a630
void FirstGPAnim(void)
{
	MechS32 i;

	for (i = 0; i < 60; i++) {
		g_lastMotionSound[i] = -2;
	}
}

// FUNCTION: MW2 0x1000365a
// FUNCTION: MW2GBL 0x100217ca
// FUNCTION: MW2MATROX 0x1008c41a
// FUNCTION: MW23DFX 0x1000b320
// FUNCTION: MW2ATI 0x1000ba90
// FUNCTION: MW2S3 0x100236d0
// FUNCTION: MW2POWERVR 0x1000a650
void StartMotion(Player* p_player)
{
	if (p_player->m_index != g_localPlayerId) {
		p_player->m_pendingSound = 0x103;
	}

	p_player->m_speedLevel = 0;
	p_player->m_nextMotionState = -1;
}

// FUNCTION: MW2 0x1000369e
// FUNCTION: MW2GBL 0x1002180e
// FUNCTION: MW2MATROX 0x1008c45e
// FUNCTION: MW23DFX 0x1000b350
// FUNCTION: MW2ATI 0x1000bac0
// FUNCTION: MW2S3 0x10023700
// FUNCTION: MW2POWERVR 0x1000a680
void StopMotion(Player* p_player)
{
	p_player->m_speedLevel = 0;
	p_player->m_nextMotionState = -1;
}

// FUNCTION: MW2 0x100036c3
// FUNCTION: MW2GBL 0x10021833
// FUNCTION: MW2MATROX 0x1008c483
// FUNCTION: MW23DFX 0x1000b370
// FUNCTION: MW2ATI 0x1000bae0
// FUNCTION: MW2S3 0x10023720
// FUNCTION: MW2POWERVR 0x1000a6a0
MechScalar* GetEyepointOffset(Player* p_player, MechScalar* p_offset)
{
	p_offset[0] = g_eyepoint->m_x - p_player->m_position.m_x;
	p_offset[1] = g_eyepoint->m_y - p_player->m_position.m_y;
	p_offset[2] = g_eyepoint->m_z - p_player->m_position.m_z;
	return p_offset;
}

// Picks the player's motion state from its mech's throttle (m_speedLevel, m_nextMotionState; 2
// while its steering's m_reverse is set), plays its pending sound m_pendingSound once (bit 8 of
// m_animFlags), and for the local player updates the looping sounds (UpdateMotionSounds).
// FUNCTION: MW2 0x10003710
// FUNCTION: MW2GBL 0x10021880
// FUNCTION: MW2MATROX 0x1008c4d0
// FUNCTION: MW23DFX 0x1000b3a0
// FUNCTION: MW2ATI 0x1000bb10
// FUNCTION: MW2S3 0x10023750
// FUNCTION: MW2POWERVR 0x1000a6d0
void UpdateMotion(Player* p_player)
{
	MechScalar* offset;
	MechScalar position[3];
	MechScalar height;

	offset = NULL;
	if (p_player->m_mech->m_throttle.m_value <= FIXED_RAW(0x420)) {
		StopMotion(p_player);
	}
	else {
		height = p_player->m_mech->m_throttle.m_value - FIXED_RAW(0x400);
#ifdef MW2_FLOAT_SCALARS
		if (height < 1e-07f) {
#else
		if (height < 0) {
#endif
			height = 0;
		}

		p_player->m_nextMotionState = 0;
		if (height < FIXED_RAW(0x100)) {
			p_player->m_speedLevel = 1;
		}
		else if (height < FIXED_RAW(0x300)) {
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

	if (p_player->m_pendingSound != -1 && p_player->m_animFlags & 8) {
		p_player->m_animFlags &= ~8;
		offset = GetEyepointOffset(p_player, position);
		PlaySoundAt(position[0], position[1], position[2], p_player->m_pendingSound, g_inCockpitView);
	}

	if (p_player->m_index == g_localPlayerId) {
		UpdateMotionSounds(p_player, g_motionSounds, offset);
	}
}

// FUNCTION: MW2 0x100038c2
// FUNCTION: MW2GBL 0x10021a32
// FUNCTION: MW2MATROX 0x1008c69f
// FUNCTION: MW23DFX 0x1000b4e0
// FUNCTION: MW2ATI 0x1000bc50
// FUNCTION: MW2S3 0x10023890
// FUNCTION: MW2POWERVR 0x1000a810
void UpdateMotionSounds(Player* p_player, MechS32 (*p_sounds)[4], MechScalar* p_offset)
{
	MechScalar offset[3];
	MechS32 sound;
	MechS32 id;

	id = p_player->m_index;
	if ((p_player->m_motionState != p_player->m_nextMotionState && g_lastMotionSound[id] != p_player->m_motionState) ||
		(p_player->m_motionState == p_player->m_nextMotionState && g_lastMotionSound[id] != -2)) {
		if (p_player->m_motionState == 0 && g_lastMotionSound[id] == 2) {
			sound = p_sounds[0][1];
		}
		else {
			sound = p_sounds[p_player->m_motionState + 1][p_player->m_nextMotionState + 1];
		}

		if (sound != -1) {
			if (p_offset == NULL) {
				p_offset = GetEyepointOffset(p_player, offset);
			}

			PlaySoundAt(p_offset[0], p_offset[1], p_offset[2], sound, g_inCockpitView);
		}

		if (p_player->m_motionState != p_player->m_nextMotionState) {
			g_lastMotionSound[id] = p_player->m_motionState;
		}
		else {
			g_lastMotionSound[id] = -2;
		}
	}
}

// FUNCTION: MW2 0x10003a10
// FUNCTION: MW2GBL 0x10021b80
// FUNCTION: MW2MATROX 0x1008c7ed
// FUNCTION: MW23DFX 0x1000b5a0
// FUNCTION: MW2ATI 0x1000bd10
// FUNCTION: MW2S3 0x10023950
// FUNCTION: MW2POWERVR 0x1000a8d0
void ResetMotion(Player* p_player)
{
	MechS32 id;

	id = p_player->m_index;
	p_player->m_speedLevel = 0;
	p_player->m_nextMotionState = -1;
	p_player->m_motionState = -1;
	p_player->m_animFlags |= 0x10;
	g_lastMotionSound[id] = -2;
}
