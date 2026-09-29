#include "eyepoint.h"

#include "decomp.h"
#include "simmain.h"
#include "soundfx.h"
#include "types.h"
#include "unk10034a40.h"

// STUB: MW2 0x10010ee0
void FirstEyepoint(void)
{
	STUB(0x10010ee0);
}

// STUB: MW2 0x100110f7
void UpdateEyepoint(void)
{
	STUB(0x100110f7);
}

// FUNCTION: MW2 0x100113af
MechS32 FUN_100113af(Eyepoint* p_eyepoint)
{
	MechS32 height;

	height = FUN_10034cbc(p_eyepoint->m_unk0x00, p_eyepoint->m_unk0x04, p_eyepoint->m_unk0x08);
	if (height > 0) {
		height += 500;
	}
	else {
		height += 200;
	}

	return height;
}

// FUNCTION: MW2 0x10011401
void FUN_10011401(MechS32 p_zoom)
{
	if (g_unk0x100a2c04) {
		if (g_unk0x100a2424 == -1) {
			g_unk0x100a2424 = 1;
		}

		p_zoom = g_unk0x100a2424;
	}

	g_unk0x100a2414 = p_zoom;
}

// FUNCTION: MW2 0x10011440
MechS32 FUN_10011440(void)
{
	return g_unk0x100a2414;
}

// Sets the eyepoint's field of view to the normal or the zoomed one (both reset to 1.0 if
// p_reset), and plays a sound when it changes.
// FUNCTION: MW2 0x10011455
void ApplyCameraFov(MechS32 p_reset)
{
	MechS32 fov;

	fov = g_eyepoint->m_fovX;
	if (p_reset) {
		g_normalFov = 0x10000;
		g_zoomFov = 0x10000;
	}

	if (!FUN_10011440()) {
		g_eyepoint->m_fovX = g_normalFov;
	}
	else {
		g_eyepoint->m_fovX = g_zoomFov;
	}

	if (g_eyepoint->m_fovX != fov) {
		g_unk0x100a2460 = 1;
		FUN_1007eb23(0x147, 100, 0x40, 5, 0x50);
	}
}

// Returns the eyepoint's base position (0x00-0x08) and orientation (0x0c-0x14), without the
// camera shake.
// STUB: MW2 0x10011e45
void FUN_10011e45(MechS32* p_unk0x10, MechS32* p_unk0x0c, MechS32* p_unk0x14, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	STUB(0x10011e45);
}
