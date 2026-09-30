#include "unk10041fa0.h"

#include "decomp.h"
#include "eyepoint.h"
#include "palette.h"
#include "render.h"
#include "rendertarget.h"
#include "simmain.h"
#include "slateheron.h"
#include "types.h"
#include "unk1004b980.h"

// The map (satellite) view's projection: a top-down view of p_worldSpan units across.

// GLOBAL: MW2 0x10109ab0
MechS32 g_unk0x10109ab0;

// GLOBAL: MW2 0x10109ac0
Eyepoint g_unk0x10109ac0;

// GLOBAL: MW2 0x10109ba0
MechS32 g_unk0x10109ba0;

// GLOBAL: MW2 0x10109ba4
MechS32 g_unk0x10109ba4;

// GLOBAL: MW2 0x10109ba8
MechS32 g_unk0x10109ba8;

// GLOBAL: MW2 0x10109bac
MechS32 g_unk0x10109bac;

// GLOBAL: MW2 0x10109bb0
MechS32 g_unk0x10109bb0;

// GLOBAL: MW2 0x10109bb4
MechS32 g_unk0x10109bb4;

// GLOBAL: MW2 0x10109bb8
MechS32 g_unk0x10109bb8;

// GLOBAL: MW2 0x10109bc0
SlateHeron0x68 g_unk0x10109bc0;

// Saves the eyepoint and the rendering settings, and sets up a view from p_pose (position, then
// rotation) of p_worldSpan units across render target p_slot, as far as p_far.
// FUNCTION: MW2 0x10041fa0
void FUN_10041fa0(MechS32* p_pose, MechS32 p_slot, MechS32 p_worldSpan, MechS32 p_far)
{
	g_unk0x10109bb8 = p_worldSpan / (g_renderTargets[p_slot].m_right - g_renderTargets[p_slot].m_left + 1);
	g_unk0x10109ba4 = -(g_unk0x10109ba0 = -(p_worldSpan / 2));
	g_unk0x10109bac =
		-(g_unk0x10109ba8 =
			  (g_renderTargets[p_slot].m_bottom - g_renderTargets[p_slot].m_top + 1) * g_unk0x10109bb8 / 2);
	g_unk0x10109bb0 = 0;
	g_unk0x10109bb4 = p_far;
	g_unk0x10109ab0 = g_palettePending;
	g_unk0x10109ac0 = *g_eyepoint;
	g_eyepoint->m_unk0x0c = p_pose[3];
	g_eyepoint->m_unk0x10 = p_pose[4];
	g_eyepoint->m_unk0x14 = p_pose[5];
	g_eyepoint->m_unk0x00 = p_pose[0];
	g_eyepoint->m_unk0x04 = p_pose[1];
	g_eyepoint->m_unk0x08 = p_pose[2];
	SelectRenderTarget(p_slot);
	g_eyepoint->m_unk0x40 = g_unk0x10109bb4;
	g_unk0x10109bc0 = g_unk0x100a6cc8;
	g_unk0x100a6cc8.m_unk0x58 = FUN_10042206;
	g_unk0x100a6cc8.m_unk0x5c = FUN_100423b3;
	FUN_1004bc2e(g_eyepoint);
	g_eyepoint->m_unk0x9c = 0x2000;
	g_eyepoint->m_unk0xa4 = 3;
	g_eyepoint->m_unk0xa0 = g_eyepoint->m_pixelAspect >> 3;
	g_eyepoint->m_unk0xa6 = 3;
	FUN_1004bfe8(g_eyepoint);
	FUN_1004b980(g_eyepoint);
	g_unk0x100a2460 = 0;
}

// Restores the eyepoint and the rendering settings FUN_10041fa0 saved.
// FUNCTION: MW2 0x10042195
void FUN_10042195(void)
{
	g_unk0x100a6cc8 = g_unk0x10109bc0;
	*g_eyepoint = g_unk0x10109ac0;
	g_palettePending = g_unk0x10109ab0;
	FUN_10012e00();
	FUN_1004bc2e(g_eyepoint);
	FUN_1004bfe8(g_eyepoint);
	FUN_1004b980(g_eyepoint);
	g_unk0x100a2460 = 0;
}

// STUB: MW2 0x10042206
MechS32 FUN_10042206(ScarletOrchid0x4c* p_shape)
{
	STUB(0x10042206);
	return 0;
}

// STUB: MW2 0x100423b3
void FUN_100423b3(void)
{
	STUB(0x100423b3);
}

// Projects the world point p_point to the map view in place. Returns whether it lies in the view.
// STUB: MW2 0x1004251e
MechS32 FUN_1004251e(MapPoint* p_point)
{
	STUB(0x1004251e);
	return 0;
}
