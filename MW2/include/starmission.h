#ifndef STARMISSION_H
#define STARMISSION_H

#include "decomp.h"
#include "missionobjective.h"
#include "types.h"

#pragma pack(push, 1)

// A star's (a team's) mission: its objectives, and how it went.
// SIZE 0x3c0a
typedef struct StarMission {
	MechS32 m_objectiveCount;          // 0x00
	MechS32 m_startTime;               // 0x04
	MechS32 m_endTime;                 // 0x08
	MechS32 m_timeLimit;               // 0x0c
	undefined m_unk0x10[0x38 - 0x10];  // 0x10
	MechU8 m_status;                   // 0x38 — 0 in progress, 2 successful, 3 failed, 4 out of time
	undefined m_unk0x39;               // 0x39
	MissionObjective m_objectives[48]; // 0x3a
} StarMission;

#pragma pack(pop)

#endif // STARMISSION_H
