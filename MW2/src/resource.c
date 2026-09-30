// resource.h stays out of this unit: its declarations ahead of FUN_1005005e change that
// function's operand order (see there).
#include "ai.h"
#include "bwd.h"
#include "bwdstreamkey.h"
#include "config.h"
#include "decomp.h"
#include "error.h"
#include "loadres.h"
#include "players.h"
#include "prjfile.h"
#include "simmain.h"
#include "team.h"
#include "types.h"
#include "unk1001ce90.h"
#include "unk10046750.h"
#include "unk1004da30.h"
#include "unk1006f480.h"
#include "weapons.h"

#include <math.h>
#include <mbstring.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

// A BWD record's header. The size counts the whole record, header included.
typedef struct BwdRecord {
	MechU32 m_tag;  // 0x00
	MechU32 m_size; // 0x04
} BwdRecord;

// A bitmap record: its size and palette-sized data.
typedef struct BitmapRecord {
	BwdRecord m_header; // 0x00
	MechS32 m_width;    // 0x08
	MechS32 m_height;   // 0x0c
	MechS32 m_data[1];  // 0x10 — up to the record's end
} BitmapRecord;

// The scenario table: the names ExecuteInclude substitutes for "^", in turn.
typedef struct ScenarioTable {
	BwdRecord m_header;       // 0x00
	MechChar m_names[1][0xc]; // 0x08 — up to the record's end
} ScenarioTable;

// A mission table: 0x97-byte entries, the first starting with the table's slot.
typedef struct MissionTable {
	BwdRecord m_header; // 0x00
	MechS32 m_slot;     // 0x08
} MissionTable;

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

// An include record: the stream to run, by id or name.
typedef struct IncludeRecord {
	BwdRecord m_header;    // 0x00
	MechS16 m_id;          // 0x08
	MechChar m_name[0x0c]; // 0x0a
} IncludeRecord;

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

// A second include record, by id and name at other offsets.
typedef struct IncludeRecord2 {
	BwdRecord m_header;               // 0x00
	undefined m_unk0x08[0x0a - 0x08]; // 0x08
	MechS16 m_id;                     // 0x0a
	undefined m_unk0x0c[0x24 - 0x0c]; // 0x0c
	MechChar m_name[0x0c];            // 0x24
} IncludeRecord2;

typedef MechS32 (*BwdStreamFn)(BwdStream* p_stream);

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

// The formation LoadStarTable gives the player's team, or NULL to use the record's.
// GLOBAL: MW2 0x100a8630
MechChar* g_unk0x100a8630 = NULL;

// The formation LoadStarTable gives the other teams, or NULL to use the record's.
// GLOBAL: MW2 0x100a8634
MechChar* g_unk0x100a8634 = NULL;

// GLOBAL: MW2 0x100ea500
MissionTable* g_missionTables[16];

// The number of entries of each of g_missionTables.
// GLOBAL: MW2 0x100ea540
MechS32 g_missionTableCounts[16];

// GLOBAL: MW2 0x100ea580
MechS32 g_unk0x100ea580[1]; // length unknown

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
	slot = p_table->m_slot;
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
		g_missionTableCounts[slot] = (g_missionTables[slot]->m_header.m_size - 8) / 0x97;
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
		stream = OpenBwdStream(&key->m_id, buffer);
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
	stream = OpenBwdStream(&key->m_id, buffer);
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

// GLOBAL: MW2 0x100a8628
MechS32 g_unk0x100a8628 = 0;

// CreateObjectNode's ".wtb", defined here until that function is decompiled: reccmp pairs identical
// strings in address order, and this one comes before the resource type table's.
// GLOBAL: MW2 0x100a8658
MechChar g_unk0x100a8658[] = ".wtb";

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

void* FUN_100508c0(MechU32 p_size);
void FUN_100508dc(void* p_block);

// FUNCTION: MW2 0x10050780
MechS32 FirstResource(void)
{
	MechS32 result;

	result = TRUE;
	SetPrjAllocator(FUN_100508c0, FUN_100508dc);
	FUN_10019d73();
	if (!g_unk0x100a8744) {
		g_unk0x100a8744 = (MechChar*) _mbsdup((unsigned char*) BuildGamePath("mw2.prj"));
	}

	g_unk0x100a8740 = OpenPrjFile(g_unk0x100a8744, 0);
	if (g_unk0x100a8740 != -1) {
		LoadPrjIndexes(g_unk0x100a8740);
	}
	else {
		Error(3, "\nCan't find file \"%s\"", g_unk0x100a8744, 0);
		result = FALSE;
	}

	return result;
}

// FUNCTION: MW2 0x1005082f
void CloseResourceFile(void)
{
	ClosePrjFile(g_unk0x100a8740);
}

// FUNCTION: MW2 0x10050848
void CachePreloads(void)
{
	FUN_10045a5b();
	FUN_1006f480();
	LoadAIScripts();
}

// FUNCTION: MW2 0x10050862
MechS32 FUN_10050862(MechS32 p_id, const char* p_type)
{
	MechS32 result;
	void* data;

	data = FUN_1001a19f(g_unk0x100a8740, p_id, p_type, 0);
	if (data) {
		result = TRUE;
		FUN_1001a163(p_id, p_type);
	}
	else {
		result = FALSE;
	}

	return result;
}

// FUNCTION: MW2 0x100508c0
void* FUN_100508c0(MechU32 p_size)
{
	return MemAlloc(p_size);
}

// FUNCTION: MW2 0x100508dc
void FUN_100508dc(void* p_block)
{
	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_block);
}
