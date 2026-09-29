#include "objective.h"

#include "clock.h"
#include "decomp.h"
#include "simmain.h"
#include "starmission.h"
#include "types.h"

// STUB: MW2 0x1001aa02
void DoFirstObjtv(StarMission* p_unk0x00, MechS32 p_unk0x04)
{
	STUB(0x1001aa02);
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
