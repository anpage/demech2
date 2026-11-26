// The early Mercenaries build's object after its effects' (premercs/tracers.c), which GBL doesn't have:
// rain and snow, falling in nine zones around the eyepoint (a 3x3 grid of 0x8000-unit cells). Each drop
// is an effect of g_tracers that falls from its zone's height until it reaches the ground: rain as two
// short streaks, snow as a flake whose size and shade follow its depth. A mission stream's event starts
// it (StartPrecipitation, from BwdExecuteStream), and SimMain spawns the drops every frame.

#include "premercs/precipitation.h"

#include "approxlen.h"
#include "clock.h"
#include "decomp.h"
#include "eyepoint.h"
#include "fixedfloat.h"
#include "fixedmul.h"
#include "muldiv14.h"
#include "object.h"
#include "polydraw.h"
#include "premercs/tracers.h"
#include "random.h"
#include "render.h"
#include "shape.h"
#include "types.h"
#include "vfx3d.h"
#include "vfxa.h"
#include "view.h"

#include <stdlib.h>

#pragma function(abs) // this unit calls the C runtime's abs, as weapons.c and others do (MW2_ABS_CALLS)

DECOMP_SIZE_ASSERT(PrecipitationSpec, 0x10)
DECOMP_SIZE_ASSERT(PrecipitationDrop, 0x24)
DECOMP_SIZE_ASSERT(PrecipitationZone, 0x730)

// Whether it rains or snows (StartPrecipitation), and which.
// GLOBAL: MW2PREMERCS 0x100c6dc0
MechS32 g_precipitating = 0;

// GLOBAL: MW2PREMERCS 0x100c6dc4
MechS32 g_precipitationKind = c_precipitationRain;

// The snowflakes' colors by depth, nearest first.
// GLOBAL: MW2PREMERCS 0x100ccc00
MechS32 g_snowShades[8];

// GLOBAL: MW2PREMERCS 0x100ccc20
PrecipitationZone g_precipitationZones[9];

// Takes one of p_zone's free drops. Returns it, or NULL.
// FUNCTION: MW2PREMERCS 0x10089830
PrecipitationDrop* AllocPrecipitationDrop(PrecipitationZone* p_zone)
{
	PrecipitationDrop* drop;

	drop = NULL;
	if (!p_zone || p_zone->m_usedDrops >= 45) {
		return drop;
	}

	drop = &p_zone->m_drops[p_zone->m_freeDrops[p_zone->m_usedDrops]];
	p_zone->m_usedDrops++;

	return drop;
}

// Returns p_zone's drop p_index to its free drops.
// FUNCTION: MW2PREMERCS 0x1008989a
void FreePrecipitationDrop(PrecipitationZone* p_zone, MechS32 p_index)
{
	PrecipitationDrop* drop;

	drop = NULL;
	if (!p_zone || p_zone->m_usedDrops <= 0 || p_index < 0 || p_index >= p_zone->m_dropCount) {
		return;
	}

	drop = &p_zone->m_drops[p_index];
	drop->m_tracer = -1;
	p_zone->m_usedDrops--;
	p_zone->m_freeDrops[p_zone->m_usedDrops] = p_index;
}

// Places the nine zones around the eyepoint, frees their drops and sets the snowflakes' shades.
// FUNCTION: MW2PREMERCS 0x10089925
void InitPrecipitation(void)
{
	PrecipitationZone* zone;
	PrecipitationDrop* drop;
	MechS32 i;
	MechS32 j;
	MechScalar x;
	MechScalar z;

	for (i = 0; i < 9; i++) {
		zone = &g_precipitationZones[i];
		zone->m_usedDrops = 0;
		zone->m_dropCount = 45;
		switch (i) {
		case 0:
			x = -0x4000;
			z = -0x4000;
			break;
		case 1:
			x = 0;
			z = -0x4000;
			break;
		case 2:
			x = 0x4000;
			z = -0x4000;
			break;
		case 3:
			x = -0x4000;
			z = 0;
			break;
		case 4:
			x = 0;
			z = 0;
			break;
		case 5:
			x = 0x4000;
			z = 0;
			break;
		case 6:
			x = -0x4000;
			z = 0x4000;
			break;
		case 7:
			x = 0;
			z = 0x4000;
			break;
		case 8:
			x = 0x4000;
			z = 0x4000;
			break;
		}

		zone->m_x = x;
		zone->m_y = 10000;
		zone->m_z = z;
		for (j = 0; j < 45; j++) {
			drop = &zone->m_drops[j];
			zone->m_freeDrops[j] = j;
			drop->m_tracer = -1;
			drop->m_index = j;
			drop->m_zone = zone;
		}
	}

	g_snowShades[0] = 0xfe;
	g_snowShades[1] = 0xfd;
	g_snowShades[2] = 0xfc;
	g_snowShades[3] = 0xfb;
	g_snowShades[4] = 0xfa;
	g_snowShades[5] = 0xec;
	g_snowShades[6] = 0xed;
	g_snowShades[7] = 0xee;
}

// Starts two drops in each zone, somewhere within 10000 units of its middle, while it rains or snows.
// FUNCTION: MW2PREMERCS 0x10089b3e
void UpdatePrecipitation(void)
{
	PrecipitationZone* zone;
	MechS32 i;

	if (!g_deltaTime || !g_precipitating) {
		return;
	}

	for (i = 0; i < 9; i++) {
		zone = &g_precipitationZones[i];
		switch (g_precipitationKind) {
		default:
			StartRaindrop(
				RandomIntBelow(20000) - 10000 + zone->m_x + g_eyepoint->m_x,
				zone->m_y,
				RandomIntBelow(20000) - 10000 + zone->m_z + g_eyepoint->m_z,
				0,
				-1000,
				0,
				zone
			);
			StartRaindrop(
				RandomIntBelow(20000) - 10000 + zone->m_x + g_eyepoint->m_x,
				zone->m_y,
				RandomIntBelow(20000) - 10000 + zone->m_z + g_eyepoint->m_z,
				0,
				-1000,
				0,
				zone
			);
			break;
		case c_precipitationSnow:
			StartSnowflake(
				RandomIntBelow(20000) - 10000 + zone->m_x + g_eyepoint->m_x,
				zone->m_y,
				RandomIntBelow(20000) - 10000 + zone->m_z + g_eyepoint->m_z,
				0,
				-500,
				0,
				zone
			);
			StartSnowflake(
				RandomIntBelow(20000) - 10000 + zone->m_x + g_eyepoint->m_x,
				zone->m_y,
				RandomIntBelow(20000) - 10000 + zone->m_z + g_eyepoint->m_z,
				0,
				-500,
				0,
				zone
			);
			break;
		}
	}
}

// Starts the rain, or the snow when p_spec's kind is c_precipitationSnow.
// FUNCTION: MW2PREMERCS 0x10089d46
void StartPrecipitation(PrecipitationSpec* p_spec)
{
	switch (p_spec->m_kind) {
	default:
		g_precipitationKind = c_precipitationRain;
		break;
	case c_precipitationSnow:
		g_precipitationKind = c_precipitationSnow;
		break;
	}

	g_precipitating = TRUE;
}

// FUNCTION: MW2PREMERCS 0x10089d9d
void FUN_10089d9d(void)
{
}

// A raindrop falls with its speed and drift until it reaches the ground, then it ends.
// FUNCTION: MW2PREMERCS 0x10089da8
MechS32 UpdateRaindrop(Tracer* p_tracer)
{
	PrecipitationDrop* drop;
	MechScalar x;
	MechScalar y;
	MechScalar z;

	drop = NULL;
	drop = (PrecipitationDrop*) p_tracer->m_data;
	GetObjPosition(p_tracer->m_obj, &x, &y, &z);
	if (y > 0) {
		MoveObj(
			p_tracer->m_obj,
			p_tracer->m_dx + drop->m_dx,
			p_tracer->m_dy - RandomIntBelow(10) + drop->m_dy,
			p_tracer->m_dz + drop->m_dz
		);
		UpdateObj(p_tracer->m_obj);
		p_tracer->m_color = 0x5e;
		return TRUE;
	}
	else {
		p_tracer->m_flags &= ~1;
		HideObjTree(p_tracer->m_obj);
		FreePrecipitationDrop(drop->m_zone, drop->m_index);
		return FALSE;
	}
}

// Draws a raindrop's first two points as streaks along its speed, a shade off what's behind them.
// FUNCTION: MW2PREMERCS 0x10089eae
MechS32 DrawRaindrop(Tracer* p_tracer)
{
	PrecipitationDrop* drop;
	MechS32 i;
	MechS32 color;
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechScalar sx;
	MechScalar sy;
	MechScalar sz;
	MechScalar ex;
	MechScalar ey;
	MechScalar ez;
	MechScalar dx;
	MechScalar dy;
	MechScalar dz;

	if (!p_tracer) {
		return FALSE;
	}

	GetObjPosition(p_tracer->m_obj, &x, &y, &z);
	drop = (PrecipitationDrop*) p_tracer->m_data;
	dx = p_tracer->m_dx >> 1;
	dy = p_tracer->m_dy >> 1;
	dz = p_tracer->m_dz >> 1;
	color = p_tracer->m_color;
	for (i = 0; i < 2; i++) {
		sx = p_tracer->m_points[i].m_x + x;
		sy = p_tracer->m_points[i].m_y + y;
		sz = p_tracer->m_points[i].m_z + z;
		if (ProjectWorldPoint(&sx, &sy, &sz)) {
			ex = p_tracer->m_points[i].m_x + x + dx;
			ey = p_tracer->m_points[i].m_y + dy + y;
			ez = p_tracer->m_points[i].m_z + dz + z;
			ProjectWorldPoint(&ex, &ey, &ez);
			color = VFX_pixel_read(&g_currentPane, sx, sy);
			if (color % 16 == 15) {
				color--;
			}
			else {
				color++;
			}

			VFX_line_draw(&g_currentPane, sx, sy, ex, ey, 0, color);
		}
	}

	return TRUE;
}

// Starts a raindrop of p_zone at (p_x, p_y, p_z), falling with (p_dx, p_dy, p_dz), when it's within the
// far plane and the zone and the effects have room.
// FUNCTION: MW2PREMERCS 0x1008a076
void StartRaindrop(
	MechScalar p_x,
	MechScalar p_y,
	MechScalar p_z,
	MechScalar p_dx,
	MechScalar p_dy,
	MechScalar p_dz,
	PrecipitationZone* p_zone
)
{
	Shape* shape;
	Tracer* tracer;
	PrecipitationDrop* drop;
	TracerPoint* point;
	MechS32 i;
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechScalar stepX;
	MechScalar stepY;
	MechScalar stepZ;

	x = p_x - g_eyepoint->m_x;
	y = p_y - g_eyepoint->m_y;
	z = p_z - g_eyepoint->m_z;
	if (ApproximateVectorLength(x, y, z) > g_eyepoint->m_farPlane) {
		return;
	}

	drop = AllocPrecipitationDrop(p_zone);
	if (!drop) {
		return;
	}

	tracer = StartTracer(UpdateRaindrop, DrawRaindrop, drop);
	if (!tracer) {
		FreePrecipitationDrop(p_zone, drop->m_index);
		return;
	}

	drop->m_tracer = tracer->m_index;
	drop->m_dx = drop->m_dy = drop->m_dz = 0;
	tracer->m_flags = 1;
	shape = tracer->m_obj->m_shape;
	shape->m_radius = ApproximateVectorLength(p_dx, p_dy, p_dz);
	shape->m_centerX = p_x;
	shape->m_centerY = p_y;
	shape->m_centerZ = p_z;
	shape->m_modelCenterX = 0;
	shape->m_modelCenterY = 0;
	shape->m_modelCenterZ = 0;
	tracer->m_dx = p_dx;
	tracer->m_dy = p_dy;
	tracer->m_dz = p_dz;
	x = stepX = p_dx >> 4;
	y = stepY = p_dy >> 4;
	z = stepZ = p_dz >> 4;
	ShowObjTree(tracer->m_obj);
	SetObjPosition(tracer->m_obj, p_x, p_y, p_z);
	UpdateObj(tracer->m_obj);
	tracer->m_startTime = g_currentClock;

	for (i = 0; i < 32; i++) {
		point = &tracer->m_points[i];
		point->m_shown = 1;
		x += stepX;
		y += stepY;
		z += stepZ;
		point->m_x = x + RandomIntBelow(10) - 5;
		point->m_y = y + RandomIntBelow(50) - 25;
		point->m_z = z + RandomIntBelow(10) - 5;
	}
}

// A snowflake falls with its speed and drift until it reaches the ground, then it ends.
// FUNCTION: MW2PREMERCS 0x1008a2ff
MechS32 UpdateSnowflake(Tracer* p_tracer)
{
	PrecipitationDrop* drop;
	MechScalar x;
	MechScalar y;
	MechScalar z;

	drop = NULL;
	drop = (PrecipitationDrop*) p_tracer->m_data;
	GetObjPosition(p_tracer->m_obj, &x, &y, &z);
	if (y > 0) {
		MoveObj(
			p_tracer->m_obj,
			p_tracer->m_dx + drop->m_dx,
			p_tracer->m_dy - RandomIntBelow(10) + drop->m_dy,
			p_tracer->m_dz + drop->m_dz
		);
		UpdateObj(p_tracer->m_obj);
		p_tracer->m_color = 0xdf;
		return TRUE;
	}
	else {
		p_tracer->m_flags &= ~1;
		HideObjTree(p_tracer->m_obj);
		FreePrecipitationDrop(drop->m_zone, drop->m_index);
		return FALSE;
	}
}

// Draws a snowflake's first point, shaded by its depth: an ellipse when it's 4 pixels or more across,
// a square of 1 to 3 pixels, or else a streak along a quarter of its drift.
// FUNCTION: MW2PREMERCS 0x1008a405
MechS32 DrawSnowflake(Tracer* p_tracer)
{
	PrecipitationDrop* drop;
	MechS32 i;
	MechS32 j;
	MechS32 color;
	MechS32 size;
	MechS32 sizeY;
	MechS32 vertices[4][6];
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechScalar sx;
	MechScalar sy;
	MechScalar sz;
	MechScalar ex;
	MechScalar ey;
	MechScalar ez;
	MechScalar dx;
	MechScalar dy;
	MechScalar dz;

	if (!p_tracer) {
		return FALSE;
	}

	GetObjPosition(p_tracer->m_obj, &x, &y, &z);
	drop = (PrecipitationDrop*) p_tracer->m_data;
	dx = drop->m_dx >> 2;
	dy = drop->m_dy >> 2;
	dz = drop->m_dz >> 2;
	for (i = 0; i < 1; i++) {
		size = 100;
		sx = p_tracer->m_points[i].m_x + x;
		sy = p_tracer->m_points[i].m_y + y;
		sz = p_tracer->m_points[i].m_z + z;
		if (ProjectWorldPoint(&sx, &sy, &sz)) {
			color = (MechS32) ((MechFloat) abs(sz) * 8.0f / g_eyepoint->m_farPlane);
			if (color < 0) {
				color = 0;
			}
			else if (color >= 8) {
				color = 7;
			}

			color = g_snowShades[color];
			size = ProjectRadius(g_eyepoint->m_projectScaleX, size, abs(sz));
			sizeY = FixedMul16(size, g_eyepoint->m_pixelAspect);
			if (size >= 4) {
				VFX_ellipse_fill(&g_currentPane, sx, sy, size, sizeY, color);
			}
			else if (size < 4 && size >= 1) {
				vertices[0][0] = sx;
				vertices[0][1] = sy;
				vertices[1][0] = sx + size;
				vertices[1][1] = sy;
				vertices[2][0] = sx + size;
				vertices[2][1] = sy + sizeY;
				vertices[3][0] = sx;
				vertices[3][1] = sy + sizeY;
				for (j = 0; j < 4; j++) {
					vertices[j][2] = color << 16;
					vertices[j][3] = 0;
					vertices[j][4] = 0;
					vertices[j][5] = 1;
				}

				VFX_flat_polygon(&g_currentPane, 4, (MechU32*) vertices);
			}
			else {
				ex = p_tracer->m_points[i].m_x + dx + x;
				ey = p_tracer->m_points[i].m_y + dy + y;
				ez = p_tracer->m_points[i].m_z + dz + z;
				ProjectWorldPoint(&ex, &ey, &ez);
				VFX_line_draw(&g_currentPane, sx, sy, ex, ey, 0, color);
			}
		}
	}

	return TRUE;
}

// Starts a snowflake of p_zone at (p_x, p_y, p_z), falling with (p_dx, p_dy, p_dz) and drifting up to
// 250 units a tick on x and z, when it's within the far plane and the zone and the effects have room.
// FUNCTION: MW2PREMERCS 0x1008a7c0
void StartSnowflake(
	MechScalar p_x,
	MechScalar p_y,
	MechScalar p_z,
	MechScalar p_dx,
	MechScalar p_dy,
	MechScalar p_dz,
	PrecipitationZone* p_zone
)
{
	Shape* shape;
	Tracer* tracer;
	PrecipitationDrop* drop;
	TracerPoint* point;
	MechS32 i;
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechScalar stepX;
	MechScalar stepY;
	MechScalar stepZ;

	x = p_x - g_eyepoint->m_x;
	y = p_y - g_eyepoint->m_y;
	z = p_z - g_eyepoint->m_z;
	if (ApproximateVectorLength(x, y, z) > g_eyepoint->m_farPlane) {
		return;
	}

	drop = AllocPrecipitationDrop(p_zone);
	if (!drop) {
		return;
	}

	tracer = StartTracer(UpdateSnowflake, DrawSnowflake, drop);
	if (!tracer) {
		FreePrecipitationDrop(p_zone, drop->m_index);
		return;
	}

	drop->m_tracer = tracer->m_index;
	drop->m_dx = RandomIntBelow(500) - 250;
	drop->m_dy = 0;
	drop->m_dz = RandomIntBelow(500) - 250;
	tracer->m_flags = 1;
	shape = tracer->m_obj->m_shape;
	shape->m_radius = ApproximateVectorLength(p_dx, p_dy, p_dz);
	shape->m_centerX = p_x;
	shape->m_centerY = p_y;
	shape->m_centerZ = p_z;
	shape->m_modelCenterX = 0;
	shape->m_modelCenterY = 0;
	shape->m_modelCenterZ = 0;
	tracer->m_dx = p_dx;
	tracer->m_dy = p_dy;
	tracer->m_dz = p_dz;
	x = stepX = p_dx >> 4;
	y = stepY = p_dy >> 4;
	z = stepZ = p_dz >> 4;
	ShowObjTree(tracer->m_obj);
	SetObjPosition(tracer->m_obj, p_x, p_y, p_z);
	UpdateObj(tracer->m_obj);
	tracer->m_startTime = g_currentClock;

	for (i = 0; i < 32; i++) {
		point = &tracer->m_points[i];
		point->m_shown = 1;
		x += stepX;
		y += stepY;
		z += stepZ;
		point->m_x = x + RandomIntBelow(10) - 5;
		point->m_y = y + RandomIntBelow(50) - 25;
		point->m_z = z + RandomIntBelow(10) - 5;
	}
}
