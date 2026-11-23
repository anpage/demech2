#ifndef WEAPONDEF_H
#define WEAPONDEF_H

#include "decomp.h"
#include "fixedfloat.h"
#include "types.h"

#ifdef MW2_WEAPON_FUNCTIONS
struct Player;
struct WeaponDef;
struct WeaponSlot;

// GBL's weapon functions: each frame's and each launch's. A launch returns
// the shots it fired (SpawnShot), 0 when the weapon can't fire again.
typedef MechS32 (*WeaponFrameFn)(struct Player* p_player, struct WeaponDef* p_def, struct WeaponSlot* p_slot);
typedef MechS32 (*WeaponLaunchFn)(struct Player* p_player, struct WeaponDef* p_def, struct WeaponSlot* p_slot);

// WeaponDef::m_flags.
enum {
	c_weaponSoundFirst = 0x1,    // the launch sound plays at a volley's first shot
	c_weaponSoundEach = 0x2,     // and at each of its shots
	c_weaponSelectable = 0x4,    // the weapon selection stops at it (all but the AMS)
	c_weaponSlowTurn = 0x10,     // its guided shot eases its turns toward the target (the torpedo)
	c_weaponAnimation = 0x20,    // its shot starts the animations from m_animation on
	c_weaponNoLaunchMove = 0x40, // the shot starts at the hardpoint, not a shot's length ahead of it
	c_weaponNarc = 0x80          // its shot marks the mech it hits (Mech::m_unk0x112 bit 0)
};
#endif

// A weapon type, from the table of weapon definitions (g_weaponDefs: LLASER, MLASER, ...). GBL's
// (MW2_WEAPON_FUNCTIONS) starts with four more words: its members from m_shotType on are 16 bytes
// further than the comments give.
// SIZE 0x58
typedef struct WeaponDef {
#ifdef MW2_WEAPON_FUNCTIONS
	WeaponFrameFn m_frame;   // 0x00 — called for each slot of the weapon each frame, or NULL
	WeaponLaunchFn m_launch; // 0x04 — launches a shot of a volley (SpawnShot)
	MechU32 m_flags;         // 0x08 — c_weaponSoundFirst etc., copied to Shot::m_weaponFlags
	MechS32 m_animation;     // 0x0c — the first of 8 animations, with c_weaponAnimation
#endif
	MechS32 m_shotType;      // 0x00 — the Shot::m_type a launch takes from the free shots
	MechS32 m_impact;        // 0x04 — the effect the shot sets off (Shot::m_impact)
	MechS32 m_launchEffect;  // 0x08 — an effect started at each launch (SpawnLaunchEffect)
	MechS32 m_launchEffect2; // 0x0c — another
	MechS32 m_autoRepeat;    // 0x10 — the next volley follows at once while the trigger is held
	MechS32 m_launchesShot;  // 0x14 — a launch takes a shot
	MechS32 m_guided;        // 0x18 — the shots follow the locked target
	MechS32 m_volley;        // 0x1c — shots per volley
	MechS32 m_volleysPerBin; // 0x20 — volleys per ammunition bin, or -1 for unlimited ammunition
	MechS32 m_sound;         // 0x24 — the launch sound, or <= 0
	MechS32 m_gravity;       // 0x28 — scales the gravity on the shot
	MechS32 m_speed;         // 0x2c
	MechScalar m_damage;     // 0x30
	MechScalar m_heat;       // 0x34 — added to the firing mech's heat per shot
	MechScalar m_shotHeat;   // 0x38 — added to the heat of the mech the shot hits
	MechS32 m_shortRange;    // 0x3c
	MechS32 m_longRange;     // 0x40 — a target lock needs the target between the two ranges
	MechS32 m_recycle;       // 0x44 — ticks between volleys
	MechS32 m_interval;      // 0x48 — ticks between the shots of a volley
	MechS32 m_lifetime;      // 0x4c — the shot's, in ticks
	MechChar m_name[8];      // 0x50
} WeaponDef;

#endif // WEAPONDEF_H
