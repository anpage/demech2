#ifndef _3DFX_GLIDEDRAW_H
#define _3DFX_GLIDEDRAW_H

#include "displaybackend.h"
#include "refreshmode.h"
#include "types.h"

// The functions and globals of 3dfx/glidedraw.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern DisplayBackend g_glideBackend;
	extern RefreshMode g_glideFlipRefreshMode;
	extern RefreshMode g_glideBlitFlipRefreshMode;
	extern RefreshMode g_glideVideoMemoryRefreshMode;
	extern RefreshMode g_glideSystemMemoryRefreshMode;

	MechS32 GlideFill(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom, MechU32 p_color);

#ifdef __cplusplus
}
#endif

#endif // _3DFX_GLIDEDRAW_H
