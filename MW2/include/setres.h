#ifndef SETRES_H
#define SETRES_H

#include "point.h"
#include "render.h"
#include "types.h"

// The functions and globals of setres.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_artResolution;
	extern MechChar g_unk0x100aa710[4][2];
	extern Point g_unk0x100aa718[3];
	extern MechS32 g_pixelAspect;

	void FUN_1005d410(GameWindowGeometry* p_geometry);
	void FUN_1005d44e(GameWindowGeometry* p_geometry);
	void SetRes(void);

#ifdef __cplusplus
}
#endif

#endif // SETRES_H
