#ifndef UNK10042E00_H
#define UNK10042E00_H

#include "types.h"

// The functions and globals of unk10042e00.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10042e00(MechS32 p_count, MechU32* p_points, MechU32 p_flags);
	void FUN_1004440d(
		MechS32 p_x0,
		MechS32 p_y0,
		MechS32 p_x1,
		MechS32 p_y1,
		MechS32 p_x2,
		MechS32 p_y2,
		MechS32 p_x3,
		MechS32 p_y3,
		MechS32 p_x4,
		MechS32 p_y4,
		MechU32 p_flags
	);
	void FUN_100444a6(
		MechS32 p_x0,
		MechS32 p_y0,
		MechS32 p_x1,
		MechS32 p_y1,
		MechS32 p_x2,
		MechS32 p_y2,
		MechS32 p_x3,
		MechS32 p_y3,
		MechU32 p_flags
	);
	void FUN_10044527(
		MechS32 p_x0,
		MechS32 p_y0,
		MechS32 p_x1,
		MechS32 p_y1,
		MechS32 p_x2,
		MechS32 p_y2,
		MechU32 p_flags
	);
	void FUN_10044590(MechS32 p_x, MechS32 p_y, MechU32 p_color);
	void FUN_100445d2(MechS32 p_count, MechU32* p_points, MechU32 p_flags);

#ifdef __cplusplus
}
#endif

#endif // UNK10042E00_H
