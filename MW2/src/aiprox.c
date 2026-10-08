/* AiMessageProx, which the Matrox edition places in another order in ai.c's object: ai.c includes this
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

// M_PROX: the nearest of p_target's targets closer than p_arg hundreds.
// Operand order: the final test (nearest < limit) compares with nearest in eax in the original.
// Stack-slot permutation: limit, nearest, nearestTarget and result and target.
// MW2MATROX: the same comparison, and a stack-slot permutation of its own.
// FUNCTION: MW2 0x10052445
// FUNCTION: MW2MATROX 0x10081676
MechS16 AiMessageProx(Player* p_player, MechS16 p_target, MechS16 p_arg)
{
	MechS16 nearestTarget;
	MechS16 result;
	MechS32 limit;
	MechS32 nearest;
	MechS16 target;

	result = 0;
	nearestTarget = -1;
	target = -1;

	if (p_arg == -3) {
		limit = 100000000;
	}
	else {
		limit = p_arg * 100;
	}

	nearest = limit;
	while ((target = NextTarget(p_player, p_target, target)) != -1) {
		if (!IsTargetDone(target, 2) && SetTarget(p_player, target) &&
			SCALAR_TO_INT(p_player->m_targetInfo.m_distance) < nearest) {
			nearest = SCALAR_TO_INT(p_player->m_targetInfo.m_distance);
			nearestTarget = target;
		}
	}

	if (nearest < limit && nearestTarget != -1) {
		result = nearestTarget;
	}

	return result;
}
