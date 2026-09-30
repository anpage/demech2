#include "unk10046750.h"

#include "ambientsound.h"
#include "callbacks.h"
#include "config.h"
#include "decomp.h"
#include "fixeddiv.h"
#include "fixedmul.h"
#include "geocache.h"
#include "object.h"
#include "quartzreel.h"
#include "resource.h"
#include "simmain.h"
#include "soundfx.h"
#include "types.h"
#include "unk1003a530.h"
#include "unk100696c0.h"
#include "unk100737e0.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// The state of FUN_1004748c's callback: the faces of a shape cycle through up to sixteen colors.
// SIZE 0x50
typedef struct JadeCycle0x50 {
	ScarletOrchid0x4c** m_shape; // 0x00 — the star's shape (FUN_1001f873)
	ScarletOrchid0x4c* m_model;  // 0x04
	MechS32 m_faceCount;         // 0x08 — -1 until counted
	MechS32 m_colorCount;        // 0x0c
	MechS32 m_colors[16];        // 0x10
} JadeCycle0x50;

// The state of FUN_1004771e's callback: an object turning at constant rates.
// SIZE 0x1c
typedef struct JadeSpin0x1c {
	ScarletOrchid0x4c** m_shape; // 0x00
	AmberWillow0x7c* m_object;   // 0x04
	MechS32 m_rateX;             // 0x08 — 16.16 per period
	MechS32 m_rateY;             // 0x0c
	MechS32 m_rateZ;             // 0x10
	MechS32 m_period;            // 0x14 — clock ticks
	MechS32 m_lastClock;         // 0x18
} JadeSpin0x1c;

// The state of FUN_100479ec's callback: an object circling while it turns.
// SIZE 0x1c
typedef struct JadeOrbit0x1c {
	ScarletOrchid0x4c** m_shape; // 0x00
	AmberWillow0x7c* m_object;   // 0x04
	MechS32 m_angle;             // 0x08 — 16.16 degrees
	MechS32 m_turnRate;          // 0x0c
	MechS32 m_speed;             // 0x10
	MechS32 m_enabled;           // 0x14
	MechS32 m_lastClock;         // 0x18
} JadeOrbit0x1c;

// An animation file LoadAnimFile has loaded: its id and the base of its animation numbers.
// SIZE 0x8
typedef struct MossLedger0x8 {
	MechS32 m_id;   // 0x00
	MechS32 m_base; // 0x04
} MossLedger0x8;

DECOMP_SIZE_ASSERT(AmbientSound, 0x1e)
DECOMP_SIZE_ASSERT(PathPoint, 0x1c)
DECOMP_SIZE_ASSERT(Path, 0x744)
DECOMP_SIZE_ASSERT(JadeCycle0x50, 0x50)
DECOMP_SIZE_ASSERT(JadeSpin0x1c, 0x1c)
DECOMP_SIZE_ASSERT(JadeOrbit0x1c, 0x1c)
DECOMP_SIZE_ASSERT(MossLedger0x8, 0x8)

// The number of entries in g_paths.
// GLOBAL: MW2 0x100a6d68
MechS32 g_pathCount = 0;

// GLOBAL: MW2 0x100a6d6c
MechS32 g_unk0x100a6d6c = 0;

// The base of the animation numbers of the file being loaded.
// GLOBAL: MW2 0x100a6d70
MechS32 g_unk0x100a6d70 = 0;

// The number of entries in g_unk0x101097e0.
// GLOBAL: MW2 0x100a6d74
MechS32 g_unk0x100a6d74 = 0;

// GLOBAL: MW2 0x100ea8e0
Path g_paths[0x40];

// The animations of the loaded animation files, by number.
// GLOBAL: MW2 0x101079e0
QuartzReel0x14* g_unk0x101079e0[0x780];

// GLOBAL: MW2 0x101097e0
MossLedger0x8 g_unk0x101097e0[60];

// STUB: MW2 0x10046750
void FUN_10046750(void)
{
	STUB(0x10046750);
}

// Scales p_value by mode p_mode: 1 by 1.5, 3 by 0.75, any other mode leaves it.
// FUNCTION: MW2 0x100472fe
MechS32 FUN_100472fe(MechS32 p_mode, MechS32 p_value)
{
	MechS32 result;

	switch (p_mode) {
	case 1:
		result = (p_value >> 1) + p_value;
		break;
	case 2:
		result = p_value;
		break;
	case 3:
		result = p_value - (p_value >> 2);
		break;
	default:
		result = p_value;
		break;
	}

	return result;
}

// Loads the animation file p_ref unless it has been, and sets the base of its animation numbers
// (g_unk0x100a6d70). Returns -1 if it was already loaded.
// FUNCTION: MW2 0x10047380
MechS32 LoadAnimFile(ResourceRef* p_ref)
{
	MechS32 result;
	MechS32 i;
	MechS32 id;

	result = 0;
	id = p_ref->m_id;
	for (i = 0; i < g_unk0x100a6d74 && g_unk0x101097e0[i].m_id != id; i++) {
	}

	if (i < g_unk0x100a6d74) {
		g_unk0x100a6d70 = g_unk0x101097e0[i].m_base;
		result = -1;
	}
	else if (g_unk0x100a6d74 < 60) {
		if (g_unk0x100a6d6c > 0) {
			g_unk0x100a6d70 = g_unk0x100a6d6c + 1;
		}

		result = FUN_100708f4(p_ref);
		g_unk0x101097e0[g_unk0x100a6d74].m_base = g_unk0x100a6d70;
		g_unk0x101097e0[g_unk0x100a6d74].m_id = id;
		g_unk0x100a6d74++;
	}

	return result;
}

// FUNCTION: MW2 0x10047462
MechS32 FUN_10047462(void)
{
	return g_unk0x100a6d70;
}

// FUNCTION: MW2 0x10047477
MechS32 FUN_10047477(void)
{
	return 0x2c;
}

// A timed callback (TimedCallbackFn) cycling the face colors of a star's shape. Its data is
// "<star id>;<color>,<color>,...": up to sixteen colors, each a palette row.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1004748c
MechS32 FUN_1004748c(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period)
{
	MechChar* token;
	JadeCycle0x50* cycle;
	MechS32 vertexCount;
	MechS32 offset;
	MechS32 i;
	MechS32 star;
	MechS32 id;
	void** slot;

	if (!p_period) {
		p_period = 1;
	}

	switch (p_event) {
	case 0:
		cycle = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(JadeCycle0x50));
		if (!cycle) {
			return 0;
		}

		slot = FUN_1007d51f(FUN_1007d2e0());
		*slot = cycle;
		cycle->m_colorCount = 0;
		cycle->m_model = NULL;
		cycle->m_faceCount = -1;
		token = strchr(p_data, ';');
		if (token) {
			*token = '\0';
			token++;
			for (token = strtok(token, ","); token; token = strtok(NULL, ",")) {
				if (cycle->m_colorCount < 16) {
					cycle->m_colors[cycle->m_colorCount] = strtoul(token, NULL, 0) << 4;
					cycle->m_colorCount++;
				}
				else {
					break;
				}
			}
		}

		id = atoi(p_data);
		id = MapResourceId(id);
		star = FindStarIdxById(id);
		if (star != -1) {
			cycle->m_shape = FUN_1001f873(star);
		}
		else {
			return 0;
		}
		break;
	case 1:
		slot = FUN_1007d51f(FUN_1007d2e0());
		cycle = *slot;
		if (!cycle) {
			return 0;
		}
		else if (!cycle->m_shape) {
			return 0;
		}

		if (!*cycle->m_shape) {
			return 0;
		}

		cycle->m_model = *cycle->m_shape;
		if (cycle->m_faceCount == -1) {
			FUN_1003b43d(cycle->m_model, &vertexCount, &cycle->m_faceCount);
		}

		offset = p_clock / p_period % cycle->m_colorCount;
		for (i = 0; i < cycle->m_faceCount; i++) {
			FUN_1003b696(cycle->m_model, i, cycle->m_colors[(offset + i) % cycle->m_colorCount]);
		}
		break;
	default:
		break;
	}

	return 1;
}

// A timed callback (TimedCallbackFn) turning a star's object. Its data is
// "<star id>;<x>,<y>,<z>,<period>": the turns in degrees per period of seconds.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1004771e
MechS32 FUN_1004771e(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period)
{
	MechS32 dz;
	MechChar* token;
	JadeSpin0x1c* spin;
	MechFloat x;
	MechFloat y;
	MechFloat z;
	MechFloat period;
	MechS32 star;
	MechS32 t;
	MechS32 id;
	MechS32 dx;
	void** slot;
	MechS32 dy;

	switch (p_event) {
	case 0:
		spin = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(JadeSpin0x1c));
		if (!spin) {
			return 0;
		}

		slot = FUN_1007d51f(FUN_1007d2e0());
		*slot = spin;
		spin->m_rateX = spin->m_rateY = spin->m_rateZ = 0;
		spin->m_object = NULL;
		token = strchr(p_data, ';');
		if (token) {
			*token = '\0';
			token++;
			sscanf(token, "%f,%f,%f,%f", &x, &y, &z, &period);
			spin->m_rateX = x * 65536.0 + 0.5;
			spin->m_rateY = y * 65536.0 + 0.5;
			spin->m_rateZ = z * 65536.0 + 0.5;
			spin->m_period = period * 181.0f;
			if (!spin->m_period) {
				spin->m_period = 181;
			}

			spin->m_lastClock = p_clock;
		}

		id = atoi(p_data);
		id = MapResourceId(id);
		star = FindStarIdxById(id);
		if (star != -1) {
			spin->m_shape = FUN_1001f873(star);
		}
		else {
			return 0;
		}
		break;
	case 1:
		slot = FUN_1007d51f(FUN_1007d2e0());
		spin = *slot;
		if (!spin) {
			return 0;
		}
		else if (!spin->m_shape) {
			return 0;
		}

		if (!*spin->m_shape) {
			return 0;
		}

		spin->m_object = FUN_1003b6e5(*spin->m_shape);
		t = ((p_clock - spin->m_lastClock) << 16) / spin->m_period;
		dx = FixedMul16(spin->m_rateX, t);
		dy = FixedMul16(spin->m_rateY, t);
		dz = FixedMul16(spin->m_rateZ, t);
		spin->m_lastClock = p_clock;
		FUN_1000184b(spin->m_object, dx, dy, dz, 0);
		FUN_10001cf8(spin->m_object);
		break;
	default:
		break;
	}

	return 1;
}

// A timed callback (TimedCallbackFn) moving a star's object around a circle. Its data is
// "<star id>;<radius>,<period>,<enabled>,<unused>".
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x100479ec
MechS32 FUN_100479ec(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period)
{
	MechChar* token;
	MechS32 z;
	MechS32 cosine;
	JadeOrbit0x1c* orbit;
	MechS32 t;
	MechS32 step;
	MechS32 star;
	MechS32 sine;
	MechS32 enabled;
	MechFloat radius;
	MechS32 unused;
	MechFloat period;
	MechS32 id;
	void** slot;
	MechS32 x;

	switch (p_event) {
	case 0:
		orbit = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(JadeOrbit0x1c));
		if (!orbit) {
			return 0;
		}

		slot = FUN_1007d51f(FUN_1007d2e0());
		*slot = orbit;
		orbit->m_enabled = 0;
		orbit->m_object = NULL;
		token = strchr(p_data, ';');
		if (token) {
			*token = '\0';
			token++;
			sscanf(token, "%f,%f,%d,%d", &radius, &period, &enabled, &unused);
			orbit->m_turnRate = 360.0 / period * 65536.0 + 0.5;
			orbit->m_speed = radius * 6.283185307179586 / period * 65536.0 + 0.5;
			orbit->m_enabled = enabled;
			orbit->m_angle = 0;
		}

		orbit->m_lastClock = p_clock;
		id = atoi(p_data);
		id = MapResourceId(id);
		star = FindStarIdxById(id);
		if (star != -1) {
			orbit->m_shape = FUN_1001f873(star);
		}
		else {
			return 0;
		}
		break;
	case 1:
		slot = FUN_1007d51f(FUN_1007d2e0());
		orbit = *slot;
		if (!orbit) {
			return 0;
		}
		else if (!orbit->m_shape || !orbit->m_enabled) {
			return 0;
		}

		if (!*orbit->m_shape) {
			return 0;
		}

		orbit->m_object = FUN_1003b6e5(*orbit->m_shape);
		cosine = FUN_1006973a(orbit->m_angle);
		sine = FUN_100696c0(orbit->m_angle);
		t = FixedDiv16(p_clock - orbit->m_lastClock, 181);
		orbit->m_lastClock = p_clock;
		step = FixedMul16(orbit->m_speed, t) >> 16;
		x = FixedMul16(cosine, step) >> 13;
		z = FixedMul16(-sine, step) >> 13;
		FUN_10001667(orbit->m_object, x, 0, z);
		FUN_10001cf8(orbit->m_object);
		step = FixedMul16(orbit->m_turnRate, t);
		FUN_1000184b(orbit->m_object, 0, step, 0, 0);
		FUN_10001cf8(orbit->m_object);
		orbit->m_angle += step;
		orbit->m_angle %= 0x1680000;
		break;
	default:
		break;
	}

	return 1;
}

// A timed callback (TimedCallbackFn) looping a sound on a star's object. Its data is
// "<star id>;<range>,<sound name>,<unk0x14>".
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10047d10
MechS32 FUN_10047d10(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period)
{
	MechChar* token;
	AmbientSound* sound;
	MechS32 star;
	MechS32 id;
	void** slot;
	MechChar name[100];

	switch (p_event) {
	case 0:
		sound = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(AmbientSound));
		if (!sound) {
			return 0;
		}

		slot = FUN_1007d51f(FUN_1007d2e0());
		*slot = sound;
		sound->m_slot = -1;
		sound->m_data = NULL;
		sound->m_skip = 1;
		token = strchr(p_data, ';');
		if (token) {
			*token = '\0';
			token++;
			sscanf(token, "%d,%[^,],%d", &sound->m_range, name, &sound->m_unk0x14);
			sound->m_range *= 100;
		}

		sound->m_id = FindResourceIdByName(0xb, name);
		id = atoi(p_data);
		id = MapResourceId(id);
		star = FindStarIdxById(id);
		if (star != -1) {
			sound->m_unk0x0c = FUN_1001f873(star);
		}
		else {
			return 0;
		}
		break;
	case 1:
		slot = FUN_1007d51f(FUN_1007d2e0());
		sound = *slot;
		if (!sound) {
			return 0;
		}
		else if (!sound->m_unk0x0c || !sound->m_unk0x14) {
			StopAmbientSound(sound);
			return 0;
		}

		if (!*sound->m_unk0x0c) {
			StopAmbientSound(sound);
			return 0;
		}

		sound->m_obj = FUN_1003b6e5(*sound->m_unk0x0c);
		UpdateAmbientSound(sound);
		break;
	case 2:
		slot = FUN_1007d51f(FUN_1007d2e0());
		sound = *slot;
		if (!sound) {
			return 1;
		}

		StopAmbientSound(sound);
		break;
	}

	return 1;
}

// Queues a face of a model for drawing, unless it faces away: projects the vertices it hasn't
// yet, clips it to the near plane through the filter hooks (g_unk0x100a6cc8) and adds the
// polygon to the list being built (g_unk0x1010b5c4) with its depth.
// STUB: MW2 0x10049155
void FUN_10049155(DuskMoth0x24* p_face, EmberFern0x2c* p_vertices)
{
	STUB(0x10049155);
}
