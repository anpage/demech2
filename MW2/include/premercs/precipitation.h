#ifndef PREMERCS_PRECIPITATION_H
#define PREMERCS_PRECIPITATION_H

#include "decomp.h"
#include "fixedfloat.h"
#include "premercs/tracers.h"
#include "types.h"

typedef struct PrecipitationZone PrecipitationZone;

// The kinds of precipitation (g_precipitationKind).
enum {
	c_precipitationRain = 1,
	c_precipitationSnow = 2
};

// What StartPrecipitation starts (BwdExecuteStream's weather record).
// SIZE 0x10
typedef struct PrecipitationSpec {
	MechS32 m_kind;                   // 0x00 — c_precipitationRain or c_precipitationSnow
	undefined m_unk0x04[0x10 - 0x04]; // 0x04
} PrecipitationSpec;

// One raindrop or snowflake of a zone, an effect of g_tracers while it falls.
// SIZE 0x24
typedef struct PrecipitationDrop {
	MechS32 m_index;           // 0x00 — its index in the zone's m_drops
	MechS32 m_tracer;          // 0x04 — its effect's index, -1 while free
	undefined m_unk0x08[0xc];  // 0x08
	MechScalar m_dx;           // 0x14 — its drift, added to the effect's speed
	MechScalar m_dy;           // 0x18
	MechScalar m_dz;           // 0x1c
	PrecipitationZone* m_zone; // 0x20
} PrecipitationDrop;

// One of the nine zones of a 3x3 grid around the eyepoint, each starting its drops at m_y.
// SIZE 0x730
struct PrecipitationZone {
	MechScalar m_x;                     // 0x00 — its offset from the eyepoint
	MechScalar m_y;                     // 0x04 — the height its drops start at
	MechScalar m_z;                     // 0x08
	undefined m_unk0x0c[0x20 - 0x0c];   // 0x0c
	PrecipitationDrop m_drops[45];      // 0x20
	MechS32 m_dropCount;                // 0x674
	MechS32 m_usedDrops;                // 0x678 — how many of m_freeDrops are taken
	MechS32 m_freeDrops[45];            // 0x67c — the free drops' indices, a stack
};

#ifdef __cplusplus
extern "C" {
#endif

	extern MechS32 g_precipitating;
	extern MechS32 g_precipitationKind;
	extern MechS32 g_snowShades[8];
	extern PrecipitationZone g_precipitationZones[9];

	PrecipitationDrop* AllocPrecipitationDrop(PrecipitationZone* p_zone);
	void FreePrecipitationDrop(PrecipitationZone* p_zone, MechS32 p_index);
	void InitPrecipitation(void);
	void UpdatePrecipitation(void);
	void StartPrecipitation(PrecipitationSpec* p_spec);
	void FUN_10089d9d(void);
	MechS32 UpdateRaindrop(Tracer* p_tracer);
	MechS32 DrawRaindrop(Tracer* p_tracer);
	void StartRaindrop(
		MechScalar p_x,
		MechScalar p_y,
		MechScalar p_z,
		MechScalar p_dx,
		MechScalar p_dy,
		MechScalar p_dz,
		PrecipitationZone* p_zone
	);
	MechS32 UpdateSnowflake(Tracer* p_tracer);
	MechS32 DrawSnowflake(Tracer* p_tracer);
	void StartSnowflake(
		MechScalar p_x,
		MechScalar p_y,
		MechScalar p_z,
		MechScalar p_dx,
		MechScalar p_dy,
		MechScalar p_dz,
		PrecipitationZone* p_zone
	);

#ifdef __cplusplus
}
#endif

#endif // PREMERCS_PRECIPITATION_H
