#include "team.h"

#include "ai.h"
#include "decomp.h"
#include "fixedfloat.h"
#include "gamething.h"
#include "object.h"
#include "players.h"
#include "speech.h"
#include "targeting.h"
#include "transform.h"
#include "types.h"

#include <string.h>

// Teams of players: each has a leader and up to eight members, a side, and a formation that
// places each member (by its slot) relative to the leader. Formations are copied from a
// table of named templates.

DECOMP_SIZE_ASSERT(TeamFormation, 0x70)
DECOMP_SIZE_ASSERT(Team, 0x38)

// GLOBAL: MW2 0x100a5918
// GLOBAL: MW2MATROX 0x100ae338
MechS32 g_localStar = -1;

// GLOBAL: MW2 0x100a591c
// GLOBAL: MW2MATROX 0x100ae33c
MechS32 g_formationTemplateCount = 0;

// GLOBAL: MW2 0x10109c90
// GLOBAL: MW2MATROX 0x10130400
TeamFormation g_formationTemplates[32];

// GLOBAL: MW2 0x1010aa90
// GLOBAL: MW2MATROX 0x10131200
Team g_teams[16];

// GLOBAL: MW2 0x1010ae10
// GLOBAL: MW2MATROX 0x10131580
MechS32 g_starSides[8];

// GLOBAL: MW2 0x1010ae30
// GLOBAL: MW2MATROX 0x1012fd00
TeamFormation g_teamFormations[16];

// FUNCTION: MW2 0x1003bbe0
// FUNCTION: MW2MATROX 0x10072cc0
void ResetTeams(void)
{
	MechU32 i;

	for (i = 0; i < 16; i++) {
		g_teams[i].m_leader = 0;
		g_teams[i].m_formation = 0;
		g_teamFormations[i] = g_formationTemplates[0];
	}
}

// Operand order: the loop test (i < g_formationTemplateCount) compares with i in eax in the
// original.
// FUNCTION: MW2 0x1003bc55
// FUNCTION: MW2MATROX 0x10072d35
MechS32 SetTeamFormationByName(MechS32 p_team, const MechChar* p_name)
{
	MechS32 result = FALSE;
	MechS32 i;

	if (p_team >= 16) {
		return FALSE;
	}

	for (i = 0; i < g_formationTemplateCount; i++) {
		if (_strcmpi(g_formationTemplates[i].m_name, p_name) == 0) {
			g_teams[p_team].m_formation = i;
			g_teamFormations[p_team] = g_formationTemplates[i];
			result = TRUE;
			break;
		}
	}

	return result;
}

// FUNCTION: MW2 0x1003bd1a
// FUNCTION: MW2MATROX 0x10072df9
MechS32 GetTeamFormation(MechS32 p_team)
{
	if (p_team >= 16) {
		return 0;
	}

	return g_teams[p_team].m_formation;
}

// FUNCTION: MW2 0x1003bd4c
// FUNCTION: MW2MATROX 0x10072e2b
void SetTeamFormation(MechS32 p_team, MechS32 p_formation)
{
	if (p_team >= 16 || p_formation >= 32) {
		return;
	}

	if (p_team == g_localStar) {
		SayFormation(p_formation);
	}

	g_teams[p_team].m_formation = p_formation;
	g_teamFormations[p_team] = g_formationTemplates[p_formation];
}

// Operand order: the original compares p_player < g_playerCount with p_player in eax.
// FUNCTION: MW2 0x1003bdcb
// FUNCTION: MW2MATROX 0x10072eaa
MechS32 SetPlayerSlot(MechU32 p_player, MechU32 p_slot)
{
	MechS32 result = FALSE;

	if (p_player < g_playerCount && p_slot < 8) {
		g_players[p_player]->m_slot = p_slot;
		result = TRUE;
	}

	return result;
}

// Makes a player the team's leader and moves the formation's origin to the leader's slot.
// Stack-slot permutation: result, i, dz and player. Operand order: the original compares
// p_player < g_playerCount with p_player in eax.
// FUNCTION: MW2 0x1003be18
// FUNCTION: MW2MATROX 0x10072ef7
MechS32 SetTeamLeader(MechS32 p_team, MechS32 p_player)
{
	MechS32 result;
	MechU16 i;
	MechScalar dz;
	Player* player;
	MechScalar dx;

	result = FALSE;
	if (p_player == -1) {
		g_teams[p_team].m_leader = p_player;
	}
	else if (p_player < g_playerCount && p_team < 16) {
		g_teams[p_team].m_leader = p_player;

		player = g_players[p_player];
		dx = g_teamFormations[p_team].m_x[player->m_slot];
		dz = g_teamFormations[p_team].m_z[player->m_slot];
		for (i = 0; i < 8; i++) {
			g_teamFormations[p_team].m_x[i] -= dx;
			g_teamFormations[p_team].m_z[i] -= dz;
		}

		result = TRUE;
	}

	return result;
}

// Places the team's leader, then every member at its formation slot around the leader.
// Stack-slot permutation: result, z, heading, x and i. Operand order: the original compares
// leader < g_playerCount with g_playerCount in eax.
// FUNCTION: MW2 0x1003bf40
// FUNCTION: MW2MATROX 0x1007304d
MechS32 PlaceTeam(MechS32 p_team, MechScalar p_x, MechScalar p_y, MechScalar p_z, MechScalar p_heading)
{
	MechS32 result;
	MechScalar z;
	MechU32 leader;
	Player* player;
	MechScalar heading;
	MechScalar x;
	MechU32 i;

	result = FALSE;
	if (p_team < 16 && (leader = g_teams[p_team].m_leader) < g_playerCount) {
		player = g_players[leader];
		player->m_position.m_x = p_x;
		player->m_position.m_y = p_y;
		player->m_position.m_z = p_z;
		player->m_heading = p_heading;

		for (i = 0; i < g_playerCount; i++) {
			player = g_players[i];
			if (player->m_team == p_team) {
				GetTeamSlotPosition(i, &x, &z, &heading);
				SetObjPosition(player->m_obj, x, p_y, z);
				SetObjRotation(player->m_obj, 0, heading, 0, 0);
				UpdateObj(player->m_obj);
				player->m_position.m_x = x;
				player->m_position.m_y = p_y;
				player->m_position.m_z = z;
				player->m_heading = heading;
			}
		}

		result = TRUE;
	}

	return result;
}

// Returns where a player's formation slot puts it: the leader's own position, or the slot's
// offset transformed by the leader's orientation.
// Stack-slot permutation: slot, leader, matrix, obj, y and team. Operand order: the original
// compares p_player >= g_playerCount with p_player in eax. MW2MATROX: the same comparisons
// with the operands the other way round, and g_teamFormations indexed with the slot loaded
// second.
// FUNCTION: MW2 0x1003c07e
// FUNCTION: MW2MATROX 0x1007318d
MechS32 GetTeamSlotPosition(MechU32 p_player, MechScalar* p_x, MechScalar* p_z, MechScalar* p_heading)
{
	MechS32 slot;
	MechU32 leader;
	Matrix* matrix;
	SceneObject* obj;
	MechScalar y;
	MechS32 team;

	y = 0;
	if (p_player >= g_playerCount) {
		return FALSE;
	}

	team = g_players[p_player]->m_team;
	leader = GetTeamLeader(team);
	slot = g_players[p_player]->m_slot;
	if (leader != p_player) {
		*p_x = g_teamFormations[team].m_x[slot];
		*p_z = g_teamFormations[team].m_z[slot];
		*p_heading = g_teamFormations[team].m_heading[slot];
		obj = g_players[leader]->m_obj;
		matrix = GetObjWorldMatrix(obj);
		TransformPoint(matrix, p_x, &y, p_z);
	}
	else {
		*p_x = g_players[leader]->m_position.m_x;
		*p_z = g_players[leader]->m_position.m_z;
		*p_heading = g_players[leader]->m_heading;
	}

	return TRUE;
}

// FUNCTION: MW2 0x1003c1b4
// FUNCTION: MW2MATROX 0x100732c3
MechS32 GetTeamLeader(MechS32 p_team)
{
	MechS32 result = 0;

	if (p_team < 16) {
		result = g_teams[p_team].m_leader;
	}

	return result;
}

// Numbers the team's active members' formation slots from 1, the leader taking slot 0.
// Stack-slot permutation: team, player, i, slot and leader.
// FUNCTION: MW2 0x1003c1ef
// FUNCTION: MW2MATROX 0x100732fe
MechS32 AssignTeamSlots(MechS32 p_team, MechS32 p_unk0x04)
{
	Team* team;
	Player* player;
	MechS16 i;
	MechS16 slot;
	MechS16 leader;

	team = &g_teams[p_team];
	leader = GetTeamLeader(p_team);
	if (leader == -1) {
		leader = ChooseTeamLeader(p_team);
		if (leader == -1) {
			return FALSE;
		}
	}

	for (i = 0, slot = 1; i < team->m_memberCount; i++) {
		player = g_players[team->m_members[i]];
		if (player->m_type == c_playerTypeMech && !(player->m_flags & 6)) {
			if (player->m_index == leader) {
				player->m_slot = 0;
			}
			else {
				player->m_slot = slot;
				slot++;
			}
		}
	}

	return TRUE;
}

// FUNCTION: MW2 0x1003c2e3
// FUNCTION: MW2MATROX 0x100733f2
MechS32 GetPlayerSide(MechS32 p_player)
{
	return g_teams[g_players[p_player]->m_team].m_side;
}

// FUNCTION: MW2 0x1003c30e
// FUNCTION: MW2MATROX 0x1007341d
MechS32 GetThingSide(MechS32 p_thing)
{
	MechS32 index;

	index = g_gameThings[p_thing].m_affiliation;
	if (index < 0) {
		return 2;
	}
	else {
		return g_starSides[index];
	}
}

// The side of the team that owns nav p_nav; 2 past the first 16 navs.
// FUNCTION: MW2 0x1003c353
// FUNCTION: MW2MATROX 0x10073462
MechS32 GetNavSide(MechU32 p_nav)
{
	MechS32 side;

	side = 2;
	if (p_nav < 16) {
		side = g_teams[g_navTable[p_nav].m_team].m_side;
	}

	return side;
}

// Operand order: MW2MATROX calls GetPlayerSide(p_playerA) first (either order of the source
// compiles the same).
// FUNCTION: MW2 0x1003c39d
// FUNCTION: MW2MATROX 0x100734ac
MechS32 OnSameSide(MechS32 p_playerA, MechS32 p_playerB)
{
	return GetPlayerSide(p_playerB) == GetPlayerSide(p_playerA);
}
