#ifndef RESOURCE_H
#define RESOURCE_H

#include "missiontable.h"
#include "types.h"

struct AmberWillow0x7c;

// The functions and globals of resource.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MissionTable* g_missionTables[16];
	extern MechS32 g_missionTableCounts[16];
	extern MechS32 g_unk0x100a8620;
	extern MechS32 g_unk0x100a8624;
	extern MechS32 g_unk0x100ea580[0x96];

	void FUN_1004fd55(void);
	MechS32 MapResourceId(MechS32 p_id);
	void SetMangleBase(MechS32 p_base);
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
