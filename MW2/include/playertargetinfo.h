#ifndef PLAYERTARGETINFO_H
#define PLAYERTARGETINFO_H

#include "decomp.h"
#include "fixedfloat.h"
#include "types.h"
#include "vector3.h"

#pragma pack(push, 1)

// What a player is aiming at: the target and the distance, bearing and pitch to it
// (GetBearingAndRange).
// SIZE 0x28
typedef struct PlayerTargetInfo {
	MechScalar m_distance; // 0x00 — to the target along the ground
	MechScalar m_range;    // 0x04 — to the target in a straight line (the target panel's)
	Vector3 m_position;    // 0x08 — the target's
	MechScalar m_heading;  // 0x14 — 16.16 degrees, towards the target
	MechScalar m_pitch;    // 0x18 — 16.16 degrees, towards the target
	MechS32 m_target;      // 0x1c — an AI target id (see ai.h)
	MechS32 m_unk0x20;     // 0x20 — only ever cleared
	MechS32 m_unk0x24;     // 0x24 — only ever cleared
} PlayerTargetInfo;

#pragma pack(pop)

#endif // PLAYERTARGETINFO_H
