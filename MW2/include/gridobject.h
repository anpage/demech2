#ifndef GRIDOBJECT_H
#define GRIDOBJECT_H

#include "object.h"
#include "types.h"

// The functions and globals of gridobject.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100a7120;
	extern AmberWillow0x7c* g_unk0x100a7128;

	void FUN_1004b130(AmberWillow0x7c* p_obj);
	void FUN_1004b344(void);
	void FUN_1004b539(MechS32 p_enable);

#ifdef __cplusplus
}
#endif

#endif // GRIDOBJECT_H
