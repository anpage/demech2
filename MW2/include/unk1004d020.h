#ifndef UNK1004D020_H
#define UNK1004D020_H

#include "rendertarget.h"
#include "types.h"

// The functions and globals of unk1004d020.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_1004d020(void);
	void FUN_1004d175(RenderTarget* p_target);
	void FUN_1004d310(RenderTarget* p_target);
	void FUN_1004d48a(RenderTarget* p_target);
	void FUN_1004d660(RenderTarget* p_target);
	void FUN_1004d732(
		RenderTarget* p_target,
		MechS32 p_x,
		MechS32 p_y,
		MechS32 p_width,
		MechS32 p_height,
		MechS32 p_color
	);
	void FUN_1004d8ae(
		RenderTarget* p_target,
		MechS32 p_x,
		MechS32 p_y,
		MechS32 p_width,
		MechS32 p_height,
		MechS32 p_color
	);

#ifdef __cplusplus
}
#endif

#endif // UNK1004D020_H
