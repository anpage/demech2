#ifndef UNK10040B30_H
#define UNK10040B30_H

#include "mech.h"
#include "point.h"
#include "rendertarget.h"
#include "types.h"

// The functions and globals of unk10040b30.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	Point* FUN_100412c8(void);
	void FUN_10041e98(MechS32 p_x, MechS32 p_y, MechS32 p_id);
	void FUN_10041f06(MechS32 p_x, MechS32 p_y, MechS32 p_id, RenderTarget* p_target);
	void FUN_10041f73(MechS32 p_x, MechS32 p_y, MechS32 p_id, RenderTarget* p_target);

#ifdef __cplusplus
}
#endif

#endif // UNK10040B30_H
