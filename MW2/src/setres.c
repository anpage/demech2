#include "setres.h"

#include "decomp.h"
#include "eyepoint.h"
#include "overlay.h"
#include "palette.h"
#include "point.h"
#include "rendertarget.h"
#include "screenscale.h"
#include "simmain.h"
#include "timedoverlays.h"
#include "types.h"
#include "unk1004b980.h"
#include "unk1006f480.h"

// Rescales the tables authored in 320x200 coordinates to the screen, and sets up what depends on
// the resolution.
// FUNCTION: MW2 0x1005d4d3
void SetRes(void)
{
	MechS32 i;
	Point point;

	for (i = 0; i < 8; i++) {
		FUN_10056ce9(&g_renderTargets[i], &g_renderTargets[i]);
		ScaleRectToScreen(&g_mainPixelBuffer, &g_renderTargets[i], &g_renderTargets[i]);
	}

	for (i = 0; i < 5; i++) {
		FUN_10056ce9(&g_unk0x100a5a68[i], &g_unk0x100a5a68[i]);
		ScaleRectToScreen(&g_mainPixelBuffer, &g_unk0x100a5a68[i], &g_unk0x100a5a68[i]);
	}

	FUN_10056ddd(g_unk0x100a5bb8[3], g_unk0x100a5bb8[3]);
	ScalePointToScreen(&g_mainPixelBuffer, g_unk0x100a5bb8[3], g_unk0x100a5bb8[3]);
	for (i = 0; i < 6; i++) {
		FUN_10056ddd(&g_unk0x100a5ee8[i], &g_unk0x100a5ee8[i]);
		ScalePointToScreen(&g_mainPixelBuffer, &g_unk0x100a5ee8[i], &g_unk0x100a5ee8[i]);
	}

	SelectRenderTarget(0);
	FUN_1004bc2e(g_eyepoint);
	FUN_1004bfe8(g_eyepoint);
	g_unk0x100a2460 = 0;
	point.m_x = g_unk0x100a6d30;
	point.m_y = 0;
	FUN_10056ddd(&point, &point);
	ScalePointToScreen(&g_mainPixelBuffer, &point, &point);
	g_unk0x100a6d30 = point.m_x;
	FUN_1006fba3();
	FUN_1006ee60();
	FUN_100592b0();
}
