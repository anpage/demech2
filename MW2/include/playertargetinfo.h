#ifndef PLAYERTARGETINFO_H
#define PLAYERTARGETINFO_H

#include "decomp.h"
#include "types.h"
#include "vector3.h"

#pragma pack(push, 1)

// What an AI player is aiming at: the target and the distance and bearing to it.
// SIZE 0x28
typedef struct PlayerTargetInfo {
	MechS32 m_distance;               // 0x00 — to the target, in world units
	MechS32 m_unk0x04;                // 0x04 — a distance: DoAudio warns within 150000
	Vector3 m_position;               // 0x08 — the target's
	MechS32 m_heading;                // 0x14 — 16.16 degrees, towards the target
	MechS32 m_unk0x18;                // 0x18
	MechS32 m_target;                 // 0x1c — an AI target id (see ai.h)
	undefined m_unk0x20[0x28 - 0x20]; // 0x20
} PlayerTargetInfo;

#pragma pack(pop)

#endif // PLAYERTARGETINFO_H
