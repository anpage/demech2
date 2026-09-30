#ifndef UNK1006DD50_H
#define UNK1006DD50_H

#include "rendertarget.h"
#include "types.h"

// The functions and globals of unk1006dd50.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_1006dd50(
		RenderTarget* p_target,
		MechU8* p_pixels,
		MechS16 p_width,
		MechS16 p_height,
		MechS32 p_count,
		MechU32* p_points,
		MechS32 p_useLuma,
		MechU16* p_luma
	);

#ifdef __cplusplus
}
#endif

#endif // UNK1006DD50_H
