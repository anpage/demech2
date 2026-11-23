#ifndef WEAPONDATA_H
#define WEAPONDATA_H

#include "effectinfo.h"
#include "weapondef.h"

// The weapon types.
#ifdef MW2_WEAPON_FUNCTIONS
#define WEAPON_DEF_COUNT 35
#else
#define WEAPON_DEF_COUNT 31
#endif

// The globals of weapondata.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern WeaponDef g_weaponDefs[WEAPON_DEF_COUNT];
	extern EffectInfo g_effectInfo[]; // EFFECT_TYPE_COUNT (effect.h)

#ifdef __cplusplus
}
#endif

#endif // WEAPONDATA_H
