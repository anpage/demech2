#ifndef DIFFICULTYCFG_H
#define DIFFICULTYCFG_H

#include "decomp.h"
#include "types.h"

#pragma pack(1)

// The difficulty settings, a block of MW2NET.CFG (or MW2DIF.CFG), the simulator's DifficultyCfg
// (MW2's config.h).
// SIZE 0x17
struct DifficultyCfg {
	MechU8 m_unlimitedAmmo;   // 0x00
	MechU8 m_invulnerable;    // 0x01
	MechU8 m_splashDamage;    // 0x02
	MechU8 m_collisionDamage; // 0x03
	MechU8 m_heatTracking;    // 0x04
	MechU8 m_enemySkill;      // 0x05
	undefined m_unk0x06[2];   // 0x06
	MechU8 m_regenerate;      // 0x08
	MechU8 m_radar;           // 0x09
	MechU8 m_teamGame;        // 0x0a
	MechS32 m_timeOfDay;      // 0x0b
	MechS32 m_gravity;        // 0x0f
	MechS32 m_temperature;    // 0x13
};

#pragma pack()

#endif // DIFFICULTYCFG_H
