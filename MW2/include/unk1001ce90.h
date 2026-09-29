#ifndef UNK1001CE90_H
#define UNK1001CE90_H

#include "classentry.h"
#include "decomp.h"
#include "types.h"
#include "unk1003a530.h"

struct AmberWillow0x7c;
struct Player;

// The functions and globals of unk1001ce90.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_classEntryCount;
	extern MechS32 g_classTableReady;
	extern ClassEntry g_classTable[0x30c];

	MechS32 FUN_1001ce90(struct Player* p_player);
	MechS32 FUN_1001cf93(
		MechS32 p_unk0x00,
		undefined4 p_unk0x04,
		undefined4 p_unk0x08,
		undefined4 p_unk0x0c,
		MechS32 p_unk0x10,
		MechS32 p_level,
		MechS32 p_index,
		MechS16 p_unk0x1c
	);
	void ResetClassTable(void);
	void FUN_1001d220(struct Player* p_player);
	MechS32 FUN_1001d292(MechS32 p_owner, MechS32 p_level);
	void FUN_1001d3a4(MechS32 p_owner, MechS32 p_level);
	MechS32 FUN_1001d3ff(MechS32 p_index, MechS32 p_level, void* p_buffer);
	void FUN_1001d88b(void);
	void FUN_1001d912(MechS32 p_index, MechS32 p_level);
	struct AmberWillow0x7c* FUN_1001d980(MechS32 p_index);
	ScarletOrchid0x4c* FUN_1001d9ca(MechS32 p_index);
	void FUN_1001da14(MechS32 p_index, MechU16 p_value);
	void FUN_1001da44(void);
	void FUN_1001ddf2(struct AmberWillow0x7c* p_obj);
	void FUN_1001de84(struct AmberWillow0x7c* p_obj);

#ifdef __cplusplus
}
#endif

#endif // UNK1001CE90_H
