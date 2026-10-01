#include "eyepoint.h"

#include "camerashake.h"
#include "clock.h"
#include "cockpit.h"
#include "decomp.h"
#include "fixedmul.h"
#include "inputmap.h"
#include "integrate.h"
#include "linengull.h"
#include "muldiv.h"
#include "object.h"
#include "players.h"
#include "ramp.h"
#include "rendertarget.h"
#include "shots.h"
#include "simmain.h"
#include "soundfx.h"
#include "transform.h"
#include "types.h"
#include "unk10034a40.h"
#include "unk1003a530.h"
#include "unk1004b130.h"
#include "unk100696c0.h"
#include "unk1006d680.h"

#include <stdlib.h>

// The view FUN_10011819 saves when it leaves the cockpit view.
// GLOBAL: MW2 0x10176f10
MechS32 g_unk0x10176f10[7];

// The external view's height limits (FUN_100118bc).
// GLOBAL: MW2 0x10176f2c
MechS32 g_unk0x10176f2c;

// GLOBAL: MW2 0x10176f30
Ramp g_unk0x10176f30;

// GLOBAL: MW2 0x10176f40
WrappedRamp g_unk0x10176f40;

// GLOBAL: MW2 0x10176f60
WrappedRamp g_unk0x10176f60;

// GLOBAL: MW2 0x10176f74
MechS32 g_unk0x10176f74;

// The cockpit view's tilt, eased toward g_sinkPilotTilt.
// GLOBAL: MW2 0x10176f80
Ramp g_unk0x10176f80;

// GLOBAL: MW2 0x10176f90
Ramp g_unk0x10176f90;

// GLOBAL: MW2 0x10176fa0
LinenGull0x1c g_unk0x10176fa0[5];

// The cockpit view's pan, eased toward g_sinkPilotPan.
// GLOBAL: MW2 0x10177030
Ramp g_unk0x10177030;

// The free camera's forward speed.
// GLOBAL: MW2 0x10177040
Ramp g_unk0x10177040;

// GLOBAL: MW2 0x10177050
Ramp g_unk0x10177050;

// Resets the camera: its ramps, the five camera slots and the view scale, follows the local player
// and resets the camera shake and the zoom.
// FUNCTION: MW2 0x10010ee0
void FirstEyepoint(void)
{
	MechS32 i;

	StartRamp(&g_unk0x10177050, 0, 0, 0.5);
	StartRamp(&g_unk0x10176f30, 0, 0, 0.5);
	StartRamp(&g_unk0x10176f90, 0, 0, 0.7);
	StartRamp(&g_unk0x10177040, 0, 0, 1.0);
	StartRamp(&g_unk0x10177030, 0, 0, 0.2);
	StartRamp(&g_unk0x10176f80, 0, 0, 0.2);
	StartWrappedRamp(&g_unk0x10176f60, 0, 0, 0.2, 0x1680000);
	StartWrappedRamp(&g_unk0x10176f40, 0, 0, 0.2, 0x1680000);
	g_sinkZoomFactor = 0x10000;
	for (i = 0; i < 5; i++) {
		g_unk0x10176fa0[i].m_unk0x00 = g_unk0x10176fa0[i].m_unk0x04 = g_unk0x10176fa0[i].m_unk0x08 = 0;
		g_unk0x10176fa0[i].m_unk0x0c = g_unk0x10176fa0[i].m_unk0x10 = g_unk0x10176fa0[i].m_unk0x14 = 0;
		g_unk0x10176fa0[i].m_unk0x18 = 0;
	}

	if (g_players[g_localPlayerId]) {
		g_localPlayer = g_players[g_localPlayerId];
		g_unk0x100a2430 = g_localPlayerId;
	}

	ResetCameraShake();
	FUN_10011401(g_unk0x100a2410);
}

// Updates the camera for the frame in the view mode FUN_10011440 picks (the free camera without a
// local player): the cockpit, tracking, external, drop or free camera, driven by INPUT.MAP's
// eyepoint and track sinks.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x100110f7
void UpdateEyepoint(void)
{
	MechS32 mode;
	MechS32 climb;
	MechS32 strafe;
	MechS32 turn;
	MechS32 speed;
	MechS32 pitch;
	MechS32 pan;

	g_normalFov = g_sinkZoomFactor;
	ApplyCameraFov(0);
	g_unk0x100a2420 = g_unk0x100a241c = 0;
	if (!g_playerCount) {
		mode = 2;
		FUN_10011401(2);
		g_localPlayer = NULL;
	}
	else {
		mode = FUN_10011440();
		if (!mode) {
			g_localPlayer = g_players[g_localPlayerId];
		}

		if (!g_localPlayer) {
			mode = 2;
		}
	}

	g_unk0x100a6cc8.m_unk0x00 = 0;
	switch (mode) {
	case 3:
		FUN_10011819();
		break;
	case 1:
		speed = MulDiv64(g_sinkTrackDistanceDelta, g_deltaTime, 0xb5) >> 16;
		strafe = MulDiv64(g_sinkTrackHeightDelta, g_deltaTime, 0xb5) >> 16;
		turn = g_sinkEyepointTilt;
		pan = MulDiv64(g_sinkEyepointPanDelta, g_deltaTime, 0xb5);
		if (g_unk0x100a2c04 && !g_unk0x100a2428) {
			pan = g_deltaTime << 14;
		}

		FUN_100118bc(speed, strafe, turn, pan);
		break;
	case 0:
	case 6:
		FUN_10011cb0();
		break;
	case 4:
		FUN_10011edc();
		break;
	default:
		climb = MulDiv64(g_sinkTrackHeightDelta, g_deltaTime, 0xb5) >> 15;
		speed = MulDiv64(g_sinkTrackDistanceDelta, g_deltaTime, 0xb5) >> 15;
		strafe = MulDiv64(g_sinkEyepointSlideDelta, g_deltaTime, 0xb5) >> 16;
		turn = MulDiv64(g_sinkEyepointPanDelta, g_deltaTime, 0xb5);
		if (g_sinkEyepointTilt > 0) {
			pitch = g_deltaTime * 0x2d0000 / 0xb5;
		}
		else if (g_sinkEyepointTilt < 0) {
			pitch = -(g_deltaTime * 0x2d0000) / 0xb5;
		}
		else {
			pitch = 0;
		}

		g_sinkEyepointTiltReset = 1;
		FUN_10011f9a(climb, speed, strafe, turn, pitch);
		break;
	}

	g_unk0x100a2408 = mode;
	FUN_1004b344();
	FUN_1001220a();
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

// Places the external view behind the local mech: p_distance and p_height move it out and up,
// within limits taken from the mech's size the first time, p_turn turns it around the mech and
// p_tilt tilts it. The first call after another view starts its ramps from the current eyepoint.
// Stack-slot permutation of the locals. The original compares g_unk0x100a23f8 with
// g_unk0x10176f2c in the other operand order.
// FUNCTION: MW2 0x100118bc
void FUN_100118bc(MechS32 p_distance, MechS32 p_height, MechS32 p_tilt, MechS32 p_turn)
{
	MechS32 y;
	MechS32 dx;
	MechS32 z;
	MechS32 dy;
	MechS32 angle;
	MechS32 dz;
	MechS32 turn;
	MechS32 tilt;
	MechS32 floor;
	MechS32 offsetX;
	MechS32 pitch;
	MechS32 offsetY;
	MechS32 sine;
	MechS32 heading;
	MechS32 offsetZ;
	MechS32 roll;
	MechU32 distance;
	MechS32 height;
	MechS32 cosine;
	MechS32 x;
	MechS32 unused;
	Mech* mech;

	FUN_100116c3(&pitch, &heading, &roll, &x, &y, &z);
	if (g_unk0x100a2408 != 1) {
		if (!g_unk0x100a2c04) {
			PlayCockpitSound(0x11, -1);
		}

		if (g_unk0x100a23ec == 0) {
			mech = g_players[0]->m_mech;
			g_unk0x100a23ec = mech->m_radius * 3;
			g_unk0x100a23f0 = g_unk0x100a23ec >> 1;
			g_unk0x100a23f4 = g_unk0x100a23ec << 2;
			g_unk0x100a23f8 = g_unk0x100a23ec >> 2;
			g_unk0x10176f2c = g_unk0x100a23f4;
			g_unk0x10176f74 = -mech->m_unk0xcc + 200;
		}

		g_unk0x10177050.m_time = g_currentClock;
		g_unk0x10176f90.m_time = g_currentClock;
		g_unk0x10176f30.m_time = g_currentClock;
		g_unk0x10176f60.m_time = g_currentClock;
		g_unk0x10176f40.m_time = g_currentClock;
		g_unk0x10177050.m_value = g_eyepoint->m_unk0x00 - x;
		g_unk0x10176f90.m_value = g_eyepoint->m_unk0x04 - y;
		g_unk0x10176f30.m_value = g_eyepoint->m_unk0x08 - z;
		g_unk0x10176f60.m_value = g_eyepoint->m_unk0x0c;
		g_unk0x10176f40.m_value = g_eyepoint->m_unk0x10;
		g_eyepoint->m_unk0x14 = 0;
		if (g_eyepoint->m_unk0x00 == x) {
			g_eyepoint->m_unk0x00 += 10;
		}

		g_zoomFov = 0x10000;
		ApplyCameraFov(0);
	}

	g_unk0x100a23ec += p_distance;
	if (g_unk0x100a23ec > g_unk0x100a23f4) {
		g_unk0x100a23ec = g_unk0x100a23f4;
	}
	else if (g_unk0x100a23ec < g_unk0x100a23f0) {
		g_unk0x100a23ec = g_unk0x100a23f0;
	}

	height = p_height + g_unk0x100a23f8;
	g_unk0x100a23fc += p_turn;
	g_unk0x100a23fc %= 0x1680000;
	dy = y - g_eyepoint->m_unk0x04;
	dz = z - g_eyepoint->m_unk0x08;
	dx = x - g_eyepoint->m_unk0x00;
	FUN_10060197(dx, dy, dz, &turn, &unused, &distance, &tilt);
	SetWrappedRampTarget(&g_unk0x10176f40, -tilt - (p_tilt >> 1));
	g_eyepoint->m_unk0x10 = UpdateWrappedRamp(&g_unk0x10176f40);
	SetWrappedRampTarget(&g_unk0x10176f60, turn);
	g_eyepoint->m_unk0x0c = UpdateWrappedRamp(&g_unk0x10176f60);
	angle = heading - g_unk0x100a23fc;
	angle %= 0x1680000;
	if (angle < -0xb40000) {
		angle += 0x1680000;
	}
	else if (angle > 0xb40000) {
		angle -= 0x1680000;
	}

	cosine = FUN_1006973a(angle);
	sine = FUN_100696c0(angle);
	offsetX = FixedMul16(g_unk0x100a23ec, sine) >> 13;
	offsetZ = FixedMul16(g_unk0x100a23ec, cosine) >> 13;
	offsetY = height;
	g_unk0x10177050.m_target = offsetX;
	g_eyepoint->m_unk0x00 = x + UpdateRamp(&g_unk0x10177050);
	g_unk0x10176f30.m_target = offsetZ;
	g_eyepoint->m_unk0x08 = z + UpdateRamp(&g_unk0x10176f30);
	floor = FUN_100113af(g_eyepoint);
	if (y + offsetY < floor) {
		offsetY = floor - y;
	}

	g_unk0x100a23f8 = height;
	if (g_unk0x100a23f8 > g_unk0x10176f2c) {
		g_unk0x100a23f8 = g_unk0x10176f2c;
	}
	else if (g_unk0x100a23f8 < g_unk0x10176f74) {
		g_unk0x100a23f8 = g_unk0x10176f74;
	}

	g_unk0x10176f90.m_target = offsetY;
	g_eyepoint->m_unk0x04 = y + UpdateRamp(&g_unk0x10176f90);
}

// Updates the cockpit view each frame: resets the pilot's look ramps after an external view, turns
// the view towards a held glance key (or back ahead once all are released), then places the
// eyepoint unless the camera is shaking.
// FUNCTION: MW2 0x10011cb0
void FUN_10011cb0(void)
{
	if (g_unk0x100a2408) {
		g_unk0x10177030.m_time = g_currentClock;
		g_unk0x10177030.m_value = 0;
		g_unk0x10176f80.m_time = g_currentClock;
		g_unk0x10176f80.m_value = 0;
		ClearCameraShakeKeys();
		ApplyCameraFov(0);
	}

	g_unk0x100a2420 = 1;
	g_unk0x100a241c = 0;
	if (g_sinkGlanceLeft) {
		g_sinkPilotPan = -0x460000;
	}
	else if (g_sinkGlanceRight) {
		g_sinkPilotPan = 0x460000;
	}
	else if (g_sinkGlanceUp) {
		g_sinkPilotTilt = -0x320000;
	}
	else if (g_sinkGlanceDown) {
		g_sinkPilotTilt = 0x280000;
	}
	else if (!g_unk0x100a2448) {
		g_sinkPilotPan = 0;
		g_sinkPilotTilt = 0;
	}

	if (!g_sinkGlanceRight && !g_sinkGlanceLeft && !g_sinkGlanceUp && !g_sinkGlanceDown) {
		g_unk0x100a2448 = 1;
	}
	else {
		g_unk0x100a2448 = 0;
	}

	if (!UpdateCameraShake()) {
		FUN_10011e45(
			&g_eyepoint->m_unk0x10,
			&g_eyepoint->m_unk0x0c,
			&g_eyepoint->m_unk0x14,
			&g_eyepoint->m_unk0x00,
			&g_eyepoint->m_unk0x04,
			&g_eyepoint->m_unk0x08
		);
	}
}

// FUN_100116c3's view, turned from the cockpit by the pilot's pan and tilt.
// Stack-slot permutation: pan and tilt.
// FUNCTION: MW2 0x10011e45
void FUN_10011e45(MechS32* p_unk0x10, MechS32* p_unk0x0c, MechS32* p_unk0x14, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	MechS32 pan;
	MechS32 tilt;

	pan = 0;
	tilt = 0;
	FUN_100116c3(p_unk0x10, p_unk0x0c, p_unk0x14, p_x, p_y, p_z);
	if (!g_unk0x100a2c04) {
		g_unk0x10177030.m_target = g_sinkPilotPan;
		pan = UpdateRamp(&g_unk0x10177030);
		g_unk0x10176f80.m_target = g_sinkPilotTilt;
		tilt = UpdateRamp(&g_unk0x10176f80);
		*p_unk0x0c += pan;
		*p_unk0x10 += tilt;
	}

	g_unk0x100a241c = 1;
}

// Switches to the drop view (mode 4): the camera starts level at the mech and falls, turning.
// Stack-slot permutation: the six locals.
// FUNCTION: MW2 0x10011edc
void FUN_10011edc(void)
{
	MechS32 unk0x10;
	MechS32 unk0x0c;
	MechS32 unk0x14;
	MechS32 x;
	MechS32 y;
	MechS32 z;

	FUN_100116c3(&unk0x10, &unk0x0c, &unk0x14, &x, &y, &z);
	if (g_unk0x100a2408 != 4) {
		g_unk0x100a243c = 0;
		g_eyepoint->m_unk0x10 = 0x5a0000;
		g_unk0x100a2444 = g_currentClock;
		ApplyCameraFov(0);
	}

	IntegrateMidpoint(&g_eyepoint->m_unk0x04, &g_unk0x100a243c, g_unk0x100a2440, g_deltaTime);
	g_eyepoint->m_unk0x00 = x;
	g_eyepoint->m_unk0x08 = z;
	g_eyepoint->m_unk0x0c += (g_currentClock - g_unk0x100a2444) * 300;
}

// Moves the free camera (mode 2): p_climb raises it, p_speed drives it forward (eased),
// p_strafe moves it sideways, and p_turn and p_pitch turn it. It stays above the ground.
// Stack-slot permutation: sinHeading, cosHeading, cosPitch and speed.
// FUNCTION: MW2 0x10011f9a
void FUN_10011f9a(MechS32 p_climb, MechS32 p_speed, MechS32 p_strafe, MechS32 p_turn, MechS32 p_pitch)
{
	MechS32 sinHeading;
	MechS32 cosHeading;
	MechS32 sinPitch;
	MechS32 floor;
	MechS32 speed;
	MechS32 cosPitch;

	sinHeading = FUN_100696c0(g_eyepoint->m_unk0x0c);
	cosHeading = FUN_1006973a(g_eyepoint->m_unk0x0c);
	sinPitch = FUN_100696c0(g_eyepoint->m_unk0x10);
	cosPitch = FUN_1006973a(g_eyepoint->m_unk0x10);
	if (g_unk0x100a2408 != 2) {
		if (g_unk0x100a2c04) {
			g_eyepoint->m_unk0x00 -= FixedMul16(g_unk0x100a23ec, sinHeading) >> 13;
			g_eyepoint->m_unk0x08 -= FixedMul16(g_unk0x100a23ec, cosHeading) >> 13;
		}

		g_eyepoint->m_unk0x14 = 0;
		g_unk0x10177040.m_value = 0;
		g_unk0x10177040.m_time = g_currentClock;
		g_zoomFov = g_normalFov;
		ApplyCameraFov(0);
	}

	g_eyepoint->m_unk0x00 += FixedMul16(p_strafe, cosHeading) >> 11;
	g_eyepoint->m_unk0x08 -= FixedMul16(p_strafe, sinHeading) >> 11;
	g_eyepoint->m_unk0x10 -= p_pitch;
	if (g_eyepoint->m_unk0x10 > 0x5a0000) {
		g_eyepoint->m_unk0x10 = 0x5a0000;
	}
	else if (g_eyepoint->m_unk0x10 < -0x5a0000) {
		g_eyepoint->m_unk0x10 = -0x5a0000;
	}

	g_eyepoint->m_unk0x04 += p_climb * 4;
	g_unk0x10177040.m_target = p_speed * 16;
	speed = UpdateRamp(&g_unk0x10177040);
	if (speed < 0x20 && speed > -0x20) {
		speed = 0;
	}

	g_eyepoint->m_unk0x0c += p_turn;
	g_eyepoint->m_unk0x00 -= FixedMul16(FixedMul16(speed, sinHeading) >> 13, cosPitch) >> 13;
	g_eyepoint->m_unk0x08 -= FixedMul16(FixedMul16(speed, cosHeading) >> 13, cosPitch) >> 13;
	floor = FUN_100113af(g_eyepoint);
	if (g_eyepoint->m_unk0x04 < floor) {
		g_eyepoint->m_unk0x04 = max(g_eyepoint->m_unk0x04, floor);
	}
}

// Turns the world's shapes of types 0x10 and 0x60 to face the eyepoint, or to a fixed angle when
// FUN_1003ee69 is set.
// FUNCTION: MW2 0x1001220a
void FUN_1001220a(void)
{
	MechS32 pitch;
	ScarletOrchid0x4c* shape;
	AmberWillow0x7c* obj;
	MechS32 z;
	MechS32 y;
	MechS32 x;
	MechS32 heading;

	for (shape = g_unk0x100ad5e8->m_unk0x08; shape; shape = shape->m_unk0x08) {
		if ((shape->m_unk0x02 & 0xf0) == 0x10 || (shape->m_unk0x02 & 0xf0) == 0x60) {
			obj = shape->m_unk0x18;
			if (obj) {
				GetObjPosition(obj, &x, &y, &z);
				if (FUN_1003ee69()) {
					heading = 0xb40000;
					pitch = -0x2d0000;
				}
				else {
					heading = FUN_100698de(g_eyepoint->m_unk0x00 - x, g_eyepoint->m_unk0x08 - z);
					pitch = 0;
				}

				SetObjRotation(obj, pitch, heading, 0, 0);
				FUN_10001cf8(obj);
			}
		}
	}
}
