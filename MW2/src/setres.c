#include "setres.h"

#include "cockpit.h"
#include "config.h"
#include "decomp.h"
#include "eyepoint.h"
#include "fixedfloat.h"
#include "hud.h"
#include "muldiv.h"
#include "overlay.h"
#include "palette.h"
#include "point.h"
#include "polydraw.h"
#include "render.h"
#include "screenscale.h"
#include "targeting.h"
#include "timedoverlays.h"
#include "types.h"
#include "view.h"

#include <stdlib.h>

// The file name suffixes of the art resolutions (ShowBanner).
// GLOBAL: MW2 0x100aa710
// GLOBAL: MW2GBL 0x100b6998
// GLOBAL: MW2MATROX 0x100bca48
// GLOBAL: MW23DFX 0x10249298
MechChar g_artResolutionSuffixes[4][2] = {"", "6", "k", ""};

// The largest coordinates of the three resolutions the art comes in.
// GLOBAL: MW2 0x100aa718
// GLOBAL: MW2GBL 0x100b69a0
// GLOBAL: MW2MATROX 0x100bca50
// GLOBAL: MW2ATI 0x1017d2a0
// GLOBAL: MW2S3 0x10077060
// GLOBAL: MW2POWERVR 0x101a82f8
Point g_artResolutionSizes[3] = {{319, 199}, {639, 479}, {1023, 767}};

// GLOBAL: MW2 0x100e9610
// GLOBAL: MW2GBL 0x100bbb58
// GLOBAL: MW2MATROX 0x1012c1e0
// GLOBAL: MW23DFX 0x102453c0
// GLOBAL: MW2ATI 0x101791e0
// GLOBAL: MW2S3 0x1007f3e0
// GLOBAL: MW2POWERVR 0x101a3d60
MechScalar g_pixelAspect;

#ifdef MW2_HUD_ART
// The Matrox edition's: the art resolution ChooseArtResolution picked before its 512x384 override.
// GLOBAL: MW2MATROX 0x1012c1e4
// GLOBAL: MW23DFX 0x102453c4
// GLOBAL: MW2ATI 0x101791e4
// GLOBAL: MW2S3 0x1007f3e4
// GLOBAL: MW2POWERVR 0x101a3d64
MechS32 g_unk0x1012c1e4;
#endif

// GLOBAL: MW2 0x100e9614
// GLOBAL: MW2GBL 0x100bbb5c
// GLOBAL: MW2MATROX 0x1012c1e8
// GLOBAL: MW23DFX 0x102453c8
// GLOBAL: MW2ATI 0x101791e8
// GLOBAL: MW2S3 0x1007f3dc
// GLOBAL: MW2POWERVR 0x101a3d68
MechS32 g_artResolution;

// FUNCTION: MW2 0x1005d410
// FUNCTION: MW2GBL 0x1007fff0
// FUNCTION: MW2MATROX 0x1008c140
// FUNCTION: MW23DFX 0x10002470
// FUNCTION: MW2ATI 0x10002370
// FUNCTION: MW2S3 0x10044a20
// FUNCTION: MW2POWERVR 0x10002350
void SetPixelAspect(GameWindowGeometry* p_geometry)
{
#ifdef MW2_FLOAT_SCALARS
	g_eyepoint->m_pixelAspect = g_pixelAspect = p_geometry->m_height * 1.3333334f / p_geometry->m_width;
#else
	g_eyepoint->m_pixelAspect = g_pixelAspect =
		MulDiv64(p_geometry->m_height << 16, 0x15555, p_geometry->m_width << 16);
#endif
}

#ifdef MW2_ART_RESOLUTION_NEAREST
// Picks the art resolution (g_artResolution) closest to the window's.
// FUNCTION: MW23DFX 0x100024b0
void ChooseArtResolution(GameWindowGeometry* p_geometry)
{
	MechS32 best;
	MechS32 distance;
	MechS32 i;

	// The 3Dfx edition's: the first size sets the distance to beat, and no overrides follow.
	g_artResolution = g_unk0x1012c1e4 = 0;
	for (i = 0; i < 3; i++) {
		distance = abs(g_artResolutionSizes[i].m_y - (p_geometry->m_height - 1)) +
				   abs(g_artResolutionSizes[i].m_x - (p_geometry->m_width - 1));
		if (i == 0) {
			best = distance;
		}
		else if (distance < best) {
			best = distance;
			g_artResolution = g_unk0x1012c1e4 = i;
		}
	}
}
#elif defined(MW2_ART_RESOLUTION_FIT)
// Picks the art resolution (g_artResolution): the first whose width reaches the window's, and
// g_unk0x1012c1e4, the last before it that fits inside the window.
// FUNCTION: MW2S3 0x10044a50
void ChooseArtResolution(GameWindowGeometry* p_geometry)
{
	MechS32 i;

	for (i = 0; i < 3; i++) {
		if (g_artResolutionSizes[i].m_x <= p_geometry->m_width - 1 &&
			g_artResolutionSizes[i].m_y <= p_geometry->m_height - 1) {
			g_unk0x1012c1e4 = i;
		}
		if (g_artResolutionSizes[i].m_x >= p_geometry->m_width - 1) {
			g_artResolution = i;
			break;
		}
	}
}
#else
// Picks the art resolution (g_artResolution) closest to the window's.
// FUNCTION: MW2 0x1005d44e
// FUNCTION: MW2GBL 0x1008002e
// FUNCTION: MW2MATROX 0x1008c195
// FUNCTION: MW2ATI 0x100023b0
// FUNCTION: MW2POWERVR 0x10002390
void ChooseArtResolution(GameWindowGeometry* p_geometry)
{
	MechS32 best;
	MechS32 distance;
	MechS32 i;

	best = 7;
	for (i = 0; i < 3; i++) {
#ifdef MW2_ABS_CALLS
		distance = labs(g_artResolutionSizes[i].m_y - (p_geometry->m_height - 1)) +
				   labs(g_artResolutionSizes[i].m_x - (p_geometry->m_width - 1));
#else
		distance = abs(g_artResolutionSizes[i].m_y - (p_geometry->m_height - 1)) +
				   abs(g_artResolutionSizes[i].m_x - (p_geometry->m_width - 1));
#endif
		if (distance < best) {
			best = distance;
#ifdef MW2_HUD_ART
			g_artResolution = g_unk0x1012c1e4 = i;
#else
			g_artResolution = i;
#endif
		}
	}
#ifdef MW2_ART_RESOLUTION_512X384

	if (p_geometry->m_width == 512 && p_geometry->m_height == 384) {
		g_artResolution = 1;
	}
#endif
#ifdef MW2_RES_640X400
	if (p_geometry->m_width == 640 && p_geometry->m_height == 400) {
		g_artResolution = g_unk0x1012c1e4 = 1;
	}
#endif
}
#endif

// Rescales the tables authored in 320x200 coordinates to the screen, and sets up what depends on
// the resolution.
// FUNCTION: MW2 0x1005d4d3
// FUNCTION: MW2GBL 0x100800b9
// FUNCTION: MW2MATROX 0x1008c280
// FUNCTION: MW23DFX 0x10002520
// FUNCTION: MW2ATI 0x10002430
// FUNCTION: MW2S3 0x10044aa0
// FUNCTION: MW2POWERVR 0x10002410
void SetRes(void)
{
	MechS32 i;
	Point point;

	for (i = 0; i < 8; i++) {
		ScaleRectFromLowRes(&g_panes[i], &g_panes[i]);
		ScaleRectToScreen(&g_mainPixelBuffer, &g_panes[i], &g_panes[i]);
	}

#ifndef MW2_COCKPIT_POINT_ONLY
	// The Matrox edition has no gauge panes to scale.
	for (i = 0; i < 5; i++) {
		ScaleRectFromLowRes(&g_cockpitGaugePanes[i], &g_cockpitGaugePanes[i]);
		ScaleRectToScreen(&g_mainPixelBuffer, &g_cockpitGaugePanes[i], &g_cockpitGaugePanes[i]);
	}

	ScalePointFromLowRes(g_unk0x100a5bb8[3], g_unk0x100a5bb8[3]);
	ScalePointToScreen(&g_mainPixelBuffer, g_unk0x100a5bb8[3], g_unk0x100a5bb8[3]);
#endif
	for (i = 0; i < 6; i++) {
		ScalePointFromLowRes(&g_hudGaugePositions[i], &g_hudGaugePositions[i]);
		ScalePointToScreen(&g_mainPixelBuffer, &g_hudGaugePositions[i], &g_hudGaugePositions[i]);
	}

	SelectPane(0);
	UpdateProjection(g_eyepoint);
	UpdateViewMatrix(g_eyepoint);
	g_projectionDirty = 0;
	point.m_x = g_horizonBandHeight;
	point.m_y = 0;
	ScalePointFromLowRes(&point, &point);
	ScalePointToScreen(&g_mainPixelBuffer, &point, &point);
	g_horizonBandHeight = point.m_x;
	ScaleCockpitLayout();
	LayoutMessageBoxes();
	ScaleOverlayPositions();
}
