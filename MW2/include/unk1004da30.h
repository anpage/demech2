#ifndef UNK1004DA30_H
#define UNK1004DA30_H

#include "types.h"

struct MissionTable;

// The functions and globals of unk1004da30.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_1004da30(struct MissionTable* p_table);
	MechU32 FindEventList(MechChar* p_name);
	void PostEventToList(MechChar* p_name, MechS32 p_types, MechU16 p_target);
	void FlushEventLists(MechU32 p_lists);

#ifdef __cplusplus
}
#endif

#endif // UNK1004DA30_H
