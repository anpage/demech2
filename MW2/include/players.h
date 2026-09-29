#ifndef PLAYERS_H
#define PLAYERS_H

#include "decomp.h"
#include "gamething.h"
#include "mech.h"
#include "types.h"

typedef struct Player Player;

typedef void (*PlayerMechFn)(Mech* p_mech);

struct Player {
	undefined m_unk0x00[0x14];                // 0x00
	MechS16 m_flags;                          // 0x14
	undefined m_unk0x16[0x20 - 0x16];         // 0x16
	Mech* m_mech;                             // 0x20
	undefined4 m_unk0x24;                     // 0x24
	void (*m_firstClassFn)(Player* p_player); // 0x28
	PlayerMechFn m_updateFn;                  // 0x2c
	PlayerMechFn m_lateUpdateFn;              // 0x30
	PlayerMechFn m_localUpdateFn;             // 0x34
	PlayerMechFn m_drawFn;                    // 0x38
	PlayerMechFn m_shutdownFn;                // 0x3c
	undefined m_unk0x40[0x50 - 0x40];         // 0x40
	MechS32 m_position[3];                    // 0x50
	undefined4 m_unk0x5c;                     // 0x5c
	MechS32 m_heading;                        // 0x60 — 16.16 degrees
};

// The functions and globals of players.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_playerCount;
	extern MechS32 g_gameThingCount;
	extern Player* g_players[];
	extern GameThing g_gameThings[254];

	void FirstClassFunctions(void);
	void UpdateAllPlayers(void);
	void LateUpdateAllPlayers(void);
	void UpdateLocalPlayer(void);
	void DrawLocalPlayer(void);
	void ShutdownAllPlayers(void);
	void ZeroGameThing(MechS32 p_index);
	void ZeroGamethings(void);

#ifdef __cplusplus
}
#endif

#endif // PLAYERS_H
