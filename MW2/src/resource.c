#include "resource.h"

#include "ai.h"
#include "bwd.h"
#include "bwdobjectrecord.h"
#include "bwdrecord.h"
#include "bwdstreamkey.h"
#include "config.h"
#include "decomp.h"
#include "error.h"
#include "geocache.h"
#include "includerecord.h"
#include "includerecord2.h"
#include "loadres.h"
#include "missiontable.h"
#include "mw2prj.h"
#include "object.h"
#include "players.h"
#include "prjfile.h"
#include "scenariotable.h"
#include "simmain.h"
#include "team.h"
#include "transform.h"
#include "types.h"
#include "unk1001ce90.h"
#include "unk1001df00.h"
#include "unk10034a40.h"
#include "unk10039a30.h"
#include "unk1003a530.h"
#include "unk10046750.h"
#include "unk1004da30.h"
#include "unk1006d680.h"
#include "unk1007f140.h"
#include "weapons.h"

#include <io.h>
#include <math.h>
#include <mbstring.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

// A bitmap record: its size and palette-sized data.
typedef struct BitmapRecord {
	BwdRecord m_header; // 0x00
	MechS32 m_width;    // 0x08
	MechS32 m_height;   // 0x0c
	MechS32 m_data[1];  // 0x10 — up to the record's end
} BitmapRecord;

// A path record: the path's name and its waypoints.
typedef struct PathRecord {
	BwdRecord m_header;    // 0x00
	MechChar m_name[0x40]; // 0x08
	PathPoint m_points[1]; // 0x48 — up to the record's end
} PathRecord;

// A formation record's slot: its offset from the leader and its heading.
typedef struct FormationSlot {
	MechS32 m_x;       // 0x00
	MechS32 m_z;       // 0x04
	MechS32 m_heading; // 0x08
} FormationSlot;

// A formation record: the formation's name and its slots.
typedef struct FormationRecord {
	BwdRecord m_header;       // 0x00
	MechChar m_name[0x10];    // 0x08
	FormationSlot m_slots[1]; // 0x18 — up to the record's end
} FormationRecord;

// A star record's entry: a team's two values and its formation's name.
typedef struct StarEntry {
	MechS32 m_unk0x00;     // 0x00
	MechS32 m_side;        // 0x04
	MechChar m_name[0x10]; // 0x08
} StarEntry;

typedef struct StarTable {
	BwdRecord m_header;   // 0x00
	StarEntry m_stars[1]; // 0x08 — up to the record's end
} StarTable;

// A formation-name record: the sixteen teams' formations.
typedef struct FormationNames {
	BwdRecord m_header;         // 0x00
	MechChar m_names[16][0x11]; // 0x08
} FormationNames;

// GLOBAL: MW2 0x100a8608
ScenarioTable* g_scenarios = NULL;

// GLOBAL: MW2 0x100a860c
void* g_unk0x100a860c = NULL;

// The next name of g_scenarios ExecuteInclude substitutes.
// GLOBAL: MW2 0x100a8610
MechS32 g_nextScenario = 0;

// The number of entries in g_unk0x100ea580, and the next one FUN_100506d8 returns.
// GLOBAL: MW2 0x100a8620
MechS32 g_unk0x100a8620 = 0;

// GLOBAL: MW2 0x100a8624
MechS32 g_unk0x100a8624 = 0;

// GLOBAL: MW2 0x100a8628
MechS32 g_unk0x100a8628 = 0;

// The mangle base of the world stream's next mangle_on section (BwdExecuteStream).
// GLOBAL: MW2 0x100a862c
MechS32 g_unk0x100a862c = 0;

// The formation LoadStarTable gives the player's team, or NULL to use the record's.
// GLOBAL: MW2 0x100a8630
MechChar* g_unk0x100a8630 = NULL;

// The formation LoadStarTable gives the other teams, or NULL to use the record's.
// GLOBAL: MW2 0x100a8634
MechChar* g_unk0x100a8634 = NULL;

// The player the world stream (BwdExecuteStream) created last, for FUN_10046750.
// GLOBAL: MW2 0x100a8638
struct Player* g_unk0x100a8638 = NULL;

// The task kinds of a world stream's task record (BwdExecuteStream).
// GLOBAL: MW2 0x100a8640
TimedCallbackFn g_unk0x100a8640[6] =
	{FUN_1004771e, FUN_1004748c, FUN_100479ec, FUN_10046750, FUN_10047d10, FUN_10047f60};

// GLOBAL: MW2 0x100ea500
MissionTable* g_missionTables[16];

// The number of entries of each of g_missionTables.
// GLOBAL: MW2 0x100ea540
MechS32 g_missionTableCounts[16];

// GLOBAL: MW2 0x100ea580
MechS32 g_unk0x100ea580[0x96];

// The number of names in g_scenarios.
// GLOBAL: MW2 0x100ea7d8
MechS32 g_scenarioCount;

// Copies a bitmap record's size to p_width and p_height and its data to p_data, each if not NULL.
// Stack-slot permutation: record, count and i (and so the loop test's operand order).
// FUNCTION: MW2 0x1004f3f0
void LoadMapBitmap(BwdRecord* p_record, MechS32* p_width, MechS32* p_height, MechS32* p_data)
{
	BitmapRecord* record;
	MechS32 count;
	MechS32 i;

	record = (BitmapRecord*) p_record;
	if (p_width) {
		*p_width = record->m_width;
	}

	if (p_height) {
		*p_height = record->m_height;
	}

	if (p_data) {
		count = (record->m_header.m_size - 0x10) >> 2;
		for (i = 0; i < count; i++) {
			p_data[i] = record->m_data[i];
		}
	}
}

// Replaces the scenario table with a copy of p_table.
// FUNCTION: MW2 0x1004f47a
MechS32 LoadScenarioTable(ScenarioTable* p_table)
{
	MechS32 result;

	result = FALSE;
	if (g_scenarios) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_scenarios);
	}

	g_scenarios = NULL;
	g_scenarioCount = 0;
	g_nextScenario = 0;
	g_scenarios = MemAlloc(p_table->m_header.m_size);
	if (g_scenarios) {
		memcpy(g_scenarios, p_table, p_table->m_header.m_size);
		g_scenarioCount = (g_scenarios->m_header.m_size - 8) / 0xc;
		result = TRUE;
	}
	else {
		Error(0x3f, NULL);
	}

	return result;
}

// Replaces the mission table in p_table's slot with a copy of it.
// FUNCTION: MW2 0x1004f545
MechS32 LoadMissionTable(MissionTable* p_table)
{
	MechS32 result;
	MechS32 slot;

	result = FALSE;
	slot = p_table->m_star;
	if (g_missionTables[slot]) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_missionTables[slot]);
	}

	g_missionTables[slot] = NULL;
	g_missionTableCounts[slot] = 0;
	g_missionTables[slot] = MemAlloc(p_table->m_header.m_size);
	if (!g_missionTables[slot]) {
		Error(0x40, NULL);
	}
	else {
		result = TRUE;
		memcpy(g_missionTables[slot], p_table, p_table->m_header.m_size);
		g_missionTableCounts[slot] =
			(g_missionTables[slot]->m_header.m_size - sizeof(BwdRecord)) / sizeof(MissionEntry);
		FUN_1004da30(g_missionTables[slot]);
	}

	return result;
}

// Adds a path to g_paths, or returns FALSE if the table or the path is full.
// Stack-slot permutation: count, index, i and record (and so the loop test's operand order and
// the order of the waypoint index's scaling).
// FUNCTION: MW2 0x1004f64a
MechS32 LoadPathTable(PathRecord* p_record)
{
	MechS32 count;
	MechS32 result;
	MechS32 index;
	MechS32 i;
	PathRecord* record;

	result = FALSE;
	record = p_record;
	count = (record->m_header.m_size - 0x48) / 0x1c;
	if (g_pathCount >= 0x40 || count > 0x40) {
		return result;
	}

	index = g_pathCount++;
	g_paths[index].m_count = count;
	strncpy(g_paths[index].m_name, record->m_name, 0x40);
	for (i = 0; i < count; i++) {
		g_paths[index].m_points[i].m_x = record->m_points[i].m_x;
		g_paths[index].m_points[i].m_y = record->m_points[i].m_y;
		g_paths[index].m_points[i].m_z = record->m_points[i].m_z;
		g_paths[index].m_points[i].m_unk0x0c = record->m_points[i].m_unk0x0c << 16;
		g_paths[index].m_points[i].m_unk0x10 = record->m_points[i].m_unk0x10 << 16;
		g_paths[index].m_points[i].m_unk0x14 = record->m_points[i].m_unk0x14 << 16;
		g_paths[index].m_points[i].m_unk0x18 = record->m_points[i].m_unk0x18;
	}

	return TRUE;
}

// Adds a formation to g_formationTemplates, or returns FALSE if the table or the formation is
// full.
// Stack-slot permutation: count, index, i and record (and so the loop test's operand order).
// FUNCTION: MW2 0x1004f891
MechS32 LoadFormationTable(FormationRecord* p_record)
{
	MechS32 count;
	MechS32 result;
	MechS32 index;
	MechS32 i;
	FormationRecord* record;

	result = FALSE;
	record = p_record;
	count = (record->m_header.m_size - 0x18) / 0xc;
	if (g_formationTemplateCount >= 0x20 || count > 8) {
		return result;
	}

	index = g_formationTemplateCount++;
	strncpy(g_formationTemplates[index].m_name, record->m_name, 0x10);
	for (i = 0; i < count; i++) {
		g_formationTemplates[index].m_x[i] = record->m_slots[i].m_x;
		g_formationTemplates[index].m_z[i] = record->m_slots[i].m_z;
		g_formationTemplates[index].m_heading[i] = record->m_slots[i].m_heading;
	}

	return TRUE;
}

// Runs p_fn on the stream an include record names. A name of "^" with id -1 or -2 takes the
// scenario table's next name.
// Stack-slot permutation: record, keyData, ok and buffer.
// FUNCTION: MW2 0x1004f9a8
MechS32 ExecuteInclude(IncludeRecord* p_record, BwdStreamFn p_fn)
{
	undefined buffer[0x20];
	BwdStreamKey keyData;
	IncludeRecord* record;
	MechS32 ok;
	BwdStreamKey* key;
	BwdStream* stream;
	MechS32 result;

	record = p_record;
	key = &keyData;
	ok = TRUE;
	result = FALSE;
	key->m_id = record->m_id;
	strncpy(key->m_name, record->m_name, 0xc);
	key->m_name[0xc] = '\0';
	if ((key->m_id == -2 || key->m_id == -1) && key->m_name[0] == '^') {
		if (!g_scenarios) {
			Error(0x39, NULL);
			ok = FALSE;
		}
		else if (g_nextScenario < g_scenarioCount) {
			strncpy(key->m_name, g_scenarios->m_names[g_nextScenario], 0xc);
			g_nextScenario++;
			key->m_name[0xc] = '\0';
		}
		else {
			Error(0x3b, NULL);
			ok = FALSE;
		}
	}

	if (ok) {
		stream = OpenBwdStream(key, (BwdStream*) buffer);
		if (stream) {
			result = TRUE;
			result &= p_fn(stream);
			UnloadResource(stream);
		}
		else {
			Error(0x34, NULL);
		}
	}

	return result;
}

// Sets up the teams from a star record: each team's values, and its formation.
// FUNCTION: MW2 0x1004fb0b
void LoadStarTable(StarTable* p_table)
{
	MechS32 count;
	StarTable* table;
	MechS32 i;

	table = p_table;
	count = (table->m_header.m_size - 8) / sizeof(StarEntry);
	for (i = 0; i < count; i++) {
		g_teams[i].m_unk0x08 = table->m_stars[i].m_unk0x00;
		g_teams[i].m_side = table->m_stars[i].m_side;
		g_unk0x1010ae10[table->m_stars[i].m_unk0x00] = table->m_stars[i].m_side;
		if (g_unk0x100a5918 + 1 == i && g_unk0x100a8630) {
			SetTeamFormationByName(i, g_unk0x100a8630);
		}
		else if (g_unk0x100a5918 + 1 != i && g_unk0x100a8634) {
			SetTeamFormationByName(i, g_unk0x100a8634);
		}
		else {
			SetTeamFormationByName(i, table->m_stars[i].m_name);
		}

		g_teams[i].m_unk0x0c = 0;
	}
}

// Gives the sixteen teams the formations a record names.
// FUNCTION: MW2 0x1004fc48
void FUN_1004fc48(FormationNames* p_record)
{
	FormationNames* record;
	MechS32 i;

	record = p_record;
	for (i = 0; i < 16; i++) {
		SetTeamFormationByName(i, record->m_names[i]);
		g_teams[i].m_unk0x0c = 0;
	}
}

// Runs p_fn on the stream a record names.
// Stack-slot permutation: record, keyData and buffer.
// FUNCTION: MW2 0x1004fcac
MechS32 FUN_1004fcac(IncludeRecord2* p_record, BwdStreamFn p_fn)
{
	undefined buffer[0x20];
	BwdStreamKey keyData;
	MechS32 id;
	IncludeRecord2* record;
	BwdStreamKey* key;
	BwdStream* stream;
	MechS32 result;

	record = p_record;
	key = &keyData;
	result = FALSE;
	id = record->m_id;
	strncpy(key->m_name, record->m_name, 0xc);
	key->m_name[0xc] = '\0';
	key->m_id = id;
	stream = OpenBwdStream(key, (BwdStream*) buffer);
	if (stream) {
		result = TRUE;
		result = p_fn(stream);
		UnloadResource(stream);
	}
	else {
		Error(0x35, NULL);
	}

	return result;
}

// Frees the mission tables' blocks.
// FUNCTION: MW2 0x1004fd55
void FUN_1004fd55(void)
{
	MechS32 i;

	if (g_scenarios) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_scenarios);
	}

	g_scenarios = NULL;
	for (i = 0; i < 16; i++) {
		if (g_missionTables[i]) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_missionTables[i]);
		}

		g_missionTables[i] = NULL;
	}

	if (g_unk0x100a860c) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_unk0x100a860c);
	}

	g_unk0x100a860c = NULL;
}

// Scales the vector (p_a, p_b, p_c) to integers whose absolute values add up to 2^29. A vector
// that small is left alone.
// FUNCTION: MW2 0x1004fe0f
void FUN_1004fe0f(MechFloat p_a, MechFloat p_b, MechFloat p_c, undefined4* p_x, undefined4* p_y, undefined4* p_z)
{
	MechFloat scale;

	scale = fabs(p_a);
	scale += fabs(p_b);
	if ((scale += fabs(p_c)) > 0.0001) {
		scale /= 536870912.0;
		*p_x = p_a / scale;
		*p_y = p_b / scale;
		*p_z = p_c / scale;
	}
}

// Scales the plane (p_a, p_b, p_c, p_d) like FUN_1004fe0f.
// FUNCTION: MW2 0x1004fe85
void FUN_1004fe85(
	MechFloat p_a,
	MechFloat p_b,
	MechFloat p_c,
	MechFloat p_d,
	undefined4* p_x,
	undefined4* p_y,
	undefined4* p_z,
	undefined4* p_w
)
{
	MechFloat scale;

	scale = fabs(p_a);
	scale += fabs(p_b);
	scale += fabs(p_c);
	if ((scale += fabs(p_d)) > 0.0001) {
		scale /= 536870912.0;
		*p_x = p_a / scale;
		*p_y = p_b / scale;
		*p_z = p_c / scale;
		*p_w = p_d / scale;
	}
}

// The normal of the triangle (p_x1, p_y1, p_z1), (p_x2, p_y2, p_z2), (p_x3, p_y3, p_z3), scaled
// by FUN_1004fe0f.
// Stack-slot permutation of a and c (and so the operand order of vz * ux).
// FUNCTION: MW2 0x1004ff16
void FUN_1004ff16(
	MechFloat p_x1,
	MechFloat p_y1,
	MechFloat p_z1,
	MechFloat p_x2,
	MechFloat p_y2,
	MechFloat p_z2,
	MechFloat p_x3,
	MechFloat p_y3,
	MechFloat p_z3,
	undefined4* p_x,
	undefined4* p_y,
	undefined4* p_z
)
{
	MechFloat a;
	MechFloat b;
	MechFloat c;
	MechFloat ux;
	MechFloat uy;
	MechFloat uz;
	MechFloat vx;
	MechFloat vy;
	MechFloat vz;

	ux = p_x2 - p_x1;
	uy = p_y2 - p_y1;
	uz = p_z2 - p_z1;
	vx = p_x3 - p_x1;
	vy = p_y3 - p_y1;
	/* The original takes p_y1, not p_z1, from p_z3. */
	a = (vz = p_z3 - p_y1) * uy - vy * uz;
	b = vz * ux - vx * uz;
	c = vy * ux - vx * uy;
	FUN_1004fe0f(a, b, c, p_x, p_y, p_z);
}

// The plane of the triangle, scaled by FUN_1004fe85.
// Stack-slot permutation of the locals, which also swaps the operands of two products.
// FUNCTION: MW2 0x1004ffaa
void FUN_1004ffaa(
	MechFloat p_x1,
	MechFloat p_y1,
	MechFloat p_z1,
	MechFloat p_x2,
	MechFloat p_y2,
	MechFloat p_z2,
	MechFloat p_x3,
	MechFloat p_y3,
	MechFloat p_z3,
	undefined4* p_x,
	undefined4* p_y,
	undefined4* p_z,
	undefined4* p_w
)
{
	MechFloat a;
	MechFloat b;
	MechFloat c;
	MechFloat d;
	MechFloat ux;
	MechFloat uy;
	MechFloat uz;
	MechFloat vx;
	MechFloat vy;
	MechFloat vz;

	ux = p_x2 - p_x1;
	uy = p_y2 - p_y1;
	uz = p_z2 - p_z1;
	vx = p_x3 - p_x1;
	vy = p_y3 - p_y1;
	/* The original takes p_y1, not p_z1, from p_z3. */
	a = (vz = p_z3 - p_y1) * uy - vy * uz;
	b = vz * ux - vx * uz;
	d = -((c = vy * ux - vx * uy) * p_z1 + b * p_y1 + a * p_x1);
	FUN_1004fe85(a, b, c, d, p_x, p_y, p_z, p_w);
}

/* The only diff is the order of the three products in d (and of each product's operands), which
   follows the symbol table, not the source. It matches with seven more symbols declared ahead of
   the function (placeholder prototypes do it); stubbing the object's other functions does not. */
// FUNCTION: MW2 0x1005005e
void FUN_1005005e(
	MechFloat p_x,
	MechFloat p_y,
	MechFloat p_z,
	MechFloat p_nx,
	MechFloat p_ny,
	MechFloat p_nz,
	undefined4* p_unk0x18,
	undefined4* p_unk0x1c,
	undefined4* p_unk0x20,
	undefined4* p_unk0x24
)
{
	MechFloat a;
	MechFloat b;
	MechFloat c;
	MechFloat d;

	a = p_nx;
	b = -p_ny;
	c = p_nz;
	d = -(p_x * a + p_y * b + p_z * c);
	FUN_1004fe85(a, b, c, d, p_unk0x18, p_unk0x1c, p_unk0x20, p_unk0x24);
}

// The original loads p_id first; the operand order follows the symbol table.
// FUNCTION: MW2 0x100500c3
MechS32 MapResourceId(MechS32 p_id)
{
	return g_unk0x100a8628 + p_id;
}

// FUNCTION: MW2 0x100500dc
void SetMangleBase(MechS32 p_base)
{
	g_unk0x100a8628 = p_base;
}

// Creates the shape of a world stream's object record: a static object of the current block
// (p_static), an entry of the class table (p_class, for level p_level), or else a shape of its own,
// in the world or under its parent's object.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x100500ef
void CreateObjectNode(
	BwdObjectRecord* p_record,
	undefined4 p_unk0x04,
	MechS32 p_unk0x08,
	MechS32 p_static,
	MechS32 p_class,
	MechS32 p_level
)
{
	MechS32 offset;
	MechS32 unk0x34;
	TwilightGrove0x24 xform;
	MechS32 parent;
	MechS32 fromResource;
	MechS32 parentIndex;
	MechU8* data;
	MechS32 flags;
	MechS32 kind;
	MechChar name[13];
	BwdObjectRecord* record;
	MechS32 mapped;
	ScarletOrchid0x4c* shape;
	MechS32 thing;
	Matrix matrix;
	MechS32 resource;
	MechS32 first;
	MechS32 id;
	MechS32 size;
	MechS32 handle;
	AmberWillow0x7c* parentObj;
	AmberWillow0x7c* obj;
	ScarletOrchid0x4c* classShape;

	record = p_record;
	flags = 0;
	data = NULL;
	size = 0;
	offset = 0;
	first = TRUE;
	fromResource = FALSE;
	resource = record->m_resource;
	id = record->m_id;
	parent = record->m_parent;
	kind = record->m_kind;
	unk0x34 = record->m_unk0x34;
	if (kind < 0 || kind >= 8) {
		kind = 4;
	}

	xform = record->m_xform;
	flags = record->m_flags;
	if (resource != -1) {
		data = FUN_1001a19f(g_mw2PrjHandle, resource, g_resourceTypeTags[c_resTagPoly], 0);
		if (data) {
			size = GetPrjResourceSize(g_mw2PrjHandle, g_resourceTypeTags[c_resTagPoly], resource);
			fromResource = TRUE;
		}
		else {
			Error(0x38, NULL);
			return;
		}
	}

	if (fromResource) {
		if (p_static && g_currentBlock != -1) {
			parentIndex = -1;
			if (parent == -2) {
				parentIndex = -2;
			}
			else if (parent != -1) {
				parent = MapResourceId(parent);
				parentIndex = FindStarIdxById(parent);
				if (parentIndex == -1) {
					Error(0x2d, NULL);
				}
			}

			id = MapResourceId(id);
			FUN_1001f8b5(id, resource, xform, g_currentBlock, parentIndex, p_unk0x08, flags, kind, unk0x34);
			FUN_1001a163(resource, g_resourceTypeTags[c_resTagPoly]);
			return;
		}
		else if (p_class && g_unk0x1010b6a0 > g_unk0x1012b7b0) {
			g_unk0x100a3854[g_unk0x1012b7b0] = MapResourceId(id);
			mapped = MapResourceId(id);
			thing = FindThingIdxById(mapped);
			parent = MapResourceId(parent);
			parentIndex = FindThingIdxById(parent);
			g_unk0x100a3850[g_unk0x1012b7b0] = FUN_1001cf93(
				resource,
				xform.m_unk0x18,
				xform.m_unk0x1c,
				xform.m_unk0x20,
				parentIndex,
				p_level,
				thing,
				unk0x34
			);
			g_unk0x1012b7b0++;
			FUN_1001a163(resource, g_resourceTypeTags[c_resTagPoly]);
			return;
		}
	}

	if (!fromResource) {
		strncpy(name, record->m_file, 12);
		name[12] = '\0';
		if (strlen(name) <= 8) {
			strcat(name, ".wtb");
		}

		handle = LoadFile(BuildGamePath(name), &size, &data, NULL);
		if (handle != -1) {
			_close(handle);
		}
		else {
			Error(0x36, NULL);
			return;
		}
	}

	SetShapeScale(xform.m_unk0x00, xform.m_unk0x04, xform.m_unk0x08);
	SetShapeFlags(flags);
	shape = LoadShapes(data, &offset, size, NULL);
	if (shape) {
		shape->m_unk0x02 = unk0x34;
		if (kind >= 0 && kind < 8) {
			FUN_10034a40(shape, kind);
		}
		else {
			FUN_10034a40(shape, 4);
			Error(0x37, NULL);
		}

		if (first) {
			mapped = MapResourceId(id);
			FUN_1001f504(mapped, shape);
		}

		first = FALSE;
		if (parent == -2) {
			FUN_1000e2b9(
				&matrix,
				xform.m_unk0x0c,
				xform.m_unk0x10,
				xform.m_unk0x14,
				xform.m_unk0x18,
				xform.m_unk0x1c,
				xform.m_unk0x20
			);
			FUN_10039c36(shape, &matrix);
			FUN_1006d732(shape);
			if (shape->m_unk0x24 == 5) {
				FUN_1001df00(shape);
			}
		}
		else {
			parentObj = NULL;
			parent = MapResourceId(parent);
			classShape = FindClassById(parent);
			if (classShape) {
				parentObj = FUN_1003b6e5(classShape);
			}

			obj = FUN_100012d0(parentObj, 10);
			FUN_10001532(obj, shape);
			FUN_1003b6fb(shape, obj);
			SetObjRotation(obj, xform.m_unk0x0c, xform.m_unk0x10, xform.m_unk0x14, 0);
			SetObjPosition(obj, xform.m_unk0x18, xform.m_unk0x1c, xform.m_unk0x20);
			FUN_10001cf8(obj);
			FUN_1006d732(shape);
		}
	}

	if (!fromResource) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, data);
	}
	else {
		FUN_1001a4e5(resource, g_resourceTypeTags[c_resTagPoly]);
	}
}

// Returns the object of the next entry of g_unk0x100ea580, or NULL after the last.
// Stack-slot permutation of id and obj.
// FUNCTION: MW2 0x100506d8
struct AmberWillow0x7c* FUN_100506d8(void)
{
	MechS32 id;
	struct AmberWillow0x7c* obj;

	if (g_unk0x100a8624 < g_unk0x100a8620) {
		id = g_unk0x100ea580[g_unk0x100a8624];
		obj = FUN_1001d980(id);
		g_unk0x100a8624++;
	}
	else {
		obj = NULL;
	}

	return obj;
}

// Returns the next free game thing index, or -1 when all 254 are taken.
// FUNCTION: MW2 0x1005072f
MechS32 FUN_1005072f(void)
{
	MechS32 index;

	index = -1;
	if (g_gameThingCount < 0xfe) {
		index = g_gameThingCount;
		g_gameThingCount++;
	}
	else {
		Error(0x2f, "Too many gamethings", 0);
	}

	return index;
}
