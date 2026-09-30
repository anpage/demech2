#ifndef OBJECTIVE_H
#define OBJECTIVE_H

#include "decomp.h"
#include "starmission.h"
#include "types.h"

// The functions and globals of objective.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_1001a910(MechChar* p_text);
	extern MechS32 g_unk0x100a374c;

	MechS32 DoFirstObjtv(StarMission* p_mission, MechS32 p_team);
	MechS32 FUN_1001ab4a(MechU8* p_target);
	MechS32 FUN_1001ac06(MechU8* p_target, MechS32 p_team);
	MechS32 FUN_1001ad5b(MechU8* p_target, MechS32 p_team);
	MechS32 FUN_1001b0cb(MechS32 p_star, MechS32 p_objective, MechS32 p_state);
	void FUN_1001b21a(void);
	MechS32 FUN_1001b3f4(MechS32 p_star, MechS32 p_status);
	MechS32 FUN_1001b580(MechS32 p_star, MechS32 p_objective, MechS32 p_condition);
	MechS32 FUN_1001b66c(MechS32 p_star, MechS32 p_objective);
	void UpdateObjectives(void);
	void EndTheMission1(void);
	void EndTheMission2(void);
	void FUN_1001cc5c(MechS32 p_star);
	void FUN_1001cdd1(void);
	MechS32 FUN_1001cde1(undefined4 p_unk0x00);
	MechU16 GetTeamHomeTarget(MechS32 p_team);

#ifdef __cplusplus
}
#endif

#endif // OBJECTIVE_H
