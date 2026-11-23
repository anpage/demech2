#ifndef EFFECT_H
#define EFFECT_H

#include "decomp.h"
#include "fixedfloat.h"
#include "object.h"
#include "types.h"

// The effect slots (g_effects) and the effect types (g_effectInfo).
#ifdef MW2_WEAPON_FUNCTIONS
#define EFFECT_COUNT 0xff
#define EFFECT_TYPE_COUNT 0x24
#else
#define EFFECT_COUNT 0x100
#define EFFECT_TYPE_COUNT 0x20
#endif

#ifdef MW2_WEAPON_FUNCTIONS
struct Shape;
#endif

// An explosion, fire or other visual effect in the world (g_effects). Each slot keeps the scene
// object for its type once one is created. GBL's (MW2_WEAPON_FUNCTIONS) has the shape it is attached to after
// m_object: its members from m_position on are 4 bytes further than the comments give.
// SIZE 0x28
typedef struct Effect {
	SceneObject* m_object; // 0x00
#ifdef MW2_WEAPON_FUNCTIONS
	struct Shape* m_attachedShape; // 0x04 — the shape whose object it is attached to (EffectInfo::m_flags 1)
#endif
	MechScalar m_position[3]; // 0x04
	MechS32 m_timeLeft;       // 0x10 — ticks
	MechS32 m_active;         // 0x14
	MechS32 m_animation;      // 0x18 — the animation it plays, or -1
	MechS32 m_hasCamera;      // 0x1c — the effect camera follows it
	MechS32 m_type;           // 0x20 — an index into g_effectInfo
	MechS32 m_owner;          // 0x24 — the player it damages on behalf of, or -2
} Effect;

#endif // EFFECT_H
