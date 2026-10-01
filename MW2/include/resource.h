#ifndef RESOURCE_H
#define RESOURCE_H

#include "bwd.h"
#include "missiontable.h"
#include "types.h"

struct AmberWillow0x7c;
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
	extern struct Player* g_unk0x100a8638;
	extern MechS32 g_unk0x100a8624;
	extern MechS32 g_unk0x100ea580[0x96];

	MechS32 LoadScenarioTable(struct ScenarioTable* p_table);
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
	struct AmberWillow0x7c* FUN_100506d8(void);
	MechS32 FUN_1005072f(void);
	MechS32 FirstResource(void);
	void CloseResourceFile(void);
	void CachePreloads(void);
	MechS32 FUN_10050862(MechS32 p_id, const char* p_type);
	void* FUN_100508c0(MechU32 p_size);
	void FUN_100508dc(void* p_block);

#ifdef __cplusplus
}
#endif

#endif // RESOURCE_H
