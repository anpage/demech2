#ifndef DEBRIS_H
#define DEBRIS_H

#include "object.h"
#include "types.h"

// The functions and globals of debris.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10004356(AmberWillow0x7c* p_obj, ShapeCallback p_callback, MechU32 p_unk0x16);
	void FUN_100044f3(AmberWillow0x7c* p_obj, ShapeCallback p_callback, MechU32 p_unk0x16);
	void UpdateDebris(void);
	void ZeroChunx(void);

#ifdef __cplusplus
}
#endif

#endif // DEBRIS_H
