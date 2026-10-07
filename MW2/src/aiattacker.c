/* TargetAttacker, which the Matrox edition places in another order in ai.c's object: ai.c includes this
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

// Turns p_player on a player attacking it (FindAttacker), unless its AI flags hold it (bit 0) or
// it is busy with that player already, calling off the teammate on it (FindNearestTarget).
// Stack-slot permutation: best, nearest and other and target.
// FUNCTION: MW2 0x100519b3
// FUNCTION: MW2MATROX 0x10081144
void TargetAttacker(Player* p_player)
{
	MechS16 target;
	MechS16 nearest;
	Player* other;
	MechS16 best;

	if (p_player->m_ai.m_flags & 1) {
		return;
	}

	target = FindAttacker(p_player);
	if (target != -1 && (p_player->m_ai.m_goal != p_player->m_nav || p_player->m_ai.m_state != c_aiStateGoDirect) &&
		((p_player->m_ai.m_state != c_aiStateTarget && p_player->m_ai.m_state != c_aiStateAttack) ||
		 p_player->m_ai.m_goal != target)) {
		other = FindNearestTarget(p_player, target, -3, &best, &nearest);
		if (other) {
			if (other->m_ai.m_flags & 0x40) {
				target = -1;
			}
			else {
				SetAIState(other, c_aiStateIdle, 0, 0);
			}
		}

		if (target != -1) {
			SetAIState(p_player, c_aiStateTarget, target, 1);
		}
	}
}
