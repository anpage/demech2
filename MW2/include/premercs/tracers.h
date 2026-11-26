#ifndef PREMERCS_TRACERS_H
#define PREMERCS_TRACERS_H

#include "decomp.h"
#include "fixedfloat.h"
#include "types.h"

struct SceneObject;
typedef struct Tracer Tracer;

// A handler of an effect: its update (returning whether it goes on) or its drawing.
typedef MechS32 (*TracerFn)(Tracer* p_tracer);

#pragma pack(push, 1)

// One of an effect's points, relative to its object.
// SIZE 0xe
typedef struct TracerPoint {
	MechScalar m_x;  // 0x00
	MechScalar m_y;  // 0x04
	MechScalar m_z;  // 0x08
	MechS16 m_shown; // 0x0c
} TracerPoint;

// One of the effects g_tracers holds.
// SIZE 0x1fa
struct Tracer {
	Tracer* m_prev;            // 0x00 — in the running list
	Tracer* m_next;            // 0x04
	MechS32 m_index;           // 0x08 — in g_tracers
	MechS32 m_kind;            // 0x0c — 1 a missile trail, 2 sparks, 3 a gun tracer
	MechU16 m_flags;           // 0x10 — bit 0: shown
	TracerPoint m_points[32];  // 0x12
	struct SceneObject* m_obj; // 0x1d2 — the effect's hidden shape's object
	TracerFn m_update;         // 0x1d6
	TracerFn m_draw;           // 0x1da
	MechS32 m_startTime;       // 0x1de — the clock when it started
	undefined4 m_unk0x1e2;     // 0x1e2
	MechS32 m_color;           // 0x1e6
	MechScalar m_dx;           // 0x1ea — its movement a tick
	MechScalar m_dy;           // 0x1ee
	MechScalar m_dz;           // 0x1f2
	void* m_data;              // 0x1f6 — its owner's data (a PrecipitationDrop)
};

#pragma pack(pop)

// The functions and globals of tracers.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_tracerCount;
	extern MechS32 g_activeTracers;
	extern Tracer* g_tracerHead;
	extern Tracer* g_tracerTail;
	extern MechS32 g_tracersEnabled;
	extern Tracer g_tracers[1000];

	void LinkTracer(Tracer* p_tracer);
	void UnlinkTracer(Tracer* p_tracer);
	Tracer* FreeTracer(MechS32 p_index);
	Tracer* AllocTracer(void);
	void InitTracers(void);
	Tracer* StartTracer(TracerFn p_update, TracerFn p_draw, void* p_data);
	void StopTracer(MechS32 p_index);
	MechS32 SetTracersEnabled(MechS32 p_enabled);
	void UpdateTracers(void);
	void DrawTracer(MechU32 p_index);
	MechS32 UpdateMissileTrail(Tracer* p_tracer);
	MechS32 DrawMissileTrail(Tracer* p_tracer);
	void StartMissileTrail(
		MechScalar p_x,
		MechScalar p_y,
		MechScalar p_z,
		MechScalar p_dx,
		MechScalar p_dy,
		MechScalar p_dz
	);
	MechS32 UpdateSparks(Tracer* p_tracer);
	MechS32 DrawSparks(Tracer* p_tracer);
	void StartSparks(MechScalar p_x, MechScalar p_y, MechScalar p_z, MechScalar p_dx, MechScalar p_dy, MechScalar p_dz);
	MechS32 UpdateGunTracer(Tracer* p_tracer);
	MechS32 DrawGunTracer(Tracer* p_tracer);
	void StartGunTracer(
		MechScalar p_x,
		MechScalar p_y,
		MechScalar p_z,
		MechScalar p_dx,
		MechScalar p_dy,
		MechScalar p_dz
	);

#ifdef __cplusplus
}
#endif

#endif // PREMERCS_TRACERS_H
