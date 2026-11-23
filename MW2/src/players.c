#include "players.h"

#include "artillery.h"
#include "decomp.h"
#include "door.h"
#include "gamething.h"
#include "inputmap.h"
#include "mechclass.h"
#include "playersteering.h"
#include "playertype.h"
#include "poolsizes.h"
#include "simmain.h"
#include "staticmem.h"
#include "types.h"

#include <string.h>

DECOMP_SIZE_ASSERT(GameThing, 0x40)
DECOMP_SIZE_ASSERT(PlayerSteering, 0x48)

// The kinds of player a gamepiece record can create, by Player::m_type: 3 has its own
// callbacks (artillery.c), 7 another (door.c); the others are mechs.
// GLOBAL: MW2 0x100ad4a0
// GLOBAL: MW2GBL 0x1009f170
// GLOBAL: MW2MATROX 0x100aa738
// GLOBAL: MW23DFX 0x1024c330
PlayerType g_playerTypes[9] = {
	{0, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{1, CreateMech, FirstMech, UpdateMech, LateUpdateMech, UpdateLocalMech, DrawMechCockpit, ShutdownMech},
	{2, CreateMech, FirstMech, UpdateMech, LateUpdateMech, UpdateLocalMech, DrawMechCockpit, ShutdownMech},
	{3, CreateArtillery, FirstArtillery, UpdateArtillery, LateUpdateArtillery, NULL, NULL, ShutdownArtillery},
	{4, CreateMech, FirstMech, UpdateMech, LateUpdateMech, UpdateLocalMech, DrawMechCockpit, ShutdownMech},
	{5, CreateMech, FirstMech, UpdateMech, LateUpdateMech, UpdateLocalMech, DrawMechCockpit, ShutdownMech},
	{6, CreateMech, FirstMech, UpdateMech, LateUpdateMech, UpdateLocalMech, DrawMechCockpit, ShutdownMech},
	{7, CreateDoor, FirstDoor, UpdateDoor, LateUpdateDoor, NULL, NULL, ShutdownDoor},
	{8, CreateMech, FirstMech, UpdateMech, LateUpdateMech, UpdateLocalMech, DrawMechCockpit, ShutdownMech},
};

// GLOBAL: MW2 0x100ad5e0
// GLOBAL: MW2GBL 0x1009f2b0
// GLOBAL: MW2MATROX 0x100aa878
// GLOBAL: MW23DFX 0x1024c470
// GLOBAL: MW2ATI 0x101807f0
// GLOBAL: MW2S3 0x10065b88
// GLOBAL: MW2POWERVR 0x101ab638
MechS32 g_playerCount = 0;

// GLOBAL: MW2 0x100ad5e4
// GLOBAL: MW2GBL 0x1009f2b4
// GLOBAL: MW2MATROX 0x100aa87c
// GLOBAL: MW23DFX 0x1024c474
// GLOBAL: MW2ATI 0x101807f4
// GLOBAL: MW2S3 0x10065b8c
// GLOBAL: MW2POWERVR 0x101ab63c
MechS32 g_gameThingCount = 0;

// The world loader stops at 0x3c players (BwdExecuteStream), which fill the original's room
// before g_gameThings.
// GLOBAL: MW2 0x100c3570
// GLOBAL: MW2GBL 0x10137c40
// GLOBAL: MW2MATROX 0x1015e8d0
// GLOBAL: MW23DFX 0x101e0190
// GLOBAL: MW2ATI 0x10113e50
// GLOBAL: MW2S3 0x100adc80
// GLOBAL: MW2POWERVR 0x1013eb10
Player* g_players[0x3c];

// GLOBAL: MW2 0x100c3660
// GLOBAL: MW2GBL 0x10137d30
// GLOBAL: MW2MATROX 0x1015e9c0
// GLOBAL: MW23DFX 0x101e0280
// GLOBAL: MW2ATI 0x10113f40
// GLOBAL: MW2S3 0x100add70
// GLOBAL: MW2POWERVR 0x1013ec00
GameThing g_gameThings[254];

// FUNCTION: MW2 0x1006cf80
// FUNCTION: MW2GBL 0x1001ce10
// FUNCTION: MW2MATROX 0x10054e10
// FUNCTION: MW23DFX 0x100187c0
// FUNCTION: MW2ATI 0x10018b80
// FUNCTION: MW2S3 0x10021f80
// FUNCTION: MW2POWERVR 0x10017ac0
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
// FUNCTION: MW2GBL 0x1001ce8a
// FUNCTION: MW2MATROX 0x10054e89
// FUNCTION: MW23DFX 0x10018800
// FUNCTION: MW2ATI 0x10018bc0
// FUNCTION: MW2S3 0x10021fc0
// FUNCTION: MW2POWERVR 0x10017b00
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

// FUNCTION: MW2 0x1006d068
// FUNCTION: MW2GBL 0x1001cef8
// FUNCTION: MW2MATROX 0x10054ef7
// FUNCTION: MW23DFX 0x10018840
// FUNCTION: MW2ATI 0x10018c00
// FUNCTION: MW2S3 0x10022000
// FUNCTION: MW2POWERVR 0x10017b40
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
// FUNCTION: MW2GBL 0x1001cf75
// FUNCTION: MW2MATROX 0x10054f73
// FUNCTION: MW23DFX 0x10018880
// FUNCTION: MW2ATI 0x10018c40
// FUNCTION: MW2S3 0x10022040
// FUNCTION: MW2POWERVR 0x10017b80
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
// FUNCTION: MW2GBL 0x1001cfc9
// FUNCTION: MW2MATROX 0x10054fc7
// FUNCTION: MW23DFX 0x100188b0
// FUNCTION: MW2ATI 0x10018c70
// FUNCTION: MW2S3 0x10022070
// FUNCTION: MW2POWERVR 0x10017bb0
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

// FUNCTION: MW2 0x1006d18d
// FUNCTION: MW2GBL 0x1001d01d
// FUNCTION: MW2MATROX 0x1005501b
// FUNCTION: MW23DFX 0x100188e0
// FUNCTION: MW2ATI 0x10018ca0
// FUNCTION: MW2S3 0x100220a0
// FUNCTION: MW2POWERVR 0x10017be0
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
// FUNCTION: MW2GBL 0x1001d085
// FUNCTION: MW2MATROX 0x10055083
// FUNCTION: MW23DFX 0x10018920
// FUNCTION: MW2ATI 0x10018ce0
// FUNCTION: MW2S3 0x100220d0
// FUNCTION: MW2POWERVR 0x10017c20
void ZeroGameThing(MechS32 p_index)
{
	GameThing* thing;

	thing = &g_gameThings[p_index];
	thing->m_flags = 0;
	thing->m_teamsReached = 0;
	thing->m_staticObject = -1;
	thing->m_hitPoints = 0;
	thing->m_affiliation = 0;
	thing->m_name[0] = '\0';
}

// FUNCTION: MW2 0x1006d247
// FUNCTION: MW2GBL 0x1001d0d7
// FUNCTION: MW2MATROX 0x100550d5
// FUNCTION: MW23DFX 0x10018950
// FUNCTION: MW2ATI 0x10018d10
// FUNCTION: MW2S3 0x10022100
// FUNCTION: MW2POWERVR 0x10017c50
void ZeroGamethings(void)
{
	MechS32 i;

	for (i = 0; i < 254; i++) {
		ZeroGameThing(i);
	}
}

// Creates player p_player: allocates it, sets it up, gives it its steering (the input sinks for
// the local player, the room after it otherwise), and passes it to p_fn if given.
// FUNCTION: MW2 0x1006d282
// FUNCTION: MW2GBL 0x1001d112
// FUNCTION: MW2MATROX 0x10055110
// FUNCTION: MW23DFX 0x10018970
// FUNCTION: MW2ATI 0x10018d30
// FUNCTION: MW2S3 0x10022120
// FUNCTION: MW2POWERVR 0x10017c70
void CreateSimPlayer(MechS32 p_player, PlayerCreatedFn p_fn)
{
	PlayerCreatedFn fn;
	Player* player;

	fn = NULL;
	g_players[p_player] = NULL;
	fn = p_fn;
	if (AllocPlayer(p_player)) {
		player = g_players[p_player];
		InitPlayer(player);
		player->m_index = p_player;
		if (p_player == g_localPlayerId) {
			player->m_steering = &g_localSteering;
		}
		else {
			player->m_steering = (PlayerSteering*) (player + 1);
		}

		if (player->m_steering) {
			memset(player->m_steering, 0, sizeof(PlayerSteering));
		}

		if (fn) {
			fn(p_player, player);
		}
	}
}

// Allocates player p_player: a remote player gets room for its steering after it.
// FUNCTION: MW2 0x1006d340
// FUNCTION: MW2MATROX 0x100551cf
// FUNCTION: MW23DFX 0x100189e0
// FUNCTION: MW2ATI 0x10018da0
// FUNCTION: MW2S3 0x10022190
// FUNCTION: MW2POWERVR 0x10017ce0
MechS32 AllocPlayer(MechS32 p_player)
{
	Player* player;
	MechU32 size;

	size = sizeof(Player);
	if (p_player != g_localPlayerId) {
		size += sizeof(PlayerSteering);
	}

	player = StaticPoolAlloc(size, g_staticPoolTags[0]);
	if (!player) {
		return FALSE;
	}

	g_players[p_player] = player;
	return TRUE;
}

// Sets up a newly allocated player.
// FUNCTION: MW2 0x1006d3a4
// FUNCTION: MW2GBL 0x1001d23a
// FUNCTION: MW2MATROX 0x10055234
// FUNCTION: MW23DFX 0x10018a20
// FUNCTION: MW2ATI 0x10018de0
// FUNCTION: MW2S3 0x100221d0
// FUNCTION: MW2POWERVR 0x10017d20
void InitPlayer(Player* p_player)
{
	p_player->m_type = 0;
	p_player->m_index = 0;
	p_player->m_team = 0;
	p_player->m_slot = 0;
	p_player->m_aiMode = 0;
	p_player->m_flags = 0;
	p_player->m_inspectedBy = 0;
	p_player->m_baseLevel = 0;
	p_player->m_detailLevel = -1;
	p_player->m_mech = NULL;
	p_player->m_mechSize = 0;
	p_player->m_firstClassFn = NULL;
	p_player->m_updateFn = NULL;
	p_player->m_lateUpdateFn = NULL;
	p_player->m_localUpdateFn = NULL;
	p_player->m_drawFn = NULL;
	p_player->m_shutdownFn = NULL;
	p_player->m_obj = NULL;
	p_player->m_eyeObj = NULL;
	p_player->m_firingObj = NULL;
	p_player->m_steering = NULL;
	p_player->m_position.m_x = p_player->m_position.m_y = p_player->m_position.m_z = 0;
	p_player->m_pitch = p_player->m_heading = p_player->m_roll = 0;
	p_player->m_torsoPitch = p_player->m_torsoTwist = p_player->m_torsoRoll = 0;
	p_player->m_groundHeight = 0;
	p_player->m_onGround = 0;
	p_player->m_collidedWith = -1;
	p_player->m_animFlags = 0;
	p_player->m_motionState = -1;
	p_player->m_nextMotionState = -1;
	p_player->m_speedLevel = 0;
	p_player->m_pendingSound = -1;
	p_player->m_animRate = 0;
	StartRamp(&p_player->m_aimRange, 50000, 50000, 0.2);
	StartRamp(&p_player->m_aimDistance, 50000, 50000, 20.0);
	p_player->m_headingSin = p_player->m_headingCos = 0;
	p_player->m_targetInfo.m_distance = 0;
	p_player->m_targetInfo.m_range = 0;
	p_player->m_targetInfo.m_position.m_x = p_player->m_targetInfo.m_position.m_y =
		p_player->m_targetInfo.m_position.m_z = 0;
	p_player->m_targetInfo.m_heading = 0;
	p_player->m_targetInfo.m_pitch = 0;
	p_player->m_targetInfo.m_target = -1;
	p_player->m_targetInfo.m_unk0x20 = 0;
	p_player->m_targetInfo.m_unk0x24 = 0;
	memset(p_player->m_name, 0, 0x16);
}
