#ifndef CAMPAIGNMISSION_H
#define CAMPAIGNMISSION_H

#include "decomp.h"
#include "types.h"

#pragma pack(1)
// SIZE 0x09
// One mission of a clan campaign. Each campaign's table holds its 16 missions in order, then a
// NULL-scenario entry titled "Retired"; m_unk0x04 is 1 on the four trials.
struct CampaignMission {
	MechChar* m_unk0x00; // 0x00 — the scenario name ("yellSCN1")
	undefined m_unk0x04; // 0x04
	MechChar* m_title;   // 0x05
};
#pragma pack()

#endif // CAMPAIGNMISSION_H
