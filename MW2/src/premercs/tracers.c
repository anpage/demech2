// The early Mercenaries build's object after shots.c's, which GBL doesn't have: a pool of up to a
// thousand small effects, each a hidden three-vertex shape in the scene (whose face DrawScenePolygon
// draws as the effect) with points of its own drawn as lines or pixels: a trail behind each missile
// (UpdateShot), sparks from debris, and a tracer ahead of each machine-gun shot (SpawnShot).

#include "premercs/tracers.h"

#include "approxlen.h"
#include "clock.h"
#include "decomp.h"
#include "fixedfloat.h"
#include "object.h"
#include "polydraw.h"
#include "random.h"
#include "render.h"
#include "shape.h"
#include "shapelists.h"
#include "types.h"
#include "vfxa.h"
#include "view.h"

#include <stdio.h>

DECOMP_SIZE_ASSERT(TracerPoint, 0xe)
DECOMP_SIZE_ASSERT(Tracer, 0x1fa)

// How many effects InitTracers made, and how many are running.
// GLOBAL: MW2PREMERCS 0x100c6da0
MechS32 g_tracerCount = 0;

// GLOBAL: MW2PREMERCS 0x100c6da4
MechS32 g_activeTracers = 0;

// The running effects, in the order they started.
// GLOBAL: MW2PREMERCS 0x100c6da8
Tracer* g_tracerHead = NULL;

// GLOBAL: MW2PREMERCS 0x100c6dac
Tracer* g_tracerTail = NULL;

// Whether effects start (SetTracersEnabled).
// GLOBAL: MW2PREMERCS 0x100c6db0
MechS32 g_tracersEnabled = 1;

// The missile trails' colors as they age (eighteen steps), and the sparks' (eight).
// GLOBAL: MW2PREMERCS 0x100d0cd0
MechU8 g_trailColors[18];

// The top of g_freeTracers.
// GLOBAL: MW2PREMERCS 0x100d0ce4
MechS32 g_freeTracerTop;

// GLOBAL: MW2PREMERCS 0x100d0ce8
MechU8 g_sparkColors[8];

// The running effects' indices, gathered before a pass that may stop them.
// GLOBAL: MW2PREMERCS 0x100d0cf0
MechS32 g_runningTracers[1000];

// The free effects' indices, a stack.
// GLOBAL: MW2PREMERCS 0x100d1c90
MechS32 g_freeTracers[1000];

// GLOBAL: MW2PREMERCS 0x100d2c30
MechChar g_tracerText[0x50];

// How many g_runningTracers holds.
// GLOBAL: MW2PREMERCS 0x100d2c80
MechS32 g_runningTracerCount;

// GLOBAL: MW2PREMERCS 0x100d2c90
Tracer g_tracers[1000];

// Appends p_tracer to the running list.
// FUNCTION: MW2PREMERCS 0x10087ee0
void LinkTracer(Tracer* p_tracer)
{
	if (!p_tracer) {
		return;
	}

	if (!g_tracerHead && !g_tracerTail) {
		p_tracer->m_prev = NULL;
		p_tracer->m_next = NULL;
		g_tracerHead = p_tracer;
		g_tracerTail = p_tracer;
	}
	else {
		p_tracer->m_prev = g_tracerTail;
		p_tracer->m_next = NULL;
		g_tracerTail->m_next = p_tracer;
		g_tracerTail = p_tracer;
	}
}

// Takes p_tracer off the running list.
// FUNCTION: MW2PREMERCS 0x10087f64
void UnlinkTracer(Tracer* p_tracer)
{
	Tracer* prev;
	Tracer* next;

	if (!p_tracer || !g_tracerHead || !g_tracerTail) {
		return;
	}

	if (p_tracer == g_tracerHead) {
		if (p_tracer == g_tracerTail) {
			g_tracerHead = NULL;
			g_tracerTail = NULL;
		}
		else {
			g_tracerHead = p_tracer->m_next;
			g_tracerHead->m_prev = NULL;
		}
	}
	else if (p_tracer == g_tracerTail) {
		g_tracerTail = p_tracer->m_prev;
		g_tracerTail->m_next = NULL;
	}
	else {
		prev = p_tracer->m_prev;
		next = p_tracer->m_next;
		prev->m_next = next;
		next->m_prev = prev;
	}

	p_tracer->m_prev = NULL;
	p_tracer->m_next = NULL;
}

// Returns effect p_index to the free stack. Returns it, or NULL.
// FUNCTION: MW2PREMERCS 0x10088049
Tracer* FreeTracer(MechS32 p_index)
{
	Tracer* tracer;

	tracer = NULL;
	if (g_tracerCount - 1 != g_freeTracerTop && p_index >= 0 && p_index < g_tracerCount) {
		tracer = &g_tracers[p_index];
		g_freeTracerTop++;
		g_freeTracers[g_freeTracerTop] = p_index;
		UnlinkTracer(tracer);
	}

	return tracer;
}

// Takes an effect off the free stack and runs it, while effects are enabled. Returns it, or NULL.
// FUNCTION: MW2PREMERCS 0x100880ca
Tracer* AllocTracer(void)
{
	Tracer* tracer;

	tracer = NULL;
	if (g_freeTracerTop >= 0 && g_tracersEnabled) {
		tracer = &g_tracers[g_freeTracers[g_freeTracerTop]];
		LinkTracer(tracer);
		g_freeTracerTop--;
	}

	return tracer;
}

// Makes the effects (up to a thousand, while shapes can be made): each a hidden, collisionless
// three-vertex shape of kind 0x60 whose face's color is 0x8000 and its index, and the colors.
// FUNCTION: MW2PREMERCS 0x10088133
void InitTracers(void)
{
	MechU16 i;
	MechU16 j;
	MechU16 color;
	Shape* shape;
	struct Face* face;
	SceneObject* obj;
	void* indices;
	MechS32 failed;

	failed = 0;
	g_tracerHead = g_tracerTail = NULL;
	g_activeTracers = 0;
	for (i = 0; i < 1000 && !failed; i++) {
		shape = CreateShape(3, 1, 0xc, &indices);
		if (shape) {
			g_tracerCount++;
			color = (MechU16) i | 0x8000;
			AddShapeVertex(shape, 0, 0, 0, 0, 0);
			AddShapeVertex(shape, 0, 100, 0, 100, 0);
			AddShapeVertex(shape, 100, 0, 0, 0, 100);
			face = AddShapeFace(shape, color, (MechU8*) indices);
			AddShapeFaceIndex(shape, face, 0);
			AddShapeFaceIndex(shape, face, 1);
			AddShapeFaceIndex(shape, face, 2);
			obj = CreateObj(NULL, 0xc);
			SetObjShape(obj, shape);
			SetShapeObject(shape, obj);
			SetShapeKind(shape, 0x60);
			ComputeNormalsAndBounds(shape);
			SetObjPosition(obj, 0, 0, 0);
			UpdateObjTree(obj);
			AddSceneShape(shape);
			HideObjTree(obj);
			DisableObjTreeCollision(obj);
			g_freeTracers[i] = i;
			g_tracers[i].m_index = i;
			g_tracers[i].m_kind = 0;
			g_tracers[i].m_obj = obj;
			g_tracers[i].m_update = NULL;
			g_tracers[i].m_draw = NULL;
			g_tracers[i].m_prev = NULL;
			g_tracers[i].m_next = NULL;
			g_tracers[i].m_data = NULL;
			for (j = 0; j < 32; j++) {
				g_tracers[i].m_points[j].m_x = g_tracers[i].m_points[j].m_y = g_tracers[i].m_points[j].m_z = 0;
				g_tracers[i].m_points[j].m_shown = 0;
			}
		}
		else {
			failed = 1;
		}
	}

	g_freeTracerTop = g_tracerCount - 1;
	g_trailColors[0] = 0x15;
	g_trailColors[1] = 0x15;
	for (i = 0; i < 8; i++) {
		g_trailColors[2 + i * 2] = '_' - i;
		g_trailColors[3 + i * 2] = '_' - i;
		g_sparkColors[i] = 0xde - i;
	}
}

// Starts an effect with its update and draw handlers. Returns it, or NULL.
// FUNCTION: MW2PREMERCS 0x100885ca
Tracer* StartTracer(TracerFn p_update, TracerFn p_draw, void* p_data)
{
	Tracer* tracer;

	tracer = NULL;
	tracer = AllocTracer();
	if (tracer) {
		tracer->m_update = p_update;
		tracer->m_draw = p_draw;
		tracer->m_data = p_data;
	}

	return tracer;
}

// Stops effect p_index.
// FUNCTION: MW2PREMERCS 0x1008861d
void StopTracer(MechS32 p_index)
{
	Tracer* tracer;

	tracer = NULL;
	if (p_index <= 0 || p_index >= g_tracerCount) {
		return;
	}

	tracer = &g_tracers[p_index];
	tracer->m_data = NULL;
	FreeTracer(p_index);
}

// Enables the effects (p_enabled; -1 asks), stopping every running one when they're disabled.
// Returns whether they're enabled.
// FUNCTION: MW2PREMERCS 0x10088680
MechS32 SetTracersEnabled(MechS32 p_enabled)
{
	Tracer* tracer;
	MechS32 i;

	if (p_enabled == -1) {
		return g_tracersEnabled;
	}

	if (p_enabled == 0 && g_tracerHead) {
		g_runningTracerCount = 0;
		for (tracer = g_tracerHead; tracer; tracer = tracer->m_next) {
			g_runningTracers[g_runningTracerCount] = tracer->m_index;
			g_runningTracerCount++;
		}

		for (i = 0; i < g_runningTracerCount; i++) {
			tracer = &g_tracers[g_runningTracers[i]];
			tracer->m_flags &= ~1;
			HideObjTree(tracer->m_obj);
			FreeTracer(tracer->m_index);
		}

		g_activeTracers = 0;
	}

	g_tracersEnabled = p_enabled;
	sprintf(g_tracerText, "pe: %x\n", g_tracersEnabled);

	return p_enabled;
}

// Runs each running effect's update handler, stopping those it ends.
// FUNCTION: MW2PREMERCS 0x100887a8
void UpdateTracers(void)
{
	Tracer* tracer;
	MechS32 i;
	MechS32 stopped;

	stopped = 0;
	if (!g_tracerHead || !g_deltaTime) {
		return;
	}

	g_runningTracerCount = 0;
	for (tracer = g_tracerHead; tracer; tracer = tracer->m_next) {
		g_runningTracers[g_runningTracerCount] = tracer->m_index;
		g_runningTracerCount++;
	}

	for (i = 0; i < g_runningTracerCount; i++) {
		tracer = &g_tracers[g_runningTracers[i]];
		if (tracer->m_update && !tracer->m_update(tracer)) {
			stopped++;
			FreeTracer(tracer->m_index);
		}
	}

	g_activeTracers -= stopped;
}

// Draws effect p_index, if it shows (DrawScenePolygon, for its shape's face).
// FUNCTION: MW2PREMERCS 0x100888ab
void DrawTracer(MechU32 p_index)
{
	Tracer* tracer;

	if (!g_activeTracers || p_index >= (MechU32) g_tracerCount || !(g_tracers[p_index].m_flags & 1)) {
		return;
	}

	tracer = &g_tracers[p_index];
	tracer->m_draw(tracer);
}

// A missile trail drifts with its speed, its color fading over 90 ticks, then it ends.
// FUNCTION: MW2PREMERCS 0x10088921
MechS32 UpdateMissileTrail(Tracer* p_tracer)
{
	MechS32 age;

	age = g_currentClock - p_tracer->m_startTime;
	if (age <= 90) {
		MoveObj(p_tracer->m_obj, p_tracer->m_dx, p_tracer->m_dy + RandomIntBelow(10), p_tracer->m_dz);
		UpdateObj(p_tracer->m_obj);
		p_tracer->m_color = g_trailColors[(MechS32) ((MechFloat) age / 90.0f * 17.0f)];
		return TRUE;
	}
	else {
		p_tracer->m_flags &= ~1;
		HideObjTree(p_tracer->m_obj);
		return FALSE;
	}
}

// Draws a missile trail's sixteen points as streaks along its speed, or as pixels beyond 20000.
// FUNCTION: MW2PREMERCS 0x100889fe
MechS32 DrawMissileTrail(Tracer* p_tracer)
{
	MechScalar x;
	MechScalar dx;
	MechScalar dy;
	MechScalar endX;
	MechScalar sz;
	MechScalar sy;
	MechS32 i;
	MechScalar sx;
	MechS32 color;
	MechScalar z;
	MechScalar endZ;
	MechScalar y;
	MechScalar dz;
	MechScalar endY;

	if (!p_tracer) {
		return 0;
	}

	GetObjPosition(p_tracer->m_obj, &x, &y, &z);
	dx = p_tracer->m_dx / 10;
	dy = p_tracer->m_dy / 10;
	dz = p_tracer->m_dz / 10;
	color = p_tracer->m_color;
	sx = 0;
	sz = z;
	ProjectWorldPoint(&sx, &sx, &sz);
	if (sz <= 20000) {
		for (i = 0; i < 16; i++) {
			sx = p_tracer->m_points[i].m_x + x;
			sy = p_tracer->m_points[i].m_y + y;
			sz = p_tracer->m_points[i].m_z + z;
			if (ProjectWorldPoint(&sx, &sy, &sz)) {
				endX = p_tracer->m_points[i].m_x + dx + x;
				endY = p_tracer->m_points[i].m_y + dy + y;
				endZ = p_tracer->m_points[i].m_z + z + dz;
				ProjectWorldPoint(&endX, &endY, &endZ);
				VFX_line_draw(&g_currentPane, sx, sy, endX, endY, 0, color);
			}
		}
	}
	else {
		for (i = 0; i < 16; i++) {
			sx = p_tracer->m_points[i].m_x + x;
			sy = p_tracer->m_points[i].m_y + y;
			sz = p_tracer->m_points[i].m_z + z;
			if (ProjectWorldPoint(&sx, &sy, &sz)) {
				VFX_pixel_write(&g_currentPane, sx, sy, color);
			}
		}
	}

	return 1;
}

// Starts a missile trail at (p_x, p_y, p_z) moving by (p_dx, p_dy, p_dz), if it's within the
// view's far plane: sixteen points scattered along the movement.
// FUNCTION: MW2PREMERCS 0x10088c5c
void StartMissileTrail(
	MechScalar p_x,
	MechScalar p_y,
	MechScalar p_z,
	MechScalar p_dx,
	MechScalar p_dy,
	MechScalar p_dz
)
{
	Shape* shape;
	Tracer* tracer;
	TracerPoint* point;
	MechS32 i;
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechScalar stepX;
	MechScalar stepY;
	MechScalar stepZ;

	if (g_activeTracers + 1 >= g_tracerCount) {
		return;
	}

	x = p_x - g_eyepoint->m_x;
	y = p_y - g_eyepoint->m_y;
	z = p_z - g_eyepoint->m_z;
	if (ApproximateVectorLength(x, y, z) > g_eyepoint->m_farPlane) {
		return;
	}

	tracer = StartTracer(UpdateMissileTrail, DrawMissileTrail, NULL);
	if (!tracer) {
		return;
	}

	tracer->m_flags = 1;
	shape = tracer->m_obj->m_shape;
	shape->m_radius = ApproximateVectorLength(p_dx, p_dy, p_dz);
	shape->m_centerX = p_x;
	shape->m_centerY = p_y;
	shape->m_centerZ = p_z;
	shape->m_modelCenterX = 0;
	shape->m_modelCenterY = 0;
	shape->m_modelCenterZ = 0;
	tracer->m_dx = p_dx >> 3;
	tracer->m_dy = p_dy >> 3;
	tracer->m_dz = p_dz >> 3;
	x = stepX = p_dx >> 4;
	y = stepY = p_dy >> 4;
	z = stepZ = p_dz >> 4;
	ShowObjTree(tracer->m_obj);
	SetObjPosition(tracer->m_obj, p_x, p_y, p_z);
	UpdateObj(tracer->m_obj);
	g_activeTracers++;
	tracer->m_startTime = g_currentClock;

	for (i = 0; i < 16; i++) {
		point = &tracer->m_points[i];
		point->m_shown = 1;
		x += stepX;
		y += stepY;
		z += stepZ;
		point->m_x = x + RandomIntBelow(10) - 5;
		point->m_y = y + RandomIntBelow(50) - 25;
		point->m_z = z + RandomIntBelow(10) - 5;
	}

	tracer->m_kind = 1;
}

// Sparks drift with their speed, their color fading over 45 ticks, then they end.
// FUNCTION: MW2PREMERCS 0x10088eb5
MechS32 UpdateSparks(Tracer* p_tracer)
{
	MechS32 age;

	age = g_currentClock - p_tracer->m_startTime;
	if (age <= 45) {
		MoveObj(p_tracer->m_obj, p_tracer->m_dx, p_tracer->m_dy + RandomIntBelow(10), p_tracer->m_dz);
		UpdateObj(p_tracer->m_obj);
		p_tracer->m_color = g_sparkColors[(MechS32) ((MechFloat) age / 45.0f * 7.0f)];
		return TRUE;
	}
	else {
		p_tracer->m_flags &= ~1;
		HideObjTree(p_tracer->m_obj);
		return FALSE;
	}
}

// Draws sparks' sixteen points as pixels, every other one beyond 10000.
// FUNCTION: MW2PREMERCS 0x10088f92
MechS32 DrawSparks(Tracer* p_tracer)
{
	MechScalar x;
	MechScalar sz;
	MechScalar sy;
	MechS32 i;
	MechScalar sx;
	MechS32 color;
	MechScalar z;
	MechScalar y;

	if (!p_tracer) {
		return 0;
	}

	GetObjPosition(p_tracer->m_obj, &x, &y, &z);
	sx = 0;
	sz = z;
	ProjectWorldPoint(&sx, &sx, &sz);
	color = p_tracer->m_color;
	if (sz <= 10000) {
		for (i = 0; i < 16; i++) {
			sx = p_tracer->m_points[i].m_x + x;
			sy = p_tracer->m_points[i].m_y + y;
			sz = p_tracer->m_points[i].m_z + z;
			if (ProjectWorldPoint(&sx, &sy, &sz)) {
				VFX_pixel_write(&g_currentPane, sx, sy, color);
			}
		}
	}
	else {
		for (i = 0; i < 16; i += 2) {
			sx = p_tracer->m_points[i].m_x + x;
			sy = p_tracer->m_points[i].m_y + y;
			sz = p_tracer->m_points[i].m_z + z;
			if (ProjectWorldPoint(&sx, &sy, &sz)) {
				VFX_pixel_write(&g_currentPane, sx, sy, color);
			}
		}
	}

	return 1;
}

// Starts sparks at (p_x, p_y, p_z) moving by half (p_dx, p_dy, p_dz), if they're within the view's
// far plane (UpdateDebrisPiece).
// FUNCTION: MW2PREMERCS 0x10089149
void StartSparks(MechScalar p_x, MechScalar p_y, MechScalar p_z, MechScalar p_dx, MechScalar p_dy, MechScalar p_dz)
{
	Shape* shape;
	Tracer* tracer;
	TracerPoint* point;
	MechS32 i;
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechScalar stepX;
	MechScalar stepY;
	MechScalar stepZ;

	if (g_activeTracers + 1 >= g_tracerCount) {
		return;
	}

	x = p_x - g_eyepoint->m_x;
	y = p_y - g_eyepoint->m_y;
	z = p_z - g_eyepoint->m_z;
	if (ApproximateVectorLength(x, y, z) > g_eyepoint->m_farPlane) {
		return;
	}

	tracer = StartTracer(UpdateSparks, DrawSparks, NULL);
	if (!tracer) {
		return;
	}

	tracer->m_flags = 1;
	shape = tracer->m_obj->m_shape;
	shape->m_radius = ApproximateVectorLength(p_dx, p_dy, p_dz);
	shape->m_centerX = p_x;
	shape->m_centerY = p_y;
	shape->m_centerZ = p_z;
	shape->m_modelCenterX = 0;
	shape->m_modelCenterY = 0;
	shape->m_modelCenterZ = 0;
	tracer->m_dx = p_dx / 2;
	tracer->m_dy = p_dy / 2;
	tracer->m_dz = p_dz / 2;
	x = stepX = p_dx / 16;
	y = stepY = p_dy / 16;
	z = stepZ = p_dz / 16;
	ShowObjTree(tracer->m_obj);
	SetObjPosition(tracer->m_obj, p_x, p_y, p_z);
	UpdateObj(tracer->m_obj);
	g_activeTracers++;
	tracer->m_startTime = g_currentClock;

	for (i = 0; i < 16; i++) {
		point = &tracer->m_points[i];
		point->m_shown = 1;
		x += stepX;
		y += stepY;
		z += stepZ;
		point->m_x = x + RandomIntBelow(4) - 2;
		point->m_y = y + RandomIntBelow(50) - 25;
		point->m_z = z + RandomIntBelow(4) - 2;
	}

	tracer->m_kind = 2;
}

// A gun tracer moves with its shot for 45 ticks, in color 3, then it ends.
// FUNCTION: MW2PREMERCS 0x100893ba
MechS32 UpdateGunTracer(Tracer* p_tracer)
{
	MechS32 age;

	age = g_currentClock - p_tracer->m_startTime;
	if (age <= 45) {
		MoveObj(p_tracer->m_obj, p_tracer->m_dx, p_tracer->m_dy, p_tracer->m_dz);
		UpdateObj(p_tracer->m_obj);
		p_tracer->m_color = 3;
		return TRUE;
	}
	else {
		p_tracer->m_flags &= ~1;
		HideObjTree(p_tracer->m_obj);
		return FALSE;
	}
}

// Draws a gun tracer's ten points as pixels, every other one beyond 10000.
// FUNCTION: MW2PREMERCS 0x1008946d
MechS32 DrawGunTracer(Tracer* p_tracer)
{
	MechScalar x;
	MechScalar sz;
	MechScalar sy;
	MechS32 i;
	MechScalar sx;
	MechS32 color;
	MechScalar z;
	MechScalar y;

	if (!p_tracer) {
		return 0;
	}

	GetObjPosition(p_tracer->m_obj, &x, &y, &z);
	sx = 0;
	sz = z;
	ProjectWorldPoint(&sx, &sx, &sz);
	color = p_tracer->m_color;
	if (sz <= 10000) {
		for (i = 0; i < 10; i++) {
			sx = p_tracer->m_points[i].m_x + x;
			sy = p_tracer->m_points[i].m_y + y;
			sz = p_tracer->m_points[i].m_z + z;
			if (ProjectWorldPoint(&sx, &sy, &sz)) {
				VFX_pixel_write(&g_currentPane, sx, sy, color);
			}
		}
	}
	else {
		for (i = 0; i < 10; i += 2) {
			sx = p_tracer->m_points[i].m_x + x;
			sy = p_tracer->m_points[i].m_y + y;
			sz = p_tracer->m_points[i].m_z + z;
			if (ProjectWorldPoint(&sx, &sy, &sz)) {
				VFX_pixel_write(&g_currentPane, sx, sy, color);
			}
		}
	}

	return 1;
}

// Starts a gun tracer at (p_x, p_y, p_z) moving with its shot (p_dx, p_dy, p_dz a tick): ten
// points along the shot's movement.
// FUNCTION: MW2PREMERCS 0x10089624
void StartGunTracer(MechScalar p_x, MechScalar p_y, MechScalar p_z, MechScalar p_dx, MechScalar p_dy, MechScalar p_dz)
{
	Shape* shape;
	Tracer* tracer;
	TracerPoint* point;
	MechS32 i;
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechScalar stepX;
	MechScalar stepY;
	MechScalar stepZ;

	if (g_activeTracers + 1 >= g_tracerCount) {
		return;
	}

	tracer = StartTracer(UpdateGunTracer, DrawGunTracer, NULL);
	if (!tracer) {
		return;
	}

	tracer->m_flags = 1;
	shape = tracer->m_obj->m_shape;
	shape->m_radius = ApproximateVectorLength(p_dx * 100, p_dy * 100, p_dz * 100);
	shape->m_centerX = p_x;
	shape->m_centerY = p_y;
	shape->m_centerZ = p_z;
	shape->m_modelCenterX = 0;
	shape->m_modelCenterY = 0;
	shape->m_modelCenterZ = 0;
	tracer->m_dx = p_dx >> 4;
	tracer->m_dy = p_dy >> 4;
	tracer->m_dz = p_dz >> 4;
	x = stepX = p_dx / 10;
	y = stepY = p_dy / 10;
	z = stepZ = p_dz / 10;
	ShowObjTree(tracer->m_obj);
	SetObjPosition(tracer->m_obj, p_x, p_y, p_z);
	UpdateObj(tracer->m_obj);
	g_activeTracers++;
	tracer->m_startTime = g_currentClock;

	for (i = 0; i < 10; i++) {
		point = &tracer->m_points[i];
		point->m_shown = 1;
		x += stepX;
		y += stepY;
		z += stepZ;
		point->m_x = x;
		point->m_y = y;
		point->m_z = z;
	}

	tracer->m_kind = 3;
}
