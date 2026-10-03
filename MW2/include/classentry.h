#ifndef CLASSENTRY_H
#define CLASSENTRY_H

#include "decomp.h"
#include "types.h"

struct SceneObject;
struct Shape;

// SIZE 0x44
// An entry of g_classTable: a shape a player's mech uses, with up to five levels of detail.
typedef struct ClassEntry {
	MechS32 m_owner;           // 0x00 — a player index; -1 free, -2 until FUN_1001d220
	MechS32 m_unk0x04;         // 0x04 — -1 while m_shape isn't loaded
	MechS32 m_unk0x08[5];      // 0x08 — by level; -1 unset
	MechS32 m_unk0x1c;         // 0x1c
	MechS32 m_unk0x20;         // 0x20
	struct Shape* m_shape;     // 0x24
	struct SceneObject* m_obj; // 0x28
	undefined4 m_unk0x2c;      // 0x2c
	undefined4 m_unk0x30;      // 0x30
	undefined4 m_unk0x34;      // 0x34
	MechU16 m_unk0x38[5];      // 0x38 — by level; the shape kind in bits 4-7
	MechS16 m_unk0x42;         // 0x42
} ClassEntry;

#endif // CLASSENTRY_H
