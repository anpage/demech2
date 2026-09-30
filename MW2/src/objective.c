#include "objective.h"

#include "clock.h"
#include "decomp.h"
#include "simmain.h"
#include "starmission.h"
#include "types.h"

#include <ctype.h>

// Collapses each run of whitespace after a character of p_text into one space.
// FUNCTION: MW2 0x1001a910
void FUN_1001a910(MechChar* p_text)
{
	MechChar* src;
	MechChar* dst;

	src = p_text;
	dst = p_text;
	while (*src) {
		*dst = *src++;
		if (isspace(*src)) {
			while (*src && isspace(*src)) {
				src++;
			}

			*++dst = ' ';
		}

		dst++;
	}

	*dst = '\0';
}

// STUB: MW2 0x1001aa02
void DoFirstObjtv(StarMission* p_unk0x00, MechS32 p_unk0x04)
{
	STUB(0x1001aa02);
}

// Returns whether condition p_condition of star p_star's objective p_objective holds.
// Stack-slot permutation of objective, other, star, state and kind.
// FUNCTION: MW2 0x1001b580
MechS32 FUN_1001b580(MechS32 p_star, MechS32 p_objective, MechS32 p_condition)
{
	MissionObjective* objective;
	MechS32 other;
	MechS32 star;
	MechS32 state;
	MechS32 kind;

	objective = &g_objectiveTable[p_star].m_objectives[p_objective];
	kind = objective->m_conditions[p_condition].m_kind;
	star = objective->m_conditions[p_condition].m_star;
	other = objective->m_conditions[p_condition].m_objective;
	state = g_objectiveTable[star].m_objectives[other].m_state;
	if ((kind == 1 && (state == 5 || state == 6)) || (kind == 2 && state == 5) || (kind == 3 && state == 6)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// Returns whether the conditions of star p_star's objective p_objective hold: one of them, or
// every one. A finished objective's don't; a type 0x10 objective's always do.
// Stack-slot permutation of all, objective, result, i and holds; the original computes the
// objective's index before the star's (index order).
// FUNCTION: MW2 0x1001b66c
MechS32 FUN_1001b66c(MechS32 p_star, MechS32 p_objective)
{
	MechS32 all;
	MissionObjective* objective;
	MechU32 result;
	MechS32 i;
	MechU32 holds;

	objective = &g_objectiveTable[p_star].m_objectives[p_objective];
	all = objective->m_allConditions;
	result = TRUE;
	if (objective->m_state == 5 || objective->m_state == 6) {
		return FALSE;
	}

	if (objective->m_type == 0x10) {
		return TRUE;
	}

	for (i = 0; i < 8; i++) {
		if (!objective->m_conditions[i].m_kind) {
			break;
		}

		holds = FUN_1001b580(p_star, p_objective, i);
		if (!all) {
			if (holds) {
				return TRUE;
			}
		}
		else {
			result &= holds;
			if (!result) {
				return FALSE;
			}
		}
	}

	if (!all) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

// STUB: MW2 0x1001c69e
void UpdateObjectives(void)
{
	STUB(0x1001c69e);
}

// Counts the mission time in seconds.
// FUNCTION: MW2 0x1001c9d5
void EndTheMission1(void)
{
	g_missionTime = g_currentClock / 181;
	return;
}

// STUB: MW2 0x1001c9f7
void EndTheMission2(void)
{
	STUB(0x1001c9f7);
}

// STUB: MW2 0x1001cc5c
void FUN_1001cc5c(MechS32 p_player)
{
	STUB(0x1001cc5c);
}

// FUNCTION: MW2 0x1001cdd1
void FUN_1001cdd1(void)
{
	return;
}

// FUNCTION: MW2 0x1001cde1
MechS32 FUN_1001cde1(undefined4 p_unk0x00)
{
	return 1;
}

// The target of team p_team's return objective (type 0x20), or else its first objective's
// first target: the AI's "home" nav.
// Stack-slot permutation: mission and i.
// FUNCTION: MW2 0x1001cdf6
MechU16 GetTeamHomeTarget(MechS32 p_team)
{
	StarMission* mission;
	MechS32 i;

	mission = &g_objectiveTable[p_team];
	for (i = 0; i < mission->m_objectiveCount; i++) {
		if (mission->m_objectives[i].m_type == 0x20) {
			return mission->m_objectives[i].m_targets[0];
		}
	}

	return mission->m_objectives[0].m_targets[0];
}
