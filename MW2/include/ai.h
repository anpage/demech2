#ifndef AI_H
#define AI_H

#include "aimessage.h"
#include "airule.h"
#include "fixedfloat.h"
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

	extern MechS32 g_debugStar;
	extern MechS32 g_debugObjective;
	extern MechS32 g_lairdoCheat;

	void UpdateAI(struct Player* p_player);
	MechS32 RunAIRules(struct Player* p_player);
	void CollectAIRules(struct Player* p_player);
	void FirstAI(void);
	void InitializeAI(struct Player* p_player);
	void ResetAI(struct Player* p_player);
	void TargetAttacker(struct Player* p_player);
	const MechChar* FindAIName(MechS16 p_value, AiName* p_names, MechS16 p_count);
	void ClearAILog(void);
	void LogPlayerStatusLines(void);
	MechU32 GetPointDistance(MechS32* p_a, MechS32* p_b);
	MechS32 ChooseTeamLeader(MechS32 p_team);
	void RetargetGoals(MechS32 p_team, MechS32 p_index, MechS32 p_target);
	MechS16 ResolveTarget(struct Player* p_player, MechS16 p_target);
	MechS16 AiMessageReach(struct Player* p_player, MechS16 p_target, MechS16 p_arg);
	MechS16 AiMessageProx(struct Player* p_player, MechS16 p_target, MechS16 p_arg);
	void LogPlayerSkillLines(void);
	MechS16 AiMessageDist(struct Player* p_player, MechS16 p_target, MechS16 p_arg);
	void LogStarMissionLines(MechS32 p_team);
	MechS32 IsTargetDetectable(struct Player* p_player, MechS16 p_target, MechS16 p_distance, MechS32 p_check);
	struct Player* FindNearestTarget(
		struct Player* p_player,
		MechS16 p_target,
		MechS16 p_arg,
		MechS16* p_nearest,
		MechS16* p_best
	);
	void LogAIStatus(void);
	MechS16 AiMessageTargetable(struct Player* p_player, MechS16 p_target, MechS16 p_arg);
	void AiStateIdle(struct Player* p_player);
	void AiStateMove(struct Player* p_player, MechU16 p_target);
	void AiStateAttack(struct Player* p_player, MechU16 p_target);
	MechS32 AiTransitionNull(struct Player* p_player, AiRule* p_rule);
	MechS32 AiTransitionClearStack(struct Player* p_player, AiRule* p_rule);
	MechS32 AiTransitionPush(struct Player* p_player, AiRule* p_rule);
	MechS32 AiTransitionPop(struct Player* p_player, AiRule* p_rule);
	MechU16 FindAttacker(struct Player* p_player);
	MechS16 ResolveRuleTarget(MechU16 p_value, MechS16 p_goal, MechS16 p_found, MechS16 p_target);
	MechS32 SetTarget(struct Player* p_player, MechS16 p_target);
	MechScalar GetTargetRange(MechS16 p_target);
	MechScalar GetApproachThrottle(struct Player* p_player, MechScalar p_range);
	MechScalar AimTorsoPan(struct Player* p_player, MechScalar p_angle, MechS32 p_delta);
	MechScalar AimTorsoTilt(struct Player* p_player, MechS32 p_delta);
	MechScalar SteerToTarget(struct Player* p_player);
	void LeaveAIState(struct Player* p_player);
	void EnterAIState(struct Player* p_player, MechU16 p_state);
	MechS16 NextTarget(struct Player* p_player, MechS16 p_target, MechS16 p_previous);
	MechScalar GetTargetBearing(struct Player* p_player);
	MechS32 IsTargetDone(MechU16 p_target, MechS32 p_check);
	void QueueAIState(struct Player* p_player, MechS16 p_state, MechU16 p_target);
	MechS32 HasAIState(struct Player* p_player, MechS16 p_state);
	void SetAIState(struct Player* p_player, MechS16 p_state, MechS16 p_target, MechS32 p_push);
	void ReleaseNavPoints(struct Player* p_player);
	void PlacePatrolNavs(struct Player* p_player);
	void AdvanceNavTarget(struct Player* p_player, MechS16 p_target);
	void PlaceFormationNav(struct Player* p_player);
	void RecordAttack(MechS32 p_index, MechU32 p_target);
	MechS32 LoadAIScripts(void);
	MechS32 AiTransitionNotify(struct Player* p_player, AiRule* p_rule);
	MechS32 FindStarSlotPlayer(MechS32 p_slot);
	void PostAIMessage(struct Player* p_player, MechS16 p_message, MechU16 p_target, MechS16 p_arg);
	MechS16 OrderPlayers(struct Player* p_player, MechS16 p_targets, MechS16 p_state, MechS16 p_target);
	MechS32 OrderStarSlot(MechS32 p_slot, MechS16 p_command);
	MechScalar AddClamped(MechScalar p_value, MechS32 p_delta, MechS32 p_sameSign);
	void ResetStarOrders(MechS32 p_team);
	void AssignStarObjective(MechS32 p_team);
#ifdef MW2_ENGAGEMENT_FLAGS_INT
	MechS32 GetEngagementAIFlags(MechS32 p_value);
#else
MechS16 GetEngagementAIFlags(MechS32 p_value);
#endif
	void SetAIScript(struct Player* p_player, MechS16 p_slot, MechS16 p_script, MechS16 p_leader);
	MechU16 GetObjectiveTarget(MechS32 p_team, MechS16 p_objective, MechS32 p_index);
	MechS32 GetTeamMembers(MechS32 p_team, struct Player** p_members, MechS32 p_aliveOnly);
	void HandleStarOrder(MechS32 p_team, AiMessage* p_order);
	MechS32 AssignStarTarget(MechS32 p_team, MechU16 p_target);
	void LeadStar(MechS32 p_team);
	void ReleaseAnchorNav(struct Player* p_player);
	MechS32 GetLocalStarSize(void);
	void RunStarCommand(MechS32 p_command, MechS32 p_slot);
	void SweepTorso(struct Player* p_player);

#ifdef __cplusplus
}
#endif

// The 3Dfx edition places these out of ai.c's order, so they are annotated here, by name.

// The 3Dfx edition's diff is register allocation, scheduling and comparison operand order only.
// FUNCTION: MW23DFX 0x10011fc0
// FUNCTION: MW2ATI 0x10012520
// FUNCTION: MW2S3 0x10015750
// FUNCTION: MW2POWERVR 0x10011460
// FirstAI

// FUNCTION: MW23DFX 0x10012000
// FUNCTION: MW2ATI 0x10012560
// FUNCTION: MW2S3 0x10015780
// FUNCTION: MW2POWERVR 0x100114a0
// InitializeAI

// The 3Dfx edition's diff is register allocation and instruction scheduling only.
// FUNCTION: MW23DFX 0x10012650
// FUNCTION: MW2ATI 0x10012bb0
// FUNCTION: MW2S3 0x10015fd0
// LogPlayerSkillLines

// The 3Dfx edition's diff is register allocation, scheduling and comparison operand order only.
// FUNCTION: MW23DFX 0x10012730
// FUNCTION: MW2ATI 0x10012c90
// FUNCTION: MW2S3 0x10016100
// LogStarMissionLines

// The 3Dfx edition's diff is register allocation, scheduling and comparison operand order only.
// FUNCTION: MW23DFX 0x10012de0
// FUNCTION: MW2ATI 0x10013290
// FUNCTION: MW2S3 0x100164b0
// FindNearestTarget

// The PowerVR edition places these out of ai.c's order, so they are annotated here, by name.

// FUNCTION: MW2POWERVR 0x10011af0
// LogPlayerSkillLines

// FUNCTION: MW2POWERVR 0x100121d0
// FindNearestTarget

// GBL places these after FirstAI, out of ai.c's order, so they are annotated here, by name.

// FUNCTION: MW2GBL 0x1007672e
// FUNCTION: MW2PREMERCS 0x1003709f
// UpdateAI

// FUNCTION: MW2GBL 0x1007687f
// FUNCTION: MW2PREMERCS 0x100371fc
// RunAIRules

// FUNCTION: MW2GBL 0x10076a4a
// FUNCTION: MW2PREMERCS 0x100373c7
// CollectAIRules

// The early Mercenaries build places these out of ai.c's order (the last two before LogAIStatus), so they are
// annotated here, by name.

// FUNCTION: MW2PREMERCS 0x10038035
// LogPlayerSkillLines

// FUNCTION: MW2PREMERCS 0x10038c7e
// IsTargetDetectable

// FUNCTION: MW2PREMERCS 0x10038d52
// FindNearestTarget

#endif // AI_H
