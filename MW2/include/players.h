#ifndef PLAYERS_H
#define PLAYERS_H

#include "decomp.h"
#include "mech.h"
#include "types.h"

typedef struct Player Player;

typedef void (*PlayerMechFn)(Mech* p_mech);

struct Player {
	undefined m_unk0x00[0x20];                // 0x00
	Mech* m_mech;                             // 0x20
	undefined4 m_unk0x24;                     // 0x24
	void (*m_firstClassFn)(Player* p_player); // 0x28
	PlayerMechFn m_updateFn;                  // 0x2c
	PlayerMechFn m_lateUpdateFn;              // 0x30
	PlayerMechFn m_localUpdateFn;             // 0x34
	PlayerMechFn m_drawFn;                    // 0x38
	PlayerMechFn m_shutdownFn;                // 0x3c
};

// The functions and globals of players.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_playerCount;
	extern Player* g_players[];

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
