#include "players.h"

#include "decomp.h"
#include "simmain.h"
#include "types.h"

// SIZE 0x40
typedef struct GameThing {
	MechS16 m_unk0x00;                // 0x00
	MechS16 m_unk0x02;                // 0x02
	MechS32 m_unk0x04;                // 0x04
	undefined4 m_unk0x08;             // 0x08
	undefined4 m_unk0x0c;             // 0x0c
	undefined m_unk0x10[0x14 - 0x10]; // 0x10
	MechU8 m_unk0x14;                 // 0x14
	undefined m_unk0x15[0x40 - 0x15]; // 0x15
} GameThing;

DECOMP_SIZE_ASSERT(GameThing, 0x40)

// GLOBAL: MW2 0x100ad5e0
MechS32 g_playerCount = 0;

// GLOBAL: MW2 0x100c3570
Player* g_players[1]; // length unknown

// GLOBAL: MW2 0x100c3660
GameThing g_gameThings[254];

// Operand order: the loop test (i < g_playerCount) compares with i in eax in the original.
// FUNCTION: MW2 0x1006cf80
void FirstClassFunctions(void)
{
	MechS32 i;
	void (*first)(Player* p_player);

	for (i = 0; i < g_playerCount; i++) {
		if (g_players[i]->m_firstClassFn != NULL && g_players[i]->m_mech != NULL) {
			first = g_players[i]->m_firstClassFn;
			first(g_players[i]);
		}
	}
}

// FUNCTION: MW2 0x1006cffa
void UpdateAllPlayers(void)
{
	PlayerMechFn update;
	Mech* mech;
	MechS32 i;

	for (i = 0; i < g_playerCount; i++) {
		update = g_players[i]->m_updateFn;
		mech = g_players[i]->m_mech;
		if (update != NULL && mech != NULL) {
			update(mech);
		}
	}
}

// Operand order: the loop test (i < g_playerCount) compares with i in eax in the original.
// FUNCTION: MW2 0x1006d068
void LateUpdateAllPlayers(void)
{
	MechS32 i;
	PlayerMechFn update;

	for (i = 0; i < g_playerCount; i++) {
		if (g_players[i]->m_lateUpdateFn != NULL && g_players[i]->m_mech != NULL) {
			update = g_players[i]->m_lateUpdateFn;
			update(g_players[i]->m_mech);
		}
	}
}

// FUNCTION: MW2 0x1006d0e5
void UpdateLocalPlayer(void)
{
	Player* player;
	PlayerMechFn update;

	player = g_players[g_localPlayerId];
	if (player != NULL) {
		update = player->m_localUpdateFn;
		if (update != NULL && player->m_mech != NULL) {
			update(player->m_mech);
		}
	}
}

// FUNCTION: MW2 0x1006d139
void DrawLocalPlayer(void)
{
	Player* player;
	PlayerMechFn draw;

	player = g_players[g_localPlayerId];
	if (player != NULL) {
		draw = player->m_drawFn;
		if (draw != NULL && player->m_mech != NULL) {
			draw(player->m_mech);
		}
	}
}

// Stack slots: shutdown and i are swapped.
// FUNCTION: MW2 0x1006d18d
void ShutdownAllPlayers(void)
{
	PlayerMechFn shutdown;
	MechS32 i;

	for (i = 0; i < g_playerCount; i++) {
		if (g_players[i]->m_shutdownFn != NULL) {
			shutdown = g_players[i]->m_shutdownFn;
			shutdown(g_players[i]->m_mech);
		}
	}
}

// FUNCTION: MW2 0x1006d1f5
void ZeroGameThing(MechS32 p_index)
{
	GameThing* thing;

	thing = &g_gameThings[p_index];
	thing->m_unk0x00 = 0;
	thing->m_unk0x02 = 0;
	thing->m_unk0x04 = -1;
	thing->m_unk0x08 = 0;
	thing->m_unk0x0c = 0;
	thing->m_unk0x14 = 0;
}

// FUNCTION: MW2 0x1006d247
void ZeroGamethings(void)
{
	MechS32 i;

	for (i = 0; i < 254; i++) {
		ZeroGameThing(i);
	}
}
