/* ResetAI, which the Matrox edition places in another order in ai.c's object: ai.c includes this
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

// Clears p_player's AI state, target, goal, scripts, stack and posted message.
// FUNCTION: MW2 0x100518cd
// FUNCTION: MW2MATROX 0x10080aab
void ResetAI(Player* p_player)
{
	MechS32 i;

	LeaveAIState(p_player);
	p_player->m_ai.m_target = 0;
	p_player->m_ai.m_goal = 0;
	p_player->m_ai.m_flags = 0;

	if (p_player->m_aiMode != 2) {
		p_player->m_ai.m_state = c_aiStateIdle;
	}
	else {
		p_player->m_ai.m_state = c_aiStateDead;
	}

	for (i = 0; i < 3; i++) {
		SetAIScript(p_player, i, -1, 0);
	}

	p_player->m_rules[0] = NULL;
	p_player->m_stackCount = 0;
	p_player->m_ai.m_posted.m_message = 0;
	p_player->m_nextFireTime = 0;
	ReleaseAnchorNav(p_player);
	CollectAIRules(p_player);
}
