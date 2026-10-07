#ifndef SETRES_H
#define SETRES_H

#include "fixedfloat.h"
#include "point.h"
#include "render.h"
#include "types.h"

// The functions and globals of setres.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_artResolution;
	extern MechChar g_artResolutionSuffixes[4][2];
	extern Point g_artResolutionSizes[3];
	extern MechScalar g_pixelAspect;
#ifdef MW2_MATROX
	extern MechS32 g_unk0x1012c1e4;
#endif

// The art resolution the HUD and the text boxes load their shapes and fonts at: the edition's
// own (ChooseArtResolution), g_artResolution in 1.1.
#ifdef MW2_MATROX
#define HUD_ART_RESOLUTION g_unk0x1012c1e4
#else
#define HUD_ART_RESOLUTION g_artResolution
#endif

	void SetPixelAspect(GameWindowGeometry* p_geometry);
	void ChooseArtResolution(GameWindowGeometry* p_geometry);
	void SetRes(void);

#ifdef __cplusplus
}
#endif

#endif // SETRES_H
