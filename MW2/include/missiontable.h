#ifndef MISSIONTABLE_H
#define MISSIONTABLE_H

#include "bwdrecord.h"
#include "decomp.h"
#include "types.h"

#pragma pack(push, 1)

// An entry of a mission table, one per objective of the star's mission (StarMission).
// SIZE 0x97
typedef struct MissionEntry {
	MechS32 m_slot;                   // 0x00 — the first entry's is the table's slot
	undefined m_unk0x04[0x68 - 0x04]; // 0x04
	MechChar m_name[0x97 - 0x68];     // 0x68 — the event list the objective waits on
} MissionEntry;

// A mission table (LoadMissionTable).
typedef struct MissionTable {
	BwdRecord m_header;        // 0x00
	MissionEntry m_entries[1]; // 0x08 — up to the record's end
} MissionTable;

#pragma pack(pop)

#endif // MISSIONTABLE_H
