#ifndef PLAYERS_H
#define PLAYERS_H

#include "aimessage.h"
#include "airule.h"
#include "aistackentry.h"
#include "decomp.h"
#include "gamething.h"
#include "mech.h"
#include "playersteering.h"
#include "playertargetinfo.h"
#include "ramp.h"
#include "types.h"
#include "vector3.h"

struct AmberWillow0x7c;
struct ScarletOrchid0x4c;
typedef struct Player Player;

typedef void (*PlayerMechFn)(Mech* p_mech);
typedef void (*PlayerCreatedFn)(MechS32 p_index, Player* p_player);

#pragma pack(push, 1)

// SIZE 0x1aa
struct Player {
	MechS32 m_unk0x00;                        // 0x00
	MechS32 m_index;                          // 0x04
	MechS32 m_team;                           // 0x08
	MechS32 m_slot;                           // 0x0c — the player's place in its team's formation
	MechS32 m_unk0x10;                        // 0x10 — 2 for an AI-driven player
	MechS16 m_flags;                          // 0x14
	MechS16 m_unk0x16;                        // 0x16 — a bit per team that reached it
	MechS32 m_unk0x18;                        // 0x18 — a level of FUN_1001ce90's shapes
	MechS32 m_unk0x1c;                        // 0x1c
	Mech* m_mech;                             // 0x20
	undefined4 m_unk0x24;                     // 0x24
	void (*m_firstClassFn)(Player* p_player); // 0x28
	PlayerMechFn m_updateFn;                  // 0x2c
	PlayerMechFn m_lateUpdateFn;              // 0x30
	PlayerMechFn m_localUpdateFn;             // 0x34
	PlayerMechFn m_drawFn;                    // 0x38
	PlayerMechFn m_shutdownFn;                // 0x3c
	struct AmberWillow0x7c* m_obj;            // 0x40
	struct AmberWillow0x7c* m_unk0x44;        // 0x44 — the object the weapons aim from
	struct AmberWillow0x7c* m_unk0x48;        // 0x48 — the hardpoint of the weapon firing
	PlayerSteering* m_steering;               // 0x4c
	Vector3 m_position;                       // 0x50
	MechS32 m_unk0x5c;                        // 0x5c
	MechS32 m_heading;                        // 0x60 — 16.16 degrees
	MechS32 m_unk0x64;                        // 0x64
	MechS32 m_unk0x68;                        // 0x68
	MechS32 m_unk0x6c;                        // 0x6c — added to the heading for the forward view
	MechS32 m_unk0x70;                        // 0x70
	undefined m_unk0x74[0x78 - 0x74];         // 0x74
	MechS32 m_unk0x78;                        // 0x78
	MechS32 m_unk0x7c;                        // 0x7c — a player index, or -1
	MechU32 m_unk0x80;                        // 0x80
	MechS32 m_unk0x84;                        // 0x84
	MechS32 m_unk0x88;                        // 0x88
	MechS32 m_unk0x8c;                        // 0x8c
	MechS32 m_unk0x90;                        // 0x90
	undefined m_unk0x94[0x98 - 0x94];         // 0x94
	Ramp m_aimRange;                          // 0x98 — eases towards m_unk0xa8's distance
	Ramp m_unk0xa8;                           // 0xa8 — the distance the weapons converge at
	MechS32 m_unk0xb8;                        // 0xb8 — the heading's cosine, 16.16 (FUN_1006831a)
	MechS32 m_unk0xbc;                        // 0xbc — the heading's sine, 16.16
	PlayerTargetInfo m_targetInfo;            // 0xc0
	MechChar m_name[0x114 - 0xe8];            // 0xe8
	MechS32 m_killer;                         // 0x114 — the player who destroyed its mech
	AiRule** m_rules;                         // 0x118 — the rules of the current state, NULL-terminated
	MechU16* m_ruleSets[3];                   // 0x11c — AI scripts, by priority
	AiStackEntry m_stack[1];                  // 0x128 — T_PUSH saves the state and goal here
	undefined m_unk0x12c[0x130 - 0x12c];      // 0x12c
	MechS8 m_unk0x130;                        // 0x130
	undefined m_unk0x131;                     // 0x131
	MechU16 m_unk0x132;                       // 0x132
	MechU16 m_unk0x134;                       // 0x134
	MechU16 m_unk0x136;                       // 0x136
	MechS8 m_unk0x138;                        // 0x138
	undefined m_unk0x139[0x140 - 0x139];      // 0x139
	MechU16 m_stackCount;                     // 0x140
	undefined2 m_unk0x142;                    // 0x142
	AiMessage m_posted;                       // 0x144
	undefined2 m_unk0x148;                    // 0x148
	MechS16 m_aiState;                        // 0x14a
	MechU16 m_aiTarget;                       // 0x14c
	MechU16 m_aiGoal;                         // 0x14e
	MechU16 m_unk0x150;                       // 0x150
	MechS16 m_aiFlags;                        // 0x152
	MechS32 m_unk0x154;                       // 0x154
	MechS8 m_unk0x158;                        // 0x158
	MechS8 m_unk0x159;                        // 0x159
	MechS32 m_unk0x15a;                       // 0x15a
	MechS32 m_unk0x15e;                       // 0x15e
	MechS32 m_unk0x162;                       // 0x162
	MechS32 m_unk0x166;                       // 0x166
	MechS32 m_nav;                            // 0x16a — a nav target id the AI placed, or 0x1000
	MechS16 m_unk0x16e;                       // 0x16e
	MechS16 m_unk0x170;                       // 0x170
	MechS16 m_unk0x172;                       // 0x172 — the previous maneuver, or -1
	MechS32 m_unk0x174;                       // 0x174
	MechS32 m_unk0x178;                       // 0x178
	MechS32 m_unk0x17c;                       // 0x17c — a clock time
	MechS32 m_unk0x180;                       // 0x180
	MechS32 m_unk0x184;                       // 0x184
	struct ScarletOrchid0x4c* m_unk0x188;     // 0x188 — a shape
	undefined m_unk0x18c[0x190 - 0x18c];      // 0x18c
	MechS16 m_unk0x190;                       // 0x190
	undefined m_unk0x192[0x196 - 0x192];      // 0x192
	MechS32 m_unk0x196;                       // 0x196
	MechS32 m_unk0x19a;                       // 0x19a — the target distance at the last FUN_10016093
	MechU32 m_unk0x19e;                       // 0x19e
	MechS8 m_unk0x1a2[8];                     // 0x1a2 — the formation places around it that are taken
};

#pragma pack(pop)

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
	void FUN_1006d282(MechS32 p_player, PlayerCreatedFn p_fn);
	MechS32 FUN_1006d340(MechS32 p_player);
	void FUN_1006d3a4(Player* p_player);

#ifdef __cplusplus
}
#endif

#endif // PLAYERS_H
