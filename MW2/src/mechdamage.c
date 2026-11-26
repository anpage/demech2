#include "mechdamage.h"

#include "ai.h"
#include "ammobin.h"
#include "classtable.h"
#include "clock.h"
#include "config.h"
#include "decomp.h"
#ifdef MW2_EXTRA_DEBUG_PRINTS
#include "debugprint.h"
#endif
#include "eyepoint.h"
#include "fixedfloat.h"
#include "maneuvers.h"
#include "mech.h"
#include "mechclass.h"
#include "mechsection.h"
#include "mw2log.h"
#include "network.h"
#include "object.h"
#include "objective.h"
#include "players.h"
#include "playersteering.h"
#include "polydraw.h"
#include "random.h"
#include "shape.h"
#include "shots.h"
#include "simmain.h"
#include "soundfx.h"
#include "speech.h"
#include "targeting.h"
#include "team.h"
#include "types.h"
#include "weapons.h"
#include "weaponslot.h"

#include <stdio.h>

// A game-key toggle (GetSystemSetting's setting 0x3c).
// GLOBAL: MW2 0x100a1590
// GLOBAL: MW2GBL 0x100a04b4
// GLOBAL: MW2MATROX 0x100a4618
// GLOBAL: MW23DFX 0x1024bfbc
// GLOBAL: MW2ATI 0x1018033c
// GLOBAL: MW2S3 0x100621bc
// GLOBAL: MW2POWERVR 0x101ab13c
MechS32 g_autoEject = 0;

// The armor per damage level of other players' sections (the local player's: g_localArmorPerLevel).
// GLOBAL: MW2 0x100a1594
// GLOBAL: MW2GBL 0x100a04b8
// GLOBAL: MW2MATROX 0x100a461c
// GLOBAL: MW23DFX 0x1024bfc0
// GLOBAL: MW2ATI 0x10180340
// GLOBAL: MW2S3 0x100621c0
// GLOBAL: MW2POWERVR 0x101ab140
MechScalar g_otherArmorPerLevel = 4;

// GLOBAL: MW2 0x100a1598
// GLOBAL: MW2GBL 0x100a04bc
// GLOBAL: MW2PREMERCS 0x100b1a2c
// GLOBAL: MW2MATROX 0x100a4620
// GLOBAL: MW23DFX 0x1024bfc4
// GLOBAL: MW2ATI 0x10180344
// GLOBAL: MW2S3 0x100621c4
// GLOBAL: MW2POWERVR 0x101ab144
MechScalar g_localArmorPerLevel = 4;

// GLOBAL: MW2 0x100a159c
// GLOBAL: MW2GBL 0x100a04c0
// GLOBAL: MW2PREMERCS 0x100b1a30
// GLOBAL: MW2MATROX 0x100a4624
// GLOBAL: MW23DFX 0x1024bfc8
MechS32 g_localMechHidden = 0;

// The kill count the cockpit shows in a network game.
// GLOBAL: MW2 0x100a15a0
// GLOBAL: MW2GBL 0x100a04c4
// GLOBAL: MW2PREMERCS 0x100b1a34
// GLOBAL: MW2MATROX 0x100a4628
// GLOBAL: MW23DFX 0x1024bfcc
// GLOBAL: MW2ATI 0x1018034c
// GLOBAL: MW2S3 0x100621cc
// GLOBAL: MW2POWERVR 0x101ab14c
MechS32 g_killCount = 0;

// Set while the local player has been warned of critical heat (CalculateHeat).
// GLOBAL: MW2 0x100a15a4
// GLOBAL: MW2GBL 0x100a04c8
// GLOBAL: MW2PREMERCS 0x100b1a38
// GLOBAL: MW2MATROX 0x100a462c
// GLOBAL: MW23DFX 0x1024bfd0
// GLOBAL: MW2S3 0x100621d0
// GLOBAL: MW2POWERVR 0x101ab150
MechS32 g_criticalHeatWarned = 0;

// When the warning was given.
// GLOBAL: MW2 0x100bdff0
// GLOBAL: MW2GBL 0x100b9118
// GLOBAL: MW2PREMERCS 0x100caef0
// GLOBAL: MW2MATROX 0x100c1e24
// GLOBAL: MW23DFX 0x1007c310
// GLOBAL: MW2S3 0x1007cab4
// GLOBAL: MW2POWERVR 0x1006f310
static MechS32 g_criticalHeatWarningTime;

// The target the autopilot steers for: the early Mercenaries build's nav target.
#ifdef MW2_NAV_TARGET_KEPT
#define AUTOPILOT_TARGET m_navTargetInfo
#else
#define AUTOPILOT_TARGET m_targetInfo
#endif

// Runs the autopilot (m_autopilot): mode 1 follows the nav points in order, skipping the ones
// already reached and marking each one it reaches (turning off after the last); then the AI
// steers, the throttle saved while AvoidObstacles has it.
// The Matrox edition copies m_maneuverParam and the steering's float throttle into each other as
// they are (the dword, no conversion).
// FUNCTION: MW2 0x100079d0
// FUNCTION: MW2GBL 0x1001ea60
// FUNCTION: MW2PREMERCS 0x1003d4b0
// FUNCTION: MW2MATROX 0x10010250
// FUNCTION: MW23DFX 0x10015c00
// FUNCTION: MW2ATI 0x10016010
// FUNCTION: MW2S3 0x1000dc10
// FUNCTION: MW2POWERVR 0x10014f50
void RunAutopilot(Mech* p_mech)
{
	MechS32 index;
	MechS32 first;

	if (p_mech->m_autopilot == 0) {
		return;
	}

	if (p_mech->m_autopilot == 1) {
		if (!(p_mech->m_player->AUTOPILOT_TARGET.m_target & 0x100) ||
			(p_mech->m_player->AUTOPILOT_TARGET.m_target & 0x1000)) {
			CycleNavTarget(p_mech->m_player, 0, 0);
			index = p_mech->m_player->AUTOPILOT_TARGET.m_target & 0xff;
			first = index;
			while (g_navTable[index].m_flags & 0x20) {
				CycleNavTarget(p_mech->m_player, 1, 0);
				index = p_mech->m_player->AUTOPILOT_TARGET.m_target & 0xff;
				if (index == first) {
					p_mech->m_player->AUTOPILOT_TARGET.m_target |= 0x1000;
					return;
				}
			}
		}

		if (p_mech->m_player->AUTOPILOT_TARGET.m_target & 0x100) {
			index = p_mech->m_player->AUTOPILOT_TARGET.m_target & 0xff;
			first = index;
			if (g_navTable[index].m_radius > p_mech->m_player->AUTOPILOT_TARGET.m_distance &&
				p_mech->m_player->m_index == g_localPlayerId) {
				if (!(g_navTable[index].m_flags & 0x20)) {
					g_navTable[index].m_flags |= 0x20;
					g_navTable[index].m_teamsReached |= 1 << p_mech->m_player->m_team;
					if (g_navTable[index].m_flags & 0x40) {
						FUN_1001cdd1();
					}

					PlaySoundEffect(0xe7, 100, 0x40, 5, 0x50);
				}

				CycleNavTarget(p_mech->m_player, 1, 0);
				if ((p_mech->m_player->AUTOPILOT_TARGET.m_target & 0xff) < first) {
					p_mech->m_autopilot = 0;
					p_mech->m_player->AUTOPILOT_TARGET.m_target |= 0x1000;
					p_mech->m_player->m_steering->m_throttle = 0;
					p_mech->m_player->m_steering->m_throttleSet = 1;
					return;
				}
			}
		}
	}

	if (!AvoidObstacles(p_mech->m_player)) {
		SteerToTarget(p_mech->m_player);
		if (p_mech->m_player->m_maneuverFlag == 1) {
#ifdef MW2_FLOAT_SCALARS
			p_mech->m_player->m_steering->m_throttle = *(MechFloat*) &p_mech->m_player->m_maneuverParam;
#else
			p_mech->m_player->m_steering->m_throttle = p_mech->m_player->m_maneuverParam;
#endif
			p_mech->m_player->m_maneuverFlag = 0;
		}

#ifdef MW2_FLOAT_SCALARS
		p_mech->m_player->m_maneuverParam = *(MechS32*) &p_mech->m_player->m_steering->m_throttle;
#else
		p_mech->m_player->m_maneuverParam = p_mech->m_player->m_steering->m_throttle;
#endif
	}
	else {
		p_mech->m_player->m_maneuverFlag = 1;
	}
}

// The PUNCH_IN_AUTO_HDG game key: sets the player's target heading (the HUD's bearing marker,
// which the autopilot steers by) to where the torso faces, unless the autopilot is on.
// FUNCTION: MW2 0x10007cb5
// FUNCTION: MW2GBL 0x1001ed45
// FUNCTION: MW2PREMERCS 0x1003d795
// FUNCTION: MW2MATROX 0x1001053c
// FUNCTION: MW23DFX 0x10015dd0
// FUNCTION: MW2ATI 0x100161e0
// FUNCTION: MW2S3 0x1000dde0
// FUNCTION: MW2POWERVR 0x10015120
void PunchInAutoHeading(Mech* p_mech)
{
	MechScalar heading;

	if (p_mech->m_autopilot == 1) {
		return;
	}

#ifdef MW2_FLOAT_SCALARS
	// The Matrox edition wraps the sum first, then adds the full turn.
	heading = FIXED_MOD360(FIXED_MOD360(p_mech->m_player->m_heading + p_mech->m_torsoTwist.m_value) + 360.0);
#else
	heading = FIXED_MOD360(p_mech->m_player->m_heading + FIXED_CONST(360) + p_mech->m_torsoTwist.m_value);
#endif
	p_mech->m_player->AUTOPILOT_TARGET.m_heading = heading;
}

#ifdef MW2_DESTROY_BLOWS_FIRST_BIN
// GBL's: blows the critical slot of the mech's first ammunition bin, whatever its ammunition; failing
// that, the pilot ejects when auto-eject is on or the mech isn't the local player's.
// FUNCTION: MW2GBL 0x1001ed96
// FUNCTION: MW2PREMERCS 0x1003d7e6
void DestroyMech(MechS32 p_killer, Mech* p_mech)
{
	MechS32 i;
	MechS32 j;
	MechSection* section;

	if (p_mech->m_ammoBins) {
		for (i = 0; i < 8; i++) {
			section = &p_mech->m_sections[i];
			if (section->m_flags & 0x2000) {
				continue;
			}

			for (j = 0; j < section->m_slotCount; j++) {
				if (section->m_slots[j] == ((AmmoBin*) p_mech->m_ammoBins)->m_id) {
					DestroyCriticalSlot(p_killer, p_mech, i + 1, j, 0);
					return;
				}
			}
		}

		if (g_autoEject || p_mech->m_player->m_index != g_localPlayerId) {
			p_mech->m_powerState = 5;
		}
	}

	if (p_mech->m_player->m_index == g_localPlayerId) {
		PlayCockpitSound(0, -1);
	}

	KillMech(p_killer, p_mech);
}
#else
// Destroys p_mech on behalf of player p_killer, unless g_mechPoweredUp is clear: an intact
// section holding an ammunition bin with ammunition left blows up first (DestroyCriticalSlot) instead.
// Matches but for the registers, which VC++ 2.2 allocates differently.
// FUNCTION: MW2 0x10007d06
// FUNCTION: MW2MATROX 0x1001059a
// FUNCTION: MW23DFX 0x10015e10
// FUNCTION: MW2ATI 0x10016220
// FUNCTION: MW2S3 0x1000de20
// FUNCTION: MW2POWERVR 0x10015160
void DestroyMech(MechS32 p_killer, Mech* p_mech)
{
	WeaponSlot* weapon;
	MechS32 i;
	MechS32 j;
	MechSection* section;
	AmmoBin* bin;
	MechS32 k;

#ifdef MW2_EXTRA_DEBUG_PRINTS
	DebugPrint("BlowAmmo\n");
#endif
	if (!g_mechPoweredUp) {
		return;
	}

	if (p_mech->m_ammoBins) {
		for (i = 0; i < 8; i++) {
			section = &p_mech->m_sections[i];
			if (section->m_flags & 0x2000) {
				continue;
			}

			for (j = 0; j < section->m_slotCount; j++) {
				if (section->m_slots[j] > 10000) {
					bin = p_mech->m_ammoBins;
					for (k = 0; k < p_mech->m_ammoBinCount; k++) {
						if (section->m_slots[j] == bin->m_id) {
							weapon = &p_mech->m_weapons[bin->m_weapon];
							if (weapon && weapon->m_ammo > 0) {
								DestroyCriticalSlot(p_killer, p_mech, i + 1, j, 0);
								return;
							}
						}

						bin++;
					}
				}
			}
		}
	}

	if (p_mech->m_player->m_index == g_localPlayerId) {
		PlayCockpitSound(0, -1);
	}

	KillMech(p_killer, p_mech);
}
#endif

#ifdef MW2_EXTERNAL_HEAT
// GBL's: the heat from outside joins the frame's, and an overheated mech dies after 1810 ticks (1.1:
// 4525). Its test for a mech shut down past 100 degrees, which would blow it up, comes after the
// test for 100 degrees and never holds.
// FUNCTION: MW2GBL 0x1001eeb7
// FUNCTION: MW2PREMERCS 0x1003d968
void CalculateHeat(Mech* p_mech)
{
	MechS32 delta;
	MechS32 cooling;
	MechS32 heat;
	MechS32 shift;
	MechS32 frameHeat;
	MechS32 unused;

	shift = 0;
	if ((p_mech->m_player->m_flags & 2) || (p_mech->m_player->m_flags & 4)) {
		return;
	}

	if (!g_difficulty->m_heatTracking && p_mech->m_player->m_index == g_localPlayerId) {
		p_mech->m_heat = 0;
		p_mech->m_deltaHeat = 0;
		return;
	}

	if (p_mech->m_powerState == 3) {
		shift = 1;
	}

	cooling = FIXED_SHL(p_mech->m_cooling * g_deltaTime, shift);
	p_mech->m_deltaHeat += p_mech->m_externalHeat;
	p_mech->m_externalHeat = 0;
	frameHeat = p_mech->m_deltaHeat;
	delta = frameHeat - cooling;
	p_mech->m_heat += delta;
	if (p_mech->m_heat < 0) {
		p_mech->m_heat = 0;
	}

	heat = FIXED_TO_INT(p_mech->m_heat);
	if ((p_mech->m_flags & 4) && !(p_mech->m_flags & 8) && p_mech->m_powerState != 3 &&
		g_currentClock - p_mech->m_stateTime > 1086) {
		p_mech->m_powerState = 3;
		p_mech->m_stateTime = g_currentClock;
		if (p_mech->m_player->m_index == g_localPlayerId) {
			PlayCockpitSound(13, -1);
		}
	}

	if (heat > 100) {
		if (p_mech->m_player->m_index == g_localPlayerId && g_difficulty->m_invulnerable) {
			return;
		}

		if (p_mech->m_flags & 8) {
			if (g_deltaTime && RandomIntBelow(4000 / g_deltaTime) < 3) {
				DestroyMech(p_mech->m_player->m_index, p_mech);
			}
		}
		else if (g_currentClock - p_mech->m_stateTime > 1810) {
			KillMech(p_mech->m_player->m_index, p_mech);
		}
	}
	else if (heat > 100 && p_mech->m_powerState == 3) {
		if (g_currentClock - p_mech->m_stateTime > 1810) {
			DestroyMech(p_mech->m_player->m_index, p_mech);
		}
	}
	else if (heat > FIXED_LITERAL(80.0, 80.0f) && !(p_mech->m_flags & 4)) {
		p_mech->m_flags |= 4;
		p_mech->m_stateTime = g_currentClock;
		if (!(p_mech->m_flags & 0x1000) && p_mech->m_player->m_index == g_localPlayerId) {
			p_mech->m_flags |= 0x1000;
			PlayCockpitSound(3, -1);
			PlaySoundEffect(0xe9, 100, 0x40, 5, 0x50);
		}
	}
	else if (heat > FIXED_LITERAL(65.0, 65.0f)) {
		if (p_mech->m_player->m_index == g_localPlayerId && !g_criticalHeatWarned && delta > 0) {
			g_criticalHeatWarningTime = g_currentClock;
			g_criticalHeatWarned = 1;
			PlayCockpitSound(0, -1);
			PlaySoundEffect(0xe9, 100, 0x40, 5, 0x50);
		}
	}
	else if (p_mech->m_flags & 4) {
		if (p_mech->m_powerState == 3 && g_currentClock - p_mech->m_stateTime > 724) {
			p_mech->m_flags &= ~4;
			p_mech->m_powerState = 0;
		}

		if (p_mech->m_flags & 8) {
			p_mech->m_flags &= ~8;
			p_mech->m_flags &= ~4;
		}
	}
	else {
		if (g_criticalHeatWarned && g_currentClock - g_criticalHeatWarningTime > 724 &&
			p_mech->m_player->m_index == g_localPlayerId) {
			g_criticalHeatWarned = 0;
		}

		p_mech->m_flags &= ~0x1000;
	}
}
#else
// Accumulates the mech's heat each frame (m_deltaHeat less its cooling, doubled while shut down)
// in m_heat, 16.16 percent: overheating (bit 4, above 80) shuts the mech down after 6 seconds
// (state 3), and above 100 the ammunition may explode (with bit 8) or the mech is destroyed after
// 25 seconds; the local player hears the warnings. Cooling below 65 ends the shutdown.
// FUNCTION: MW2 0x10007e86
// FUNCTION: MW2MATROX 0x10010727
// FUNCTION: MW23DFX 0x10015f50
// FUNCTION: MW2ATI 0x10016360
// FUNCTION: MW2S3 0x1000df60
// FUNCTION: MW2POWERVR 0x100152a0
void CalculateHeat(Mech* p_mech)
{
#ifdef MW2_FLOAT_SCALARS
	MechFloat delta;
	MechFloat cooling;
	MechFloat heat;
	MechFloat factor;

	factor = 1.0f;
#else
	MechS32 delta;
	MechS32 cooling;
	MechS32 heat;
	MechS32 shift;

	shift = 0;
#endif
	if ((p_mech->m_player->m_flags & 2) || (p_mech->m_player->m_flags & 4)) {
		return;
	}

	if (g_isNetworkGame && p_mech->m_player->m_index != g_localPlayerId) {
		return;
	}

	if (!g_difficulty->m_heatTracking && p_mech->m_player->m_index == g_localPlayerId) {
		p_mech->m_heat = 0;
		p_mech->m_deltaHeat = 0;
		return;
	}

#ifdef MW2_FLOAT_SCALARS
	// The Matrox edition doubles the cooling of a shut-down mech with a factor, not a shift.
	if (p_mech->m_powerState == 3) {
		factor = 2.0f;
	}

	cooling = p_mech->m_cooling * factor * (MechDouble) g_deltaTime;
	delta = p_mech->m_deltaHeat - cooling;
	p_mech->m_heat += delta;
	if (FIXED_IS_NEGATIVE(p_mech->m_heat)) {
		p_mech->m_heat = 0;
	}

	heat = p_mech->m_heat;
#else
	if (p_mech->m_powerState == 3) {
		shift = 1;
	}

	cooling = FIXED_SHL(p_mech->m_cooling * g_deltaTime, shift);
	delta = p_mech->m_deltaHeat - cooling;
	p_mech->m_heat += delta;
	if (p_mech->m_heat < 0) {
		p_mech->m_heat = 0;
	}

	heat = FIXED_TO_INT(p_mech->m_heat);
#endif
	if ((p_mech->m_flags & 4) && !(p_mech->m_flags & 8) && p_mech->m_powerState != 3 &&
		g_currentClock - p_mech->m_stateTime > 1086) {
		p_mech->m_powerState = 3;
		p_mech->m_stateTime = g_currentClock;
		if (p_mech->m_player->m_index == g_localPlayerId) {
			PlayCockpitSound(13, -1);
		}
	}

	if (heat > 100) {
		if (p_mech->m_player->m_index == g_localPlayerId && g_difficulty->m_invulnerable) {
			return;
		}

		if (p_mech->m_flags & 8) {
			if (g_deltaTime && RandomIntBelow(4000 / g_deltaTime) < 3) {
				DestroyMech(p_mech->m_player->m_index, p_mech);
			}
		}
		else if (g_currentClock - p_mech->m_stateTime > 4525) {
			KillMech(p_mech->m_player->m_index, p_mech);
		}
	}
	else if (heat > FIXED_LITERAL(80.0, 80.0f)) {
		if (!(p_mech->m_flags & 4)) {
			p_mech->m_stateTime = g_currentClock;
			p_mech->m_flags |= 4;
			if (!(p_mech->m_flags & 0x1000) && p_mech->m_player->m_index == g_localPlayerId) {
				p_mech->m_flags |= 0x1000;
				PlayCockpitSound(3, -1);
				PlaySoundEffect(0xe9, 100, 0x40, 5, 0x50);
			}
		}
	}
	else if (heat > FIXED_LITERAL(65.0, 65.0f)) {
		if (p_mech->m_player->m_index == g_localPlayerId && !g_criticalHeatWarned && delta > 0) {
			g_criticalHeatWarningTime = g_currentClock;
			g_criticalHeatWarned = 1;
			PlayCockpitSound(0, -1);
		}
	}
	else if (p_mech->m_flags & 4) {
		if (p_mech->m_powerState == 3 && g_currentClock - p_mech->m_stateTime > 724) {
			p_mech->m_flags &= ~4;
			p_mech->m_powerState = 0;
		}

		if (p_mech->m_flags & 8) {
			p_mech->m_flags &= ~8;
			p_mech->m_flags &= ~4;
		}
	}
	else {
		if (g_criticalHeatWarned && g_currentClock - g_criticalHeatWarningTime > 724 &&
			p_mech->m_player->m_index == g_localPlayerId) {
			g_criticalHeatWarned = 0;
		}

		p_mech->m_flags &= ~0x1000;
	}
}
#endif

#ifdef MW2_KILLS_BY_LAST_SHOOTER
// GBL's: a kill counts for the player whose shot hit last (g_lastHitShooter), the local player's
// outcome is 4 unless they ejected, and in a network game only the local mech's death is played
// out here.
// FUNCTION: MW2GBL 0x1001f3bd
// FUNCTION: MW2PREMERCS 0x1003de0d
void KillMech(MechS32 p_killer, Mech* p_mech)
{
	MechS32 leader;
	MechChar text[80];

	if (p_mech->m_player->m_index == g_localPlayerId && g_difficulty->m_invulnerable) {
		return;
	}

	if ((p_mech->m_player->m_flags & 2) || (p_mech->m_player->m_flags & 4)) {
		return;
	}

	p_mech->m_player->m_killer = p_killer;
	if (p_mech->m_player->m_killer >= 0 && p_mech->m_player->m_killer < 8 && p_mech->m_player->m_index < 8) {
		g_careerRecord.m_kills[p_mech->m_player->m_killer][p_mech->m_player->m_index]++;
	}

	if (p_mech->m_powerState == 5) {
		if (g_hostileAtmosphere) {
			if (p_mech->m_player->m_index == g_localPlayerId) {
				g_careerRecord.m_outcome = 4;
				PlayCockpitSound(0x20, -1);
			}
			p_mech->m_powerState = 4;
		}
		else if (p_mech->m_player->m_index == g_localPlayerId) {
			g_careerRecord.m_outcome = 2;
			PlayCockpitSound(0xc, -1);
		}
	}
	else if (p_mech->m_player->m_index == g_localPlayerId) {
		g_careerRecord.m_outcome = 4;
	}

	if (g_lastHitShooter != -1) {
		if (g_lastHitShooter == g_localPlayerId && (p_mech->m_player->m_flags & 0x1400)) {
			if (p_mech->m_player->m_type == c_playerTypeMech) {
				switch (GetPlayerSide(p_mech->m_player->m_index)) {
				case 0:
					g_careerRecord.m_directFriendlyMechKills++;
					break;
				case 1:
					g_careerRecord.m_directMechKills++;
					break;
				case 2:
					g_careerRecord.m_directNeutralMechKills++;
					break;
				}
			}
			else {
				switch (GetPlayerSide(p_mech->m_player->m_index)) {
				case 0:
					g_careerRecord.m_directFriendlyVehicleKills++;
					break;
				case 1:
					g_careerRecord.m_directVehicleKills++;
					break;
				case 2:
					g_careerRecord.m_directNeutralVehicleKills++;
					break;
				}
			}
		}

		g_lastHitShooter = -1;
	}

	if (p_mech->m_player->m_flags & 0x1400) {
		if (p_mech->m_player->m_type == c_playerTypeMech) {
			switch (GetPlayerSide(p_mech->m_player->m_index)) {
			case 0:
				g_careerRecord.m_friendlyMechKills++;
				break;
			case 1:
				g_careerRecord.m_mechKills++;
				break;
			case 2:
				g_careerRecord.m_neutralMechKills++;
				break;
			}
		}
		else {
			switch (GetPlayerSide(p_mech->m_player->m_index)) {
			case 0:
				g_careerRecord.m_friendlyVehicleKills++;
				break;
			case 1:
				g_careerRecord.m_vehicleKills++;
				break;
			case 2:
				g_careerRecord.m_neutralVehicleKills++;
				break;
			}
		}
	}

	if (g_players[g_localPlayerId]->m_team == p_mech->m_player->m_team) {
		if (p_mech->m_powerState == 5) {
			g_careerRecord.m_wingmenEjected++;
		}
		else {
			g_careerRecord.m_wingmenLost++;
		}
	}

	if (g_netRole && p_mech->m_player->m_index != g_localPlayerId) {
		return;
	}

	if (GetPlayerSide(p_mech->m_player->m_index) == 1) {
		switch (p_mech->m_player->m_type) {
		case 1:
			PlayCockpitSound(0x15, -1);
			break;
		case 2:
			PlayCockpitSound(0x16, -1);
			break;
		case 3:
			PlayCockpitSound(0x19, -1);
			break;
		case 4:
			PlayCockpitSound(0x16, -1);
			break;
		case 5:
			PlayCockpitSound(0x17, -1);
			break;
		case 8:
			PlayCockpitSound(0x17, -1);
			break;
		case 6:
		case 7:
			break;
		}
	}

	p_mech->m_player->m_flags |= 6;
	p_mech->m_stateTime = 0;
	p_mech->m_torsoObj = NULL;
	p_mech->m_pitchObj = NULL;
	if (!g_isNetworkGame) {
		if (p_mech->m_powerState != 5 || p_mech->m_player->m_index != g_localPlayerId) {
			p_mech->m_powerState = 4;
		}

		ResetAI(p_mech->m_player);
		if (GetTeamLeader(p_mech->m_player->m_team) == p_mech->m_player->m_index) {
			leader = ChooseTeamLeader(p_mech->m_player->m_team);
			RetargetGoals(p_mech->m_player->m_team, p_mech->m_player->m_index, leader);
			sprintf(text, "%6ld : New leader for group %d : %d\n", g_currentClock, p_mech->m_player->m_team, leader);
			WriteToMw2Log(text);
		}
	}
	else {
		p_mech->m_powerState = 4;
	}

	if (p_mech->m_player->m_index != g_localPlayerId && GetTeamLeader(p_mech->m_player->m_team) == g_localPlayerId) {
		SayLancemateReport(6, p_mech->m_player->m_slot);
	}

	LoadClassLevel(p_mech->m_player->m_index, 1);
}
#else
// Destroys p_mech, on behalf of player p_killer (-2: its player left the game).
// FUNCTION: MW2 0x1000832b
// FUNCTION: MW2MATROX 0x10010bd8
// FUNCTION: MW23DFX 0x10016280
// FUNCTION: MW2ATI 0x10016690
// FUNCTION: MW2S3 0x1000e290
// FUNCTION: MW2POWERVR 0x100155d0
void KillMech(MechS32 p_killer, Mech* p_mech)
{
	MechS32 leader;
	MechS32 next;
	MechChar text[80];

	if (p_mech->m_player->m_index == g_localPlayerId && g_difficulty->m_invulnerable) {
		return;
	}

	if ((p_mech->m_player->m_flags & 2) || (p_mech->m_player->m_flags & 4)) {
		return;
	}

	if (g_aimedShape && (g_aimedShape->m_kind & 0x100) && p_mech->m_player->m_index == g_aimedShape->m_owner) {
		g_aimedShape = NULL;
	}

	p_mech->m_player->m_killer = p_killer;
	if (p_mech->m_player->m_killer >= 0 && p_mech->m_player->m_killer < 8 && p_mech->m_player->m_index < 8) {
		g_careerRecord.m_kills[p_mech->m_player->m_killer][p_mech->m_player->m_index]++;
	}

	ChooseNetworkWinner();
	if (p_mech->m_powerState == 5) {
		if (g_hostileAtmosphere) {
			if (p_mech->m_player->m_index == g_localPlayerId) {
				g_careerRecord.m_outcome = 4;
				PlayCockpitSound(0x20, -1);
			}
			p_mech->m_powerState = 4;
		}
		else if (p_mech->m_player->m_index == g_localPlayerId) {
			g_careerRecord.m_outcome = 2;
			PlayCockpitSound(0xc, -1);
		}
	}

	if (p_killer == g_localPlayerId) {
		if (p_mech->m_player->m_type == c_playerTypeMech) {
			if (p_mech->m_player->m_index == g_localPlayerId) {
				g_killCount--;
			}
			else {
				g_killCount++;
			}

			switch (GetPlayerSide(p_mech->m_player->m_index)) {
			case 0:
				g_careerRecord.m_directFriendlyMechKills++;
				break;
			case 1:
				g_careerRecord.m_directMechKills++;
				break;
			case 2:
				g_careerRecord.m_directNeutralMechKills++;
				break;
			}
		}
		else {
			switch (GetPlayerSide(p_mech->m_player->m_index)) {
			case 0:
				g_careerRecord.m_directFriendlyVehicleKills++;
				break;
			case 1:
				g_careerRecord.m_directVehicleKills++;
				break;
			case 2:
				g_careerRecord.m_directNeutralVehicleKills++;
				break;
			}
		}
	}
	else {
		next = p_mech->m_player->m_index;
		next++;
	}

	if (p_mech->m_player->m_flags & 0x1400) {
		if (p_mech->m_player->m_type == c_playerTypeMech) {
			switch (GetPlayerSide(p_mech->m_player->m_index)) {
			case 0:
				g_careerRecord.m_friendlyMechKills++;
				break;
			case 1:
				g_careerRecord.m_mechKills++;
				break;
			case 2:
				g_careerRecord.m_neutralMechKills++;
				break;
			}
		}
		else {
			switch (GetPlayerSide(p_mech->m_player->m_index)) {
			case 0:
				g_careerRecord.m_friendlyVehicleKills++;
				break;
			case 1:
				g_careerRecord.m_vehicleKills++;
				break;
			case 2:
				g_careerRecord.m_neutralVehicleKills++;
				break;
			}
		}
	}

	if (g_players[g_localPlayerId]->m_team == p_mech->m_player->m_team) {
		if (p_mech->m_powerState == 5) {
			g_careerRecord.m_wingmenEjected++;
		}
		else {
			g_careerRecord.m_wingmenLost++;
		}
	}

	if (GetPlayerSide(p_mech->m_player->m_index) == 1) {
		switch (p_mech->m_player->m_type) {
		case 1:
			PlayCockpitSound(0x15, -1);
			break;
		case 2:
			PlayCockpitSound(0x16, -1);
			break;
		case 3:
			PlayCockpitSound(0x19, -1);
			break;
		case 4:
			PlayCockpitSound(0x16, -1);
			break;
		case 5:
			PlayCockpitSound(0x17, -1);
			break;
		case 8:
			PlayCockpitSound(0x17, -1);
			break;
		case 6:
		case 7:
			break;
		}
	}

	p_mech->m_player->m_flags |= 6;
	p_mech->m_stateTime = 0;
	if (!g_isNetworkGame) {
		if (p_mech->m_powerState != 5 || p_mech->m_player->m_index != g_localPlayerId) {
			p_mech->m_powerState = 4;
		}

		ResetAI(p_mech->m_player);
		if (GetTeamLeader(p_mech->m_player->m_team) == p_mech->m_player->m_index) {
			leader = ChooseTeamLeader(p_mech->m_player->m_team);
			RetargetGoals(p_mech->m_player->m_team, p_mech->m_player->m_index, leader);
			sprintf(text, "%6ld : New leader for group %d : %d\n", g_currentClock, p_mech->m_player->m_team, leader);
			WriteToMw2Log(text);
		}
	}
	else {
		p_mech->m_powerState = 4;
	}

	if (!g_isNetworkGame && p_mech->m_player->m_index != g_localPlayerId &&
		GetTeamLeader(p_mech->m_player->m_team) == g_localPlayerId) {
		SayLancemateReport(6, p_mech->m_player->m_slot);
	}

	LoadClassLevel(p_mech->m_player->m_index, 1);
}
#endif

// Calls DestroyCriticalSlot once for each of section p_section's m_slotCount.
// FUNCTION: MW2 0x10008938
// FUNCTION: MW2GBL 0x1001f9c1
// FUNCTION: MW2PREMERCS 0x1003e41c
// FUNCTION: MW2MATROX 0x100111e6
// FUNCTION: MW23DFX 0x10016600
// FUNCTION: MW2ATI 0x10016a10
// FUNCTION: MW2S3 0x1000e610
// FUNCTION: MW2POWERVR 0x10015950
void DestroySectionSlots(MechS32 p_attacker, Mech* p_mech, MechU32 p_section)
{
	MechS32 i;
	MechSection* section;
	MechS32 count;

	section = p_mech->m_sections + p_section - 1;
	count = section->m_slotCount;
	for (i = 0; i < count; i++) {
		DestroyCriticalSlot(p_attacker, p_mech, p_section, 0, 1);
	}
}

#ifdef MW2_HEAD_SHUTDOWN
// The early Mercenaries build's: a hit to the head (section 1) of an AI player's mech shuts it down
// (power state 5) its first ten times, and the head goes only after that, shutting it down for
// good (2).
// FUNCTION: MW2PREMERCS 0x1003e481
void DestroySection(MechS32 p_attacker, Mech* p_mech, MechU32 p_section)
{
	if (p_mech && p_section == 1 && p_mech->m_player->m_index != g_localPlayerId && p_mech->m_headHits < 10) {
		p_mech->m_powerState = 5;
		p_mech->m_headHits++;
	}
	else {
		if (p_section == 1) {
			p_mech->m_powerState = 2;
		}

		DestroySectionEx(p_attacker, p_mech, p_section, 0);
	}
}

// 1.1's DestroySection, which with p_quiet takes the sections' objects out of the scene
// (DestroyPartTree) instead of blowing them off, and doesn't kill the mech. A leg's first loss
// cripples an AI mech instead, 95 times in 100, and in a game that isn't a network game gives
// m_unk0x120 a random value.
// FUNCTION: MW2PREMERCS 0x1003e50c
void DestroySectionEx(MechS32 p_attacker, Mech* p_mech, MechU32 p_section, MechS32 p_quiet)
{
	MechSection* section;
	MechS32 roll;

	if (!p_section) {
		return;
	}

	section = p_mech->m_sections + p_section - 1;
	if (section->m_flags & 0x2000) {
		return;
	}

	section->m_internal = 0;
	section->m_flags |= 0x2000;
	DestroySectionSlots(p_attacker, p_mech, p_section);
	switch (p_section) {
	case 2:
		DestroySectionEx(p_attacker, p_mech, 5, 0);
		section->m_flags &= ~0x2000;
		return;
	case 4:
		DestroySectionEx(p_attacker, p_mech, 6, 0);
		section->m_flags &= ~0x2000;
		return;
	case 7:
	case 8:
		if (!(p_mech->m_flags & 0x20)) {
			roll = RandomIntBelow(100);
			if (roll < 0x5f && !p_quiet && p_mech->m_player->m_type == c_playerTypeMech) {
				section->m_internal = FIXED_CONST(1);
				section->m_flags &= ~0x2000;
				p_mech->m_mobility = 0x3333;
				return;
			}

			if (!p_quiet) {
				BlowOffPart(p_mech->m_player->m_obj, p_section);
			}
			else {
				DestroyPartTree(p_mech->m_player->m_obj, p_section);
			}

			p_mech->m_mobility = 0;
			p_mech->m_flags |= 0x20;
			if (p_mech->m_player->m_type == c_playerTypeMech && !g_isNetworkGame) {
				p_mech->m_unk0x112 |= 4;
				p_mech->m_unk0x120 = (RandomNormal() << 16) / 1024;
			}
			return;
		}
		else {
			DestroySectionEx(p_attacker, p_mech, 3, 0);
		}
		break;
	case 3:
		DestroySectionEx(p_attacker, p_mech, 5, 0);
		DestroySectionEx(p_attacker, p_mech, 2, 0);
		DestroySectionEx(p_attacker, p_mech, 4, 0);
		DestroySectionEx(p_attacker, p_mech, 6, 0);
		DestroySectionEx(p_attacker, p_mech, 1, 0);
		break;
	case 5:
	case 6:
		if (!p_quiet) {
			BlowOffPart(p_mech->m_player->m_obj, p_section);
		}
		else {
			DestroyPartTree(p_mech->m_player->m_obj, p_section);
		}
		return;
	}

	if (!p_quiet) {
		BlowOffPart(p_mech->m_player->m_obj, p_section);
		if (p_mech->m_powerState != 4 && p_mech->m_powerState != 5 &&
			(p_mech->m_player->m_index == g_localPlayerId || !g_isNetworkGame)) {
			KillMech(p_attacker, p_mech);
		}
	}
	else {
		DestroyPartTree(p_mech->m_player->m_obj, p_section);
	}
}
#else
// Destroys section p_section of p_mech, on behalf of player p_attacker.
// Destroying section 3 takes sections 1, 2 and 4 to 6 with it; section 2 takes 5, and 4 takes 6.
// Of sections 7 and 8, the first to go only marks the mech (0x20); the second destroys 1 and 3.
// FUNCTION: MW2 0x1000899d
// FUNCTION: MW2GBL 0x1001fa26
// FUNCTION: MW2MATROX 0x1001124b
// FUNCTION: MW23DFX 0x10016640
// FUNCTION: MW2ATI 0x10016a50
// FUNCTION: MW2S3 0x1000e650
// FUNCTION: MW2POWERVR 0x10015990
void DestroySection(MechS32 p_attacker, Mech* p_mech, MechU32 p_section)
{
	MechSection* section;

	if (!p_section) {
		return;
	}

	section = p_mech->m_sections + p_section - 1;
	if (section->m_flags & 0x2000) {
		return;
	}

	section->m_internal = 0;
	section->m_flags |= 0x2000;
	DestroySectionSlots(p_attacker, p_mech, p_section);
	switch (p_section) {
	case 2:
		DestroySection(p_attacker, p_mech, 5);
		section->m_flags &= ~0x2000;
		return;
	case 4:
		DestroySection(p_attacker, p_mech, 6);
		section->m_flags &= ~0x2000;
		return;
	case 7:
	case 8:
		if (!(p_mech->m_flags & 0x20)) {
			BlowOffPart(p_mech->m_player->m_obj, p_section);
			p_mech->m_mobility = 0;
			p_mech->m_flags |= 0x20;
			return;
		}
		else {
			DestroySection(p_attacker, p_mech, 1);
			DestroySection(p_attacker, p_mech, 3);
		}
		break;
	case 3:
		DestroySection(p_attacker, p_mech, 5);
		DestroySection(p_attacker, p_mech, 2);
		DestroySection(p_attacker, p_mech, 4);
		DestroySection(p_attacker, p_mech, 6);
		DestroySection(p_attacker, p_mech, 1);
		break;
	case 5:
	case 6:
		BlowOffPart(p_mech->m_player->m_obj, p_section);
		return;
	}

	BlowOffPart(p_mech->m_player->m_obj, p_section);
#ifdef MW2_KILLS_BY_LAST_SHOOTER
	if (p_mech->m_powerState != 4 && p_mech->m_powerState != 5) {
#else
	if (p_mech->m_powerState != 4 && p_mech->m_powerState != 5 &&
		(p_mech->m_player->m_index == g_localPlayerId || !g_isNetworkGame)) {
#endif
		KillMech(p_attacker, p_mech);
	}
}
#endif

// Destroys critical slot p_slot of section p_section of p_mech, on behalf of player p_attacker,
// and drops it from the section: a weapon (ids below 5000) stops working, an ammunition bin (above
// 10000) explodes with its shots, damaging the section, and the equipment loses its function (the
// jump jets, the heat sinks, the engine, the gyro...). The local player hears which, unless
// p_recursing; 8000 and 9000 hit another, random slot instead.
// FUNCTION: MW2 0x10008c0f
// FUNCTION: MW2GBL 0x1001fc77
// FUNCTION: MW2PREMERCS 0x1003e8a1
// FUNCTION: MW2MATROX 0x100114bd
// FUNCTION: MW23DFX 0x100167b0
// FUNCTION: MW2ATI 0x10016bc0
// FUNCTION: MW2S3 0x1000e7c0
// FUNCTION: MW2POWERVR 0x10015b00
void DestroyCriticalSlot(MechS32 p_attacker, Mech* p_mech, MechU32 p_section, MechS32 p_slot, MechS32 p_recursing)
{
	MechS32 id;
	WeaponSlot* weapon;
	MechScalar x;
	MechScalar y;
	MechS32 i;
	MechScalar z;
	struct SceneObject* obj;
	MechS32 kind;
	MechSection* section;
	AmmoBin* bin;
	MechScalar damage;

	section = p_mech->m_sections + p_section - 1;
	if (!section) {
		return;
	}

	if (p_slot >= section->m_slotCount || p_slot < 0) {
		return;
	}

	id = section->m_slots[p_slot];
	kind = id / 100;
	if (kind < 50) {
		weapon = p_mech->m_weapons;
		for (i = 0; i < WEAPON_SLOT_COUNT; i++) {
			if (weapon->m_type < 0) {
				weapon++;
				continue;
			}

			if (weapon->m_slotId == id) {
				if (!p_recursing && p_mech->m_player->m_index == g_localPlayerId && weapon->m_status) {
					switch (kind) {
					case 0:
					case 1:
					case 2:
					case 3:
						SayCriticalHit(0x16);
						break;
					case 4:
					case 5:
					case 6:
						SayCriticalHit(0x17);
						break;
					case 7:
					case 8:
					case 9:
						SayCriticalHit(0x18);
						break;
					case 10:
						SayCriticalHit(0x19);
						break;
					case 11:
						SayCriticalHit(0x1a);
						break;
					case 12:
					case 13:
					case 14:
					case 15:
						SayCriticalHit(0x1b);
						break;
					case 16:
					case 17:
					case 18:
					case 19:
						SayCriticalHit(0x1c);
						break;
					case 20:
#ifdef MW2_MORE_CRITICAL_HITS
						SayCriticalHit(0);
#endif
						break;
#ifdef MW2_MORE_CRITICAL_HITS
					case 29:
						SayCriticalHit(3);
						break;
#endif
					case 21:
						SayCriticalHit(0x1d);
						break;
					case 22:
						SayCriticalHit(0x1e);
						break;
					case 23:
						SayCriticalHit(0x1f);
						break;
					case 24:
						SayCriticalHit(0x20);
						break;
					case 25:
						SayCriticalHit(0x21);
						break;
					case 26:
						SayCriticalHit(0x22);
						break;
					case 27:
						SayCriticalHit(0x23);
						break;
					}

					if (p_mech->m_powerState == 2) {
						PlaySoundEffect(0xd0, 100, 0x40, 5, 0x50);
					}

					SayCriticalHit(0);
				}

				weapon->m_status = 0;
				weapon->m_state = c_weaponEmpty;
				break;
			}

			weapon++;
		}
	}
	else if (kind > 100) {
		bin = p_mech->m_ammoBins;
		for (i = 0; i < p_mech->m_ammoBinCount; i++) {
			if (bin->m_id == id) {
				weapon = &p_mech->m_weapons[bin->m_weapon];
#ifndef MW2_DESTROY_BLOWS_FIRST_BIN
				if (weapon->m_ammo < 1) {
					break;
				}
#endif

				if (weapon) {
					weapon->m_ammo -= bin->m_shots;
					if (!weapon->m_ammo) {
						weapon->m_status = 0;
						weapon->m_state = c_weaponEmpty;
					}
				}

				if (!p_recursing && bin->m_shots) {
					if (p_mech->m_player->m_index == g_localPlayerId) {
						if (p_mech->m_powerState == 2) {
							PlaySoundEffect(0xd0, 100, 0x40, 5, 0x50);
						}

						x = p_mech->m_player->m_position.m_x - g_eyepoint->m_x;
						y = p_mech->m_player->m_position.m_y - g_eyepoint->m_y;
						z = p_mech->m_player->m_position.m_z - g_eyepoint->m_z;
						g_hitFadePending = 1;
						PlaySoundAt(x, y, z, 0xbc, g_inCockpitView);
						SayCriticalHit(1);
					}

					obj = FindObjByPart(p_mech->m_player->m_obj, p_section);
					if (!obj) {
						obj = FindObjByPart(p_mech->m_player->m_obj, 3);
					}

					if (obj) {
						GetShapeBounds(GetObjShape(obj), &x, &y, &z);
						SpawnEffect(p_attacker, 7, x, y, z, x, y, z);
					}

					damage = bin->m_shots * bin->m_damage;
					section->m_internal -= FIXED_FROM_INT(damage);
					if (FIXED_IS_NEGATIVE(section->m_internal)) {
						section->m_internal = 0;
						section->m_slots[p_slot] = 0;
						DestroySection(p_attacker, p_mech, p_section);
					}

#ifdef MW2_DESTROY_BLOWS_FIRST_BIN
					if (g_autoEject || p_mech->m_player->m_index != g_localPlayerId) {
#else
					if (g_autoEject && (p_mech->m_player->m_index == g_localPlayerId || !g_isNetworkGame)) {
#endif
						p_mech->m_powerState = 5;
						KillMech(p_attacker, p_mech);
					}
				}

				bin->m_shots = 0;
			}

			bin++;
		}
	}
	else {
		kind = id / 10 * 10;
		switch (kind) {
		case 9000:
			if (!p_recursing) {
				DestroyCriticalSlot(p_attacker, p_mech, p_section, RandomIntBelow(section->m_slotCount), 1);
				return;
			}
			else {
				break;
			}
		case 8000:
			if (!p_recursing) {
				DestroyCriticalSlot(p_attacker, p_mech, p_section, RandomIntBelow(section->m_slotCount), 1);
				return;
			}
			else {
				break;
			}
		case 7000:
			if (!p_recursing && p_mech->m_player->m_index == g_localPlayerId && p_mech->m_jumpFuel != 2) {
				if (p_mech->m_powerState == 2) {
					PlaySoundEffect(0xd0, 100, 0x40, 5, 0x50);
				}

				SayCriticalHit(4);
			}

			if (p_mech->m_jumpJets > 0) {
				p_mech->m_jumpThrust -= (MechScalar) (p_mech->m_jumpThrust / p_mech->m_jumpJets);
			}
			else {
				p_mech->m_jumpThrust = 0;
			}

			if (p_mech->m_jumpJets > 0) {
				p_mech->m_jumpJets--;
			}

			if (!p_mech->m_jumpJets) {
				p_mech->m_jumpFuel = -2;
			}
			break;
		case 6000:
			if (!p_recursing && p_mech->m_player->m_index == g_localPlayerId && p_mech->m_cooling) {
				if (p_mech->m_powerState == 2) {
					PlaySoundEffect(0xd0, 100, 0x40, 5, 0x50);
				}

				SayCriticalHit(5);
			}

			if (p_mech->m_cooling > 0) {
				p_mech->m_cooling -= FIXED_RAW(50);
			}
			else {
				p_mech->m_cooling = 0;
			}
			break;
		case 5900:
			if (!p_recursing && p_mech->m_player->m_index == g_localPlayerId) {
				if (p_mech->m_powerState == 2) {
					PlaySoundEffect(0xd0, 100, 0x40, 5, 0x50);
				}

				SayCriticalHit(6);
			}

#ifdef MW2_KILLS_BY_LAST_SHOOTER
			if (g_hostileAtmosphere) {
#else
			if (g_hostileAtmosphere && (p_mech->m_player->m_index == g_localPlayerId || !g_isNetworkGame)) {
#endif
				KillMech(p_attacker, p_mech);
			}
			break;
		case 5850:
			if (!p_recursing && p_mech->m_player->m_index == g_localPlayerId) {
				if (p_mech->m_powerState == 2) {
					PlaySoundEffect(0xd0, 100, 0x40, 5, 0x50);
				}

				SayCriticalHit(7);
			}

			p_mech->m_mobility -= FIXED_RAW(0x199a);
			if (FIXED_IS_NEGATIVE(p_mech->m_mobility)) {
				p_mech->m_mobility = 0;
			}
			break;
		case 5800:
			if (!p_recursing && p_mech->m_player->m_index == g_localPlayerId) {
				if (p_mech->m_powerState == 2) {
					PlaySoundEffect(0xd0, 100, 0x40, 5, 0x50);
				}

				SayCriticalHit(8);
			}

			p_mech->m_mobility -= FIXED_RAW(0x199a);
			if (FIXED_IS_NEGATIVE(p_mech->m_mobility)) {
				p_mech->m_mobility = 0;
			}

			p_mech->m_jumpFuel = -2;
			p_mech->m_jumpThrust = 0;
			p_mech->m_jumpJets = 0;
			break;
		case 5750:
#ifdef MW2_KILLS_BY_LAST_SHOOTER
			KillMech(p_attacker, p_mech);
#else
			if (p_mech->m_player->m_index == g_localPlayerId || !g_isNetworkGame) {
				KillMech(p_attacker, p_mech);
			}
#endif
			break;
		case 5700:
			if (!p_recursing && p_mech->m_player->m_index == g_localPlayerId) {
				if (p_mech->m_powerState == 2) {
					PlaySoundEffect(0xd0, 100, 0x40, 5, 0x50);
				}

				SayCriticalHit(9);
			}

			DamageCockpitPanels(p_mech, 1);
			break;
		case 5550:
		case 5600:
		case 5650:
			if (!p_recursing && p_mech->m_player->m_index == g_localPlayerId) {
				if (p_mech->m_powerState == 2) {
					PlaySoundEffect(0xd0, 100, 0x40, 5, 0x50);
				}

				SayCriticalHit(10);
			}

			p_mech->m_mobility -= FIXED_RAW(0x199a);
			if (FIXED_IS_NEGATIVE(p_mech->m_mobility)) {
				p_mech->m_mobility = 0;
			}
			break;
		case 5500:
			if (!p_recursing && p_mech->m_player->m_index == g_localPlayerId) {
				if (p_mech->m_powerState == 2) {
					PlaySoundEffect(0xd0, 100, 0x40, 5, 0x50);
				}

				SayCriticalHit(11);
			}

			p_mech->m_mobility -= FIXED_RAW(0x199a);
			if (FIXED_IS_NEGATIVE(p_mech->m_mobility)) {
				p_mech->m_mobility = 0;
			}
			break;
		case 5350:
		case 5400:
		case 5450:
			if (!p_recursing && p_mech->m_player->m_index == g_localPlayerId) {
				if (p_mech->m_powerState == 2) {
					PlaySoundEffect(0xd0, 100, 0x40, 5, 0x50);
				}

				SayCriticalHit(12);
			}

			DamageCockpitPanels(p_mech, 0);
			break;
		case 5300:
			if (!p_recursing && p_mech->m_player->m_index == g_localPlayerId) {
				if (p_mech->m_powerState == 2) {
					PlaySoundEffect(0xd0, 100, 0x40, 5, 0x50);
				}

				SayCriticalHit(9);
			}

			DamageCockpitPanels(p_mech, 0);
			break;
		}

#ifdef MW2_FLOAT_SCALARS
		if (FIXED_IS_NONZERO(p_mech->m_mobility) && p_mech->m_mobility < 0.4f) {
			p_mech->m_mobility = 0.4f;
		}
#else
		if (p_mech->m_mobility && p_mech->m_mobility < 0x6666) {
			p_mech->m_mobility = 0x6666;
		}
#endif
	}

	for (i = p_slot; i < section->m_slotCount - 1; i++) {
		section->m_slots[i] = section->m_slots[i + 1];
	}

	section->m_slots[i] = 0;
	section->m_slotCount--;
}

#ifdef MW2_KILLS_BY_LAST_SHOOTER
// GBL's: counts the hit for the player whose shot hit last (g_lastHitShooter), then forgets them,
// and in a network game damages only the local mech.
// FUNCTION: MW2GBL 0x10020941
// FUNCTION: MW2PREMERCS 0x1003f712
void ApplyDamageToMech(MechS32 p_attacker, Mech* p_mech, MechScalar p_damage, MechS32 p_section)
{
	MechS32 side;
	MechU32 levels;
	MechS32 rear;
	MechS32 i;
	MechS32 level;
	MechS32 front;
	MechS32 roll;
	MechSection* section;

	side = 0;
	front = TRUE;
	rear = FALSE;
	level = 0;
	roll = 0;
	if (p_mech->m_player->m_index == g_localPlayerId && g_difficulty->m_invulnerable) {
		return;
	}

	if (g_lastHitShooter != -1) {
		if (g_lastHitShooter == g_localPlayerId) {
			switch (GetPlayerSide(p_mech->m_player->m_index)) {
			case 0:
				g_careerRecord.m_friendlyHits++;
				break;
			case 1:
				g_careerRecord.m_hits++;
				break;
			case 2:
				g_careerRecord.m_neutralHits++;
				break;
			}
		}

		if (g_players[g_localPlayerId]->m_team == g_players[g_lastHitShooter]->m_team) {
			switch (GetPlayerSide(p_mech->m_player->m_index)) {
			case 0:
				g_careerRecord.m_teamFriendlyHits++;
				break;
			case 1:
				g_careerRecord.m_teamHits++;
				break;
			case 2:
				g_careerRecord.m_teamNeutralHits++;
				break;
			}
		}

		if (p_mech->m_player->m_index == g_localPlayerId) {
			g_careerRecord.m_hitsTaken++;
		}

		if (g_players[g_localPlayerId]->m_team == p_mech->m_player->m_team) {
			g_careerRecord.m_teamHitsTaken++;
		}
	}

	if (g_netRole && p_mech->m_player->m_index != g_localPlayerId) {
		return;
	}

	if (p_damage <= 0) {
		return;
	}

	if (p_section & 0x8000) {
		rear = TRUE;
		p_section &= ~0x8000;
	}

	if (p_section < 1 || p_section > 8) {
		return;
	}

	if (p_section == 3) {
		p_section = RandomIntBelow(3);
		switch (p_section) {
		case 0:
			p_section = 2;
			break;
		case 1:
			p_section = 3;
			break;
		case 2:
			p_section = 4;
			break;
		}
	}

	section = p_mech->m_sections + p_section - 1;
	if (rear && (p_section == 2 || p_section == 4 || p_section == 3)) {
		side = 1;
		front = FALSE;
		levels = (section->m_flags & 0xf0) >> 4;
	}
	else {
		levels = section->m_flags & 0xf;
	}

	section->m_armor[side] -= p_damage;
	section->m_flags |= 0x8000;
	if (section->m_armor[side] <= 0) {
		if (!(section->m_flags & 0x4000) && p_mech->m_player->m_index == g_localPlayerId && p_mech->m_powerState == 2) {
			PlaySoundEffect(0xec, 100, 0x40, 5, 0x50);
		}

		section->m_flags |= 0x4000;
		section->m_internal += section->m_armor[side];
		section->m_armor[side] = 0;
		if (p_mech->m_player->m_index == g_localPlayerId && (p_section == 1 || p_section == 3) &&
			p_mech->m_powerState != 4 && p_damage > FIXED_RAW(0x20000)) {
			g_hitFadePending = 1;
		}

		if (section->m_internal <= 0) {
			DestroySection(p_attacker, p_mech, p_section);
			return;
		}
		else {
			for (i = 0; i < RandomIntBelow(5); i++) {
				roll = RandomIntBelow(100);
				if (roll < 20 && section->m_slotCount > 0) {
					if (roll == 12) {
						DestroySection(p_attacker, p_mech, p_section);
						return;
					}
					else {
						DestroyCriticalSlot(p_attacker, p_mech, p_section, RandomIntBelow(section->m_slotCount), 0);
					}
				}
			}
		}
	}

	if (levels) {
		if (p_mech->m_player->m_index == g_localPlayerId) {
			level = 15 - (MechS32) (((section->m_internal + section->m_armor[side] / g_localArmorPerLevel) * 3) /
									(MechS32) FIXED_FROM_INT(levels));
		}
		else {
			level = 15 - (MechS32) (((section->m_internal + section->m_armor[side] / g_otherArmorPerLevel) * 3) /
									(MechS32) FIXED_FROM_INT(levels));
		}
	}

	RaisePartDamageLevel(p_mech->m_player->m_obj, level, p_section);
	g_lastHitShooter = -1;
	return;
}
#else
// Deals p_damage (16.16) to section p_section of the mech, on behalf of player p_attacker: to its
// rear armor with bit 0x8000 (sections 2 to 4), and past the armor to the internal structure,
// which may destroy the section or hit its critical slots. Section 3 stands for a random one of
// 2 to 4. The local player takes none while invulnerable, and in a network game each machine
// only damages its own mech.
// The original ends in an explicit return (the jmp to the epilogue).
// FUNCTION: MW2 0x1000991b
// FUNCTION: MW2MATROX 0x10012243
// FUNCTION: MW23DFX 0x100171d0
// FUNCTION: MW2ATI 0x100175d0
// FUNCTION: MW2S3 0x1000f1b0
// FUNCTION: MW2POWERVR 0x10016510
void ApplyDamageToMech(MechS32 p_attacker, Mech* p_mech, MechScalar p_damage, MechS32 p_section)
{
	MechS32 side;
	MechU32 levels;
	MechS32 rear;
	MechS32 i;
	MechS32 level;
	MechS32 front;
	MechS32 roll;
	MechSection* section;

	side = 0;
	front = TRUE;
	rear = FALSE;
	level = 0;
	roll = 0;
	if (p_mech->m_player->m_index == g_localPlayerId && g_difficulty->m_invulnerable) {
		return;
	}

	if (!g_mechPoweredUp) {
		return;
	}

	if (g_isNetworkGame && p_mech->m_player->m_index != g_localPlayerId) {
		return;
	}

#ifdef MW2_FLOAT_SCALARS
	if (p_damage < 1e-07f) {
#else
	if (p_damage <= 0) {
#endif
		return;
	}

	if (p_section & 0x8000) {
		rear = TRUE;
		p_section &= ~0x8000;
	}

	if (p_section < 1 || p_section > 8) {
		return;
	}

	if (p_section == 3) {
		p_section = RandomIntBelow(3);
		switch (p_section) {
		case 0:
			p_section = 2;
			break;
		case 1:
			p_section = 3;
			break;
		case 2:
			p_section = 4;
			break;
		}
	}

	section = p_mech->m_sections + p_section - 1;
	if (rear && (p_section == 2 || p_section == 4 || p_section == 3)) {
		side = 1;
		front = FALSE;
		levels = (section->m_flags & 0xf0) >> 4;
	}
	else {
		levels = section->m_flags & 0xf;
	}

	section->m_armor[side] -= p_damage;
	section->m_flags |= 0x8000;
#ifdef MW2_FLOAT_SCALARS
	if (section->m_armor[side] < 1e-07f) {
#else
	if (section->m_armor[side] <= 0) {
#endif
		if (!(section->m_flags & 0x4000) && p_mech->m_player->m_index == g_localPlayerId && p_mech->m_powerState == 2) {
			PlaySoundEffect(0xec, 100, 0x40, 5, 0x50);
		}

		section->m_flags |= 0x4000;
		section->m_internal += section->m_armor[side];
		section->m_armor[side] = 0;
		if (p_mech->m_player->m_index == g_localPlayerId && (p_section == 1 || p_section == 3) &&
			p_mech->m_powerState != 4 && p_damage > FIXED_RAW(0x20000)) {
			g_hitFadePending = 1;
		}

#ifdef MW2_FLOAT_SCALARS
		if (section->m_internal < 1e-07f) {
#else
		if (section->m_internal <= 0) {
#endif
			DestroySection(p_attacker, p_mech, p_section);
			return;
		}
		else {
			for (i = 0; i < RandomIntBelow(5); i++) {
				roll = RandomIntBelow(100);
				if (roll < 20 && section->m_slotCount > 0) {
					if (roll == 12) {
						DestroySection(p_attacker, p_mech, p_section);
						return;
					}
					else {
						DestroyCriticalSlot(p_attacker, p_mech, p_section, RandomIntBelow(section->m_slotCount), 0);
					}
				}
			}
		}
	}

	if (levels) {
		if (p_mech->m_player->m_index == g_localPlayerId) {
			level = 15 - (MechS32) (((section->m_internal + section->m_armor[side] / g_localArmorPerLevel) * 3) /
									(MechS32) FIXED_FROM_INT(levels));
		}
		else {
#if defined(MW2_ATI) || defined(MW2_S3) || defined(MW2_POWERVR) || defined(MW2_3DFX)
			// The ATI, S3, PowerVR and 3Dfx editions divide before they multiply here.
			level = 15 - (MechS32) (((section->m_internal + section->m_armor[side] / g_otherArmorPerLevel) /
									 (MechS32) FIXED_FROM_INT(levels)) *
									3);
#else
			level = 15 - (MechS32) (((section->m_internal + section->m_armor[side] / g_otherArmorPerLevel) * 3) /
									(MechS32) FIXED_FROM_INT(levels));
#endif
		}
	}

	RaisePartDamageLevel(p_mech->m_player->m_obj, level, p_section);
	return;
}
#endif

// Ejects from p_mech, unless it is already shutting down or ejecting: the local player (p_eject)
// ejects with a sound, or hears that the ejection system is disabled, and the mech is destroyed.
// FUNCTION: MW2 0x10009d2a
// FUNCTION: MW2GBL 0x10020e94
// FUNCTION: MW2PREMERCS 0x1003fb21
// FUNCTION: MW2MATROX 0x100126e4
// FUNCTION: MW23DFX 0x100174f0
// FUNCTION: MW2ATI 0x100178f0
// FUNCTION: MW2S3 0x1000f470
// FUNCTION: MW2POWERVR 0x10016830
void EjectPlayer(Mech* p_mech, MechS32 p_eject)
{
	if (p_mech->m_powerState == 4 || p_mech->m_powerState == 5) {
		return;
	}

	if (p_eject && p_mech->m_player->m_index == g_localPlayerId) {
		if (!g_hostileAtmosphere) {
			p_mech->m_powerState = 5;
			PlaySoundEffect(0xc5, 100, 0x40, 5, 0x32);
		}
		else {
			PlayCockpitSound(0x20, -1);
		}
	}
#ifdef MW2_SELF_DESTRUCT_MESSAGE
	else if (p_mech->m_player->m_index == g_localPlayerId) {
		PlayCockpitSound(4, -1);
	}
#endif

	KillMech(p_mech->m_player->m_index, p_mech);
	return;
}

// Toggles the local player's object between HideObjTree and ShowObjTree.
// FUNCTION: MW2 0x10009dd2
// FUNCTION: MW2GBL 0x10020f61
// FUNCTION: MW2PREMERCS 0x1003fbee
// FUNCTION: MW2MATROX 0x1001278c
// FUNCTION: MW23DFX 0x10017560
void ToggleLocalMechVisible(void)
{
	if (!g_localMechHidden) {
		HideObjTree(g_players[g_localPlayerId]->m_obj);
	}
	else {
		ShowObjTree(g_players[g_localPlayerId]->m_obj);
	}

	g_localMechHidden = !g_localMechHidden;
}
