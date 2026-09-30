#include "objective.h"

#include "carcfg.h"
#include "clock.h"
#include "config.h"
#include "decomp.h"
#include "gamething.h"
#include "geocache.h"
#include "missionaudio.h"
#include "navpoint.h"
#include "network.h"
#include "players.h"
#include "rendertarget.h"
#include "shots.h"
#include "simmain.h"
#include "soundfx.h"
#include "speech.h"
#include "speechline.h"
#include "starmission.h"
#include "team.h"
#include "types.h"
#include "unk10004ec0.h"
#include "unk100737e0.h"

#include <ctype.h>
#include <stdio.h>

// Set once the local team's mission result has been announced (FUN_1001b3f4).
// GLOBAL: MW2 0x100a374c
MechS32 g_unk0x100a374c = 0;

// Collapses each run of whitespace after a character of p_text into one space.
// FUNCTION: MW2 0x1001a910
void FUN_1001a910(MechChar* p_text)
{
	MechChar* src;
	MechChar* dst;

	src = p_text;
	dst = p_text;
	while (*src) {
		*dst = *src++;
		if (isspace(*src)) {
			while (*src && isspace(*src)) {
				src++;
			}

			*++dst = ' ';
		}

		dst++;
	}

	*dst = '\0';
}

// STUB: MW2 0x1001aa02
void DoFirstObjtv(StarMission* p_unk0x00, MechS32 p_unk0x04)
{
	STUB(0x1001aa02);
}

// Returns the state bits (0xe) of an objective target: a player's or a game thing's flags. A target
// is an AI target id stored as two bytes, the index and the type.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1001ab4a
MechS32 FUN_1001ab4a(MechU8* p_target)
{
	MechU16 index;
	MechU16 kind;
	Player* player;
	GameThing* thing;

	index = p_target[0];
	kind = p_target[1] << 8;
	switch (kind) {
	case 0x200:
		player = g_players[index];
		return player->m_flags & 0xe;
	case 0x400:
		thing = &g_gameThings[index];
		return thing->m_unk0x00 & 0xe;
	case 0x100:
		return 0;
	}

	return 0;
}

// Returns whether team p_team has reached an objective target.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1001ac06
MechS32 FUN_1001ac06(MechU8* p_target, MechS32 p_team)
{
	MechU16 index;
	MechU16 kind;
	MechS32 reached;
	Player* player;
	GameThing* thing;
	NavPoint* nav;

	index = p_target[0];
	kind = p_target[1] << 8;
	reached = FALSE;
	switch (kind) {
	case 0x200:
		player = g_players[index];
		if ((player->m_flags & 0x20) && ((1 << p_team) & player->m_unk0x16)) {
			reached = TRUE;
		}
		break;
	case 0x400:
		thing = &g_gameThings[index];
		if ((thing->m_unk0x00 & 0x20) && ((1 << p_team) & thing->m_unk0x02)) {
			reached = TRUE;
		}
		break;
	case 0x100:
		nav = &g_navTable[index];
		if ((nav->m_flags & 0x20) && ((1 << p_team) & nav->m_unk0x26)) {
			reached = TRUE;
		}
		break;
	}

	return reached;
}

// Returns whether a live member of team p_team is near an objective target (within 20000, or a
// nav's radius). The local player reaching a nav marks it reached and plays sound 0xe7.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1001ad5b
MechS32 FUN_1001ad5b(MechU8* p_target, MechS32 p_team)
{
	MechU16 index;
	MechU16 kind;
	MechS32 i;
	Player* target;
	GameThing* thing;
	MechS32 x;
	MechS32 y;
	MechS32 z;
	MechS32 radius;
	NavPoint* nav;

	index = p_target[0];
	kind = p_target[1] << 8;
	if (g_teams[p_team].m_leader < 0 && !g_isNetworkGame) {
		return FALSE;
	}

	switch (kind) {
	case 0x200:
		target = g_players[index];
		for (i = 0; i < g_playerCount; i++) {
			if (g_players[i]->m_team == p_team && !(g_players[i]->m_flags & 6) &&
				FUN_10004ec0(
					g_players[i]->m_position.m_x - target->m_position.m_x,
					g_players[i]->m_position.m_y - target->m_position.m_y,
					g_players[i]->m_position.m_z - target->m_position.m_z,
					20000
				)) {
				return TRUE;
			}
		}
		return FALSE;
	case 0x400:
		thing = &g_gameThings[index];
		FUN_10020c6f(thing->m_unk0x04, &x, &y, &z);
		for (i = 0; i < g_playerCount; i++) {
			if (g_players[i]->m_team == p_team && FUN_10004ec0(
													  g_players[i]->m_position.m_x - x,
													  g_players[i]->m_position.m_y - y,
													  g_players[i]->m_position.m_z - z,
													  20000
												  )) {
				return TRUE;
			}
		}
		return FALSE;
	case 0x100:
		nav = &g_navTable[index];
		radius = nav->m_radius;
		if (radius <= 0) {
			radius = 20000;
		}

		for (i = 0; i < g_playerCount; i++) {
			if (g_players[i]->m_team == p_team && FUN_10004ec0(
													  g_players[i]->m_position.m_x - nav->m_position[0],
													  g_players[i]->m_position.m_y - nav->m_position[1],
													  g_players[i]->m_position.m_z - nav->m_position[2],
													  radius
												  )) {
				if (i == g_localPlayerId && (g_players[i]->m_flags & 0x2000) && !(nav->m_flags & 0x20) &&
					nav->m_unk0x00) {
					nav->m_flags |= 0x20;
					nav->m_unk0x26 |= 1 << p_team;
					FUN_1007eb23(0xe7, 100, 0x40, 5, 0x50);
				}

				return TRUE;
			}
		}
		return FALSE;
	}

	return FALSE;
}

// Announces the local team's objective p_objective as successful (p_state 5) or failed (6), once.
// Stack-slot permutation; the original computes the objective's index before the star's (index order)
// and compares p_star with g_unk0x100a5918 in the other operand order.
// FUNCTION: MW2 0x1001b0cb
MechS32 FUN_1001b0cb(MechS32 p_star, MechS32 p_objective, MechS32 p_state)
{
	MechChar text[100];
	SpeechLine line;
	MissionObjective* objective;

	objective = &g_objectiveTable[p_star].m_objectives[p_objective];
	if (p_star != g_unk0x100a5918 || g_unk0x10138760[p_objective]) {
		return TRUE;
	}

	if (p_state == 5) {
		sprintf(text, "%s successful", objective->m_name);
		line.m_id = objective->m_successSpeech;
		line.m_data = FUN_100073bb(objective->m_successSound);
	}
	else if (p_state == 6) {
		sprintf(text, "%s failed", objective->m_name);
		line.m_id = objective->m_failSpeech;
		line.m_data = FUN_100073bb(objective->m_failSound);
	}

	if (text[0] == ' ' || !objective->m_unk0x74) {
		line.m_text = "";
	}
	else {
		line.m_text = text;
	}

	FUN_1001a910(line.m_text);
	FUN_10059f9c(&line);
	g_unk0x10138760[p_objective] = TRUE;
	return TRUE;
}

// In a network game with a single listed objective, a secondary one, picks the player with the
// best kill score (kills of others minus kills of itself) as the winner, -2 on a tie.
// Stack-slot permutation; score > best compares in the other operand order.
// FUNCTION: MW2 0x1001b21a
void FUN_1001b21a(void)
{
	MissionObjective* objective;
	MechS32 best;
	MechS32 team;
	MechS32 winner;
	MechS32 i;
	MechS32 player;
	StarMission* mission;
	MechS32 victim;
	MechS32 score;
	MechS32 deathmatch;
	MechS32 secondary;
	MechS32 listed;

	best = -0x7fff;
	deathmatch = FALSE;
	mission = &g_objectiveTable[g_unk0x100a5918];
	team = g_unk0x100a5918;
	if (g_isNetworkGame && !g_difficulty->m_unk0x0a) {
		listed = 0;
		secondary = FALSE;
		for (i = 0; i < mission->m_objectiveCount; i++) {
			objective = &g_objectiveTable[g_unk0x100a5918].m_objectives[i];
			listed += objective->m_unk0x74;
			if (objective->m_unk0x74 && objective->m_priority == 2) {
				secondary = TRUE;
			}
		}

		if (secondary && listed == 1) {
			deathmatch = TRUE;
		}

		if (deathmatch) {
			winner = 0;
			for (player = 0; player < 8; player++) {
				score = 0;
				for (victim = 0; victim < 8; victim++) {
					if (player == victim) {
						score -= g_carCfg.m_unk0x52[player][victim];
					}
					else {
						score += g_carCfg.m_unk0x52[player][victim];
					}
				}

				if (score > best) {
					best = score;
					winner = player;
				}
				else if (score == best) {
					winner = -2;
				}
			}

			g_carCfg.m_unk0xd2 = winner;
		}
	}
}

// Announces the local team's mission result: successful (2), failed (3) or out of time (4). In a
// network game, only the first result; a successful one names the local player the winner.
// FUNCTION: MW2 0x1001b3f4
MechS32 FUN_1001b3f4(MechS32 p_star, MechS32 p_status)
{
	MechChar text[100];
	SpeechLine line;

	if (p_star != g_unk0x100a5918) {
		return TRUE;
	}

	if (g_unk0x100a374c && g_isNetworkGame) {
		return TRUE;
	}
	else {
		g_unk0x100a374c = 1;
	}

	if (p_status == 2) {
		sprintf(text, "Mission successful");
		line.m_id = g_objectiveTable[p_star].m_successSpeech;
		line.m_data = FUN_100073bb(g_objectiveTable[p_star].m_successSound);
		if (g_isNetworkGame) {
			g_carCfg.m_unk0xd2 = g_localPlayerId;
			FUN_1000ff29();
		}
	}
	else if (p_status == 3) {
		sprintf(text, "Mission failed");
		line.m_id = g_objectiveTable[p_star].m_failSpeech;
		line.m_data = FUN_100073bb(g_objectiveTable[p_star].m_failSound);
	}
	else if (p_status == 4) {
		sprintf(text, "Mission time exceeded");
		line.m_id = FindResourceIdByName(0xb, "BET68");
		line.m_data = FUN_100073bb("BET68");
	}

	line.m_text = text;
	FUN_10059f9c(&line);
	return TRUE;
}

// Returns whether condition p_condition of star p_star's objective p_objective holds.
// Stack-slot permutation of objective, other, star, state and kind.
// FUNCTION: MW2 0x1001b580
MechS32 FUN_1001b580(MechS32 p_star, MechS32 p_objective, MechS32 p_condition)
{
	MissionObjective* objective;
	MechS32 other;
	MechS32 star;
	MechS32 state;
	MechS32 kind;

	objective = &g_objectiveTable[p_star].m_objectives[p_objective];
	kind = objective->m_conditions[p_condition].m_kind;
	star = objective->m_conditions[p_condition].m_star;
	other = objective->m_conditions[p_condition].m_objective;
	state = g_objectiveTable[star].m_objectives[other].m_state;
	if ((kind == 1 && (state == 5 || state == 6)) || (kind == 2 && state == 5) || (kind == 3 && state == 6)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// Returns whether the conditions of star p_star's objective p_objective hold: one of them, or
// every one. A finished objective's don't; a type 0x10 objective's always do.
// Stack-slot permutation of all, objective, result, i and holds; the original computes the
// objective's index before the star's (index order).
// FUNCTION: MW2 0x1001b66c
MechS32 FUN_1001b66c(MechS32 p_star, MechS32 p_objective)
{
	MechS32 all;
	MissionObjective* objective;
	MechU32 result;
	MechS32 i;
	MechU32 holds;

	objective = &g_objectiveTable[p_star].m_objectives[p_objective];
	all = objective->m_allConditions;
	result = TRUE;
	if (objective->m_state == 5 || objective->m_state == 6) {
		return FALSE;
	}

	if (objective->m_type == 0x10) {
		return TRUE;
	}

	for (i = 0; i < 8; i++) {
		if (!objective->m_conditions[i].m_kind) {
			break;
		}

		holds = FUN_1001b580(p_star, p_objective, i);
		if (!all) {
			if (holds) {
				return TRUE;
			}
		}
		else {
			result &= holds;
			if (!result) {
				return FALSE;
			}
		}
	}

	if (!all) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

// STUB: MW2 0x1001c69e
void UpdateObjectives(void)
{
	STUB(0x1001c69e);
}

// Counts the mission time in seconds.
// FUNCTION: MW2 0x1001c9d5
void EndTheMission1(void)
{
	g_missionTime = g_currentClock / 181;
	return;
}

// STUB: MW2 0x1001c9f7
void EndTheMission2(void)
{
	STUB(0x1001c9f7);
}

// STUB: MW2 0x1001cc5c
void FUN_1001cc5c(MechS32 p_player)
{
	STUB(0x1001cc5c);
}

// FUNCTION: MW2 0x1001cdd1
void FUN_1001cdd1(void)
{
	return;
}

// FUNCTION: MW2 0x1001cde1
MechS32 FUN_1001cde1(undefined4 p_unk0x00)
{
	return 1;
}

// The target of team p_team's return objective (type 0x20), or else its first objective's
// first target: the AI's "home" nav.
// Stack-slot permutation: mission and i.
// FUNCTION: MW2 0x1001cdf6
MechU16 GetTeamHomeTarget(MechS32 p_team)
{
	StarMission* mission;
	MechS32 i;

	mission = &g_objectiveTable[p_team];
	for (i = 0; i < mission->m_objectiveCount; i++) {
		if (mission->m_objectives[i].m_type == 0x20) {
			return mission->m_objectives[i].m_targets[0];
		}
	}

	return mission->m_objectives[0].m_targets[0];
}
