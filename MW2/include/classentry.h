#ifndef CLASSENTRY_H
#define CLASSENTRY_H

#include "decomp.h"
#include "types.h"

struct SceneObject;
struct Shape;

// The type of an entry's position: a float in the Matrox edition (MW2_MATROX). A macro, so
// that 1.1's units see no new symbols.
#ifdef MW2_MATROX
#define ClassEntryCoord MechFloat
#else
#define ClassEntryCoord undefined4
#endif

// SIZE 0x44
// An entry of g_classTable: a shape a player's mech uses, with up to five levels of detail.
typedef struct ClassEntry {
	MechS32 m_owner;           // 0x00 — a player index; -1 free, -2 until ClaimNewClassEntries
	MechS32 m_loadedLevel;     // 0x04 — -1 while m_shape isn't loaded
	MechS32 m_resourceIds[5];  // 0x08 — by level; -1 unset
	MechS32 m_parent;          // 0x1c
	MechS32 m_partId;          // 0x20
	struct Shape* m_shape;     // 0x24
	struct SceneObject* m_obj; // 0x28
	ClassEntryCoord m_x;       // 0x2c
	ClassEntryCoord m_y;       // 0x30
	ClassEntryCoord m_z;       // 0x34
	MechU16 m_kinds[5];        // 0x38 — by level; the shape kind in bits 4-7
	MechS16 m_released;        // 0x42
} ClassEntry;

#endif // CLASSENTRY_H
