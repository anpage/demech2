#include "hud.h"

#include "bargauges.h"
#include "cockpitpanel.h"
#include "config.h"
#include "decomp.h"
#include "environment.h"
#include "eyepoint.h"
#include "fixedmul.h"
#include "geocache.h"
#include "loadres.h"
#include "muldiv.h"
#include "muldiv14.h"
#include "mw2prj.h"
#include "object.h"
#include "players.h"
#include "polydraw.h"
#include "ray.h"
#include "render.h"
#include "rendertarget.h"
#include "screenscale.h"
#include "setres.h"
#include "shape.h"
#include "team.h"
#include "types.h"
#include "vfxa.h"
#include "view.h"
#include "weapondata.h"
#include "weapons.h"

// The heading tape's shape width (FUN_10040f91).
// GLOBAL: MW2 0x100a5ed0
MechS32 g_unk0x100a5ed0 = 0xf0f;

// GLOBAL: MW2 0x100a5ed4
MechS32 g_unk0x100a5ed4 = 0xd79;

// The altimeter's place, in 16.16 fractions of its gauge until FUN_10040f91 scales it.
// GLOBAL: MW2 0x100a5ed8
Point g_unk0x100a5ed8 = {0xb333, 0x8000};

// The compass's place, likewise.
// GLOBAL: MW2 0x100a5ee0
Point g_unk0x100a5ee0 = {0x8000, 0x6666};

// GLOBAL: MW2 0x100a5ee8
Point g_unk0x100a5ee8[6] = {{0x73, 0x10}, {8, 0x4a}, {4, 0x28}, {4, 0x4a}, {0, 0}, {0, 0}};

// GLOBAL: MW2 0x100a5f18
undefined4 g_unk0x100a5f18 = 1;

// GLOBAL: MW2 0x100a5f1c
MechS32 g_unk0x100a5f1c = 1;

// GLOBAL: MW2 0x100a5f20
MechS32 g_unk0x100a5f20 = 1;

// GLOBAL: MW2 0x100a5f24
MechS32 g_unk0x100a5f24 = 1;

// GLOBAL: MW2 0x100a5f2c
MechS32 g_unk0x100a5f2c = 1;

// The layout of the altimeter and the compass, from their shapes' extents (FUN_10040f91).

// GLOBAL: MW2 0x100be5a0
static MechS32 g_unk0x100be5a0;

// GLOBAL: MW2 0x100be5a4
static MechS32 g_unk0x100be5a4;

// GLOBAL: MW2 0x100be5a8
static MechS32 g_unk0x100be5a8;

// GLOBAL: MW2 0x100be5ac
static MechS32 g_unk0x100be5ac;

// GLOBAL: MW2 0x100be5b0
static MechS32 g_unk0x100be5b0;

// GLOBAL: MW2 0x100be5b4
static MechS32 g_unk0x100be5b4;

// The altimeter's scale: pixels per 16.16 unit of height.
// GLOBAL: MW2 0x100be5b8
static MechS32 g_unk0x100be5b8;

// The compass's scale: pixels per degree, 16.16.
// GLOBAL: MW2 0x100be5bc
static MechS32 g_unk0x100be5bc;

// GLOBAL: MW2 0x100be5c0
static MechS32 g_unk0x100be5c0;

// GLOBAL: MW2 0x100be5c4
static MechS32 g_unk0x100be5c4;

// GLOBAL: MW2 0x100be5c8
static MechS32 g_unk0x100be5c8;

// Draws the cockpit overlays enabled in the display options: the message boxes, the radar
// (FUN_100412dd), FUN_100414ab and FUN_10040cbc.
// FUNCTION: MW2 0x10040b30
void FUN_10040b30(
	Mech* p_mech,
	MechS32 p_unk0x04,
	MechS32 p_unk0x08,
	MechS32 p_unk0x0c,
	MechS32 p_unk0x10,
	MechS32 p_unk0x14,
	MechS32 p_unk0x18
)
{
	if (!g_unk0x100a5f18) {
		return;
	}

	if (g_unk0x100a5f24) {
		FUN_1004183a(g_unk0x100a5ee8[0].m_x, g_unk0x100a5ee8[0].m_y, p_unk0x04, p_unk0x08);
		FUN_1004161f(p_mech, g_unk0x100a5ee8[0].m_x, g_unk0x100a5ee8[0].m_y, p_unk0x0c, p_unk0x10, p_unk0x14);
	}

	if (g_unk0x100a5f1c) {
		FUN_100412dd(p_mech, p_unk0x10, p_unk0x14, p_unk0x18);
	}

	if (g_unk0x100a5f20) {
		FUN_100414ab(p_mech);
	}

	if (g_unk0x100a5f2c) {
		FUN_10040cbc(p_mech, g_unk0x100a5ee8[1].m_x, g_unk0x100a5ee8[1].m_y);
	}
}

// FUN_10040b30 with the overlays at their default places, and the radar only with p_unk0x1c.
// FUNCTION: MW2 0x10040bfd
void FUN_10040bfd(
	Mech* p_mech,
	MechS32 p_unk0x04,
	MechS32 p_unk0x08,
	MechS32 p_unk0x0c,
	MechS32 p_unk0x10,
	MechS32 p_unk0x14,
	MechS32 p_unk0x18,
	MechS32 p_unk0x1c
)
{
	if (!g_unk0x100a5f18) {
		return;
	}

	if (g_unk0x100a5f24) {
		FUN_1004183a(0x73, 0x10, p_unk0x04, p_unk0x08);
		FUN_1004161f(p_mech, 0x73, 0x10, p_unk0x0c, p_unk0x10, p_unk0x14);
	}

	if (p_unk0x1c && g_unk0x100a5f1c) {
		FUN_100412dd(p_mech, p_unk0x10, p_unk0x14, p_unk0x18);
	}

	if (g_unk0x100a5f20) {
		FUN_100414ab(p_mech);
	}

	if (g_unk0x100a5f2c) {
		FUN_10040cbc(p_mech, 8, 0x4a);
	}
}

// Draws the altimeter: the mech's height, the height of m_player's mark (Player::m_unk0x74) and
// the height of the target, clamped to the gauge. p_x and p_y go unused.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10040cbc
void FUN_10040cbc(Mech* p_mech, MechS32 p_x, MechS32 p_y)
{
	CockpitPanel* gauge;
	MechS32 height;
	Mech* mech;
	MechS32 x;
	PANE* target;
	MechS32 y;
	MechS32 mark;
	MechS32 unused;
	MechS32 shape;
	MechS32 level;

	gauge = g_unk0x100c3280[23];
	target = g_unk0x100c3280[23]->m_target;
	height = p_mech->m_player->m_position.m_y - p_mech->m_height;
	level = (MulDiv64(g_unk0x100be5b8, height - 20100, 100) >> 16) + g_unk0x100a5ed8.m_y;
	if (g_unk0x100a5ed8.m_y < level) {
		FUN_10041f06(g_unk0x100a5ed8.m_x, g_unk0x100a5ed8.m_y, 0x115, target);
	}

	FUN_10041f06(g_unk0x100a5ed8.m_x, level, 7, target);
	FUN_10041f06(g_unk0x100be5c4, g_unk0x100a5ed8.m_y, 1, target);
	mark = (MulDiv64(g_unk0x100be5b8, height - p_mech->m_player->m_groundHeight, 100) >> 16) + g_unk0x100a5ed8.m_y;
	FUN_10041f06(g_unk0x100be5b0, mark, 4, target);
	if (!(p_mech->m_player->m_targetInfo.m_target & 0xf00) || (p_mech->m_player->m_targetInfo.m_target & 0x1000)) {
		return;
	}

	x = g_unk0x100be5b4;
	if ((p_mech->m_player->m_targetInfo.m_target & 0xf00) == 0x200) {
		mech = g_players[p_mech->m_player->m_targetInfo.m_target & 0xff]->m_mech;
		y = (MulDiv64(g_unk0x100be5b8, height - (mech->m_player->m_position.m_y - mech->m_height), 100) >> 16) +
			g_unk0x100a5ed8.m_y;
	}
	else if ((p_mech->m_player->m_targetInfo.m_target & 0xf00) == 0x400) {
		FUN_10020c6f(g_gameThings[p_mech->m_player->m_targetInfo.m_target & 0xff].m_unk0x04, &unused, &y, &unused);
		y = (MulDiv64(g_unk0x100be5b8, height - y, 100) >> 16) + g_unk0x100a5ed8.m_y;
	}
	else if ((p_mech->m_player->m_targetInfo.m_target & 0xf00) == 0x100) {
		y = (MulDiv64(
				 g_unk0x100be5b8,
				 height - g_navTable[p_mech->m_player->m_targetInfo.m_target & 0xff].m_position[1],
				 100
			 ) >>
			 16) +
			g_unk0x100a5ed8.m_y;
	}
	else {
		return;
	}

	if (y < 0) {
		y = 0;
		shape = 0x25;
		x += g_unk0x100a5ed0;
	}
	else if (gauge->m_height < y) {
		y = gauge->m_height;
		shape = 0x1c;
		x += g_unk0x100a5ed0;
	}
	else {
		shape = 0x1f;
	}

	FUN_10041f73(x, y, shape, target);
}

// Lays out the altimeter and the compass from their shapes' extents. Each shape is released by
// its extent plus its id, not by the id it was loaded with.
// Stack-slot permutation; the second g_unk0x100a5ed0 sum loads its operands in the other order.
// FUNCTION: MW2 0x10040f91
void FUN_10040f91(void)
{
	void* shape;
	MechS32 height;
	MechS32 width;
	PANE* target;

	target = g_unk0x100c3280[23]->m_target;
	FUN_10056bc1(target, &g_unk0x100a5ed8, &g_unk0x100a5ed8);
	shape = FUN_1001a19f(g_mw2PrjHandle, g_unk0x100e9614 + 1, g_resourceTypeTags[c_resTagShp], 0);
	if (shape) {
		MechS32 extent;

		extent = VFX_shape_resolution(shape, 0);
		g_unk0x100a5ed0 = extent >> 16;
		g_unk0x100a5ed4 = extent & 0xffff;
		FUN_1001a163(extent + 1, g_resourceTypeTags[c_resTagShp]);
	}

	g_unk0x100be5c4 = g_unk0x100a5ed8.m_x;
	g_unk0x100be5b0 = g_unk0x100a5ed0 + g_unk0x100be5c4;
	g_unk0x100be5b4 = g_unk0x100a5ed0 + g_unk0x100be5b0;
	shape = FUN_1001a19f(g_mw2PrjHandle, g_unk0x100e9614 + 7, g_resourceTypeTags[c_resTagShp], 0);
	if (shape) {
		MechS32 extent;

		extent = VFX_shape_resolution(shape, 0);
		width = extent >> 16;
		height = extent & 0xffff;
		g_unk0x100a5ed8.m_x -= width;
		FUN_1001a163(extent + 7, g_resourceTypeTags[c_resTagShp]);
		g_unk0x100be5b8 = (height << 16) / 0xe8;
	}

	target = g_unk0x100c3280[24]->m_target;
	FUN_10056bc1(target, &g_unk0x100a5ee0, &g_unk0x100a5ee0);
	shape = FUN_1001a19f(g_mw2PrjHandle, g_unk0x100e9614 + 0x19, g_resourceTypeTags[c_resTagShp], 0);
	if (shape) {
		MechS32 extent;

		extent = VFX_shape_resolution(shape, 0);
		width = extent >> 16;
		height = extent & 0xffff;
		FUN_1001a163(extent + 0x19, g_resourceTypeTags[c_resTagShp]);
		g_unk0x100be5bc = (width << 16) / 0x168;
	}

	shape = FUN_1001a19f(g_mw2PrjHandle, g_unk0x100e9614 + 0x13, g_resourceTypeTags[c_resTagShp], 0);
	if (shape) {
		MechS32 extent;
		MechS32 origin;

		extent = VFX_shape_resolution(shape, 0);
		origin = VFX_shape_origin(shape, 0);
		extent &= 0xffff;
		origin &= 0xffff;
		g_unk0x100be5c8 = extent - origin;
		g_unk0x100be5c0 = origin;
		FUN_1001a163(extent + 0x13, g_resourceTypeTags[c_resTagShp]);
	}

	shape = FUN_1001a19f(g_mw2PrjHandle, g_unk0x100e9614 + 0x25, g_resourceTypeTags[c_resTagShp], 0);
	if (shape) {
		MechS32 extent;

		extent = VFX_shape_resolution(shape, 0);
		g_unk0x100be5a0 = extent >> 16;
		g_unk0x100be5a4 = extent & 0xffff;
		FUN_1001a163(extent + 0x25, g_resourceTypeTags[c_resTagShp]);
	}

	shape = FUN_1001a19f(g_mw2PrjHandle, g_unk0x100e9614 + 0x1f, g_resourceTypeTags[c_resTagShp], 0);
	if (shape) {
		MechS32 extent;

		extent = VFX_shape_resolution(shape, 0);
		g_unk0x100be5a8 = extent >> 16;
		g_unk0x100be5ac = extent & 0xffff;
		FUN_1001a163(extent + 0x1f, g_resourceTypeTags[c_resTagShp]);
	}
}

// Returns the positions of the cockpit's message boxes.
// FUNCTION: MW2 0x100412c8
Point* FUN_100412c8(void)
{
	return g_unk0x100a5ee8;
}

// Draws the crosshair at the end of the aim ray: for a selected weapon that follows its locked
// target, the locked shape when the target is in range and within 3 units and 3 degrees of the
// aim (returns TRUE then); otherwise the lock or guided-state shapes, or 0x67 without a ready
// weapon.
// Stack-slot permutation; range > p_unk0x08 compares in the other operand order.
// FUNCTION: MW2 0x100412dd
MechS32 FUN_100412dd(Mech* p_mech, MechS32 p_unk0x04, MechS32 p_unk0x08, MechS32 p_unk0x0c)
{
	MechS32 x;
	MechS32 pitch;
	MechS32 shape;
	MechS32 shapeOut;
	MechS32 shapeIn;
	WeaponSlot* slot;
	WeaponDef* def;
	MechS32 range;
	MechS32 result;
	MechS32 y;

	result = FALSE;
	shape = 0x76;
	slot = &p_mech->m_weapons[p_mech->m_selectedWeapon];
	def = &g_weaponDefs[slot->m_type];
	if (slot->m_state == 1) {
		if (def->m_shotType == 3) {
			shapeIn = 0x6a;
			shapeOut = 0x6d;
		}
		else {
			shapeOut = 0x76;
			shapeIn = 0x73;
		}

		if (def->m_unk0x18 == 0) {
			range = 0x30000;
			pitch = 3;
			if (p_mech->m_player->m_targetInfo.m_target && !(p_mech->m_player->m_targetInfo.m_target & 0x1100) &&
				def->m_unk0x3c < p_unk0x0c && def->m_unk0x40 > p_unk0x0c && range > p_unk0x08 && -range < p_unk0x08 &&
				pitch > p_unk0x04 && -pitch < p_unk0x04) {
				result = TRUE;
				shape = shapeIn;
			}
			else {
				shape = shapeOut;
			}
		}
		else if (p_mech->m_flags & 0x80) {
			shape = 0x61;
		}
		else if (p_mech->m_flags & 0x8000) {
			shape = 0x70;
		}
		else {
			shape = 0x6d;
		}
	}
	else {
		shape = 0x67;
	}

	if (!FUN_10041998(p_mech, &x, &y)) {
		return result;
	}

	FUN_10041e98(x, y, shape);
	return result;
}

// Marks the player's target on the screen: a player's or a game thing's mech with its side's
// markers (FUN_10041a14, FUN_10041c3c), a nav with shape 0xe5, or 0xeb at the edge when it is
// off the screen.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x100414ab
void FUN_100414ab(Mech* p_mech)
{
	MechS32 index;
	Player* player;
	MechS32 x;
	MechS32 y;
	MechS32 z;
	MechS32 onScreen;
	MechS32 offScreen;
	MechS32 target;
	Point point;

	player = p_mech->m_player;
	target = player->m_targetInfo.m_target;
	if (!target || target & 0x1000) {
		return;
	}

	index = target & 0xff;
	target &= 0xf00;
	switch (target) {
	case 0x200:
		FUN_10041a14(g_players[index], GetPlayerSide(index));
		return;
	case 0x400:
		FUN_10041c3c(FUN_1005ff56(), FUN_1003c30e(index));
		return;
	case 0x100:
		onScreen = 0xe5;
		offScreen = 0xeb;
		break;
	default:
		return;
	}

	x = player->m_targetInfo.m_position.m_x;
	y = player->m_targetInfo.m_position.m_y;
	z = player->m_targetInfo.m_position.m_z;
	if (FUN_1004c11d(&x, &y, &z)) {
		FUN_10041e98(x, y, onScreen);
	}
	else {
		point.m_x = x;
		point.m_y = y;
		FUN_10057a03(&g_currentPane, &point, &point);
		FUN_10041e98(point.m_x, point.m_y, offScreen);
	}
}

// Draws the compass's aim markers: the torso's offset p_unk0x0c (p_unk0x10 without a target
// bit 0x100), and for a target the arrows above and below while p_unk0x14 is beyond 3 units.
// p_x and p_y go unused.
// Stack-slot permutation; the two y sums load g_unk0x100a5ee0.m_y first (commutative operands).
// FUNCTION: MW2 0x1004161f
void FUN_1004161f(Mech* p_mech, MechS32 p_x, MechS32 p_y, MechS32 p_unk0x0c, MechS32 p_unk0x10, MechS32 p_unk0x14)
{
	CockpitPanel* gauge;
	PANE* target;
	MechS32 x;
	MechS32 y;
	MechS32 x2;
	MechS32 y2;

	gauge = g_unk0x100c3280[24];
	target = g_unk0x100c3280[24]->m_target;
	if (!(p_mech->m_player->m_targetInfo.m_target & 0x100)) {
		p_unk0x0c = p_unk0x10;
	}

	if (p_mech->m_autopilot != 2) {
		if (!(p_mech->m_player->m_targetInfo.m_target & 0xf00) || (p_mech->m_player->m_targetInfo.m_target & 0x1000)) {
			return;
		}

		if (p_unk0x14 > -0x30000) {
			FUN_10041f73(g_unk0x100a5ee0.m_x, g_unk0x100a5ee0.m_y - g_unk0x100be5a4 - g_unk0x100be5c0, 0x25, target);
		}

		if (p_unk0x14 < 0x30000) {
			FUN_10041f73(g_unk0x100a5ee0.m_x, g_unk0x100be5a4 + g_unk0x100a5ee0.m_y + g_unk0x100be5c8, 0x1c, target);
		}
	}

	if (p_unk0x0c == 0) {
		FUN_10041f06(g_unk0x100a5ee0.m_x, g_unk0x100a5ee0.m_y, 0x10, target);
	}
	else {
		FUN_10041f06(g_unk0x100a5ee0.m_x + FixedMul16(g_unk0x100be5bc, p_unk0x0c), g_unk0x100a5ee0.m_y, 0xd, target);
	}

	if (p_unk0x0c == 0) {
		FUN_10041f06(g_unk0x100a5ee0.m_x, g_unk0x100a5ee0.m_y, 0x16, target);
	}

	if (p_unk0x0c > -3) {
		x = gauge->m_width + g_unk0x100be5a8 - 1;
		y = g_unk0x100a5ee0.m_y + g_unk0x100be5ac / 2;
		FUN_10041f73(x, y, 0x22, target);
		if (p_unk0x0c > 0x5a) {
			FUN_10041f73(x + 1, y, 0x22, target);
		}
	}

	if (p_unk0x0c < 3) {
		x2 = -g_unk0x100be5a8;
		y2 = g_unk0x100a5ee0.m_y + g_unk0x100be5ac / 2;
		FUN_10041f73(x2, y2, 0x1f, target);
		if (p_unk0x0c < -0x5a) {
			FUN_10041f73(x2 - 1, y2, 0x1f, target);
		}
	}
}

// Draws the compass tape at heading p_unk0x08 (degrees), twice to wrap around, and the turn rate
// p_unk0x0c as a bar from the center. p_x and p_y go unused.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1004183a
void FUN_1004183a(MechS32 p_x, MechS32 p_y, MechS32 p_unk0x08, MechS32 p_unk0x0c)
{
	CockpitPanel* gauge;
	MechS32 x2;
	MechS32 x;
	MechS32 heading;
	PANE* target;
	MechS32 offset;

	gauge = g_unk0x100c3280[24];
	target = g_unk0x100c3280[24]->m_target;
	heading = (p_unk0x08 + 360) % 360;
	x = g_unk0x100a5ee0.m_x + FixedMul16(g_unk0x100be5bc, heading);
	if (gauge->m_width > x) {
		x2 = x + FixedMul16(g_unk0x100be5bc, 360);
	}
	else {
		x2 = x - FixedMul16(g_unk0x100be5bc, 360);
	}

	FUN_10041f06(x, g_unk0x100a5ee0.m_y, 0x19, target);
	FUN_10041f06(x2, g_unk0x100a5ee0.m_y, 0x19, target);
	if (p_unk0x0c) {
		offset = p_unk0x0c;
		if (offset < 0) {
			FUN_1004d8ae(
				target,
				g_unk0x100a5ee0.m_x + offset,
				g_unk0x100a5ee0.m_y - g_unk0x100be5c0,
				-offset,
				g_unk0x100be5c0,
				0xf
			);
		}
		else {
			FUN_1004d8ae(
				target,
				g_unk0x100a5ee0.m_x,
				g_unk0x100a5ee0.m_y - g_unk0x100be5c0,
				offset,
				g_unk0x100be5c0,
				0xf
			);
		}
	}

	FUN_10041f06(g_unk0x100a5ee0.m_x, g_unk0x100a5ee0.m_y, 0x13, target);
}

// Projects the end of the mech's player's aim ray to the screen: returns FUN_1004c11d's result,
// and the point in p_x and p_y.
// Stack-slot permutation: result, ray, x, y and z.
// FUNCTION: MW2 0x10041998
MechS32 FUN_10041998(Mech* p_mech, MechS32* p_x, MechS32* p_y)
{
	MechS32 result;
	Ray ray;
	MechS32 z;
	MechS32 y;
	MechS32 x;

	FUN_100463e5(p_mech->m_player, &ray);
	SetRayLength(&ray, FUN_1004635c(p_mech->m_player));
	x = ray.m_x1;
	y = ray.m_y1;
	z = ray.m_z1;
	result = FUN_1004c11d(&x, &y, &z);
	*p_x = x;
	*p_y = y;
	return result;
}

// Marks player p_player on the screen: brackets at the corners of its mech, sized by its radius
// and depth, in its side's shapes; off the screen, an arrow at the edge.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x10041a14
void FUN_10041a14(Player* p_player, MechS32 p_side)
{
	MechS32 topLeft;
	MechS32 topRight;
	MechS32 bottomLeft;
	MechS32 bottomRight;
	MechS32 sx;
	MechS32 sy;
	MechS32 x;
	MechS32 y;
	MechS32 z;
	MechS32 size;
	Point point;

	x = p_player->m_position.m_x;
	y = p_player->m_position.m_y;
	z = p_player->m_position.m_z;
	if (!FUN_1004c11d(&x, &y, &z)) {
		point.m_x = x;
		point.m_y = y;
		FUN_10057a03(&g_currentPane, &point, &point);
		switch (p_side) {
		case 0:
			topLeft = 0xeb;
			break;
		case 2:
			topLeft = 0xee;
			break;
		default:
			topLeft = 0xe8;
			break;
		}

		FUN_10041e98(point.m_x, point.m_y, topLeft);
	}
	else {
		size = p_player->m_mech->m_radius;
		size = ProjectRadius(g_eyepoint->m_projectScaleX, size, z);
		switch (p_side) {
		case 0:
			topLeft = 0xb8;
			topRight = 0xc1;
			bottomLeft = 0xca;
			bottomRight = 0xd3;
			break;
		case 2:
			topLeft = 0xbe;
			topRight = 0xc7;
			bottomLeft = 0xd0;
			bottomRight = 0xd9;
			break;
		case 1:
			topLeft = 0xbb;
			topRight = 0xc4;
			bottomLeft = 0xcd;
			bottomRight = 0xd6;
			break;
		}

		sx = x - size;
		sy = y - size;
		FUN_10041e98(sx, sy, topLeft);
		sx = x + size;
		sy = y - size;
		FUN_10041e98(sx, sy, topRight);
		sx = x - size;
		sy = y + size;
		FUN_10041e98(sx, sy, bottomLeft);
		sx = x + size;
		sy = y + size;
		FUN_10041e98(sx, sy, bottomRight);
	}
}

// Marks the object p_object on the screen like FUN_10041a14 a player: brackets around its shape,
// at most half the screen apart, or an arrow at the edge.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x10041c3c
void FUN_10041c3c(struct SceneObject* p_object, MechS32 p_side)
{
	MechS32 maxSize;
	MechS32 topLeft;
	MechS32 topRight;
	MechS32 bottomLeft;
	MechS32 bottomRight;
	MechS32 sx;
	MechS32 sy;
	MechS32 x;
	MechS32 y;
	MechS32 z;
	MechS32 size;
	Point point;

	if (!p_object || !p_object->m_unk0x6c) {
		return;
	}

	size = FUN_1003adc9(p_object->m_unk0x6c, &x, &y, &z);
	if (!FUN_1004c11d(&x, &y, &z)) {
		point.m_x = x;
		point.m_y = y;
		FUN_10057a03(&g_currentPane, &point, &point);
		switch (p_side) {
		case 0:
			topLeft = 0xeb;
			break;
		case 2:
			topLeft = 0xee;
			break;
		default:
			topLeft = 0xe8;
			break;
		}

		FUN_10041e98(point.m_x, point.m_y, topLeft);
	}
	else {
		size = ProjectRadius(g_eyepoint->m_projectScaleX, size >> 1, z);
		maxSize = g_eyepoint->m_halfWidth >> 1;
		if (size > maxSize) {
			size = maxSize;
		}
		switch (p_side) {
		case 0:
			topLeft = 0xb8;
			topRight = 0xc1;
			bottomLeft = 0xca;
			bottomRight = 0xd3;
			break;
		case 2:
			topLeft = 0xbe;
			topRight = 0xc7;
			bottomLeft = 0xd0;
			bottomRight = 0xd9;
			break;
		case 1:
			topLeft = 0xbb;
			topRight = 0xc4;
			bottomLeft = 0xcd;
			bottomRight = 0xd6;
			break;
		}

		sx = x - size;
		sy = y - size;
		FUN_10041e98(sx, sy, topLeft);
		sx = x + size;
		sy = y - size;
		FUN_10041e98(sx, sy, topRight);
		sx = x - size;
		sy = y + size;
		FUN_10041e98(sx, sy, bottomLeft);
		sx = x + size;
		sy = y + size;
		FUN_10041e98(sx, sy, bottomRight);
	}
}

// Draws frame 0 of the "SHP" resource p_id (relative to g_unk0x100e9614) at p_x, p_y of the
// current pane.
// FUNCTION: MW2 0x10041e98
void FUN_10041e98(MechS32 p_x, MechS32 p_y, MechS32 p_id)
{
	void* shape;

	shape = FUN_1001a19f(g_mw2PrjHandle, p_id + g_unk0x100e9614, g_resourceTypeTags[c_resTagShp], 0);
	if (shape) {
		VFX_shape_draw(&g_currentPane, shape, 0, p_x, p_y);
		FUN_1001a163(p_id + g_unk0x100e9614, g_resourceTypeTags[c_resTagShp]);
	}
}

// Draws frame 0 of the "SHP" resource p_id (relative to g_unk0x100e9614) at p_x, p_y.
// Operand order: p_id + g_unk0x100e9614 loads p_id first in the original.
// FUNCTION: MW2 0x10041f06
void FUN_10041f06(MechS32 p_x, MechS32 p_y, MechS32 p_id, PANE* p_target)
{
	void* shape;

	shape = FUN_1001a19f(g_mw2PrjHandle, p_id + g_unk0x100e9614, g_resourceTypeTags[c_resTagShp], 0);
	if (shape) {
		VFX_shape_draw(p_target, shape, 0, p_x, p_y);
		FUN_1001a163(p_id + g_unk0x100e9614, g_resourceTypeTags[c_resTagShp]);
	}
}

// Draws FUN_10041e98's shape at p_x, p_y of pane p_target.
// FUNCTION: MW2 0x10041f73
void FUN_10041f73(MechS32 p_x, MechS32 p_y, MechS32 p_id, PANE* p_target)
{
	FUN_10041e98(p_target->m_x0 + p_x, p_target->m_y0 + p_y, p_id);
}
