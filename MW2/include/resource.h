#ifndef RESOURCE_H
#define RESOURCE_H

#include "bwd.h"
#include "callbacks.h"
#include "decomp.h"
#include "missiontable.h"
#include "types.h"

struct SceneObject;
struct BwdRecord;
struct FormationNames;
struct FormationRecord;
struct PathRecord;
struct StarTable;
struct BwdObjectRecord;
struct IncludeRecord;
struct IncludeRecord2;
struct ScenarioTable;

// The functions and globals of resource.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MissionTable* g_missionTables[16];
	extern MechS32 g_missionTableCounts[16];
	extern MechS32 g_unk0x100a8620;
	extern MechChar* g_unk0x100a8630;
	extern MechChar* g_unk0x100a8634;
	extern struct Player* g_lastPlayer;
	extern MechS32 g_unk0x100a862c;
	extern TimedCallbackFn g_unk0x100a8640[6];
	extern MechS32 g_unk0x100a8624;
	extern MechS32 g_unk0x100ea580[0x96];

	void LoadMapBitmap(struct BwdRecord* p_record, MechS32* p_width, MechS32* p_height, MechS32* p_data);
	MechS32 LoadScenarioTable(struct ScenarioTable* p_table);
	MechS32 LoadMissionTable(MissionTable* p_table);
	MechS32 LoadPathTable(struct PathRecord* p_record);
	MechS32 LoadFormationTable(struct FormationRecord* p_record);
	void LoadStarTable(struct StarTable* p_table);
	void FUN_1004fc48(struct FormationNames* p_record);
	MechS32 ExecuteInclude(struct IncludeRecord* p_record, BwdStreamFn p_fn);
	MechS32 FUN_1004fcac(struct IncludeRecord2* p_record, BwdStreamFn p_fn);
	void FUN_1004fd55(void);
	MechS32 MapResourceId(MechS32 p_id);
	void SetMangleBase(MechS32 p_base);
	void CreateObjectNode(
		struct BwdObjectRecord* p_record,
		undefined4 p_unk0x04,
		MechS32 p_unk0x08,
		MechS32 p_static,
		MechS32 p_class,
		MechS32 p_level
	);
	struct SceneObject* FUN_100506d8(void);
	MechS32 FUN_1005072f(void);

#ifdef __cplusplus
}
#endif

#endif // RESOURCE_H
