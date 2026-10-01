#ifndef AI_H
#define AI_H

#include "aimessage.h"
#include "airule.h"
#include "types.h"

struct Player;
typedef struct AiName AiName;

// The AI states (Player::m_ai.m_state), and the commands the local player gives its star.
enum {
	c_aiStateIdle = 0,
	c_aiStateAvoid = 1,
	c_aiStateTarget = 2,
	c_aiStateAttack = 3,
	c_aiStateFlee = 4,
	c_aiStateFollow = 5,
	c_aiStateRecon = 6,
	c_aiStatePatrol = 7,
	c_aiStateGoDirect = 8,
	c_aiStateRest = 10,
	c_aiStateShutdown = 11,
	c_aiStateDead = 12
};

// The messages a rule waits for (AiRule::m_message).
enum {
	c_aiMessageNone = 0,
	c_aiMessageProx = 1,
	c_aiMessageDist = 2,
	c_aiMessageReach = 3,
	c_aiMessageTrue = 4,
	c_aiMessageFalse = 5,
	c_aiMessageDestroy = 6,
	c_aiMessageTargetable = 7
};

// The transitions a rule makes (AiRule::m_transition).
enum {
	c_aiTransitionNull = 0,
	c_aiTransitionClearStack = 1,
	c_aiTransitionPush = 2,
	c_aiTransitionPop = 3,
	c_aiTransitionNotify = 4,
	c_aiTransitionEvaluate = 5
};

// AI target ids: a type in bits 8-11 and an index in the low byte, or a symbolic target.
enum {
	c_aiTargetNav = 0x100,
	c_aiTargetPlayer = 0x200,
	c_aiTargetThing = 0x400,
	c_aiTargetHome = 0x2100,
	c_aiTargetRbAnchor = 0x2101,
	c_aiTargetUser = 0x2200,
	c_aiTargetMyLeader = 0x2201,
	c_aiTargetMe = 0x2202,
	c_aiTargetFriendly = 0x4200,
	c_aiTargetEnemy = 0x4201
};

// The functions and globals of ai.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10051100(struct Player* p_player);
	MechS32 FUN_10051251(struct Player* p_player);
	void FUN_1005141c(struct Player* p_player);
	void FirstAI(void);
	void FUN_100516c5(struct Player* p_player);
	void FUN_100518cd(struct Player* p_player);
	void FUN_100519b3(struct Player* p_player);
	const MechChar* FUN_10051ad8(MechS16 p_value, AiName* p_names, MechS16 p_count);
	void FUN_10051b35(void);
	void LogPlayerStatusLines(void);
	MechU32 FUN_100520d7(MechS32* p_a, MechS32* p_b);
	MechS32 FUN_1005212a(MechS32 p_team);
	void FUN_100521e0(MechS32 p_team, MechS32 p_index, MechS32 p_target);
	MechS16 FUN_10052284(struct Player* p_player, MechS16 p_target);
	MechS16 FUN_10052311(struct Player* p_player, MechU16 p_target);
	MechS16 FUN_10052325(void);
	MechS16 FUN_10052338(struct Player* p_player, MechS16 p_target, MechS16 p_arg);
	MechS16 FUN_10052445(struct Player* p_player, MechS16 p_target, MechS16 p_arg);
	void FUN_1005253c(void);
	MechS16 FUN_10052617(struct Player* p_player, MechS16 p_target, MechS16 p_arg);
	void FUN_1005276a(MechS32 p_team);
	MechS32 FUN_10052cb7(struct Player* p_player, MechS16 p_target, MechS16 p_distance, MechS32 p_check);
	struct Player* FUN_10052d8b(
		struct Player* p_player,
		MechS16 p_target,
		MechS16 p_arg,
		MechS16* p_nearest,
		MechS16* p_best
	);
	void FUN_10053072(void);
	MechS16 FUN_100530f7(struct Player* p_player, MechS16 p_target, MechS16 p_arg);
	MechS16 FUN_100531d1(void);
	MechS16 FUN_100531e4(void);
	MechS16 FUN_100531f7(struct Player* p_player, MechU16 p_target);
	void FUN_10053258(struct Player* p_player);
	void FUN_10053275(struct Player* p_player, MechU16 p_target);
	void FUN_100534a0(struct Player* p_player, MechU16 p_target);
	MechS32 FUN_100534c5(struct Player* p_player, AiRule* p_rule);
	MechS32 FUN_100534da(struct Player* p_player, AiRule* p_rule);
	MechS32 FUN_100534fb(struct Player* p_player, AiRule* p_rule);
	MechS32 FUN_10053577(struct Player* p_player, AiRule* p_rule);
	MechU16 FUN_100535e3(struct Player* p_player);
	MechS16 FUN_100536b8(MechU16 p_value, MechS16 p_goal, MechS16 p_found, MechS16 p_target);
	MechS32 FUN_1005372c(struct Player* p_player, MechS16 p_target);
	MechS32 FUN_10053769(MechS16 p_target);
	MechS32 FUN_10053811(struct Player* p_player, MechS32 p_range);
	MechS32 FUN_1005391f(struct Player* p_player, MechS32 p_angle, MechS32 p_delta);
	MechS32 FUN_10053954(struct Player* p_player, MechS32 p_delta);
	MechS32 FUN_1005398f(struct Player* p_player);
	void FUN_10053a2e(struct Player* p_player);
	void FUN_10053be9(struct Player* p_player, MechU16 p_state);
	MechS16 FUN_10054043(struct Player* p_player, MechS16 p_target, MechS16 p_previous);
	MechS32 FUN_1005432f(struct Player* p_player);
	MechS32 FUN_10054384(MechU16 p_target, MechS32 p_check);
	void FUN_10054584(struct Player* p_player, MechS16 p_state, MechU16 p_target);
	MechS32 FUN_100545ea(struct Player* p_player, MechS16 p_state);
	void FUN_10054684(struct Player* p_player, MechS16 p_state, MechS16 p_target, MechS32 p_push);
	void FUN_10054778(struct Player* p_player);
	void FUN_10054851(struct Player* p_player);
	void FUN_10054a30(struct Player* p_player, MechS16 p_target);
	void FUN_10054a93(struct Player* p_player);
	void FUN_10054b50(MechS32 p_index, MechU32 p_attacker);
	MechS32 LoadAIScripts(void);
	MechS32 FUN_10054c6a(struct Player* p_player, AiRule* p_rule);
	MechS32 FUN_10054ccc(MechS32 p_slot);
	void FUN_10054d4c(struct Player* p_player, MechS16 p_message, MechU16 p_target, MechS16 p_arg);
	MechS16 FUN_10054d88(struct Player* p_player, MechS16 p_targets, MechS16 p_state, MechS16 p_target);
	MechS32 FUN_10054f50(MechS32 p_slot, MechS16 p_command);
	MechS32 FUN_10055131(MechS32 p_value, MechS32 p_delta, MechS32 p_sameSign);
	void FUN_100551ad(MechS32 p_team);
	void FUN_100551c6(MechS32 p_team);
	MechS16 FUN_10055485(MechS32 p_value);
	void FUN_100554c8(struct Player* p_player, MechS16 p_slot, MechS16 p_script, MechS16 p_leader);
	MechU16 FUN_100556a4(MechS32 p_team, MechS16 p_objective, MechS32 p_index);
	MechS32 FUN_100556fe(MechS32 p_team, struct Player** p_members, MechS32 p_aliveOnly);
	void FUN_1005579a(MechS32 p_team, AiMessage* p_order);
	MechS32 FUN_10055811(MechS32 p_team, MechU16 p_target);
	void FUN_10055bb7(MechS32 p_team);
	void FUN_100561ea(struct Player* p_player);
	MechS32 FUN_10056230(void);
	void FUN_1005625d(MechS32 p_command, MechS32 p_slot);
	void FUN_100562b4(struct Player* p_player);

#ifdef __cplusplus
}
#endif

#endif // AI_H
