#include "eyepoint.h"

#include "decomp.h"
#include "object.h"
#include "players.h"
#include "shots.h"
#include "simmain.h"
#include "soundfx.h"
#include "transform.h"
#include "types.h"
#include "unk10034a40.h"

// The view FUN_10011819 saves when it leaves the cockpit view.
// GLOBAL: MW2 0x10176f10
MechS32 g_unk0x10176f10[7];

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

// Saves the eyepoint's position and orientation (0x00-0x14) to p_view, marking it (p_view[6])
// as set. Returns 0 without both.
// FUNCTION: MW2 0x100114ea
MechS32 FUN_100114ea(Eyepoint* p_eyepoint, MechS32* p_view)
{
	if (p_view == NULL || p_eyepoint == NULL) {
		return 0;
	}

	p_view[0] = p_eyepoint->m_unk0x00;
	p_view[1] = p_eyepoint->m_unk0x04;
	p_view[2] = p_eyepoint->m_unk0x08;
	p_view[3] = p_eyepoint->m_unk0x0c;
	p_view[4] = p_eyepoint->m_unk0x10;
	p_view[5] = p_eyepoint->m_unk0x14;
	p_view[6] = 1;
	return 1;
}

// Restores the eyepoint's position and orientation from p_view, if FUN_100114ea set it. Returns
// 0 without both, or when the view is not set.
// FUNCTION: MW2 0x1001156a
MechS32 FUN_1001156a(Eyepoint* p_eyepoint, MechS32* p_view)
{
	if (p_view == NULL || p_eyepoint == NULL) {
		return 0;
	}

	if (!p_view[6]) {
		return 0;
	}

	p_eyepoint->m_unk0x00 = p_view[0];
	p_eyepoint->m_unk0x04 = p_view[1];
	p_eyepoint->m_unk0x08 = p_view[2];
	p_eyepoint->m_unk0x0c = p_view[3];
	p_eyepoint->m_unk0x10 = p_view[4];
	p_eyepoint->m_unk0x14 = p_view[5];
	return 1;
}

// Moves the camera to the next (p_next) or previous player, or back to the local player
// (p_home), skipping players who left or whose mechs are gone.
// FUNCTION: MW2 0x100115f4
void FUN_100115f4(MechS32 p_next, MechS32 p_home)
{
	MechS32 step;

	step = -1;
	if (!g_playerCount) {
		return;
	}

	if (p_home) {
		g_unk0x100a2430 = g_localPlayerId;
		step = 0;
	}
	else if (p_next) {
		step = 1;
	}

	g_unk0x100a2430 += step;
	if (g_unk0x100a2430 < 0) {
		g_unk0x100a2430 = g_playerCount - 1;
	}
	else if (g_unk0x100a2430 >= g_playerCount) {
		g_unk0x100a2430 = 0;
	}

	g_localPlayer = g_players[g_unk0x100a2430];
	if (g_localPlayer->m_flags & 0x4800) {
		FUN_100115f4(p_next, p_home);
	}

	g_unk0x100a2408 = -1;
}

// Returns the camera player's view: its orientation and, from the cockpit, the position of its
// eye object (at half the object's height outside the external views).
// Stack-slot permutation of player, x, y and z.
// FUNCTION: MW2 0x100116c3
void FUN_100116c3(MechS32* p_unk0x10, MechS32* p_unk0x0c, MechS32* p_unk0x14, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	Player* player;
	MechS32 x;
	MechS32 y;
	MechS32 z;

	*p_unk0x10 = *p_unk0x0c = *p_unk0x14 = *p_x = *p_y = *p_z = 0;
	player = g_localPlayer;
	if (!player) {
		return;
	}

	*p_unk0x10 = player->m_unk0x5c;
	*p_unk0x0c = player->m_heading;
	*p_unk0x14 = player->m_unk0x64;
	if (player->m_unk0x44) {
		if (g_unk0x100a2434) {
			*p_y += *g_unk0x100a2434;
		}

		FUN_1000d650(FUN_10001e01(player->m_unk0x44), p_x, p_y, p_z);
		GetObjWorldPos(player->m_unk0x44, &x, &y, &z);
		if (g_unk0x100a2c04) {
			*p_unk0x10 = x;
			*p_unk0x0c = y;
			*p_unk0x14 = z;
		}
		else {
			*p_unk0x10 += player->m_unk0x68;
			*p_unk0x0c += player->m_unk0x6c;
			*p_unk0x14 = z >> 1;
		}
	}
	else {
		*p_x = player->m_position.m_x;
		*p_y = player->m_position.m_y;
		*p_z = player->m_position.m_z;
	}
}

// Switches to the external view (mode 3): saves the view and zooms out the first time, then
// follows the view camera, or else restores the saved view.
// FUNCTION: MW2 0x10011819
void FUN_10011819(void)
{
	MechS32* camera;

	if (g_unk0x100a2408 != 3) {
		g_unk0x100a240c = g_unk0x100a2408;
		FUN_100114ea(g_eyepoint, g_unk0x10176f10);
		g_zoomFov = 0x20000;
		ApplyCameraFov(0);
	}

	camera = FUN_1006beb5();
	if (camera) {
		FUN_1001156a(g_eyepoint, camera);
	}
	else {
		FUN_10011401(g_unk0x100a240c);
		if (g_unk0x100a240c == 2) {
			FUN_1001156a(g_eyepoint, g_unk0x10176f10);
		}
	}
}

// Returns the eyepoint's base position (0x00-0x08) and orientation (0x0c-0x14), without the
// camera shake.
// STUB: MW2 0x10011e45
void FUN_10011e45(MechS32* p_unk0x10, MechS32* p_unk0x0c, MechS32* p_unk0x14, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	STUB(0x10011e45);
}
