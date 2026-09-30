#ifndef HOLLOWSPIRE_H
#define HOLLOWSPIRE_H

#include "callbacks.h"
#include "decomp.h"
#include "transform.h"
#include "twilightgrove.h"
#include "types.h"
#include "unk1003a530.h"

struct AmberWillow0x7c;

// An entry of the static object cache (g_unk0x1010c630, 0x402 of them): the resource
// FUN_10020704 loads as its shape, the block and parent it hangs from, its placement and
// its timed callbacks. FUN_1001feef resets it.
// SIZE 0x7c
typedef struct HollowSpire0x7c {
	MechS32 m_unk0x00;                 // 0x00 — resource ID, -1: none
	MechS32 m_unk0x04;                 // 0x04 — block, -1: none
	MechS32 m_unk0x08;                 // 0x08 — parent entry; -1: none, -2: a shape only
	MechU32 m_unk0x0c;                 // 0x0c
	undefined4 m_unk0x10;              // 0x10
	MechS32 m_unk0x14;                 // 0x14
	MechS32 m_unk0x18;                 // 0x18
	ScarletOrchid0x4c* m_unk0x1c;      // 0x1c
	struct AmberWillow0x7c* m_unk0x20; // 0x20
	TwilightGrove0x24 m_xform;         // 0x24 — the placement: scale, rotation and translation
	Matrix m_unk0x48;                  // 0x48
	TimedCallback* m_unk0x78;          // 0x78
} HollowSpire0x7c;

#endif // HOLLOWSPIRE_H
