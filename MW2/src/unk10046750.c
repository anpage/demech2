/* Hand-written assembly: FUN_10048c50, FUN_10048d46, FUN_10048ebe and FUN_10048faf are C
   functions with __asm bodies. */
#include "unk10046750.h"

#include "ambientsound.h"
#include "callbacks.h"
#include "clock.h"
#include "compat.h"
#include "config.h"
#include "copperwren.h"
#include "decomp.h"
#include "duskmoth.h"
#include "emberfern.h"
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
#include "unk1004b980.h"
#include "unk100696c0.h"
#include "unk100737e0.h"
#include "unk1007d120.h"

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

// Set to shade from the origin rather than the light (FUN_10048faf).
// GLOBAL: MW2 0x1010b530
MechS32 g_unk0x1010b530;

// The outcodes of the polygon being built: any vertex's (or) and every vertex's (and).
// GLOBAL: MW2 0x1010b53c
MechU8 g_unk0x1010b53c;

// GLOBAL: MW2 0x1010b5b8
MechU8 g_unk0x1010b5b8;

// The projected vertices of the polygon being built, and their count.
// GLOBAL: MW2 0x1010b550
CopperWren0x20* g_unk0x1010b550[20];

// GLOBAL: MW2 0x1010b5b0
MechS32 g_unk0x1010b5b0;

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

// A timed callback (TimedCallbackFn).
// STUB: MW2 0x10047f60
MechS32 FUN_10047f60(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period)
{
	STUB(0x10047f60);
	return 0;
}

// Returns the vertex's projected copy (m_unk0x24), transforming it into view space
// (g_unk0x100ea864's rows, from the eyepoint g_unk0x100ea8b4) the first time. The transform is an
// __asm block.
// Stack-slot permutation of the locals the __asm blocks name.
// FUNCTION: MW2 0x10048c50
CopperWren0x20* FUN_10048c50(EmberFern0x2c* p_vertex)
{
#ifdef COMPAT_MODE
	STUB(0x10048c50);
	return NULL;
#else
	MechS32 u;
	MechS32 v;
	CopperWren0x20* result;
	MechS32 deltaX;
	MechS32 deltaY;
	MechS32 deltaZ;

	__asm {
		mov ebx, p_vertex
		mov eax, dword ptr [ebx + 0x24]
		mov result, eax
		or eax, eax
		je jmp_10048c72
	}

	return result;

jmp_10048c72:
	result = FUN_1007d248();
	__asm {
		mov ebx, p_vertex
		mov eax, dword ptr [ebx + 0xc]
		sub eax, dword ptr [g_unk0x100ea8b8]
		mov deltaX, eax
		mov eax, dword ptr [ebx + 0x10]
		sub eax, dword ptr [g_unk0x100ea8b4]
		mov deltaY, eax
		mov eax, dword ptr [ebx + 0x14]
		sub eax, dword ptr [g_unk0x100ea8bc]
		mov deltaZ, eax
		mov eax, dword ptr [ebx + 0x18]
		mov u, eax
		mov eax, dword ptr [ebx + 0x1c]
		mov v, eax
		mov eax, dword ptr [g_unk0x100ea864]
		mov edx, deltaX
		imul edx
		mov esi, eax
		mov edi, edx
		mov eax, dword ptr [g_unk0x100ea868]
		mov edx, deltaY
		imul edx
		add esi, eax
		adc edi, edx
		mov eax, dword ptr [g_unk0x100ea86c]
		mov edx, deltaZ
		imul edx
		add esi, eax
		adc edi, edx
		shrd esi, edi, 0x1b
		adc esi, 0
		mov ecx, esi
		mov eax, dword ptr [g_unk0x100ea870]
		mov edx, deltaX
		imul edx
		mov esi, eax
		mov edi, edx
		mov eax, dword ptr [g_unk0x100ea874]
		mov edx, deltaY
		imul edx
		add esi, eax
		adc edi, edx
		mov eax, dword ptr [g_unk0x100ea878]
		mov edx, deltaZ
		imul edx
		add esi, eax
		adc edi, edx
		shrd esi, edi, 0x1b
		adc esi, 0
		mov eax, esi
		mov esi, result
		mov dword ptr [ebx + 0x24], esi
		mov esi, dword ptr [ebx + 0x20]
		mov ebx, result
		mov dword ptr [ebx], ecx
		mov dword ptr [ebx + 4], eax
		mov dword ptr [ebx + 8], esi
		mov eax, u
		shl eax, 0x10
		mov dword ptr [ebx + 0x14], eax
		mov eax, v
		shl eax, 0x10
		mov dword ptr [ebx + 0x18], eax
	}

	return result;
#endif
}

// Returns a new projected vertex where the edge from p_a to p_b crosses the near plane
// (g_unk0x100ea820), its position and texture coordinates interpolated. The body is an __asm
// block.
// Stack-slot permutation of the locals the __asm block names.
// FUNCTION: MW2 0x10048d46
CopperWren0x20* FUN_10048d46(EmberFern0x2c* p_a, EmberFern0x2c* p_b)
{
	MechS32 z0;
	MechS32 z1;
	MechS32 u0;
	MechS32 u1;
	CopperWren0x20* a;
	CopperWren0x20* b;
	MechS32 v0;
	CopperWren0x20* result;
	MechS32 v1;
	MechS32 x0;
	MechS32 x1;
	MechS32 y0;
	MechS32 y1;

	__asm {
		mov ebx, p_a
		mov eax, dword ptr [ebx + 0x24]
		mov a, eax
		or eax, eax
		jne jmp_10048d6f
		mov eax, p_a
		push eax
		call FUN_10048c50
		add esp, 4
		mov a, eax
jmp_10048d6f:
		mov ebx, p_b
		mov eax, dword ptr [ebx + 0x24]
		mov b, eax
		or eax, eax
		jne jmp_10048d8f
		mov eax, p_b
		push eax
		call FUN_10048c50
		add esp, 4
		mov b, eax
jmp_10048d8f:
		call FUN_1007d248
		mov result, eax
		mov ebx, a
		mov eax, dword ptr [ebx]
		mov x0, eax
		mov eax, dword ptr [ebx + 4]
		mov y0, eax
		mov eax, dword ptr [ebx + 0x14]
		mov u0, eax
		mov eax, dword ptr [ebx + 0x18]
		mov v0, eax
		mov eax, dword ptr [ebx + 8]
		mov z0, eax
		mov ebx, b
		mov eax, dword ptr [ebx]
		mov x1, eax
		mov eax, dword ptr [ebx + 4]
		mov y1, eax
		mov eax, dword ptr [ebx + 0x14]
		mov u1, eax
		mov eax, dword ptr [ebx + 0x18]
		mov v1, eax
		mov eax, dword ptr [ebx + 8]
		mov z1, eax
		cmp eax, z0
		jg jmp_10048e3a
		mov ecx, z0
		sub ecx, z1
		je jmp_10048e35
		mov edi, dword ptr [g_unk0x100ea820]
		sub edi, z1
		mov eax, x0
		sub eax, x1
		imul edi
		idiv ecx
		add eax, x1
		mov x0, eax
		mov eax, y0
		sub eax, y1
		imul edi
		idiv ecx
		add eax, y1
		mov y0, eax
		mov eax, u0
		sub eax, u1
		imul edi
		idiv ecx
		add eax, u1
		mov u0, eax
		mov eax, v0
		sub eax, v1
		imul edi
		idiv ecx
		add eax, v1
		mov v0, eax
jmp_10048e35:
		jmp jmp_10048e8f
jmp_10048e3a:
		mov ecx, z1
		sub ecx, z0
		je jmp_10048e8f
		mov edi, dword ptr [g_unk0x100ea820]
		sub edi, z0
		mov eax, x1
		sub eax, x0
		imul edi
		idiv ecx
		add eax, x0
		mov x0, eax
		mov eax, y1
		sub eax, y0
		imul edi
		idiv ecx
		add eax, y0
		mov y0, eax
		mov eax, u1
		sub eax, u0
		imul edi
		idiv ecx
		add eax, u0
		mov u0, eax
		mov eax, v1
		sub eax, v0
		imul edi
		idiv ecx
		add eax, v0
		mov v0, eax
jmp_10048e8f:
		mov ebx, result
		mov eax, x0
		mov dword ptr [ebx], eax
		mov eax, y0
		mov dword ptr [ebx + 4], eax
		mov eax, dword ptr [g_unk0x100ea820]
		mov dword ptr [ebx + 8], eax
		mov eax, u0
		mov dword ptr [ebx + 0x14], eax
		mov eax, v0
		mov dword ptr [ebx + 0x18], eax
	}

	return result;
}

// Projects a view-space vertex onto the screen once per frame (m_unk0x1d), with its clip
// outcodes (m_unk0x1c: 1 left, 2 right, 4 top, 8 bottom), accumulates the outcodes of the
// polygon being built and adds the vertex to its list (up to 20). The body is an __asm block.
// FUNCTION: MW2 0x10048ebe
CopperWren0x20* FUN_10048ebe(CopperWren0x20* p_vertex)
{
#ifdef COMPAT_MODE
	STUB(0x10048ebe);
	return NULL;
#else
	__asm {
		mov ebx, p_vertex
		test byte ptr [ebx + 0x1d], 0xff
		je jmp_10048ed6
		jmp jmp_10048f61
jmp_10048ed6:
		xor ecx, ecx
		mov cl, byte ptr [g_unk0x100ea824]
		mov esi, dword ptr [ebx + 8]
		mov eax, dword ptr [ebx]
		cdq
		shld edx, eax, cl
		shl eax, cl
		idiv esi
		add eax, 2
		sar eax, 2
		add eax, dword ptr [g_unk0x100ea834]
		mov dword ptr [ebx + 0xc], eax
		xor ch, ch
		cmp eax, dword ptr [g_unk0x100ea84c]
		jle jmp_10048f0b
		or ch, 2
jmp_10048f0b:
		cmp eax, dword ptr [g_unk0x100ea830]
		jge jmp_10048f1a
		or ch, 1
jmp_10048f1a:
		mov cl, byte ptr [g_unk0x100ea828]
		mov eax, dword ptr [ebx + 4]
		cdq
		shld edx, eax, cl
		shl eax, cl
		idiv esi
		add eax, 2
		sar eax, 2
		neg eax
		add eax, dword ptr [g_unk0x100ea858]
		mov dword ptr [ebx + 0x10], eax
		cmp eax, dword ptr [g_unk0x100ea840]
		jle jmp_10048f4b
		or ch, 8
jmp_10048f4b:
		cmp eax, dword ptr [g_unk0x100ea850]
		jge jmp_10048f5a
		or ch, 4
jmp_10048f5a:
		mov byte ptr [ebx + 0x1c], ch
		mov byte ptr [ebx + 0x1d], 1
jmp_10048f61:
		mov al, byte ptr [ebx + 0x1c]
		or byte ptr [g_unk0x1010b53c], al
		and byte ptr [g_unk0x1010b5b8], al
		cmp dword ptr [g_unk0x1010b5b0], 0x14
		jl jmp_10048f8c
		mov dword ptr [g_unk0x1010b5ac], 0
		jmp jmp_10048fa2
jmp_10048f8c:
		mov eax, p_vertex
		mov ecx, dword ptr [g_unk0x1010b5b0]
		mov dword ptr [g_unk0x1010b550 + ecx*4], eax
		inc dword ptr [g_unk0x1010b5b0]
	}

	jmp_10048fa2 : return p_vertex;
#endif
}

// Returns the shade (0x7f: full) of p_face from the angle between its normal and the direction
// from its first vertex to the light (g_unk0x100ea8c0, or the origin with g_unk0x1010b530). The
// shading is an __asm block.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x10048faf
MechS32 FUN_10048faf(DuskMoth0x24* p_face, EmberFern0x2c* p_vertices)
{
#ifdef COMPAT_MODE
	STUB(0x10048faf);
	return 0;
#else
	MechS32 x;
	MechS32 y;
	MechS32 z;
	MechS16 shade;
	MechS32 nx;
	MechS32 ny;
	MechS32 nz;
	EmberFern0x2c* vertex;

	vertex = &p_vertices[((MechU8*) p_face)[p_face->m_unk0x04]];
	nx = p_face->m_normal[0];
	ny = p_face->m_normal[1];
	nz = p_face->m_normal[2];
	x = vertex->m_unk0x0c;
	y = vertex->m_unk0x10;
	z = vertex->m_unk0x14;
	if (g_unk0x1010b530) {
		x = y = z = 0;
	}

	__asm {
		mov eax, dword ptr [g_unk0x100ea8c4]
		sub eax, x
		mov ecx, eax
		jge jmp_1004903e
		neg ecx
jmp_1004903e:
		mov x, ecx
		mov ebx, ecx
		imul nx
		mov edi, edx
		mov esi, eax
		mov eax, dword ptr [g_unk0x100ea8c8]
		sub eax, y
		mov ecx, eax
		jge jmp_1004905c
		neg ecx
jmp_1004905c:
		mov y, ecx
		or ebx, ecx
		imul ny
		add esi, eax
		adc edi, edx
		mov eax, dword ptr [g_unk0x100ea8c0]
		sub eax, z
		mov ecx, eax
		jge jmp_1004907a
		neg ecx
jmp_1004907a:
		mov z, ecx
		or ebx, ecx
		jne jmp_10049092
		mov ax, 0x7f
		mov shade, ax
		jmp jmp_10049147
jmp_10049092:
		imul nz
		add esi, eax
		adc edi, edx
		shrd esi, edi, 0x10
		sar edi, 0x10
		xor ecx, ecx
		test ebx, 0xff000000
		je jmp_100490b7
		add cx, 0x10
		jmp jmp_100490c7
jmp_100490b7:
		test ebx, 0xffff0000
		je jmp_100490c9
		add cx, 8
jmp_100490c7:
		shr ebx, cl
jmp_100490c9:
		bsr ax, bx
		add cx, ax
		sub cx, 7
		je jmp_10049103
		jl jmp_100490f3
		shr x, cl
		shr y, cl
		shr z, cl
		shrd esi, edi, cl
		sar edi, cl
		jmp jmp_10049103
jmp_100490f3:
		neg cl
		shl x, cl
		shl y, cl
		shl z, cl
		shld edi, esi, cl
		shl esi, cl
jmp_10049103:
		mov al, byte ptr x
		mul al
		mov bx, ax
		xor dx, dx
		mov al, byte ptr y
		mul al
		add bx, ax
		adc dx, 0
		mov al, byte ptr z
		mul al
		add bx, ax
		adc dx, 0
		shrd bx, dx, 7
		and ebx, 0xfffe
		add ebx, dword ptr [g_sqrtTable]
		mov ax, word ptr [ebx]
		cwde
		mov ebx, eax
		mov edx, edi
		mov eax, esi
		idiv ebx
		mov shade, ax
	}

	jmp_10049147 : return shade;
#endif
}

// Queues a face of a model for drawing, unless it faces away: projects the vertices it hasn't
// yet, clips it to the near plane through the filter hooks (g_unk0x100a6cc8) and adds the
// polygon to the list being built (g_unk0x1010b5c4) with its depth.
// STUB: MW2 0x10049155
void FUN_10049155(DuskMoth0x24* p_face, EmberFern0x2c* p_vertices)
{
	STUB(0x10049155);
}
