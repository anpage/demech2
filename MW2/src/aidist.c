/* AiMessageDist, which the Matrox edition places in another order in ai.c's object: ai.c includes this
   file at 1.1's position, and at the Matrox edition's. */
#include "ai.h"
#include "aiweapons.h"
#include "clock.h"
#include "debugbreak.h"
#include "decomp.h"
#include "eyepoint.h"
#include "fixedfloat.h"
#include "fixedmul.h"
#include "geocache.h"
#include "lineofsight.h"
#include "loadres.h"
#include "maneuvers.h"
#include "mw2log.h"
#include "mw2prj.h"
#include "network.h"
#include "objective.h"
#include "overlay.h"
#include "players.h"
#include "random.h"
#include "shots.h"
#include "simmain.h"
#include "speech.h"
#include "targeting.h"
#include "team.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>

// M_DIST: the farthest of p_target's targets farther than p_arg hundreds (-4: p_player's leash
// range, -2: its alert range).
// Operand order: the final test (limit < farthest) compares the other way in the original.
// Stack-slot permutation: farthest, farthestTarget, limit and result and target.
// MW2MATROX: the same comparison, and a stack-slot permutation of its own.
// FUNCTION: MW2 0x10052617
// FUNCTION: MW2MATROX 0x10081ce3
MechS16 AiMessageDist(Player* p_player, MechS16 p_target, MechS16 p_arg)
{
	MechS16 farthestTarget;
	MechS16 result;
	MechS32 limit;
	MechS32 farthest;
	MechS16 target;

	result = 0;
	farthestTarget = -1;
	target = -1;

	if (p_arg == -3) {
		limit = 0;
	}
	else if (p_arg == -4) {
		limit = p_player->m_leashRange;
	}
	else if (p_arg == -2) {
		limit = p_player->m_alertRange;
	}
	else if (p_arg == 0) {
		limit = GetTargetRange(p_target);
	}
	else {
		limit = p_arg * 100;
	}

	farthest = limit;
	while ((target = NextTarget(p_player, p_target, target)) != -1) {
		if (!IsTargetDone(target, 2) && SetTarget(p_player, target) &&
			farthest < SCALAR_TO_INT(p_player->m_targetInfo.m_distance)) {
			farthest = SCALAR_TO_INT(p_player->m_targetInfo.m_distance);
			farthestTarget = target;
		}
	}

	if (limit < farthest && farthestTarget != -1) {
		result = farthestTarget;
	}

	return result;
}
