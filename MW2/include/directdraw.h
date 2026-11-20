#ifndef DIRECTDRAW_H
#define DIRECTDRAW_H

#include "displaybackend.h"
#include "palettecolor.h"
#include "refreshmode.h"
#include "types.h"
#include "window.h"

#if defined(MW2_ATI) || defined(MW2_S3)
#include <ddraw.h>
#endif

// The functions and globals of directdraw.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern DisplayBackend g_directDrawBackend;
	extern RefreshMode g_ddrawFlipRefreshMode;
	extern RefreshMode g_ddrawBlitFlipRefreshMode;
	extern RefreshMode g_ddrawVideoMemoryRefreshMode;
	extern RefreshMode g_ddrawSystemMemoryRefreshMode;

	MechS32 DdrawFill(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom, MechU32 p_color);

#if defined(MW2_RES_512X384) || defined(MW2_S3)
	MechS32 FUN_100059b0(MechS32 p_width, MechS32 p_height, MechS32 p_bitCount);
#endif
#if defined(MW2_ATI) || defined(MW2_S3)
	// The ATI and S3 editions' renderers create their textures' surfaces with it. Declared for those
	// editions only: a declaration more moves the code of the other editions' units that include this
	// header.
	extern LPDIRECTDRAW g_ddraw;
#endif

#ifdef __cplusplus
}
#endif

#endif // DIRECTDRAW_H
