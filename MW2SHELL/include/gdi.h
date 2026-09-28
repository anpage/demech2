#ifndef GDI_H
#define GDI_H

#include "decomp.h"
#include "drawmode.h"
#include "drawmodeextension.h"
#include "types.h"

// The functions and globals of gdi.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern DrawModeExtension g_gdiDrawModeExtension;
	extern DrawMode g_gdiDrawMode;
	MechS32 FUN_10031001(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
	MechS32 FUN_10031106(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);

#ifdef __cplusplus
}
#endif

#endif // GDI_H
