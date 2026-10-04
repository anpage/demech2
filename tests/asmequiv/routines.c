// The routines under test: how each one's cases are set up, called and compared, and for which
// arguments the original computes a result.

#include "asmequiv.h"
#include "depthsort.h"
#include "eyepoint.h"
#include "face.h"
#include "portable.h"
#include "projectedvertex.h"
#include "queuedpolygon.h"
#include "ray.h"
#include "rendersettings.h"
#include "shape.h"
#include "shapegeom.h"
#include "targeting.h"
#include "transform.h"
#include "types.h"
#include "vertex.h"
#include "vfxrend.h"
#include "window.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

#define INT_MIN32 (-0x7fffffff - 1)

static AsmFn Function(const AsmModule* p_module, const char* p_name)
{
	return p_module->m_function(p_module->m_handle, p_name);
}

static void* Data(const AsmModule* p_module, const char* p_name)
{
	return p_module->m_data(p_module->m_handle, p_name);
}

// Random bytes: the initial contents of every buffer a routine gets, guard bands included, so
// that a stray write shows as a difference whatever it writes.
static void Fill(MechU8* p_buffer, MechU32 p_size, MechU32* p_state)
{
	MechU32 i;

	for (i = 0; i < p_size; i++) {
		p_buffer[i] = (MechU8) AsmNext(p_state);
	}
}

static void FillWords(MechS32* p_words, MechU32 p_count, MechU32* p_state)
{
	MechU32 i;

	for (i = 0; i < p_count; i++) {
		p_words[i] = PortableS32(AsmNext(p_state));
	}
}

// --- Domains ---

// A quotient an idiv leaves in eax.
static MechS32 FitsS32(MechS64 p_value)
{
	return p_value >= INT_MIN32 && p_value <= 0x7fffffff;
}

// Whether idiv computes p_dividend / p_divisor without faulting. A divisor of -1 negates, which
// the most negative dividend can't be in C.
static MechS32 Divides(MechS64 p_dividend, MechS32 p_divisor)
{
	if (p_divisor == -1) {
		return p_dividend >= -0x7fffffff && p_dividend <= 0x80000000u;
	}

	return p_divisor != 0 && FitsS32(p_dividend / p_divisor);
}

static MechS32 DivideDomain(MechS64 p_dividend, MechS32 p_divisor)
{
	return Divides(p_dividend, p_divisor) ? c_domainIn : c_domainFault;
}

static MechS32 FixedDiv16Domain(const MechS32* p_args)
{
	return DivideDomain((MechS64) p_args[0] * 0x10000, p_args[1]);
}

static MechS32 FixedDiv29Domain(const MechS32* p_args)
{
	return DivideDomain((MechS64) p_args[0] * 0x20000000, p_args[1]);
}

static MechS32 FixedDivU16Domain(const MechS32* p_args)
{
	MechU64 dividend = (MechU64) ((MechS64) p_args[0] * 0x10000);
	MechU32 divisor = (MechU32) p_args[1];

	return divisor != 0 && dividend / divisor <= 0xffffffff ? c_domainIn : c_domainFault;
}

static MechS32 MulDiv64Domain(const MechS32* p_args)
{
	return DivideDomain((MechS64) p_args[0] * p_args[1], p_args[2]);
}

// bsr of 0 leaves the shift undefined.
static MechS32 FixedSqrtGuessDomain(const MechS32* p_args)
{
	return p_args[0] ? c_domainIn : c_domainUndefined;
}

static MechS32 Domain100349c0(const MechS32* p_args)
{
	return DivideDomain((MechS64) p_args[0] * p_args[3] - (MechS64) p_args[1] * p_args[2], p_args[4]);
}

static MechS32 Domain10042740(const MechS32* p_args)
{
	return DivideDomain((MechS64) p_args[0] * ((MechS64) 1 << (p_args[2] & 31)), p_args[1]);
}

static MechS32 Domain10013340(const MechS32* p_args)
{
	return DivideDomain(PortableSar64((MechS64) p_args[0] * p_args[1], 8), p_args[2]);
}

static MechS32 Domain100349f0(const MechS32* p_args)
{
	MechU32 x0 = (MechU32) p_args[0];
	MechU32 x1 = (MechU32) p_args[1];
	MechU32 x = (MechU32) p_args[2];
	MechU32 y0 = (MechU32) p_args[3];
	MechU32 y1 = (MechU32) p_args[4];

	if (p_args[1] > p_args[0]) {
		return DivideDomain((MechS64) PortableS32(x - x1) * PortableS32(y0 - y1), PortableS32(x0 - x1));
	}

	return DivideDomain((MechS64) PortableS32(x - x0) * PortableS32(y1 - y0), PortableS32(x1 - x0));
}

static MechS32 Domain1004c800(const MechS32* p_args)
{
	return DivideDomain((MechS64) p_args[0] * p_args[1], PortableS32((MechU32) p_args[1] + (MechU32) p_args[2]));
}

static MechS32 Domain1004c860(const MechS32* p_args)
{
	return DivideDomain((MechS64) p_args[0] * p_args[1] + (MechS64) p_args[2] * 0x10000, p_args[3]);
}

// --- Unsigned arguments ---

typedef MechU32 (*FixedSqrtGuessFn)(MechU32 p_value);

// Called through its own type: a call through another one is undefined (Clang's UBSan checks).
static void RunFixedSqrtGuess(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	AsmOutputWord(p_output, ((FixedSqrtGuessFn) Function(p_module, "FixedSqrtGuess"))((MechU32) p_args[0]));
}

// --- Strings ---

typedef MechU32 (*NameHashFn)(const MechChar* p_name);

// Arguments: the length (modulo 64), and whether the name is made of any bytes or of the
// characters resource names use.
static void RunNameHash(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	static const char c_nameCharacters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.";
	MechU8 name[68];
	MechU32 state = AsmSeed(p_args, 2);
	MechU32 length = (MechU32) p_args[0] % 64;
	MechU32 i;

	Fill(name, sizeof(name), &state);
	for (i = 0; i < length; i++) {
		MechU32 bits = AsmNext(&state);

		name[i] = (MechU8) (p_args[1] & 1 ? 1 + bits % 255 : (MechU8) c_nameCharacters[bits % 64]);
	}

	name[length] = 0;
	AsmOutputWord(p_output, ((NameHashFn) Function(p_module, "HashName"))((const MechChar*) name));
}

// --- Pointer arguments ---

typedef void (*IntegrateMidpointFn)(MechS32* p_position, MechS32* p_velocity, MechS32 p_acceleration, MechS32 p_time);

// Arguments: the position, the velocity, the acceleration and the time.
static void RunIntegrateMidpoint(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	MechS32 cells[5];
	MechU32 state = AsmSeed(p_args, 4);
	MechS32 i;

	FillWords(cells, 5, &state);
	cells[1] = p_args[0];
	cells[3] = p_args[1];
	((IntegrateMidpointFn) Function(p_module, "IntegrateMidpoint"))(&cells[1], &cells[3], p_args[2], p_args[3]);
	for (i = 0; i < 5; i++) {
		AsmOutputWord(p_output, (MechU32) cells[i]);
	}
}

typedef void (*Fn1004c820)(MechS32* p_low, MechS32* p_scaled, MechS16* p_shift, MechU32 p_a, MechU32 p_b);

// Arguments: the initial low dword, scaled dword and shift (its low word), and the factors.
static void Run1004c820(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	MechS32 cells[5];
	MechS16 shifts[4];
	MechU32 state = AsmSeed(p_args, 5);
	MechS32 i;

	FillWords(cells, 5, &state);
	for (i = 0; i < 4; i++) {
		shifts[i] = PortableS16((MechU16) AsmNext(&state));
	}

	cells[1] = p_args[0];
	cells[3] = p_args[1];
	shifts[1] = PortableS16((MechU16) p_args[2]);
	((
		Fn1004c820
	) Function(p_module, "MulNormalize16"))(&cells[1], &cells[3], &shifts[1], (MechU32) p_args[3], (MechU32) p_args[4]);
	for (i = 0; i < 5; i++) {
		AsmOutputWord(p_output, (MechU32) cells[i]);
	}

	AsmOutputWord(p_output, (MechU16) shifts[0] | ((MechU32) (MechU16) shifts[1] << 16));
	AsmOutputWord(p_output, (MechU16) shifts[2] | ((MechU32) (MechU16) shifts[3] << 16));
}

// --- Memory blocks ---

#define ARENA_SIZE 0x200

typedef void* (*MemCopyFn)(void* p_dst, const void* p_src, MechU32 p_size);
typedef void* (*MemSetFn)(void* p_dst, MechS32 p_value, MechU32 p_size);

static void OutputArena(AsmOutput* p_output, const MechU8* p_arena, const void* p_result)
{
	AsmOutputWord(p_output, (MechU32) ((const MechU8*) p_result - p_arena));
	AsmOutputBytes(p_output, p_arena, ARENA_SIZE);
}

// Arguments: the size (modulo 193), the destination's and the source's offsets in one arena
// (modulo 160: they overlap in either order), and the arena's contents.
static void RunMemCopy(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	MechU8 arena[ARENA_SIZE];
	MechU32 state = AsmSeed(p_args, 4);
	void* result;

	Fill(arena, ARENA_SIZE, &state);
	result = ((MemCopyFn) Function(p_module, "MemCopy"))(
		arena + (MechU32) p_args[1] % 160,
		arena + (MechU32) p_args[2] % 160,
		(MechU32) p_args[0] % 193
	);
	OutputArena(p_output, arena, result);
}

// Arguments: the size (modulo 193), the destination's offset (modulo 160), the value, and the
// arena's contents.
static void RunMemSet(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	MechU8 arena[ARENA_SIZE];
	MechU32 state = AsmSeed(p_args, 4);
	void* result;

	Fill(arena, ARENA_SIZE, &state);
	result = ((
		MemSetFn
	) Function(p_module, "MemSet"))(arena + (MechU32) p_args[1] % 160, p_args[2], (MechU32) p_args[0] % 193);
	OutputArena(p_output, arena, result);
}

// --- The record stacks (recordstacks.c) ---

typedef ProjectedVertex* (*PopVertexFn)(void);
typedef MechU8* (*PopRecordFn)(void);

// Arguments: the top's offset in the arena (0x20 to 0x200), the bottom's (0 to 0x1f4), both
// dword-aligned as the game's are, and the flag the routines clear.
static void RunRecordStack(const AsmModule* p_module, MechS32 p_top, const MechS32* p_args, AsmOutput* p_output)
{
	MechU32 words[ARENA_SIZE / 4];
	MechU8* arena = (MechU8*) words;
	MechU8** top = (MechU8**) Data(p_module, "g_drawBufferTop");
	MechU8** bottom = (MechU8**) Data(p_module, "g_drawBufferBottom");
	MechS32* flag = (MechS32*) Data(p_module, "g_queueHasRoom");
	MechU32 state = AsmSeed(p_args, 3);
	void* result;

	Fill(arena, ARENA_SIZE, &state);
	*top = arena + 0x20 + (MechU32) p_args[0] % ((ARENA_SIZE - 0x20) / 4 + 1) * 4;
	*bottom = arena + (MechU32) p_args[1] % ((ARENA_SIZE - 0xc) / 4 + 1) * 4;
	*flag = p_args[2];
	if (p_top) {
		result = ((PopVertexFn) Function(p_module, "AllocProjectedVertex"))();
	}
	else {
		result = ((PopRecordFn) Function(p_module, "AllocQueuedPolygon"))();
	}

	AsmOutputWord(p_output, (MechU32) (*top - arena));
	AsmOutputWord(p_output, (MechU32) (*bottom - arena));
	AsmOutputWord(p_output, (MechU32) *flag);
	OutputArena(p_output, arena, result);
}

static void Run1007d248(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunRecordStack(p_module, 1, p_args, p_output);
}

static void Run1007d296(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunRecordStack(p_module, 0, p_args, p_output);
}

// --- Trigonometry (fixedtrig.c, over clock.c's tables) ---

#define TABLE_SIZE 0x102

typedef MechS32 (*TrigFn)(MechS32 p_value);
typedef MechS32 (*BearingFn)(MechS32 p_x, MechS32 p_z);

enum {
	c_tableGame,       // the game's, as InitSinAtanTables computes it
	c_tableIncreasing, // random, non-decreasing from 0, below 0x80000000
	c_tableRandom      // random
};

static MechS32 g_gameSinTable[TABLE_SIZE];
static MechS32 g_gameAtanTable[TABLE_SIZE];
static AsmHash g_gameTablesHash;
static MechS32 g_gameTablesReady = 0;

static void InitGameTables(void)
{
	MechS32 i;

	if (g_gameTablesReady) {
		return;
	}

	for (i = 0; i < 0x100; i++) {
		g_gameSinTable[i] = (MechS32) (sin(i * (3.14159265 / 512)) * 536870912.0);
		g_gameAtanTable[i] = (MechS32) (atan(i / 256.0) * 3754936.210460003);
	}

	g_gameSinTable[0x100] = g_gameSinTable[0x101] = 0x20000000;
	g_gameAtanTable[0x100] = g_gameAtanTable[0x101] = 0x2d0000;

	AsmHashInit(&g_gameTablesHash);
	for (i = 0; i < TABLE_SIZE; i++) {
		AsmHashWord(&g_gameTablesHash, (MechU32) g_gameSinTable[i]);
		AsmHashWord(&g_gameTablesHash, (MechU32) g_gameAtanTable[i]);
	}

	g_gameTablesReady = 1;
}

// A case's table: the kind from p_selector's low bits, the random entries from p_selector and
// p_seed. One in eight steps of an increasing table repeats the entry; bits 4 and 5 of
// p_selector scale the steps down, for tables that end below the arcsine's largest sine.
static void MakeTable(MechS32* p_table, const MechS32* p_game, MechS32 p_kind, MechS32 p_selector, MechS32 p_seed)
{
	MechU32 state = AsmMix((MechU32) p_selector ^ AsmMix((MechU32) p_seed));
	MechU32 value = 0;
	MechS32 i;

	InitGameTables();
	switch (p_kind) {
	case c_tableGame:
		memcpy(p_table, p_game, TABLE_SIZE * sizeof(MechS32));
		break;
	case c_tableIncreasing:
		for (i = 0; i < TABLE_SIZE; i++) {
			MechU32 bits = AsmNext(&state);

			p_table[i] = (MechS32) value;
			if (bits & 7) {
				value += (bits >> 3) % (0x7f0000 >> ((p_selector >> 4) & 3));
			}
		}
		break;
	default:
		FillWords(p_table, TABLE_SIZE, &state);
		break;
	}
}

// The arcsine's table: the game's or an increasing one (the binary search assumes it increases).
static MechS32 ArcsineTableKind(MechS32 p_selector)
{
	return p_selector & 2 ? c_tableIncreasing : c_tableGame;
}

// The sine's and the bearing's: any.
static MechS32 AnyTableKind(MechS32 p_selector)
{
	return p_selector & 2 ? (p_selector & 1 ? c_tableRandom : c_tableIncreasing) : c_tableGame;
}

static void HashGameTables(MechS32 p_kind, AsmHash* p_hash)
{
	if (p_kind == c_tableGame) {
		InitGameTables();
		AsmHashWord(p_hash, (MechU32) g_gameTablesHash.m_value);
		AsmHashWord(p_hash, (MechU32) (g_gameTablesHash.m_value >> 32));
	}
}

static void HashSineInputs(const MechS32* p_args, AsmHash* p_hash)
{
	HashGameTables(AnyTableKind(p_args[1]), p_hash);
}

static void HashArcsineInputs(const MechS32* p_args, AsmHash* p_hash)
{
	HashGameTables(ArcsineTableKind(p_args[1]), p_hash);
}

static void HashBearingInputs(const MechS32* p_args, AsmHash* p_hash)
{
	HashGameTables(AnyTableKind(p_args[2]), p_hash);
}

// Arguments: the angle (16.16 degrees), and the table.
static void RunSine(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	MechS32* table = (MechS32*) Data(p_module, "g_sinTable");

	MakeTable(table, g_gameSinTable, AnyTableKind(p_args[1]), p_args[1], p_args[0]);
	AsmOutputWord(p_output, (MechU32) ((TrigFn) Function(p_module, "FixedSin"))(p_args[0]));
}

// The arcsine's argument: p_args[0], or, when bits 2 and 3 of the selector are 1 and 0, an
// entry of the table chosen by p_args[0] (the search's and the interpolation's boundaries),
// offset by -1, 0 or 1 and negated with p_args[0].
static MechS32 ArcsineArgument(const MechS32* p_args, const MechS32* p_table)
{
	MechU32 sine;

	if (((p_args[1] >> 2) & 3) != 1) {
		return p_args[0];
	}

	sine = (MechU32) p_table[(MechU32) p_args[0] % 0x101] + ((MechU32) p_args[0] >> 16) % 3 - 1;
	return PortableS32(p_args[0] < 0 ? 0 - sine : sine);
}

// Arguments: the sine (2.29, or a table entry), and the table.
static void RunArcsine(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	MechS32* table = (MechS32*) Data(p_module, "g_sinTable");

	MakeTable(table, g_gameSinTable, ArcsineTableKind(p_args[1]), p_args[1], 0);
	AsmOutputWord(p_output, (MechU32) ((TrigFn) Function(p_module, "FixedAsin"))(ArcsineArgument(p_args, table)));
}

// The search and the idiv of FixedAsin, over the case's table: the idiv faults when the
// sine lies at or beyond two equal last entries.
static MechS32 ArcsineDomain(const MechS32* p_args)
{
	MechS32 table[TABLE_SIZE];
	MechS32 argument;
	MechU32 sine;
	MechU32 index = 0;
	MechU32 step;
	MechU32 offset;
	MechU32 span;

	MakeTable(table, g_gameSinTable, ArcsineTableKind(p_args[1]), p_args[1], 0);
	argument = ArcsineArgument(p_args, table);
	sine = argument < 0 ? 0 - (MechU32) argument : (MechU32) argument;
	if (argument == 0 || PortableS32(sine) >= 0x20000000) {
		return c_domainIn;
	}

	for (step = 0x80; step; step >>= 1) {
		if (sine >= (MechU32) table[index + step]) {
			index += step;
		}
	}

	offset = sine - (MechU32) table[index];
	span = (MechU32) table[index + 1] - (MechU32) table[index];
	if (!offset || (span && offset >= span)) {
		return c_domainIn;
	}

	return DivideDomain((MechS64) PortableS32(offset) * 0x10000, PortableS32(span));
}

// Arguments: x, z, and the table.
static void RunBearing(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	MechS32* table = (MechS32*) Data(p_module, "g_atanTable");

	MakeTable(table, g_gameAtanTable, AnyTableKind(p_args[2]), p_args[2], p_args[0]);
	AsmOutputWord(p_output, (MechU32) ((BearingFn) Function(p_module, "FixedAtan2"))(p_args[0], p_args[1]));
}

// The div of the smaller magnitude, shifted left by 24, by the larger: neg leaves INT_MIN
// negative, so a magnitude of 0x80000000 can overflow it or leave the other at 0.
static MechS32 BearingDomain(const MechS32* p_args)
{
	MechS32 smaller = p_args[0] < 0 ? PortableS32(0 - (MechU32) p_args[0]) : p_args[0];
	MechS32 larger = p_args[1] < 0 ? PortableS32(0 - (MechU32) p_args[1]) : p_args[1];
	MechS32 swap;

	if (smaller == larger) {
		return c_domainIn;
	}

	if (smaller > larger) {
		swap = smaller;
		smaller = larger;
		larger = swap;
	}

	if (smaller == 0) {
		return c_domainIn;
	}

	if (larger == 0 || ((MechU64) (MechU32) smaller << 24) / (MechU32) larger > 0xffffffff) {
		return c_domainFault;
	}

	return c_domainIn;
}

// --- The horizon (horizon.c) ---

// Arguments, the same for the three routines: the screen x and y, the view's centre, the view
// matrix's second column (a, c, e) and the two scales (b, d), in the order a, b, c, d, e.
enum {
	c_argX,
	c_argY,
	c_argCenterX,
	c_argCenterY,
	c_argA,
	c_argB,
	c_argC,
	c_argD,
	c_argE
};

typedef MechS32 (*HorizonTestFn)(MechS32 p_x, MechS32 p_y, Eyepoint* p_eyepoint);
typedef MechS32 (*HorizonFn)(MechS32 p_value, Eyepoint* p_eyepoint);

static void MakeEyepoint(Eyepoint* p_eyepoint, const MechS32* p_args)
{
	MechU32 state = AsmSeed(p_args, 9);

	Fill((MechU8*) p_eyepoint, sizeof(Eyepoint), &state);
	p_eyepoint->m_centerX = p_args[c_argCenterX];
	p_eyepoint->m_centerY = p_args[c_argCenterY];
	p_eyepoint->m_viewMatrix.m_rows[0][1] = p_args[c_argA];
	p_eyepoint->m_projectScaleX = p_args[c_argB];
	p_eyepoint->m_viewMatrix.m_rows[1][1] = p_args[c_argC];
	p_eyepoint->m_projectScaleY = p_args[c_argD];
	p_eyepoint->m_viewMatrix.m_rows[2][1] = p_args[c_argE];
}

static void OutputEyepoint(AsmOutput* p_output, MechS32 p_result, Eyepoint* p_eyepoint)
{
	AsmOutputWord(p_output, (MechU32) p_result);
	AsmOutputBytes(p_output, (const MechU8*) p_eyepoint, sizeof(Eyepoint));
}

static void Run10071930(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Eyepoint eyepoint;
	MechS32 result;

	MakeEyepoint(&eyepoint, p_args);
	result = ((HorizonTestFn) Function(p_module, "IsAboveHorizon"))(p_args[c_argX], p_args[c_argY], &eyepoint);
	OutputEyepoint(p_output, result, &eyepoint);
}

static void Run100719ca(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Eyepoint eyepoint;
	MechS32 result;

	MakeEyepoint(&eyepoint, p_args);
	result = ((HorizonFn) Function(p_module, "HorizonYAtX"))(p_args[c_argX], &eyepoint);
	OutputEyepoint(p_output, result, &eyepoint);
}

static void Run10071a4c(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Eyepoint eyepoint;
	MechS32 result;

	MakeEyepoint(&eyepoint, p_args);
	result = ((HorizonFn) Function(p_module, "HorizonXAtY"))(p_args[c_argY], &eyepoint);
	OutputEyepoint(p_output, result, &eyepoint);
}

static MechS32 Difference(MechS32 p_a, MechS32 p_b)
{
	return PortableS32((MechU32) p_a - (MechU32) p_b);
}

static MechS32 Domain10071930(const MechS32* p_args)
{
	MechS64 x = Difference(p_args[c_argX], p_args[c_argCenterX]);
	MechS64 y = Difference(p_args[c_argCenterY], p_args[c_argY]);

	return Divides(x * p_args[c_argA], p_args[c_argB]) && Divides(y * p_args[c_argC], p_args[c_argD]) ? c_domainIn
																									  : c_domainFault;
}

// The second quotient's dividend: minus (the first quotient plus e >> 16).
static MechS64 HorizonDividend(MechS64 p_quotient, MechS32 p_e, MechS32 p_factor)
{
	MechU32 sum = (MechU32) p_quotient + (MechU32) PortableSar32(p_e, 16);

	return (MechS64) PortableS32(0 - sum) * p_factor;
}

static MechS32 Domain100719ca(const MechS32* p_args)
{
	MechS64 dividend = (MechS64) Difference(p_args[c_argX], p_args[c_argCenterX]) * p_args[c_argA];

	if (!Divides(dividend, p_args[c_argB])) {
		return c_domainFault;
	}

	dividend = HorizonDividend(dividend / p_args[c_argB], p_args[c_argE], p_args[c_argD]);
	return DivideDomain(dividend, p_args[c_argC]);
}

static MechS32 Domain10071a4c(const MechS32* p_args)
{
	MechS64 dividend = (MechS64) Difference(p_args[c_argCenterY], p_args[c_argY]) * p_args[c_argC];

	if (!Divides(dividend, p_args[c_argD])) {
		return c_domainFault;
	}

	dividend = HorizonDividend(dividend / p_args[c_argD], p_args[c_argE], p_args[c_argB]);
	return DivideDomain(dividend, p_args[c_argA]);
}

// --- The tick counters (ticks.asm) ---

#define TICK_HANDLES 64

typedef void (*TickCallbackFn)(void);
typedef MechS16 (*AllocTicksFn)(MechU32 p_flags);
typedef MechS32 (*GetTicksFn)(MechU32 p_handle);
typedef void (*TickHandleFn)(MechU32 p_handle);
typedef void (*SetTicksFn)(MechU32 p_handle, MechS32 p_ticks);
typedef void (*PauseTimerFn)(MechS32 p_flags, MechS32 p_paused);

typedef struct TickState {
	MechU32* m_paused;
	MechS32* m_bases1;
	MechS32* m_bases2;
	MechS32* m_ticks1;
	MechS32* m_ticks2;
} TickState;

// A counter: 0 (where AllocTicks starts handles at 1), the largest value (where it wraps), or
// any.
static MechS32 RandomTicks(MechU32* p_state)
{
	MechU32 bits = AsmNext(p_state);

	switch (bits & 7) {
	case 0:
		return 0;
	case 1:
		return 0x7fffffff;
	default:
		return PortableS32(AsmNext(p_state));
	}
}

// A handle table: free slots (0) at one of four densities, the odd -1 (where AllocTicks gives
// up), and at least one of either, so that AllocTicks stops inside the table.
static void MakeTickBases(MechS32* p_bases, MechU32* p_state)
{
	MechU32 density = AsmNext(p_state) % 4;
	MechU32 slot;
	MechS32 i;

	for (i = 0; i < TICK_HANDLES; i++) {
		MechU32 bits = AsmNext(p_state);

		if (density < 3 && bits % (2u << (density * 2)) == 0) {
			p_bases[i] = 0;
		}
		else if (bits % 61 == 1) {
			p_bases[i] = -1;
		}
		else {
			p_bases[i] = PortableS32(AsmNext(p_state));
		}
	}

	slot = AsmNext(p_state) % TICK_HANDLES;
	p_bases[slot] = AsmNext(p_state) % 8 ? 0 : -1;
}

// Every case starts from random counters, pause bits and tables, from its third argument word.
static void SetUpTicks(const AsmModule* p_module, const MechS32* p_args, TickState* p_ticks)
{
	MechU32 state = AsmSeed(p_args, 3);

	p_ticks->m_paused = (MechU32*) Data(p_module, "g_ticksPaused");
	p_ticks->m_bases1 = (MechS32*) Data(p_module, "g_ticks1Bases");
	p_ticks->m_bases2 = (MechS32*) Data(p_module, "g_ticks2Bases");
	p_ticks->m_ticks1 = (MechS32*) Data(p_module, "g_ticks1");
	p_ticks->m_ticks2 = (MechS32*) Data(p_module, "g_ticks2");

	*p_ticks->m_paused = AsmNext(&state);
	*p_ticks->m_ticks1 = RandomTicks(&state);
	*p_ticks->m_ticks2 = RandomTicks(&state);
	MakeTickBases(p_ticks->m_bases1, &state);
	MakeTickBases(p_ticks->m_bases2, &state);
}

static void OutputTicks(AsmOutput* p_output, MechU32 p_result, const TickState* p_ticks)
{
	MechS32 i;

	AsmOutputWord(p_output, p_result);
	AsmOutputWord(p_output, *p_ticks->m_paused);
	AsmOutputWord(p_output, (MechU32) *p_ticks->m_ticks1);
	AsmOutputWord(p_output, (MechU32) *p_ticks->m_ticks2);
	for (i = 0; i < TICK_HANDLES; i++) {
		AsmOutputWord(p_output, (MechU32) p_ticks->m_bases1[i]);
		AsmOutputWord(p_output, (MechU32) p_ticks->m_bases2[i]);
	}
}

// A handle the game could have: a slot, with or without the first counter's bit.
static MechU32 TickHandle(MechS32 p_arg)
{
	return ((MechU32) p_arg & 0x80) | ((MechU32) p_arg >> 8) % TICK_HANDLES;
}

static void RunGameTickTimerCallback(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	TickState ticks;

	SetUpTicks(p_module, p_args, &ticks);
	((TickCallbackFn) Function(p_module, "GameTickTimerCallback"))();
	OutputTicks(p_output, 0, &ticks);
}

// Arguments: the flags (bit 0x80 selects the counter), unused, and the state.
static void RunAllocTicks(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	TickState ticks;
	MechS16 handle;

	SetUpTicks(p_module, p_args, &ticks);
	handle = ((AllocTicksFn) Function(p_module, "AllocTicks"))((MechU32) p_args[0]);
	OutputTicks(p_output, (MechU16) handle, &ticks);
}

// Arguments: the handle (TickHandle), unused, and the state.
static void RunGetTicks(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	TickState ticks;
	MechS32 result;

	SetUpTicks(p_module, p_args, &ticks);
	result = ((GetTicksFn) Function(p_module, "GetTicks"))(TickHandle(p_args[0]));
	OutputTicks(p_output, (MechU32) result, &ticks);
}

static void RunResetTicks(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	TickState ticks;

	SetUpTicks(p_module, p_args, &ticks);
	((TickHandleFn) Function(p_module, "ResetTicks"))(TickHandle(p_args[0]));
	OutputTicks(p_output, 0, &ticks);
}

static void RunFreeTicks(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	TickState ticks;

	SetUpTicks(p_module, p_args, &ticks);
	((TickHandleFn) Function(p_module, "FreeTicks"))(TickHandle(p_args[0]));
	OutputTicks(p_output, 0, &ticks);
}

// Arguments: the handle (TickHandle), the ticks, and the state.
static void RunSetTicks(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	TickState ticks;

	SetUpTicks(p_module, p_args, &ticks);
	((SetTicksFn) Function(p_module, "SetTicks"))(TickHandle(p_args[0]), p_args[1]);
	OutputTicks(p_output, 0, &ticks);
}

// Arguments: the flags, whether to pause, and the state.
static void RunPauseTimer(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	TickState ticks;

	SetUpTicks(p_module, p_args, &ticks);
	((PauseTimerFn) Function(p_module, "PauseTimer"))(p_args[0], p_args[1]);
	OutputTicks(p_output, 0, &ticks);
}

// --- The sound-block decoder (sndunpack.asm) ---

#define SOUND_FRAME_MAX 0x400
#define SOUND_COUNT_MAX 4
#define SOUND_GUARD 64
#define SOUND_SRC_SIZE (SOUND_COUNT_MAX * (1 + 16 + SOUND_FRAME_MAX) + SOUND_GUARD)
#define SOUND_DST_SIZE (SOUND_COUNT_MAX * SOUND_FRAME_MAX + SOUND_GUARD)
#define SOUND_SCRATCH_SIZE 0x400
#define SOUND_FRAME_BUFFER_SIZE 0x401
#define SOUND_DELTAS_SIZE 0x43
#define SOUND_GLOBALS_SIZE 0x848 // the three, padded to a dword

typedef MechU8* (*SoundUnpackFn)(MechU8* p_src, MechU8* p_dst, MechU32 p_count, MechU32 p_frameSize, MechS32* p_state);

// The frame size: small (1 to 8, where the upsampling's loops run once), at most 0x400 (the
// buffers' size), or any between.
static MechU32 SoundFrameSize(MechS32 p_arg)
{
	MechU32 bits = (MechU32) p_arg;

	switch (bits % 4) {
	case 0:
		return 1 + (bits >> 2) % 8;
	case 1:
		return SOUND_FRAME_MAX - (bits >> 2) % 4;
	default:
		return 1 + (bits >> 2) % SOUND_FRAME_MAX;
	}
}

// A stream of p_count well-formed frames: random upsampling and coding (one in 32 an unknown
// coding, which ends the decoding), each followed by its delta table and as many bytes of
// indices or samples as it decodes. A raw frame needs a sample before the upsampling: the
// original's copy loop runs 2^32 times otherwise.
static void MakeSoundStream(MechU8* p_src, MechU32 p_count, MechU32 p_frameSize, MechU32* p_state)
{
	static const MechU32 c_tableSizes[] = {0, 0, 2, 4, 16, 0};
	static const MechU32 c_perByte[] = {0, 0, 8, 4, 2, 0};
	MechU8* src = p_src;
	MechU32 frame;

	Fill(p_src, SOUND_SRC_SIZE, p_state);
	for (frame = 0; frame < p_count; frame++) {
		MechU32 bits = AsmNext(p_state);
		MechU32 upsampling = (bits >> 6) & 3;
		MechU32 samples = upsampling == 1 ? p_frameSize >> 1 : upsampling == 2 ? p_frameSize >> 2 : p_frameSize;
		MechU32 coding = (bits >> 8) % 32 ? (bits >> 16) % 6 : 6 + (bits >> 16) % 10;
		MechU32 payload;

		if (coding == 5 && !samples) {
			coding = 2;
		}

		*src++ = (MechU8) ((bits & 0xf0) | coding);
		if (coding >= 6) {
			return;
		}

		if (coding == 5) {
			payload = samples;
		}
		else if (c_perByte[coding]) {
			payload = c_tableSizes[coding] + (samples ? (samples + c_perByte[coding] - 1) / c_perByte[coding] : 1);
		}
		else {
			payload = 0;
		}

		src += payload;
	}
}

// Arguments: the frame count (1 to 4), the frame size (SoundFrameSize), the running value, and
// the stream, the output buffer's and the decoder's globals' initial contents.
static void RunSoundUnpack(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	MechU8 src[SOUND_SRC_SIZE];
	MechU8 dst[SOUND_DST_SIZE];
	MechU8 globals[SOUND_GLOBALS_SIZE];
	MechU8* scratch = (MechU8*) Data(p_module, "g_soundUpsampleBuffer");
	MechU8* frame = (MechU8*) Data(p_module, "g_soundFrame");
	MechU8* deltas = (MechU8*) Data(p_module, "g_soundDeltas");
	MechU32 count = 1 + (MechU32) p_args[0] % SOUND_COUNT_MAX;
	MechU32 frameSize = SoundFrameSize(p_args[1]);
	MechS32 value = p_args[2];
	MechU32 state = AsmSeed(p_args, 4);
	MechU8* end;

	MakeSoundStream(src, count, frameSize, &state);
	Fill(dst, SOUND_DST_SIZE, &state);
	Fill(scratch, SOUND_SCRATCH_SIZE, &state);
	Fill(frame, SOUND_FRAME_BUFFER_SIZE, &state);
	Fill(deltas, SOUND_DELTAS_SIZE, &state);

	end = ((SoundUnpackFn) Function(p_module, "DecodeSoundFrames"))(src, dst, count, frameSize, &value);

	AsmOutputWord(p_output, end ? (MechU32) (end - src) : 0xffffffff);
	AsmOutputWord(p_output, (MechU32) value);
	AsmOutputBytes(p_output, dst, SOUND_DST_SIZE);
	memset(globals, 0, sizeof(globals));
	memcpy(globals, scratch, SOUND_SCRATCH_SIZE);
	memcpy(globals + SOUND_SCRATCH_SIZE, frame, SOUND_FRAME_BUFFER_SIZE);
	memcpy(globals + SOUND_SCRATCH_SIZE + SOUND_FRAME_BUFFER_SIZE, deltas, SOUND_DELTAS_SIZE);
	AsmOutputBytes(p_output, globals, SOUND_GLOBALS_SIZE);
}

// --- Fixed-point transforms (transform.c, shapegeom.c) ---

// A word for a matrix, a position or a normal: mostly the magnitudes the game uses (2.29 within
// +-1.0, 16.16 within +-256.0, small), and the extremes, any width and any value.
static MechS32 RandomWord(MechU32* p_state)
{
	static const MechS32 c_extremes[] = {0, 1, -1, 0x20000000, -0x20000000, 0x7fffffff, INT_MIN32, INT_MIN32 + 1};
	MechU32 kind = AsmNext(p_state);
	MechU32 bits = AsmNext(p_state);
	MechU32 width;
	MechS32 value;

	switch (kind & 7) {
	case 0:
	case 1:
		return (MechS32) (bits & 0x3fffffff) - 0x20000000;
	case 2:
		return (MechS32) (bits & 0x1ffffff) - 0x1000000;
	case 3:
		return (MechS32) (bits % 513) - 256;
	case 4:
		return c_extremes[bits % 8];
	case 5:
		width = 1 + (kind >> 3) % 31;
		value = (MechS32) (bits & (((MechU32) 1 << width) - 1));
		return (kind >> 8) & 1 ? -value : value;
	default:
		return PortableS32(bits);
	}
}

// A word near the extremes, whose products reach 2^62.
static MechS32 ExtremeWord(MechU32* p_state)
{
	static const MechS32 c_extremes[] =
		{0x7fffffff, 0x7ffffffe, 0x40000000, -0x40000000, INT_MIN32, INT_MIN32 + 1, 0, 1};

	return c_extremes[AsmNext(p_state) % 8];
}

static void OutputWords(AsmOutput* p_output, const MechS32* p_words, MechS32 p_count)
{
	MechS32 i;

	for (i = 0; i < p_count; i++) {
		AsmOutputWord(p_output, (MechU32) p_words[i]);
	}
}

// A matrix between two guard words.
typedef struct GuardedMatrix {
	MechS32 m_before;
	Matrix m_matrix;
	MechS32 m_after;
} GuardedMatrix;

static void MakeMatrix(GuardedMatrix* p_matrix, MechU32* p_state)
{
	MechS32 i;
	MechS32 j;

	p_matrix->m_before = PortableS32(AsmNext(p_state));
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 3; j++) {
			p_matrix->m_matrix.m_rows[i][j] = RandomWord(p_state);
		}
	}

	p_matrix->m_after = PortableS32(AsmNext(p_state));
}

static void OutputMatrix(AsmOutput* p_output, const GuardedMatrix* p_matrix)
{
	MechS32 i;

	AsmOutputWord(p_output, (MechU32) p_matrix->m_before);
	for (i = 0; i < 4; i++) {
		OutputWords(p_output, p_matrix->m_matrix.m_rows[i], 3);
	}

	AsmOutputWord(p_output, (MechU32) p_matrix->m_after);
}

typedef void (*TransformPointFn)(Matrix* p_matrix, MechS32* p_x, MechS32* p_y, MechS32* p_z);

// Arguments: the point. The matrix comes from them, and so does whether x and y share a cell
// (one case in sixteen).
static void RunTransformPoint(const AsmModule* p_module, const char* p_name, const MechS32* p_args, AsmOutput* p_output)
{
	GuardedMatrix matrix;
	MechS32 cells[7];
	MechU32 state = AsmSeed(p_args, 3);
	MechS32* y = &cells[3];

	FillWords(cells, 7, &state);
	MakeMatrix(&matrix, &state);
	cells[1] = p_args[0];
	cells[3] = p_args[1];
	cells[5] = p_args[2];
	if (AsmNext(&state) % 16 == 0) {
		y = &cells[1];
	}

	((TransformPointFn) Function(p_module, p_name))(&matrix.m_matrix, &cells[1], y, &cells[5]);
	OutputWords(p_output, cells, 7);
	OutputMatrix(p_output, &matrix);
}

static void Run1000d650(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunTransformPoint(p_module, "TransformPoint", p_args, p_output);
}

static void Run1000d708(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunTransformPoint(p_module, "RotatePoint", p_args, p_output);
}

typedef void (*CrossColumnFn)(Matrix* p_matrix, MechS32 p_column);

// Arguments: the column (-1 to 4: the others store nothing), and the matrix's seed.
static void Run1000d7c0(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	GuardedMatrix matrix;
	MechU32 state = AsmSeed(p_args, 2);

	MakeMatrix(&matrix, &state);
	((CrossColumnFn)
		 Function(p_module, "OrthogonalizeMatrixColumn"))(&matrix.m_matrix, (MechS32) ((MechU32) p_args[0] % 6) - 1);
	OutputMatrix(p_output, &matrix);
}

typedef void (*ComposeFn)(Matrix* p_a, Matrix* p_b, Matrix* p_result);

// Arguments: where the product goes (a third matrix, either operand, or the one matrix all three
// are), and the matrices' seed.
static void Run1000da0c(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	GuardedMatrix matrices[3];
	MechU32 state = AsmSeed(p_args, 2);
	ComposeFn fn = (ComposeFn) Function(p_module, "MultiplyRotations");
	Matrix* a = &matrices[0].m_matrix;
	Matrix* b = &matrices[1].m_matrix;
	MechS32 i;

	for (i = 0; i < 3; i++) {
		MakeMatrix(&matrices[i], &state);
	}

	switch ((MechU32) p_args[0] % 5) {
	case 0:
	case 1:
		fn(a, b, &matrices[2].m_matrix);
		break;
	case 2:
		fn(a, b, a);
		break;
	case 3:
		fn(a, b, b);
		break;
	default:
		fn(a, a, a);
		break;
	}

	for (i = 0; i < 3; i++) {
		OutputMatrix(p_output, &matrices[i]);
	}
}

typedef void (*BuildMatrixFn)(
	Matrix* p_matrix,
	MechS32 p_a,
	MechS32 p_b,
	MechS32 p_c,
	MechS32 p_x,
	MechS32 p_y,
	MechS32 p_z,
	MechU32 p_flags
);

static void HashMatrixInputs(const MechS32* p_args, AsmHash* p_hash)
{
	HashGameTables(AnyTableKind(p_args[7]), p_hash);
}

// Arguments: the three angles, the translation, the flags and the sine table (as RunSine's);
// bits 8 to 10 of the last one zero the angles, for the rotations about one axis.
static void Run1000de3b(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	GuardedMatrix matrix;
	MechS32* table = (MechS32*) Data(p_module, "g_sinTable");
	MechU32 state = AsmSeed(p_args, 8);
	MechS32 angles[3];
	MechS32 i;

	MakeTable(table, g_gameSinTable, AnyTableKind(p_args[7]), p_args[7], p_args[0]);
	MakeMatrix(&matrix, &state);
	for (i = 0; i < 3; i++) {
		angles[i] = (p_args[7] >> (8 + i)) & 1 ? 0 : p_args[i];
	}

	((BuildMatrixFn) Function(p_module, "BuildMatrixEx"))(
		&matrix.m_matrix,
		angles[0],
		angles[1],
		angles[2],
		p_args[3],
		p_args[4],
		p_args[5],
		(MechU32) p_args[6]
	);
	OutputMatrix(p_output, &matrix);
}

// A vertex with random fields and no projection.
static void MakeVertex(Vertex* p_vertex, MechU32* p_state)
{
	p_vertex->m_modelX = RandomWord(p_state);
	p_vertex->m_modelY = RandomWord(p_state);
	p_vertex->m_modelZ = RandomWord(p_state);
	p_vertex->m_worldX = RandomWord(p_state);
	p_vertex->m_worldY = RandomWord(p_state);
	p_vertex->m_worldZ = RandomWord(p_state);
	p_vertex->m_u = (MechU32) RandomWord(p_state);
	p_vertex->m_v = (MechU32) RandomWord(p_state);
	p_vertex->m_depth = (MechU32) RandomWord(p_state);
	p_vertex->m_projection = NULL;
	Fill(&p_vertex->m_flags, 1, p_state);
	Fill(p_vertex->m_unk0x29, sizeof(p_vertex->m_unk0x29), p_state);
}

// The places a case's pointers may point to: a pointer is output as its region's index (in the
// top byte) and its offset, NULL as 0xffffffff and anything else as 0xfffffffe. Pointers differ
// between modules and platforms; these don't.
typedef struct Region {
	const void* m_start;
	MechU32 m_size;
} Region;

static MechU32 Locate(const Region* p_regions, MechS32 p_count, const void* p_pointer)
{
	MechS32 i;

	if (!p_pointer) {
		return 0xffffffff;
	}

	for (i = 0; i < p_count; i++) {
		const MechU8* start = (const MechU8*) p_regions[i].m_start;

		if ((const MechU8*) p_pointer >= start && (const MechU8*) p_pointer < start + p_regions[i].m_size) {
			return ((MechU32) i << 24) | (MechU32) ((const MechU8*) p_pointer - start);
		}
	}

	return 0xfffffffe;
}

static void OutputVertex(AsmOutput* p_output, const Vertex* p_vertex, const Region* p_regions, MechS32 p_count)
{
	AsmOutputWord(p_output, (MechU32) p_vertex->m_modelX);
	AsmOutputWord(p_output, (MechU32) p_vertex->m_modelY);
	AsmOutputWord(p_output, (MechU32) p_vertex->m_modelZ);
	AsmOutputWord(p_output, (MechU32) p_vertex->m_worldX);
	AsmOutputWord(p_output, (MechU32) p_vertex->m_worldY);
	AsmOutputWord(p_output, (MechU32) p_vertex->m_worldZ);
	AsmOutputWord(p_output, p_vertex->m_u);
	AsmOutputWord(p_output, p_vertex->m_v);
	AsmOutputWord(p_output, p_vertex->m_depth);
	AsmOutputWord(p_output, Locate(p_regions, p_count, p_vertex->m_projection));
	AsmOutputWord(
		p_output,
		p_vertex->m_flags | ((MechU32) p_vertex->m_unk0x29[0] << 8) | ((MechU32) p_vertex->m_unk0x29[1] << 16) |
			((MechU32) p_vertex->m_unk0x29[2] << 24)
	);
}

static void MakeFace(Face* p_face, MechU32* p_state)
{
	p_face->m_color = (MechU16) AsmNext(p_state);
	p_face->m_indexCount = (MechU16) AsmNext(p_state);
	p_face->m_indexOffset = AsmNext(p_state);
	p_face->m_modelNormalX = RandomWord(p_state);
	p_face->m_modelNormalY = RandomWord(p_state);
	p_face->m_modelNormalZ = RandomWord(p_state);
	p_face->m_normal[0] = RandomWord(p_state);
	p_face->m_normal[1] = RandomWord(p_state);
	p_face->m_normal[2] = RandomWord(p_state);
	p_face->m_shape = NULL;
}

static void OutputFace(AsmOutput* p_output, const Face* p_face)
{
	AsmOutputWord(p_output, p_face->m_color | ((MechU32) p_face->m_indexCount << 16));
	AsmOutputWord(p_output, p_face->m_indexOffset);
	AsmOutputWord(p_output, (MechU32) p_face->m_modelNormalX);
	AsmOutputWord(p_output, (MechU32) p_face->m_modelNormalY);
	AsmOutputWord(p_output, (MechU32) p_face->m_modelNormalZ);
	OutputWords(p_output, p_face->m_normal, 3);
	AsmOutputWord(p_output, p_face->m_shape ? 1 : 0);
}

#define MODEL_VERTICES_MAX 6
#define MODEL_FACES_MAX 6

// A model as the game lays it out: the header, the vertices right after it and the faces at an
// offset, here after a vertex past the last one, and with a face past the last one.
typedef struct ModelBuffer {
	Model m_header;
	Vertex m_vertices[MODEL_VERTICES_MAX + 1];
	Face m_faces[MODEL_FACES_MAX + 1];
} ModelBuffer;

typedef void (*TransformModelFn)(Model* p_model, Matrix* p_matrix);

// Arguments: the vertex count (1 to MODEL_VERTICES_MAX), the face count (1 to MODEL_FACES_MAX),
// and the seed of the rest. A count of 0 runs 0x10000 times.
static void Run10039a30(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	ModelBuffer model;
	GuardedMatrix matrix;
	MechU32 state = AsmSeed(p_args, 3);
	MechS32 i;

	model.m_header.m_key = PortableS32(AsmNext(&state));
	model.m_header.m_vertexCount = (MechS16) (1 + (MechU32) p_args[0] % MODEL_VERTICES_MAX);
	model.m_header.m_faceCount = (MechS16) (1 + (MechU32) p_args[1] % MODEL_FACES_MAX);
	model.m_header.m_faceOffset = (MechU32) offsetof(ModelBuffer, m_faces);
	model.m_header.m_next = NULL;
	model.m_header.m_transformCount = AsmNext(&state);
	model.m_header.m_unk0x14 = (MechU16) AsmNext(&state);
	Fill(model.m_header.m_unk0x16, sizeof(model.m_header.m_unk0x16), &state);
	for (i = 0; i <= MODEL_VERTICES_MAX; i++) {
		MakeVertex(&model.m_vertices[i], &state);
	}

	for (i = 0; i <= MODEL_FACES_MAX; i++) {
		MakeFace(&model.m_faces[i], &state);
	}

	MakeMatrix(&matrix, &state);
	((TransformModelFn) Function(p_module, "TransformModel"))(&model.m_header, &matrix.m_matrix);

	AsmOutputWord(p_output, (MechU32) model.m_header.m_key);
	AsmOutputWord(
		p_output,
		(MechU16) model.m_header.m_vertexCount | ((MechU32) (MechU16) model.m_header.m_faceCount << 16)
	);
	AsmOutputWord(p_output, model.m_header.m_faceOffset == offsetof(ModelBuffer, m_faces) && !model.m_header.m_next);
	AsmOutputWord(p_output, model.m_header.m_transformCount);
	AsmOutputWord(
		p_output,
		model.m_header.m_unk0x14 | ((MechU32) model.m_header.m_unk0x16[0] << 16) |
			((MechU32) model.m_header.m_unk0x16[1] << 24)
	);
	for (i = 0; i <= MODEL_VERTICES_MAX; i++) {
		OutputVertex(p_output, &model.m_vertices[i], NULL, 0);
	}

	for (i = 0; i <= MODEL_FACES_MAX; i++) {
		OutputFace(p_output, &model.m_faces[i]);
	}

	OutputMatrix(p_output, &matrix);
}

// A shape with random fields and no lists.
static void MakeShape(Shape* p_shape, MechU32* p_state)
{
	p_shape->m_flags = (MechU16) AsmNext(p_state);
	p_shape->m_kind = (MechU16) AsmNext(p_state);
	p_shape->m_prev = NULL;
	p_shape->m_next = NULL;
	p_shape->m_prevCollider = NULL;
	p_shape->m_nextCollider = NULL;
	p_shape->m_owner = (MechU16) AsmNext(p_state);
	p_shape->m_partId = (MechU16) AsmNext(p_state);
	p_shape->m_object = NULL;
	p_shape->m_models = NULL;
	p_shape->m_model = NULL;
	p_shape->m_collisionType = RandomWord(p_state);
	p_shape->m_modelCenterX = RandomWord(p_state);
	p_shape->m_modelCenterY = RandomWord(p_state);
	p_shape->m_modelCenterZ = RandomWord(p_state);
	p_shape->m_centerX = RandomWord(p_state);
	p_shape->m_centerY = RandomWord(p_state);
	p_shape->m_centerZ = RandomWord(p_state);
	p_shape->m_radius = RandomWord(p_state);
	p_shape->m_collisionData = NULL;
	p_shape->m_transformCount = (MechU32) RandomWord(p_state);
}

static void OutputShape(AsmOutput* p_output, const Shape* p_shape)
{
	AsmOutputWord(p_output, p_shape->m_flags | ((MechU32) p_shape->m_kind << 16));
	AsmOutputWord(p_output, p_shape->m_owner | ((MechU32) p_shape->m_partId << 16));
	AsmOutputWord(p_output, (MechU32) p_shape->m_collisionType);
	AsmOutputWord(p_output, (MechU32) p_shape->m_modelCenterX);
	AsmOutputWord(p_output, (MechU32) p_shape->m_modelCenterY);
	AsmOutputWord(p_output, (MechU32) p_shape->m_modelCenterZ);
	AsmOutputWord(p_output, (MechU32) p_shape->m_centerX);
	AsmOutputWord(p_output, (MechU32) p_shape->m_centerY);
	AsmOutputWord(p_output, (MechU32) p_shape->m_centerZ);
	AsmOutputWord(p_output, (MechU32) p_shape->m_radius);
	AsmOutputWord(p_output, p_shape->m_transformCount);
}

typedef void (*TransformShapeFn)(Shape* p_shape, Matrix* p_matrix);

// Arguments: the shape's position, and the seed of the rest.
static void Run10039b94(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Shape shape;
	GuardedMatrix matrix;
	MechU32 state = AsmSeed(p_args, 4);

	MakeShape(&shape, &state);
	MakeMatrix(&matrix, &state);
	shape.m_modelCenterX = p_args[0];
	shape.m_modelCenterY = p_args[1];
	shape.m_modelCenterZ = p_args[2];
	((TransformShapeFn) Function(p_module, "TransformShapeCenter"))(&shape, &matrix.m_matrix);
	OutputShape(p_output, &shape);
	OutputMatrix(p_output, &matrix);
}

static MechS32 Domain10039c96(const MechS32* p_args)
{
	MechU64 sum =
		(MechU64) ((MechS64) p_args[0] * p_args[4]) + (MechU64) ((MechS64) p_args[2] * p_args[5]) + (MechU32) p_args[3];

	return DivideDomain(PortableS64(sum), p_args[1]);
}

// A coordinate near p_point (the shape's center near the point the routines measure from): within
// p_radius scaled by 5/4 (to reach past it), on it, anywhere, or the point itself. A radius that
// isn't positive is taken as 0x100, but for the offset on it, which is the radius itself.
static MechS32 NearValue(MechS32 p_point, MechS32 p_radius, MechU32 p_kind, MechU32* p_state)
{
	MechU32 bits = AsmNext(p_state);
	MechS64 range = p_radius > 0 ? p_radius : 0x100;
	MechS64 offset;

	switch (p_kind % 4) {
	case 0:
		offset = (MechS64) (((MechU64) (bits >> 2) * (MechU64) (range * 5 / 2 + 1)) >> 30) - range * 5 / 4;
		break;
	case 1:
		offset = p_radius > 0 ? (bits & 1 ? range : -range) : p_radius;
		break;
	case 2:
		return PortableS32(bits);
	default:
		offset = 0;
		break;
	}

	return PortableS32((MechU32) p_point + (MechU32) (MechU64) offset);
}

typedef MechS32 (*ShapeDistanceFn)(Shape* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z);

// Arguments: the point, the shape's radius, and where its center is (NearValue, from bits 0-5).
static void Run10039ccc(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Shape shape;
	MechU32 state = AsmSeed(p_args, 5);
	MechS32 result;

	MakeShape(&shape, &state);
	shape.m_radius = p_args[3];
	shape.m_centerX = NearValue(p_args[0], p_args[3], (MechU32) p_args[4], &state);
	shape.m_centerY = NearValue(p_args[1], p_args[3], (MechU32) p_args[4] >> 2, &state);
	shape.m_centerZ = NearValue(p_args[2], p_args[3], (MechU32) p_args[4] >> 4, &state);
	result =
		((ShapeDistanceFn) Function(p_module, "ApproximateShapeDistance"))(&shape, p_args[0], p_args[1], p_args[2]);
	AsmOutputWord(p_output, (MechU32) result);
	OutputShape(p_output, &shape);
}

#define SQRT_TABLE_SIZE 0x1000

static MechU32 IntegerSqrt(MechU32 p_value)
{
	MechU32 root = 0;
	MechU32 bit = (MechU32) 1 << 30;

	while (bit > p_value) {
		bit >>= 2;
	}

	while (bit) {
		if (p_value >= root + bit) {
			p_value -= root + bit;
			root = (root >> 1) + bit;
		}
		else {
			root >>= 1;
		}

		bit >>= 2;
	}

	return root;
}

// The square root table g_sqrtTable points to: the game's (sqrt(i) * 1024, past its 0x400 entries
// too), random words, or small values with zeros (divisions that fault).
static void MakeSqrtTable(MechS16* p_table, MechU32* p_state)
{
	MechU32 kind = AsmNext(p_state) % 8;
	MechS32 i;

	for (i = 0; i < SQRT_TABLE_SIZE; i++) {
		if (kind < 6) {
			p_table[i] = PortableS16((MechU16) IntegerSqrt((MechU32) i << 20));
		}
		else if (kind == 6) {
			p_table[i] = PortableS16((MechU16) AsmNext(p_state));
		}
		else {
			p_table[i] = (MechS16) (AsmNext(p_state) % 4);
		}
	}
}

// A normal case: the triangle (the arguments; one case in eight with two corners the same, one
// in eight with the corners in a line) and the square root table.
static void MakeNormalCase(const MechS32* p_args, MechS32* p_corners, MechS16* p_table)
{
	MechU32 state = AsmSeed(p_args, 9);
	MechU32 kind = AsmNext(&state) % 8;
	MechS32 i;

	memcpy(p_corners, p_args, 9 * sizeof(MechS32));
	if (kind == 0) {
		memcpy(p_corners + 6, p_corners + 3, 3 * sizeof(MechS32));
	}
	else if (kind == 1) {
		for (i = 0; i < 3; i++) {
			MechU32 step = (MechU32) p_corners[3 + i] - (MechU32) p_corners[i];

			p_corners[6 + i] = PortableS32((MechU32) p_corners[3 + i] + step);
		}
	}

	MakeSqrtTable(p_table, &state);
}

// ComputeTriangleNormal's scaled normal (its words before the division), and the divisor; returns 0 for
// a degenerate triangle.
static MechS32 ScaledNormal(const MechS32* p_c, const MechS16* p_table, MechU32* p_values, MechS32* p_divisor)
{
	MechS64 normal[3];
	MechU32 high = 0;
	MechU32 low = 0;
	MechU32 squares = 0;
	MechS32 scale;
	MechS32 i;

	normal[0] = (MechS64) Difference(p_c[4], p_c[1]) * Difference(p_c[8], p_c[5]) -
				(MechS64) Difference(p_c[7], p_c[4]) * Difference(p_c[5], p_c[2]);
	normal[1] = (MechS64) Difference(p_c[5], p_c[2]) * Difference(p_c[6], p_c[3]) -
				(MechS64) Difference(p_c[8], p_c[5]) * Difference(p_c[3], p_c[0]);
	normal[2] = (MechS64) Difference(p_c[3], p_c[0]) * Difference(p_c[7], p_c[4]) -
				(MechS64) Difference(p_c[6], p_c[3]) * Difference(p_c[4], p_c[1]);
	for (i = 0; i < 3; i++) {
		MechU64 magnitude = (MechU64) (normal[i] < 0 ? -normal[i] : normal[i]);

		high |= (MechU32) (magnitude >> 32);
		low |= (MechU32) magnitude;
	}

	if (!high && !low) {
		return 0;
	}

	for (i = 0; i < 3; i++) {
		if (high) {
			scale = PortableBsr(high) + 3;
			p_values[i] = (MechU32) ((MechU64) normal[i] >> (scale & 31));
		}
		else {
			scale = 29 - PortableBsr(low);
			p_values[i] = scale < 0 ? (MechU32) ((MechU64) normal[i] >> -scale) : (MechU32) normal[i] << scale;
		}

		squares += (MechU32) (PortableS16((MechU16) (p_values[i] >> 16)) * PortableS16((MechU16) (p_values[i] >> 16)));
	}

	*p_divisor = (MechU16) p_table[squares >> 20];
	return 1;
}

static MechS32 Domain10039dda(const MechS32* p_args)
{
	MechS32 corners[9];
	MechS16 table[SQRT_TABLE_SIZE];
	MechU32 values[3];
	MechS32 divisor;
	MechS32 i;

	MakeNormalCase(p_args, corners, table);
	if (!ScaledNormal(corners, table, values, &divisor)) {
		return c_domainIn;
	}

	for (i = 0; i < 3; i++) {
		if (!Divides((MechS64) PortableS32(values[i]) * 0x2000, divisor)) {
			return c_domainFault;
		}
	}

	return c_domainIn;
}

typedef MechS32 (*TriangleNormalFn)(
	MechS32 p_x0,
	MechS32 p_y0,
	MechS32 p_z0,
	MechS32 p_x1,
	MechS32 p_y1,
	MechS32 p_z1,
	MechS32 p_x2,
	MechS32 p_y2,
	MechS32 p_z2,
	MechS32* p_nx,
	MechS32* p_ny,
	MechS32* p_nz
);

// Arguments: the triangle's corners (MakeNormalCase).
static void Run10039dda(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	MechS32 corners[9];
	MechS16 table[SQRT_TABLE_SIZE];
	MechS32 cells[7];
	MechU32 state = AsmSeed(p_args, 9) ^ 0x5bd1e995;
	MechS32 result;

	MakeNormalCase(p_args, corners, table);
	*(MechS16**) Data(p_module, "g_sqrtTable") = table;
	FillWords(cells, 7, &state);
	result = ((TriangleNormalFn) Function(p_module, "ComputeTriangleNormal"))(
		corners[0],
		corners[1],
		corners[2],
		corners[3],
		corners[4],
		corners[5],
		corners[6],
		corners[7],
		corners[8],
		&cells[1],
		&cells[3],
		&cells[5]
	);
	AsmOutputWord(p_output, (MechU32) result);
	OutputWords(p_output, cells, 7);
}

static MechS32 Domain1003a05d(const MechS32* p_args)
{
	MechS32 divisor = Difference(p_args[0], p_args[1]);

	return divisor ? DivideDomain((MechS64) p_args[2] * 0x20000, divisor) : c_domainIn;
}

// A ray case: the ray from its start (arguments 0-2) along its delta (3-5) with its length (7),
// and the shape's radius (6) and center (NearValue, from bits 3-8 of argument 8). One ray in
// eight has no length yet (bits 0-2), which GetRayLength computes from the delta: its divisions
// can't fault with the delta shifted right by 4.
static void MakeRayCase(const MechS32* p_args, Shape* p_shape, Ray* p_ray)
{
	MechU32 state = AsmSeed(p_args, 9);
	MechU32 selector = (MechU32) p_args[8];

	MakeShape(p_shape, &state);
	p_ray->m_x1 = RandomWord(&state);
	p_ray->m_y1 = RandomWord(&state);
	p_ray->m_z1 = RandomWord(&state);
	p_ray->m_dirX = RandomWord(&state);
	p_ray->m_dirY = RandomWord(&state);
	p_ray->m_dirZ = RandomWord(&state);
	p_ray->m_x0 = p_args[0];
	p_ray->m_y0 = p_args[1];
	p_ray->m_z0 = p_args[2];
	p_ray->m_dx = p_args[3];
	p_ray->m_dy = p_args[4];
	p_ray->m_dz = p_args[5];
	p_ray->m_length = p_args[7];
	p_ray->m_state = (MechS32) (1 + AsmNext(&state) % 2);
	if (!(selector & 7)) {
		p_ray->m_state = c_rayNone;
		p_ray->m_dx = PortableSar32(p_ray->m_dx, 4);
		p_ray->m_dy = PortableSar32(p_ray->m_dy, 4);
		p_ray->m_dz = PortableSar32(p_ray->m_dz, 4);
	}

	p_shape->m_radius = p_args[6];
	p_shape->m_centerX = NearValue(p_args[0], p_args[6], selector >> 3, &state);
	p_shape->m_centerY = NearValue(p_args[1], p_args[6], selector >> 5, &state);
	p_shape->m_centerZ = NearValue(p_args[2], p_args[6], selector >> 7, &state);
}

static MechU64 Square(MechS32 p_value)
{
	return (MechU64) ((MechS64) p_value * p_value);
}

// RayShapeDistance up to its idiv.
static MechS32 Domain1003a096(const MechS32* p_args)
{
	Shape shape;
	Ray ray;
	MechS32 deltaX;
	MechS32 deltaY;
	MechS32 deltaZ;
	MechU64 partial;
	MechU64 dot;

	MakeRayCase(p_args, &shape, &ray);
	if (shape.m_radius <= 0) {
		return c_domainIn;
	}

	deltaX = Difference(shape.m_centerX, ray.m_x0);
	deltaY = Difference(shape.m_centerY, ray.m_y0);
	deltaZ = Difference(shape.m_centerZ, ray.m_z0);
	if (Square(deltaX) + Square(deltaY) + Square(deltaZ) < Square(shape.m_radius)) {
		return c_domainIn;
	}

	partial = (MechU64) ((MechS64) deltaX * ray.m_dx) + (MechU64) ((MechS64) deltaY * ray.m_dy);
	dot = partial + (MechU64) ((MechS64) deltaZ * ray.m_dz);
	if (dot < partial) {
		return c_domainIn;
	}

	return DivideDomain(PortableS64(dot), ray.m_length);
}

static void OutputRay(AsmOutput* p_output, const Ray* p_ray)
{
	AsmOutputWord(p_output, (MechU32) p_ray->m_x0);
	AsmOutputWord(p_output, (MechU32) p_ray->m_y0);
	AsmOutputWord(p_output, (MechU32) p_ray->m_z0);
	AsmOutputWord(p_output, (MechU32) p_ray->m_x1);
	AsmOutputWord(p_output, (MechU32) p_ray->m_y1);
	AsmOutputWord(p_output, (MechU32) p_ray->m_z1);
	AsmOutputWord(p_output, (MechU32) p_ray->m_dx);
	AsmOutputWord(p_output, (MechU32) p_ray->m_dy);
	AsmOutputWord(p_output, (MechU32) p_ray->m_dz);
	AsmOutputWord(p_output, (MechU32) p_ray->m_dirX);
	AsmOutputWord(p_output, (MechU32) p_ray->m_dirY);
	AsmOutputWord(p_output, (MechU32) p_ray->m_dirZ);
	AsmOutputWord(p_output, (MechU32) p_ray->m_length);
	AsmOutputWord(p_output, (MechU32) p_ray->m_state);
}

typedef MechS32 (*RayDistanceFn)(Shape* p_shape, Ray* p_ray);

// Arguments: MakeRayCase's.
static void Run1003a096(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Shape shape;
	Ray ray;
	MechS32 result;

	MakeRayCase(p_args, &shape, &ray);
	result = ((RayDistanceFn) Function(p_module, "RayShapeDistance"))(&shape, &ray);
	AsmOutputWord(p_output, (MechU32) result);
	OutputShape(p_output, &shape);
	OutputRay(p_output, &ray);
}

// --- Projection and clipping (objectanim.c) ---

// The view globals (view.c) the routines read.
enum {
	c_viewNear,                      // g_viewNear
	c_viewShiftX,                    // g_viewShiftX
	c_viewShiftY,                    // g_viewShiftY
	c_viewFar,                       // g_viewFar
	c_viewLeft,                      // g_viewLeft
	c_viewCenterX,                   // g_viewCenterX
	c_viewBottom,                    // g_viewBottom
	c_viewRight,                     // g_viewRight
	c_viewTop,                       // g_viewTop
	c_viewCenterY,                   // g_viewCenterY
	c_viewRowX,                      // g_viewProjX0, 868, 86c
	c_viewRowY = c_viewRowX + 3,     // g_viewProjY0, 874, 878
	c_viewRowDepth = c_viewRowY + 3, // g_viewProjZ0, 880, 884
	c_viewEyeY = c_viewRowDepth + 3, // g_viewEyeY
	c_viewEyeX,                      // g_viewEyeX
	c_viewEyeZ,                      // g_viewEyeZ
	c_viewLightZ,                    // g_viewLightZ
	c_viewLightX,                    // g_viewLightX
	c_viewLightY,                    // g_viewLightY
	c_viewCount
};

static const char* const g_viewNames[c_viewCount] = {
	"g_viewNear",   "g_viewShiftX", "g_viewShiftY",  "g_viewFar",    "g_viewLeft",   "g_viewCenterX", "g_viewBottom",
	"g_viewRight",  "g_viewTop",    "g_viewCenterY", "g_viewProjX0", "g_viewProjX1", "g_viewProjX2",  "g_viewProjY0",
	"g_viewProjY1", "g_viewProjY2", "g_viewProjZ0",  "g_viewProjZ1", "g_viewProjZ2", "g_viewEyeY",    "g_viewEyeX",
	"g_viewEyeZ",   "g_viewLightZ", "g_viewLightX",  "g_viewLightY",
};

static void MakeView(MechS32* p_view, MechU32* p_state)
{
	MechS32 i;

	for (i = 0; i < c_viewCount; i++) {
		p_view[i] = RandomWord(p_state);
	}
}

static void SetView(const AsmModule* p_module, const MechS32* p_view)
{
	MechS32 i;

	for (i = 0; i < c_viewCount; i++) {
		*(MechS32*) Data(p_module, g_viewNames[i]) = p_view[i];
	}
}

// GetViewVertex's view-space x, y (p_row c_viewRowX, c_viewRowY) or depth (c_viewRowDepth) of a
// vertex.
static MechS32 ViewValue(const MechS32* p_view, MechS32 p_row, const Vertex* p_vertex)
{
	MechU64 sum = (MechU64) ((MechS64) p_view[p_row] * Difference(p_vertex->m_worldX, p_view[c_viewEyeX])) +
				  (MechU64) ((MechS64) p_view[p_row + 1] * Difference(p_vertex->m_worldY, p_view[c_viewEyeY])) +
				  (MechU64) ((MechS64) p_view[p_row + 2] * Difference(p_vertex->m_worldZ, p_view[c_viewEyeZ]));

	return PortableS32(PortableShrdRound(sum, 27));
}

#define RECORD_ARENA_SIZE 0x600

// The two record stacks' buffer (recordstacks.c): the projected vertices from the top, the
// polygons from the bottom.
typedef struct RecordStacks {
	MechU32 m_words[RECORD_ARENA_SIZE / 4];
	MechU8** m_top;
	MechU8** m_bottom;
	MechS32* m_flag;
} RecordStacks;

// p_top and p_bottom are dword offsets in the buffer.
static void SetUpRecords(
	const AsmModule* p_module,
	RecordStacks* p_stacks,
	MechU32 p_top,
	MechU32 p_bottom,
	MechU32* p_state
)
{
	Fill((MechU8*) p_stacks->m_words, RECORD_ARENA_SIZE, p_state);
	p_stacks->m_top = (MechU8**) Data(p_module, "g_drawBufferTop");
	p_stacks->m_bottom = (MechU8**) Data(p_module, "g_drawBufferBottom");
	p_stacks->m_flag = (MechS32*) Data(p_module, "g_queueHasRoom");
	*p_stacks->m_top = (MechU8*) p_stacks->m_words + p_top;
	*p_stacks->m_bottom = (MechU8*) p_stacks->m_words + p_bottom;
	*p_stacks->m_flag = PortableS32(AsmNext(p_state));
}

static void OutputRecords(AsmOutput* p_output, const RecordStacks* p_stacks, const Region* p_regions, MechS32 p_count)
{
	MechS32 i;

	AsmOutputWord(p_output, Locate(p_regions, p_count, *p_stacks->m_top));
	AsmOutputWord(p_output, Locate(p_regions, p_count, *p_stacks->m_bottom));
	AsmOutputWord(p_output, (MechU32) *p_stacks->m_flag);
	for (i = 0; i < RECORD_ARENA_SIZE / 4; i++) {
		AsmOutputWord(p_output, p_stacks->m_words[i]);
	}
}

static void FillRecords(ProjectedVertex* p_records, MechS32 p_count, MechU32* p_state)
{
	Fill((MechU8*) p_records, p_count * (MechU32) sizeof(ProjectedVertex), p_state);
}

static void OutputRecord(AsmOutput* p_output, const ProjectedVertex* p_record)
{
	AsmOutputWord(p_output, (MechU32) p_record->m_x);
	AsmOutputWord(p_output, (MechU32) p_record->m_y);
	AsmOutputWord(p_output, (MechU32) p_record->m_z);
	AsmOutputWord(p_output, (MechU32) p_record->m_screenX);
	AsmOutputWord(p_output, (MechU32) p_record->m_screenY);
	AsmOutputWord(p_output, (MechU32) p_record->m_u);
	AsmOutputWord(p_output, (MechU32) p_record->m_v);
	AsmOutputWord(
		p_output,
		p_record->m_outcode | ((MechU32) p_record->m_projected << 8) | ((MechU32) p_record->m_unk0x1e[0] << 16) |
			((MechU32) p_record->m_unk0x1e[1] << 24)
	);
}

typedef ProjectedVertex* (*GetViewVertexFn)(Vertex* p_vertex);

// Arguments: the vertex's position, and whether it has a projected copy already (one case in
// four, from the last).
static void Run10048c50(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RecordStacks stacks;
	Vertex vertex;
	ProjectedVertex copy;
	MechS32 view[c_viewCount];
	Region regions[2];
	MechU32 state = AsmSeed(p_args, 4);
	MechU32 top = 0x20 + AsmNext(&state) % ((RECORD_ARENA_SIZE - 0x20) / 4 + 1) * 4;
	ProjectedVertex* result;

	MakeView(view, &state);
	SetView(p_module, view);
	SetUpRecords(p_module, &stacks, top, AsmNext(&state) % (top / 4) * 4, &state);
	FillRecords(&copy, 1, &state);
	MakeVertex(&vertex, &state);
	vertex.m_worldX = p_args[0];
	vertex.m_worldY = p_args[1];
	vertex.m_worldZ = p_args[2];
	if (!(p_args[3] & 3)) {
		vertex.m_projection = &copy;
	}

	result = ((GetViewVertexFn) Function(p_module, "GetViewVertex"))(&vertex);
	regions[0].m_start = stacks.m_words;
	regions[0].m_size = RECORD_ARENA_SIZE;
	regions[1].m_start = &copy;
	regions[1].m_size = sizeof(copy);
	AsmOutputWord(p_output, Locate(regions, 2, result));
	OutputVertex(p_output, &vertex, regions, 2);
	OutputRecord(p_output, &copy);
	OutputRecords(p_output, &stacks, regions, 2);
}

// An end of an edge ClipEdgeToNearPlane clips: its vertex, and the projected copy it may have already.
typedef struct ClipEnd {
	Vertex m_vertex;
	ProjectedVertex m_copy;
	MechS32 m_hasCopy;
} ClipEnd;

// A clipping case: the edge's ends, whether it's one vertex twice (bit 2 of argument 7), and the
// view. The ends' depths are arguments 0 and 1, the near plane argument 2, their x arguments 3 and
// 4; whether they have a copy, bits 0 and 1 of argument 7.
typedef struct ClipCase {
	ClipEnd m_ends[2];
	MechS32 m_same;
	MechS32 m_view[c_viewCount];
} ClipCase;

static void MakeClipCase(const MechS32* p_args, ClipCase* p_case)
{
	MechU32 state = AsmSeed(p_args, 9);
	MechS32 i;

	MakeView(p_case->m_view, &state);
	p_case->m_view[c_viewNear] = p_args[2];
	for (i = 0; i < 2; i++) {
		ClipEnd* end = &p_case->m_ends[i];

		MakeVertex(&end->m_vertex, &state);
		FillRecords(&end->m_copy, 1, &state);
		end->m_hasCopy = (p_args[7] >> i) & 1;
		if (end->m_hasCopy) {
			end->m_copy.m_z = p_args[i];
			end->m_copy.m_x = p_args[3 + i];
		}
		else {
			end->m_vertex.m_depth = (MechU32) p_args[i];
		}
	}

	p_case->m_same = (p_args[7] >> 2) & 1;
}

// An end's position, depth and texture coordinates, as ClipEdgeToNearPlane reads them.
static void ClipEndValues(const ClipCase* p_case, MechS32 p_end, MechS32* p_values)
{
	const ClipEnd* end = &p_case->m_ends[p_case->m_same ? 0 : p_end];

	if (end->m_hasCopy) {
		p_values[0] = end->m_copy.m_x;
		p_values[1] = end->m_copy.m_y;
		p_values[2] = end->m_copy.m_u;
		p_values[3] = end->m_copy.m_v;
		p_values[4] = end->m_copy.m_z;
	}
	else {
		p_values[0] = ViewValue(p_case->m_view, c_viewRowX, &end->m_vertex);
		p_values[1] = ViewValue(p_case->m_view, c_viewRowY, &end->m_vertex);
		p_values[2] = PortableS32(end->m_vertex.m_u << 16);
		p_values[3] = PortableS32(end->m_vertex.m_v << 16);
		p_values[4] = PortableS32(end->m_vertex.m_depth);
	}
}

// The interpolation's four divisions, from the nearer end.
static MechS32 Domain10048d46(const MechS32* p_args)
{
	ClipCase clip;
	MechS32 a[5];
	MechS32 b[5];
	MechS32* from;
	MechS32* to;
	MechS32 span;
	MechS32 toPlane;
	MechS32 i;

	MakeClipCase(p_args, &clip);
	ClipEndValues(&clip, 0, a);
	ClipEndValues(&clip, 1, b);
	from = b[4] <= a[4] ? a : b;
	to = b[4] <= a[4] ? b : a;
	span = Difference(from[4], to[4]);
	if (!span) {
		return c_domainIn;
	}

	toPlane = Difference(clip.m_view[c_viewNear], to[4]);
	for (i = 0; i < 4; i++) {
		if (!Divides((MechS64) Difference(from[i], to[i]) * toPlane, span)) {
			return c_domainFault;
		}
	}

	return c_domainIn;
}

typedef ProjectedVertex* (*ClipEdgeFn)(Vertex* p_a, Vertex* p_b);

// Arguments: MakeClipCase's (arguments 5 and 6 are unused).
static void Run10048d46(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	ClipCase clip;
	RecordStacks stacks;
	Region regions[3];
	MechU32 state = AsmSeed(p_args, 9) ^ 0x5bd1e995;
	MechU32 bottom;
	ProjectedVertex* result;
	MechS32 i;

	MakeClipCase(p_args, &clip);
	SetView(p_module, clip.m_view);
	bottom = AsmNext(&state) % 0x40 * 4;
	SetUpRecords(p_module, &stacks, RECORD_ARENA_SIZE - AsmNext(&state) % 0x40 * 4, bottom, &state);
	for (i = 0; i < 2; i++) {
		if (clip.m_ends[i].m_hasCopy) {
			clip.m_ends[i].m_vertex.m_projection = &clip.m_ends[i].m_copy;
		}
	}

	result = ((
		ClipEdgeFn
	) Function(p_module, "ClipEdgeToNearPlane"))(&clip.m_ends[0].m_vertex, &clip.m_ends[clip.m_same ? 0 : 1].m_vertex);
	regions[0].m_start = stacks.m_words;
	regions[0].m_size = RECORD_ARENA_SIZE;
	regions[1].m_start = &clip.m_ends[0].m_copy;
	regions[1].m_size = sizeof(ProjectedVertex);
	regions[2].m_start = &clip.m_ends[1].m_copy;
	regions[2].m_size = sizeof(ProjectedVertex);
	AsmOutputWord(p_output, Locate(regions, 3, result));
	for (i = 0; i < 2; i++) {
		OutputVertex(p_output, &clip.m_ends[i].m_vertex, regions, 3);
		OutputRecord(p_output, &clip.m_ends[i].m_copy);
	}

	OutputRecords(p_output, &stacks, regions, 3);
}

// The polygon list ProjectVertex adds to, and its outcodes.
typedef struct PolygonPoints {
	MechU8* m_or;
	MechU8* m_and;
	ProjectedVertex** m_points;
	MechS32* m_count;
} PolygonPoints;

static void FindPolygonPoints(const AsmModule* p_module, PolygonPoints* p_points)
{
	p_points->m_or = (MechU8*) Data(p_module, "g_polygonOrCodes");
	p_points->m_and = (MechU8*) Data(p_module, "g_polygonAndCodes");
	p_points->m_points = (ProjectedVertex**) Data(p_module, "g_polygonPoints");
	p_points->m_count = (MechS32*) Data(p_module, "g_polygonPointCount");
}

static void OutputPolygonPoints(
	AsmOutput* p_output,
	const PolygonPoints* p_points,
	const Region* p_regions,
	MechS32 p_count
)
{
	MechS32 i;

	AsmOutputWord(p_output, *p_points->m_or | ((MechU32) *p_points->m_and << 8));
	AsmOutputWord(p_output, (MechU32) *p_points->m_count);
	for (i = 0; i < 20; i++) {
		AsmOutputWord(p_output, Locate(p_regions, p_count, p_points->m_points[i]));
	}
}

// A projection case: the record (its x, y and depth are arguments 0-2; one in four isn't
// projected yet) and the view, whose shifts are arguments 3 and 4.
static void MakeProjectCase(const MechS32* p_args, ProjectedVertex* p_record, MechS32* p_view, MechU32* p_state)
{
	MakeView(p_view, p_state);
	p_view[c_viewShiftX] = p_args[3];
	p_view[c_viewShiftY] = p_args[4];
	FillRecords(p_record, 1, p_state);
	p_record->m_x = p_args[0];
	p_record->m_y = p_args[1];
	p_record->m_z = p_args[2];
	if (AsmNext(p_state) % 4) {
		p_record->m_projected = 0;
	}
}

static MechS32 ProjectDivides(MechS32 p_value, MechS32 p_shift, MechS32 p_depth)
{
	return Divides((MechS64) p_value * ((MechS64) 1 << ((MechU32) p_shift & 31)), p_depth);
}

static MechS32 Domain10048ebe(const MechS32* p_args)
{
	ProjectedVertex record;
	MechS32 view[c_viewCount];
	MechU32 state = AsmSeed(p_args, 6);

	MakeProjectCase(p_args, &record, view, &state);
	if (record.m_projected) {
		return c_domainIn;
	}

	return ProjectDivides(record.m_x, view[c_viewShiftX], record.m_z) &&
				   ProjectDivides(record.m_y, view[c_viewShiftY], record.m_z)
			   ? c_domainIn
			   : c_domainFault;
}

typedef ProjectedVertex* (*AddPointFn)(ProjectedVertex* p_vertex);

// Arguments: MakeProjectCase's (argument 5 is unused). The list has 0 to 21 points.
static void Run10048ebe(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	ProjectedVertex record;
	ProjectedVertex others[20];
	MechS32 view[c_viewCount];
	PolygonPoints points;
	MechS32* flag = (MechS32*) Data(p_module, "g_queueHasRoom");
	Region regions[2];
	MechU32 state = AsmSeed(p_args, 6);
	ProjectedVertex* result;
	MechS32 i;

	MakeProjectCase(p_args, &record, view, &state);
	SetView(p_module, view);
	FindPolygonPoints(p_module, &points);
	*points.m_or = (MechU8) AsmNext(&state);
	*points.m_and = (MechU8) AsmNext(&state);
	*points.m_count = (MechS32) (AsmNext(&state) % 22);
	*flag = PortableS32(AsmNext(&state));
	for (i = 0; i < 20; i++) {
		points.m_points[i] = &others[i];
	}

	result = ((AddPointFn) Function(p_module, "ProjectVertex"))(&record);
	regions[0].m_start = &record;
	regions[0].m_size = sizeof(record);
	regions[1].m_start = others;
	regions[1].m_size = sizeof(others);
	AsmOutputWord(p_output, Locate(regions, 2, result));
	OutputRecord(p_output, &record);
	OutputPolygonPoints(p_output, &points, regions, 2);
	AsmOutputWord(p_output, (MechU32) *flag);
}

// A face and the indices of its vertices, at m_indexOffset from it.
typedef struct FaceBuffer {
	Face m_face;
	MechU8 m_indices[24];
} FaceBuffer;

#define SHADE_VERTICES 4

// A shading case: the face's first vertex (its position is arguments 0-2), the light (3-5; one
// case in eight on the vertex instead), the face's normal (6-8), whether the vertex is taken to be
// at the origin, and the square root table.
typedef struct ShadeCase {
	FaceBuffer m_face;
	Vertex m_vertices[SHADE_VERTICES];
	MechS32 m_view[c_viewCount];
	MechS32 m_atOrigin;
	MechS16 m_table[SQRT_TABLE_SIZE];
} ShadeCase;

static void MakeShadeCase(const MechS32* p_args, ShadeCase* p_case)
{
	MechU32 state = AsmSeed(p_args, 9);
	Vertex* vertex;
	MechS32 i;

	for (i = 0; i < SHADE_VERTICES; i++) {
		MakeVertex(&p_case->m_vertices[i], &state);
	}

	MakeFace(&p_case->m_face.m_face, &state);
	Fill(p_case->m_face.m_indices, sizeof(p_case->m_face.m_indices), &state);
	p_case->m_face.m_face.m_indexOffset = (MechU32) offsetof(FaceBuffer, m_indices);
	p_case->m_face.m_indices[0] %= SHADE_VERTICES;
	vertex = &p_case->m_vertices[p_case->m_face.m_indices[0]];
	vertex->m_worldX = p_args[0];
	vertex->m_worldY = p_args[1];
	vertex->m_worldZ = p_args[2];
	MakeView(p_case->m_view, &state);
	p_case->m_view[c_viewLightX] = p_args[3];
	p_case->m_view[c_viewLightY] = p_args[4];
	p_case->m_view[c_viewLightZ] = p_args[5];
	p_case->m_face.m_face.m_normal[0] = p_args[6];
	p_case->m_face.m_face.m_normal[1] = p_args[7];
	p_case->m_face.m_face.m_normal[2] = p_args[8];
	p_case->m_atOrigin = AsmNext(&state) % 4 == 0;
	if (AsmNext(&state) % 8 == 0) {
		p_case->m_view[c_viewLightX] = p_case->m_atOrigin ? 0 : p_args[0];
		p_case->m_view[c_viewLightY] = p_case->m_atOrigin ? 0 : p_args[1];
		p_case->m_view[c_viewLightZ] = p_case->m_atOrigin ? 0 : p_args[2];
	}

	MakeSqrtTable(p_case->m_table, &state);
}

// The magnitude GetFaceShade takes of a light coordinate's difference from the vertex's: negated
// when the light's is the smaller one.
static MechU32 ShadeMagnitude(MechS32 p_light, MechS32 p_vertex)
{
	MechU32 difference = (MechU32) p_light - (MechU32) p_vertex;

	return p_light < p_vertex ? 0 - difference : difference;
}

// GetFaceShade up to its idiv.
static MechS32 Domain10048faf(const MechS32* p_args)
{
	ShadeCase shade;
	Vertex* vertex;
	MechS32 position[3];
	MechS32 light[3];
	MechU32 magnitudes[3];
	MechU32 bits = 0;
	MechU32 squares = 0;
	MechU64 dot = 0;
	MechS32 scale;
	MechS32 i;

	MakeShadeCase(p_args, &shade);
	vertex = &shade.m_vertices[shade.m_face.m_indices[0]];
	position[0] = shade.m_atOrigin ? 0 : vertex->m_worldX;
	position[1] = shade.m_atOrigin ? 0 : vertex->m_worldY;
	position[2] = shade.m_atOrigin ? 0 : vertex->m_worldZ;
	light[0] = shade.m_view[c_viewLightX];
	light[1] = shade.m_view[c_viewLightY];
	light[2] = shade.m_view[c_viewLightZ];
	for (i = 0; i < 3; i++) {
		magnitudes[i] = ShadeMagnitude(light[i], position[i]);
		bits |= magnitudes[i];
		dot += (MechU64) ((MechS64) Difference(light[i], position[i]) * shade.m_face.m_face.m_normal[i]);
	}

	if (!bits) {
		return c_domainIn;
	}

	dot = (MechU64) PortableSar64(PortableS64(dot), 16);
	scale = PortableBsr(bits) - 7;
	for (i = 0; i < 3; i++) {
		MechU32 magnitude = scale >= 0 ? magnitudes[i] >> scale : magnitudes[i] << -scale;

		squares += (magnitude & 0xff) * (magnitude & 0xff);
	}

	dot = scale >= 0 ? (MechU64) PortableSar64(PortableS64(dot), scale) : dot << -scale;
	return DivideDomain(PortableS64(dot), shade.m_table[squares >> 8]);
}

typedef MechS32 (*GetFaceShadeFn)(Face* p_face, Vertex* p_vertices);

// Arguments: MakeShadeCase's.
static void Run10048faf(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	ShadeCase shade;
	MechS32 result;

	MakeShadeCase(p_args, &shade);
	SetView(p_module, shade.m_view);
	*(MechS32*) Data(p_module, "g_directionalLight") = shade.m_atOrigin;
	*(MechS16**) Data(p_module, "g_sqrtTable") = shade.m_table;
	result = ((GetFaceShadeFn) Function(p_module, "GetFaceShade"))(&shade.m_face.m_face, shade.m_vertices);
	AsmOutputWord(p_output, (MechU32) result);
}

// --- Queueing a face (QueueFace) ---

#define QUEUE_VERTICES 8
#define QUEUE_POLYGONS 8
#define QUEUE_LOG 64
#define POLYGON_RECORD_SIZE (0xc + 20 * 4) // a polygon and its points, on x86

// The hooks QueueFace calls through g_renderSettings, and what they're passed. The projection
// hook stands in for ProjectVertex: it adds the point to the polygon (up to 20) and ands random
// outcodes, mostly 0, into g_polygonAndCodes, so that most polygons get queued.
typedef struct QueueHooks {
	PolygonPoints m_points;
	MechS32* m_flag;
	const Region* m_regions;
	MechS32 m_regionCount;
	MechU32 m_state;
	MechU32 m_log[QUEUE_LOG];
	MechS32 m_logCount;
} QueueHooks;

static QueueHooks g_queueHooks;

static void LogHook(MechU32 p_word)
{
	if (g_queueHooks.m_logCount < QUEUE_LOG) {
		g_queueHooks.m_log[g_queueHooks.m_logCount] = p_word;
	}

	g_queueHooks.m_logCount++;
}

static ProjectedVertex* QueueProjectHook(ProjectedVertex* p_vertex)
{
	MechU32 bits = AsmNext(&g_queueHooks.m_state);
	MechU8 outcode = (MechU8) (bits % 4 ? 0 : (bits >> 8) & 0xf);
	PolygonPoints* points = &g_queueHooks.m_points;

	LogHook(Locate(g_queueHooks.m_regions, g_queueHooks.m_regionCount, p_vertex));
	*points->m_or |= outcode;
	*points->m_and &= outcode;
	if (*points->m_count >= 20) {
		*g_queueHooks.m_flag = 0;
	}
	else {
		points->m_points[(*points->m_count)++] = p_vertex;
	}

	return p_vertex;
}

static MechS32 QueueDrawHook(Face* p_face, Vertex* p_vertices, MechS32 p_flags, MechS32 p_depth)
{
	LogHook(Locate(g_queueHooks.m_regions, g_queueHooks.m_regionCount, p_face));
	LogHook(Locate(g_queueHooks.m_regions, g_queueHooks.m_regionCount, p_vertices));
	LogHook((MechU32) p_flags);
	LogHook((MechU32) p_depth);
	return PortableS32(AsmNext(&g_queueHooks.m_state));
}

// QueueFace's counters, and the polygon list's count and capacity.
static const char* const g_queueCounterNames[] = {
	"g_facesTried",
	"g_facesFrontFacing",
	"g_verticesTransformed",
	"g_polygonsQueued",
	"g_depthEntryCount",
	"g_polygonCount",
	"g_depthListCapacity",
};

#define QUEUE_COUNTERS ((MechS32) (sizeof(g_queueCounterNames) / sizeof(g_queueCounterNames[0])))

// The polygon QueueFace takes from the bottom of the record buffer, and the points it copies
// after it (its words up to the bottom's new position), with their pointers located; the rest of
// the region as it is. A polygon only fits its record on x86, the only platform this runs on.
static void OutputPolygonRecord(
	AsmOutput* p_output,
	const MechU8* p_record,
	const MechU8* p_end,
	const Region* p_regions,
	MechS32 p_count
)
{
	const QueuedPolygon* poly = (const QueuedPolygon*) p_record;
	const MechU8* cursor = p_record;

	if (p_end > p_record) {
		AsmOutputWord(p_output, (MechU16) poly->m_count | ((MechU32) poly->m_flags << 16));
		AsmOutputWord(p_output, Locate(p_regions, p_count, poly->m_face));
		AsmOutputWord(p_output, (MechU32) poly->m_depth);
		for (cursor = p_record + 0xc; cursor + sizeof(void*) <= p_end; cursor += sizeof(void*)) {
			void* pointer;

			memcpy(&pointer, cursor, sizeof(pointer));
			AsmOutputWord(p_output, Locate(p_regions, p_count, pointer));
		}
	}

	if (cursor < p_record + POLYGON_RECORD_SIZE) {
		AsmOutputBytes(p_output, cursor, (MechU32) (p_record + POLYGON_RECORD_SIZE - cursor));
	}
}

typedef void (*QueueFaceFn)(Face* p_face, Vertex* p_vertices);

// Arguments: the face's vertex count (1 to QUEUE_VERTICES), the depth rule (g_queuedShapeFlags),
// and the seed of the rest: a scene the game could draw. The near plane is at 0 or above, the
// depth row within +-1.0 (5.27) and the vertices within 2^24 of the eyepoint, so every depth is
// below 2^26; the vertices projected already have depths and clip codes that agree, and their
// copies the same depth. The clipping's divisions and the average's can't fault then. One scene
// in eight has a depth row of 0 instead, and extremes for the eyepoint, the vertices and the
// normal: the back-face test's products reach 2^62 (it tests the exact sum of a wrapped one and
// the last), and the depths are all 0.
static void Run10049155(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	FaceBuffer face;
	Vertex vertices[QUEUE_VERTICES];
	ProjectedVertex copies[QUEUE_VERTICES];
	ProjectedVertex others[20];
	DepthEntry polygons[QUEUE_POLYGONS];
	RecordStacks stacks;
	RenderSettings* hooks = (RenderSettings*) Data(p_module, "g_renderSettings");
	MechU8** cursor = (MechU8**) Data(p_module, "g_polygonPointCursor");
	MechS32* counters[QUEUE_COUNTERS];
	MechS32 view[c_viewCount];
	Region regions[6];
	MechU32 state = AsmSeed(p_args, 3);
	MechU32 bottom;
	MechS32 count;
	MechS32 wide;
	MechS32 i;

	MakeView(view, &state);
	wide = AsmNext(&state) % 8 == 0;
	view[c_viewNear] = AsmNext(&state) % 8 ? (MechS32) (AsmNext(&state) & 0xffffff) : 0;
	view[c_viewFar] =
		AsmNext(&state) % 8 ? view[c_viewNear] + (MechS32) (AsmNext(&state) & 0x3ffffff) : RandomWord(&state);
	for (i = 0; i < 3; i++) {
		view[c_viewRowDepth + i] = wide ? 0 : (MechS32) (AsmNext(&state) & 0xfffffff) - 0x8000000;
		if (wide) {
			view[c_viewEyeY + i] = ExtremeWord(&state);
		}
	}

	SetView(p_module, view);

	// The vertices: not projected yet (half), projected, or projected with a copy
	FillRecords(copies, QUEUE_VERTICES, &state);
	for (i = 0; i < QUEUE_VERTICES; i++) {
		Vertex* vertex = &vertices[i];
		MechU32 kind = AsmNext(&state) % 4;

		MakeVertex(vertex, &state);
		if (wide) {
			vertex->m_worldX = ExtremeWord(&state);
			vertex->m_worldY = ExtremeWord(&state);
			vertex->m_worldZ = ExtremeWord(&state);
		}
		else {
			vertex->m_worldX = PortableS32((MechU32) view[c_viewEyeX] + (AsmNext(&state) & 0x1ffffff) - 0x1000000);
			vertex->m_worldY = PortableS32((MechU32) view[c_viewEyeY] + (AsmNext(&state) & 0x1ffffff) - 0x1000000);
			vertex->m_worldZ = PortableS32((MechU32) view[c_viewEyeZ] + (AsmNext(&state) & 0x1ffffff) - 0x1000000);
		}

		if (kind < 2) {
			vertex->m_flags &= ~4;
		}
		else {
			MechS32 depth = (MechS32) (AsmNext(&state) % 0x4400000) - 0x400000;

			vertex->m_depth = (MechU32) depth;
			vertex->m_flags =
				(MechU8) ((vertex->m_flags & ~3) | 4 | (depth < view[c_viewNear]) | ((depth > view[c_viewFar]) << 1));
			if (kind == 3) {
				copies[i].m_z = depth;
				vertex->m_projection = &copies[i];
			}
		}
	}

	MakeFace(&face.m_face, &state);
	if (wide) {
		for (i = 0; i < 3; i++) {
			face.m_face.m_normal[i] = ExtremeWord(&state);
		}
	}

	Fill(face.m_indices, sizeof(face.m_indices), &state);
	count = 1 + (MechS32) ((MechU32) p_args[0] % QUEUE_VERTICES);
	face.m_face.m_indexCount = (MechU16) count;
	face.m_face.m_indexOffset = (MechU32) offsetof(FaceBuffer, m_indices);
	for (i = 0; i < count; i++) {
		face.m_indices[i] %= QUEUE_VERTICES;
	}

	// The record buffer, the polygon's region cleared (its pointers are output located)
	bottom = AsmNext(&state) % 0x61 * 4;
	SetUpRecords(p_module, &stacks, 0x400 + AsmNext(&state) % 0x81 * 4, bottom, &state);
	memset((MechU8*) stacks.m_words + bottom, 0, POLYGON_RECORD_SIZE);
	*cursor = NULL;

	// The polygon being built, the list of polygons, the counters
	FindPolygonPoints(p_module, &g_queueHooks.m_points);
	*g_queueHooks.m_points.m_or = (MechU8) AsmNext(&state);
	*g_queueHooks.m_points.m_and = (MechU8) AsmNext(&state);
	*g_queueHooks.m_points.m_count = (MechS32) (AsmNext(&state) % 21);
	for (i = 0; i < 20; i++) {
		g_queueHooks.m_points.m_points[i] = &others[i];
	}

	for (i = 0; i < QUEUE_POLYGONS; i++) {
		polygons[i].m_poly = NULL;
		polygons[i].m_depth = PortableS32(AsmNext(&state));
	}

	*(DepthEntry**) Data(p_module, "g_depthList") = polygons;
	*(MechU32*) Data(p_module, "g_queuedShapeFlags") = (MechU32) p_args[1];
	for (i = 0; i < QUEUE_COUNTERS; i++) {
		counters[i] = (MechS32*) Data(p_module, g_queueCounterNames[i]);
		*counters[i] = PortableS32(AsmNext(&state));
	}

	*counters[4] = (MechS32) (AsmNext(&state) % QUEUE_POLYGONS);
	*counters[6] = *counters[4] + (MechS32) (AsmNext(&state) % 3) - 1;

	// The hooks
	regions[0].m_start = stacks.m_words;
	regions[0].m_size = RECORD_ARENA_SIZE;
	regions[1].m_start = copies;
	regions[1].m_size = sizeof(copies);
	regions[2].m_start = others;
	regions[2].m_size = sizeof(others);
	regions[3].m_start = vertices;
	regions[3].m_size = sizeof(vertices);
	regions[4].m_start = &face;
	regions[4].m_size = sizeof(face);
	regions[5].m_start = polygons;
	regions[5].m_size = sizeof(polygons);
	g_queueHooks.m_flag = stacks.m_flag;
	g_queueHooks.m_regions = regions;
	g_queueHooks.m_regionCount = 6;
	g_queueHooks.m_state = AsmNext(&state);
	g_queueHooks.m_logCount = 0;
	hooks->m_projectVertex = QueueProjectHook;
	hooks->m_drawFace = QueueDrawHook;

	((QueueFaceFn) Function(p_module, "QueueFace"))(&face.m_face, vertices);

	for (i = 0; i < QUEUE_COUNTERS; i++) {
		AsmOutputWord(p_output, (MechU32) *counters[i]);
	}

	OutputPolygonPoints(p_output, &g_queueHooks.m_points, regions, 6);
	AsmOutputWord(p_output, Locate(regions, 6, *cursor));
	for (i = 0; i < QUEUE_POLYGONS; i++) {
		AsmOutputWord(p_output, Locate(regions, 6, polygons[i].m_poly));
		AsmOutputWord(p_output, (MechU32) polygons[i].m_depth);
	}

	for (i = 0; i < QUEUE_VERTICES; i++) {
		OutputVertex(p_output, &vertices[i], regions, 6);
		OutputRecord(p_output, &copies[i]);
	}

	AsmOutputWord(p_output, Locate(regions, 6, *stacks.m_top));
	AsmOutputWord(p_output, Locate(regions, 6, *stacks.m_bottom));
	AsmOutputWord(p_output, (MechU32) *stacks.m_flag);
	AsmOutputBytes(p_output, (const MechU8*) stacks.m_words, bottom);
	OutputPolygonRecord(p_output, (const MechU8*) stacks.m_words + bottom, *stacks.m_bottom, regions, 6);
	AsmOutputBytes(
		p_output,
		(const MechU8*) stacks.m_words + bottom + POLYGON_RECORD_SIZE,
		RECORD_ARENA_SIZE - bottom - POLYGON_RECORD_SIZE
	);
	AsmOutputWord(p_output, (MechU32) g_queueHooks.m_logCount);
	for (i = 0; i < g_queueHooks.m_logCount && i < QUEUE_LOG; i++) {
		AsmOutputWord(p_output, g_queueHooks.m_log[i]);
	}
}

// --- The polygon fillers (VFX3D.ASM) ---

#define POLY_BUFFER_WIDTH 48
#define POLY_BUFFER_HEIGHT 40
#define POLY_GUARD 64
#define POLY_ARENA_SIZE (POLY_GUARD * 2 + POLY_BUFFER_WIDTH * POLY_BUFFER_HEIGHT + 4)
#define POLY_MAX_VERTICES 10
#define POLY_VERTEX_WORDS 6
#define POLY_TEXTURE_SIZE 32

typedef void (*FillPolygonFn)(PANE* p_view, MechS32 p_count, MechU32* p_points);
typedef void (*FillDitheredFn)(PANE* p_view, MechS32 p_dither, MechS32 p_count, MechU32* p_points);
typedef void (*FillRemappedFn)(PANE* p_view, MechS32 p_count, MechU32* p_points, MechU8* p_table);
typedef void (*SetLumaTableFn)(MechU16* p_table);
typedef void (*FillTexturedFn)(PANE* p_view, MechS32 p_count, MechU32* p_points, WINDOW* p_source, MechS32 p_mode);

// The directions of a convex polygon's vertices from its centre, 1024 cos(2 pi k / 32): the sine
// is the cosine 8 entries on.
static const MechS32 g_polyDirections[32] = {
	1024,  1004,  946,  851,  724,  569,  392,  200,  0, -200, -392, -569, -724, -851, -946, -1004,
	-1024, -1004, -946, -851, -724, -569, -392, -200, 0, 200,  392,  569,  724,  851,  946,  1004,
};

// The pixel buffer and the view a case draws into: the buffer at an offset of 0 to 3 in an arena
// of random bytes, guard bands included, and a view over it that may reach beyond it, start
// before it, or be empty.
typedef struct PolyTarget {
	MechU32 m_words[POLY_ARENA_SIZE / 4];
	WINDOW m_buffer;
	PANE m_view;
} PolyTarget;

static MechS32 RandomRange(MechU32* p_state, MechS32 p_low, MechS32 p_high)
{
	return p_low + (MechS32) (AsmNext(p_state) % (MechU32) (p_high - p_low + 1));
}

static void MakePolyTarget(PolyTarget* p_target, MechU32* p_state)
{
	MechU8* arena = (MechU8*) p_target->m_words;
	MechS32 width = RandomRange(p_state, 1, POLY_BUFFER_WIDTH);
	MechS32 height = RandomRange(p_state, 1, POLY_BUFFER_HEIGHT);
	MechU32 kind = AsmNext(p_state);

	Fill(arena, POLY_ARENA_SIZE, p_state);
	p_target->m_buffer.m_buffer = arena + POLY_GUARD + AsmNext(p_state) % 4;
	p_target->m_buffer.m_xMax = width - 1;
	p_target->m_buffer.m_yMax = height - 1;
	p_target->m_buffer.m_bitmapInfo = NULL;
	p_target->m_buffer.m_shadow = 0;
	p_target->m_view.m_window = &p_target->m_buffer;
	if (kind % 4 == 0) {
		// The whole buffer, as the game draws
		p_target->m_view.m_x0 = 0;
		p_target->m_view.m_y0 = 0;
		p_target->m_view.m_x1 = width - 1;
		p_target->m_view.m_y1 = height - 1;
	}
	else {
		p_target->m_view.m_x0 = RandomRange(p_state, -4, width + 4);
		p_target->m_view.m_y0 = RandomRange(p_state, -4, height + 4);
		p_target->m_view.m_x1 = RandomRange(p_state, -4, width + 4);
		p_target->m_view.m_y1 = RandomRange(p_state, -4, height + 4);
	}
}

static void OutputPolyTarget(AsmOutput* p_output, const PolyTarget* p_target)
{
	AsmOutputBytes(p_output, (const MechU8*) p_target->m_words, POLY_ARENA_SIZE);
}

// p_count vertices on an ellipse, rounded down: p_count of the 32 directions, in order, in either
// winding and starting at any vertex. The words after x and y are random.
static void PlaceVertices(
	MechU32* p_points,
	MechS32 p_count,
	MechU32 p_reverse,
	MechS32 p_centerX,
	MechS32 p_centerY,
	MechS32 p_radiusX,
	MechS32 p_radiusY,
	MechU32* p_state
)
{
	MechS32 directions[POLY_MAX_VERTICES];
	MechS32 start;
	MechS32 used;
	MechS32 i;

	used = 0;
	for (i = 0; i < 32 && used < p_count; i++) {
		if (AsmNext(p_state) % (MechU32) (32 - i) < (MechU32) (p_count - used)) {
			directions[used++] = i;
		}
	}

	start = (MechS32) (AsmNext(p_state) % (MechU32) p_count);
	for (i = 0; i < p_count; i++) {
		MechS32 index = (start + i) % p_count;
		MechS32 direction = directions[p_reverse ? p_count - 1 - index : index];
		MechU32* point = &p_points[i * POLY_VERTEX_WORDS];

		FillWords((MechS32*) point, POLY_VERTEX_WORDS, p_state);
		point[0] = (MechU32) (p_centerX + PortableSar32(g_polyDirections[direction] * p_radiusX, 10));
		point[1] = (MechU32) (p_centerY + PortableSar32(g_polyDirections[(direction + 8) % 32] * p_radiusY, 10));
	}
}

// A polygon the game could draw: convex, y-monotone on each side of its top and bottom (its
// vertices lie on an ellipse, rounded down), in either winding and starting at any vertex. One
// to POLY_MAX_VERTICES vertices; a case may flatten it into a line or a point. Its corners lie
// around the target, sometimes far beyond (within +-0x3fff: the routines' divisions overflow
// for spans and edges 0x8000 pixels long). The words after x and y are random; the callers set
// the ones the routine reads. Returns the vertex count.
static MechS32 MakePolygon(MechU32* p_points, const PolyTarget* p_target, MechU32* p_state)
{
	MechS32 width = p_target->m_buffer.m_xMax + 1;
	MechS32 height = p_target->m_buffer.m_yMax + 1;
	MechU32 kind = AsmNext(p_state);
	MechS32 count = 1 + (MechS32) (AsmNext(p_state) % POLY_MAX_VERTICES);
	MechS32 centerX;
	MechS32 centerY;
	MechS32 radiusX;
	MechS32 radiusY;

	if (kind % 8 == 0) {
		// Far and large: the clipping's extremes
		centerX = RandomRange(p_state, -0x1fff, 0x1fff);
		centerY = RandomRange(p_state, -0x1fff, 0x1fff);
		radiusX = RandomRange(p_state, 0, 0x2000);
		radiusY = RandomRange(p_state, 0, 0x2000);
	}
	else {
		centerX = RandomRange(p_state, -width, width * 2);
		centerY = RandomRange(p_state, -height, height * 2);
		radiusX = RandomRange(p_state, 0, width * 2);
		radiusY = RandomRange(p_state, 0, height * 2);
	}

	// Lines and points
	if ((kind >> 3) % 16 == 0) {
		radiusX = 0;
	}
	else if ((kind >> 3) % 16 == 1) {
		radiusY = 0;
	}

	PlaceVertices(p_points, count, (kind >> 7) & 1, centerX, centerY, radiusX, radiusY, p_state);
	return count;
}

// A vertex color, 16.16: a color and its fraction, or any word (the color steps wrap).
static MechU32 RandomColor(MechU32* p_state)
{
	MechU32 kind = AsmNext(p_state);
	MechU32 bits = AsmNext(p_state);

	switch (kind % 4) {
	case 0:
		return bits;
	case 1:
		return bits & 0xff0000;
	default:
		return bits & 0xffffff;
	}
}

// The dither offset: half a color either way, as the game passes, or any word.
static MechS32 RandomDither(MechU32* p_state)
{
	MechU32 kind = AsmNext(p_state);

	switch (kind % 4) {
	case 0:
		return 0x7fff;
	case 1:
		return 0x8000;
	case 2:
		return 0;
	default:
		return PortableS32(AsmNext(p_state));
	}
}

// Arguments: four words that seed the target, the polygon and the colors.
static void RunFillPolygon(const AsmModule* p_module, const char* p_name, const MechS32* p_args, AsmOutput* p_output)
{
	PolyTarget target;
	MechU32 points[POLY_MAX_VERTICES * POLY_VERTEX_WORDS];
	MechU32 state = AsmSeed(p_args, 4);
	MechS32 count;
	MechS32 dither;
	MechS32 i;

	MakePolyTarget(&target, &state);
	count = MakePolygon(points, &target, &state);
	for (i = 0; i < count; i++) {
		points[i * POLY_VERTEX_WORDS + 2] = RandomColor(&state);
	}

	dither = RandomDither(&state);
	if (!strcmp(p_name, "VFX_dithered_Gouraud_polygon") || !strcmp(p_name, "VFX_illuminate_polygon")) {
		((FillDitheredFn) Function(p_module, p_name))(&target.m_view, dither, count, points);
	}
	else {
		((FillPolygonFn) Function(p_module, p_name))(&target.m_view, count, points);
	}

	OutputPolyTarget(p_output, &target);
}

static void RunFillPolygonFlat(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunFillPolygon(p_module, "VFX_flat_polygon", p_args, p_output);
}

static void Run1002ae41(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunFillPolygon(p_module, "VFX_Gouraud_polygon", p_args, p_output);
}

static void Run1002b68b(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunFillPolygon(p_module, "VFX_dithered_Gouraud_polygon", p_args, p_output);
}

static void Run1002c48d(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunFillPolygon(p_module, "VFX_illuminate_polygon", p_args, p_output);
}

static void Run1002bf39(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	PolyTarget target;
	MechU32 points[POLY_MAX_VERTICES * POLY_VERTEX_WORDS];
	MechU8 table[0x100];
	MechU32 state = AsmSeed(p_args, 4);
	MechS32 count;

	MakePolyTarget(&target, &state);
	count = MakePolygon(points, &target, &state);
	Fill(table, sizeof(table), &state);
	((FillRemappedFn) Function(p_module, "VFX_translate_polygon"))(&target.m_view, count, points, table);
	OutputPolyTarget(p_output, &target);
}

// Arguments: the table's seed. The table VFX_map_lookaside copies is VFX3D's own (its lookaside, not
// public): the case reads it back through VFX_map_polygon, mapping a 16 by 16 texture of the 256
// texel values one to one onto a square, in the mode that translates each texel through it.
static void RunSetLumaTable(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	MechU16 table[0x80 + 2];
	MechU8 texels[0x100];
	MechU8 pixels[0x100];
	MechU32 points[4 * POLY_VERTEX_WORDS];
	WINDOW texture;
	WINDOW window;
	PANE pane;
	MechU32 state = AsmSeed(p_args, 2);
	MechS32 i;

	Fill((MechU8*) table, sizeof(table), &state);
	for (i = 0; i < 0x100; i++) {
		texels[i] = (MechU8) i;
		pixels[i] = 0;
	}

	texture.m_buffer = texels;
	texture.m_xMax = 15;
	texture.m_yMax = 15;
	texture.m_bitmapInfo = NULL;
	texture.m_shadow = 0;
	window = texture;
	window.m_buffer = pixels;
	pane.m_window = &window;
	pane.m_x0 = 0;
	pane.m_y0 = 0;
	pane.m_x1 = 15;
	pane.m_y1 = 15;
	memset(points, 0, sizeof(points));
	for (i = 0; i < 4; i++) {
		MechU32 x = i == 1 || i == 2 ? 15 : 0;
		MechU32 y = i >= 2 ? 15 : 0;

		points[i * POLY_VERTEX_WORDS] = x;
		points[i * POLY_VERTEX_WORDS + 1] = y;
		points[i * POLY_VERTEX_WORDS + 3] = x << 16;
		points[i * POLY_VERTEX_WORDS + 4] = y << 16;
	}

	((SetLumaTableFn) Function(p_module, "VFX_map_lookaside"))(&table[1]);
	((FillTexturedFn) Function(p_module, "VFX_map_polygon"))(&pane, 4, points, &texture, 1);
	AsmOutputBytes(p_output, pixels, sizeof(pixels));
}

// A texture of 1 to 32 texels each way, with the odd transparent texel (0xff), in a buffer of
// random bytes.
typedef struct PolyTexture {
	MechU8 m_texels[POLY_TEXTURE_SIZE * POLY_TEXTURE_SIZE];
	WINDOW m_buffer;
	MechS32 m_width;
	MechS32 m_height;
} PolyTexture;

static void MakePolyTexture(PolyTexture* p_texture, MechU32* p_state)
{
	MechU32 transparency = AsmNext(p_state) % 4;
	MechU32 i;

	p_texture->m_width = RandomRange(p_state, 1, POLY_TEXTURE_SIZE);
	p_texture->m_height = RandomRange(p_state, 1, POLY_TEXTURE_SIZE);
	for (i = 0; i < sizeof(p_texture->m_texels); i++) {
		MechU32 bits = AsmNext(p_state);

		p_texture->m_texels[i] = (MechU8) (transparency && bits % (4u << transparency) == 0 ? 0xff : bits >> 8);
	}

	p_texture->m_buffer.m_buffer = p_texture->m_texels;
	p_texture->m_buffer.m_xMax = p_texture->m_width - 1;
	p_texture->m_buffer.m_yMax = p_texture->m_height - 1;
	p_texture->m_buffer.m_bitmapInfo = NULL;
	p_texture->m_buffer.m_shadow = 0;
}

// A texture coordinate, 16.16, inside the texture once the routine adds a half for rounding.
static MechU32 RandomTexel(MechU32* p_state, MechS32 p_size)
{
	return AsmNext(p_state) % (((MechU32) p_size << 16) - 0x8000);
}

// Arguments: four words that seed the target, the polygon, the texture and the luma table; the
// mode is the first's low two bits.
static void RunFillPolygonTextured(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	PolyTarget target;
	PolyTexture texture;
	MechU32 points[POLY_MAX_VERTICES * POLY_VERTEX_WORDS];
	MechU8 luma[0x100];
	MechU32 state = AsmSeed(p_args, 4);
	MechU32 lumaTransparency;
	MechS32 count;
	MechS32 i;

	MakePolyTarget(&target, &state);
	count = MakePolygon(points, &target, &state);
	MakePolyTexture(&texture, &state);
	for (i = 0; i < count; i++) {
		points[i * POLY_VERTEX_WORDS + 3] = RandomTexel(&state, texture.m_width);
		points[i * POLY_VERTEX_WORDS + 4] = RandomTexel(&state, texture.m_height);
	}

	lumaTransparency = AsmNext(&state) % 4;
	for (i = 0; i < 0x100; i++) {
		MechU32 bits = AsmNext(&state);

		luma[i] = (MechU8) (lumaTransparency && bits % (4u << lumaTransparency) == 0 ? 0xff : bits >> 8);
	}

	((SetLumaTableFn) Function(p_module, "VFX_map_lookaside"))((MechU16*) luma);
	((FillTexturedFn)
		 Function(p_module, "VFX_map_polygon"))(&target.m_view, count, points, &texture.m_buffer, p_args[0] & 3);
	OutputPolyTarget(p_output, &target);
}

// --- The polygon renderer (VFXREND.ASM) ---

#define REND_TEXTURE_SIZE 32
#define REND_TEXTURE_PAD 2
#define REND_TEXTURE_STRIDE (REND_TEXTURE_SIZE + REND_TEXTURE_PAD * 2)
#define REND_CUEING_SIZE 0x10100
#define REND_PRIMITIVES 25
#define REND_LIST_VERTICES 24
#define REND_PRIME_VERTICES 32

typedef void (*SetDitherLevelFn)(MechS32 p_dither1, MechS32 p_dither2);
typedef MechS32 (*GetCodeBlockFn)(undefined4* p_start, undefined4* p_selector);
typedef void (*RenderPolygonFn)(
	PANE* p_pane,
	MechU32* p_vlist,
	MechS32 p_nvertices,
	MechS32 p_operation,
	undefined4 p_color,
	VFX_TEXTURE* p_texture,
	void* p_cueing,
	void* p_translucency
);

// The operations VFXREND builds (RENDOPTS.INC), in its order.
static const MechS32 g_renderPrimitives[REND_PRIMITIVES] = {
	0x691, 0x611, 0x491, 0x411, 0x6a0, 0x680, 0x697, 0x689, 0x6b1, 0x690, 0x600, 0x610, 0x650,
	0x497, 0x490, 0x400, 0x410, 0x408, 0x450, 0x080, 0x280, 0x040, 0x300, 0x2c0, 0x240,
};

// The lookaside tables: p_cueing's 256 rows of 256 bytes, at the start or a row on. A case
// fills the first two rows; the rest are the same for every case.
static MechU8 g_renderCueing[REND_CUEING_SIZE];
static MechS32 g_renderCueingReady = 0;

// A texture of 1 to 32 texels each way (powers of two half the time), the odd one transparent
// (0xff), its rows in a buffer with REND_TEXTURE_PAD texels and rows of random bytes around it:
// where the texture isn't tiled, rounding can take the walk just past its edge.
typedef struct RenderTexture {
	MechU8 m_texels[REND_TEXTURE_STRIDE * REND_TEXTURE_STRIDE];
	MechU8* m_rows[REND_TEXTURE_STRIDE];
	VFX_TEXTURE m_texture;
} RenderTexture;

// What a case passes besides the polygon: two of each table, which its calls choose between, so
// that each primitive repatches its operands or finds them set.
typedef struct RenderTables {
	RenderTexture m_textures[2];
	MechU8 m_translucency[2][0x100];
	MechU32 m_color;
} RenderTables;

static MechS32 RandomTextureSize(MechU32* p_state)
{
	if (AsmNext(p_state) % 2) {
		return 1 << (AsmNext(p_state) % 6);
	}

	return RandomRange(p_state, 1, REND_TEXTURE_SIZE);
}

// A byte, 0xff (transparent) one time in p_odds.
static MechU8 RandomRenderByte(MechU32* p_state, MechU32 p_odds)
{
	MechU32 bits = AsmNext(p_state);

	return (MechU8) (p_odds && bits % p_odds == 0 ? 0xff : bits >> 8);
}

static void MakeRenderTexture(RenderTexture* p_texture, MechU32* p_state)
{
	MechU32 odds = (AsmNext(p_state) % 4) * 4;
	MechS32 i;

	for (i = 0; i < (MechS32) sizeof(p_texture->m_texels); i++) {
		p_texture->m_texels[i] = RandomRenderByte(p_state, odds);
	}

	for (i = 0; i < REND_TEXTURE_STRIDE; i++) {
		p_texture->m_rows[i] = &p_texture->m_texels[i * REND_TEXTURE_STRIDE + REND_TEXTURE_PAD];
	}

	p_texture->m_texture.m_vAddrs = &p_texture->m_rows[REND_TEXTURE_PAD];
	p_texture->m_texture.m_width = RandomTextureSize(p_state);
	p_texture->m_texture.m_height = RandomTextureSize(p_state);
}

static void MakeRenderTables(RenderTables* p_tables, MechS32 p_operation, MechU32* p_state)
{
	MechU32 odds = (AsmNext(p_state) % 3) * 8;
	MechS32 i;

	if (!g_renderCueingReady) {
		MechU32 state = 0x5eed1e55;

		Fill(g_renderCueing, sizeof(g_renderCueing), &state);
		g_renderCueingReady = 1;
	}

	for (i = 0; i < 0x200; i++) {
		g_renderCueing[i] = RandomRenderByte(p_state, odds);
	}

	MakeRenderTexture(&p_tables->m_textures[0], p_state);
	MakeRenderTexture(&p_tables->m_textures[1], p_state);
	Fill(p_tables->m_translucency[0], sizeof(p_tables->m_translucency), p_state);

	// A solid Gouraud fill's color picks a column of p_cueing, in its low byte
	p_tables->m_color = AsmNext(p_state);
	if ((p_operation & 0x7c0) == 0x240) {
		p_tables->m_color &= 0xffff;
	}
}

// A primitive or, one time in 26, any operation (most of which VFXREND doesn't build).
static MechS32 RenderOperation(MechS32 p_arg, MechU32* p_state)
{
	MechU32 index = (MechU32) p_arg % (REND_PRIMITIVES + 1);

	if (index < REND_PRIMITIVES) {
		return g_renderPrimitives[index];
	}

	return (MechS32) (AsmNext(p_state) % 0x800);
}

// A texture coordinate, 16.16: inside the texture (once the routine adds a half and rounding
// errors) where it isn't tiled, within four tiles where it is, and anywhere near for the mask.
static MechU32 RandomTextureCoordinate(MechS32 p_operation, MechS32 p_size, MechU32* p_state)
{
	switch (p_operation & 0x18) {
	case 0:
		return AsmNext(p_state) % (((MechU32) p_size << 16) - 0x8100);
	case 0x08:
		return AsmNext(p_state) % ((MechU32) p_size << 18);
	default:
		return (MechU32) RandomRange(p_state, -0x400000, 0x400000);
	}
}

// The words the operation reads after x and y: the color (any word), u and v, and for
// perspective, w (0.25 to 1.0, 2.30) and u and v times it.
static void SetRenderVertices(
	MechU32* p_points,
	MechS32 p_count,
	MechS32 p_operation,
	const VFX_TEXTURE* p_texture,
	MechU32* p_state
)
{
	MechS32 i;

	for (i = 0; i < p_count; i++) {
		MechU32* point = &p_points[i * POLY_VERTEX_WORDS];
		MechU32 u = RandomTextureCoordinate(p_operation, p_texture->m_width, p_state);
		MechU32 v = RandomTextureCoordinate(p_operation, p_texture->m_height, p_state);

		point[2] = RandomColor(p_state);
		if ((p_operation & 0x600) == 0x600) {
			MechU32 w = 0x10000000 + AsmNext(p_state) % 0x30000001;

			point[3] = (MechU32) PortableSar64((MechS64) PortableS32(u) * (MechS64) w, 30);
			point[4] = (MechU32) PortableSar64((MechS64) PortableS32(v) * (MechS64) w, 30);
			point[5] = w;
		}
		else {
			point[3] = u;
			point[4] = v;
		}
	}
}

// A polygon inside the buffer, which VFX_polygon_render doesn't clip to: as MakePolygon's, with an
// ellipse that the buffer contains.
static MechS32 MakeInsidePolygon(MechU32* p_points, const PolyTarget* p_target, MechU32* p_state)
{
	MechS32 width = p_target->m_buffer.m_xMax + 1;
	MechS32 height = p_target->m_buffer.m_yMax + 1;
	MechU32 kind = AsmNext(p_state);
	MechS32 count = 1 + (MechS32) (AsmNext(p_state) % POLY_MAX_VERTICES);
	MechS32 centerX = RandomRange(p_state, 0, width - 1);
	MechS32 centerY = RandomRange(p_state, 0, height - 1);
	MechS32 radiusX = RandomRange(p_state, 0, centerX < width - 1 - centerX ? centerX : width - 1 - centerX);
	MechS32 radiusY = RandomRange(p_state, 0, centerY < height - 1 - centerY ? centerY : height - 1 - centerY);

	if (kind % 16 == 0) {
		radiusX = 0;
	}
	else if (kind % 16 == 1) {
		radiusY = 0;
	}

	PlaceVertices(p_points, count, (kind >> 4) & 1, centerX, centerY, radiusX, radiusY, p_state);
	return count;
}

static void SetRenderDither(const AsmModule* p_module, MechU32* p_state)
{
	MechS32 dither1 = RandomDither(p_state);
	MechS32 dither2 = RandomDither(p_state);

	((SetDitherLevelFn) Function(p_module, "VFX_set_Gouraud_dither_level"))(dither1, dither2);
}

// Draws a polygon of p_count vertices with one of the case's tables of each kind.
static void CallRender(
	const AsmModule* p_module,
	const char* p_name,
	PolyTarget* p_target,
	MechU32* p_points,
	MechS32 p_count,
	MechS32 p_operation,
	RenderTables* p_tables,
	MechU32* p_state
)
{
	MechU32 choice = AsmNext(p_state);
	RenderTexture* texture = &p_tables->m_textures[choice & 1];

	((RenderPolygonFn) Function(p_module, p_name))(
		&p_target->m_view,
		p_points,
		p_count,
		p_operation,
		p_tables->m_color,
		&texture->m_texture,
		&g_renderCueing[(choice >> 1) & 1 ? 0x100 : 0],
		p_tables->m_translucency[(choice >> 2) & 1]
	);
}

// Arguments: the dither levels. Read back through a range Gouraud fill, which adds them.
static void RunSetDitherLevel(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	PolyTarget target;
	RenderTables tables;
	MechU32 points[POLY_MAX_VERTICES * POLY_VERTEX_WORDS];
	MechU32 state = AsmSeed(p_args, 2);
	MechS32 count;

	MakePolyTarget(&target, &state);
	MakeRenderTables(&tables, 0x300, &state);
	count = MakeInsidePolygon(points, &target, &state);
	SetRenderVertices(points, count, 0x300, &tables.m_textures[0].m_texture, &state);
	((SetDitherLevelFn) Function(p_module, "VFX_set_Gouraud_dither_level"))(p_args[0], p_args[1]);
	CallRender(p_module, "VFX_polygon_render", &target, points, count, 0x300, &tables, &state);
	OutputPolyTarget(p_output, &target);
}

// The range to make writable, which only the assembly needs, and its selector, which only it has:
// both return a range.
static void RunGetCodeBlock(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	undefined4 start = 0;
	undefined4 selector = 0;
	MechS32 size = ((GetCodeBlockFn) Function(p_module, "GetCodeBlock"))(&start, &selector);

	(void) p_args;
	AsmOutputWord(p_output, size > 0);
	AsmOutputWord(p_output, start != 0);
}

static MechS32 GetCodeBlockDomain(const MechS32* p_args)
{
	(void) p_args;
	return c_domainPointers;
}

// Arguments: the operation (the first, modulo 26) and three words that seed the rest. Two
// polygons inside the buffer.
static void RunPolygonRender(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	PolyTarget target;
	RenderTables tables;
	MechU32 points[POLY_MAX_VERTICES * POLY_VERTEX_WORDS];
	MechU32 state = AsmSeed(p_args, 4);
	MechS32 operation = RenderOperation(p_args[0], &state);
	MechS32 i;

	MakePolyTarget(&target, &state);
	MakeRenderTables(&tables, operation, &state);
	SetRenderDither(p_module, &state);
	for (i = 0; i < 2; i++) {
		MechS32 count = MakeInsidePolygon(points, &target, &state);

		SetRenderVertices(points, count, operation, &tables.m_textures[i].m_texture, &state);
		CallRender(p_module, "VFX_polygon_render", &target, points, count, operation, &tables, &state);
	}

	OutputPolyTarget(p_output, &target);
}

// Sets the clipper's working vertices, which it keeps between calls, to the same words in every
// case: they show through the words a clipped vertex doesn't interpolate. A polygon of
// REND_PRIME_VERTICES vertices above a pane, with some beyond its right: clipped there, in an
// operation that interpolates every word, into the working vertices, then clipped away at the
// top (VFXREND's bottom).
static void PrimeClipper(const AsmModule* p_module)
{
	MechU32 points[REND_PRIME_VERTICES * POLY_VERTEX_WORDS];
	MechU8 pixels[16 * 16];
	WINDOW window;
	PANE pane;
	MechU32 state = 0x9f1a2b3c;
	MechS32 i;

	for (i = 0; i < REND_PRIME_VERTICES; i++) {
		MechU32* point = &points[i * POLY_VERTEX_WORDS];

		FillWords((MechS32*) point, POLY_VERTEX_WORDS, &state);
		point[0] = (MechU32) PortableSar32(g_polyDirections[i] * 20, 10);
		point[1] = (MechU32) (PortableSar32(g_polyDirections[(i + 8) % 32] * 20, 10) - 200);
	}

	window.m_buffer = pixels;
	window.m_xMax = 15;
	window.m_yMax = 15;
	window.m_bitmapInfo = NULL;
	window.m_shadow = 0;
	pane.m_window = &window;
	pane.m_x0 = 0;
	pane.m_y0 = 0;
	pane.m_x1 = 15;
	pane.m_y1 = 15;
	((RenderPolygonFn) Function(p_module, "VFX_polygon_clip_XY_and_render"))(
		&pane,
		points,
		REND_PRIME_VERTICES,
		0x697,
		0,
		NULL,
		NULL,
		NULL
	);
}

// Arguments: the operation (the first, modulo 26) and three words that seed the rest. Two
// polygons around a pane inside the buffer, in vertex lists with room for the clipped polygons,
// which the clipper writes to; the lists are output too.
static void RunPolygonClip(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	PolyTarget target;
	RenderTables tables;
	MechU32 lists[2][REND_LIST_VERTICES * POLY_VERTEX_WORDS];
	MechU32 state = AsmSeed(p_args, 4);
	MechS32 operation = RenderOperation(p_args[0], &state);
	MechS32 width;
	MechS32 height;
	MechS32 i;

	PrimeClipper(p_module);
	MakePolyTarget(&target, &state);
	width = target.m_buffer.m_xMax + 1;
	height = target.m_buffer.m_yMax + 1;
	target.m_view.m_x0 = RandomRange(&state, 0, width - 1);
	target.m_view.m_y0 = RandomRange(&state, 0, height - 1);
	if (AsmNext(&state) % 8) {
		target.m_view.m_x1 = RandomRange(&state, target.m_view.m_x0, width - 1);
		target.m_view.m_y1 = RandomRange(&state, target.m_view.m_y0, height - 1);
	}
	else {
		// Any order, empty included
		target.m_view.m_x1 = RandomRange(&state, 0, width - 1);
		target.m_view.m_y1 = RandomRange(&state, 0, height - 1);
	}

	MakeRenderTables(&tables, operation, &state);
	SetRenderDither(p_module, &state);
	for (i = 0; i < 2; i++) {
		MechS32 count;

		FillWords((MechS32*) lists[i], REND_LIST_VERTICES * POLY_VERTEX_WORDS, &state);
		count =
			AsmNext(&state) % 2 ? MakePolygon(lists[i], &target, &state) : MakeInsidePolygon(lists[i], &target, &state);
		SetRenderVertices(lists[i], count, operation, &tables.m_textures[i].m_texture, &state);
		CallRender(p_module, "VFX_polygon_clip_XY_and_render", &target, lists[i], count, operation, &tables, &state);
	}

	OutputPolyTarget(p_output, &target);
	AsmOutputBytes(p_output, (const MechU8*) lists, sizeof(lists));
}

// F16_div_to_F30 and F30_reciprocal divide the magnitudes, the dividend shifted up by 30 (the
// high word `sar 2` of it, so that 0x80000000 faults) or 1.0: an unsigned div that faults when
// the high word isn't less than the divisor.
static MechU32 Magnitude32(MechS32 p_value)
{
	return p_value < 0 ? 0 - (MechU32) p_value : (MechU32) p_value;
}

static MechS32 F16DivToF30Domain(const MechS32* p_args)
{
	MechU32 high = (MechU32) PortableSar32(PortableS32(Magnitude32(p_args[0])), 2);

	return high < Magnitude32(p_args[1]) ? c_domainIn : c_domainFault;
}

static MechS32 F30ReciprocalDomain(const MechS32* p_args)
{
	return 0x4000 < Magnitude32(p_args[0]) ? c_domainIn : c_domainFault;
}

// --- The 2D primitives (VFXA.ASM) ---

#define BLIT_WIDTH 64
#define BLIT_HEIGHT 40
#define BLIT_GUARD 128
#define BLIT_ARENA_SIZE (BLIT_GUARD * 2 + BLIT_WIDTH * BLIT_HEIGHT + 4)

// A pixel buffer and a view over it, as MakePolyTarget's but larger. p_empty allows a buffer
// with a negative maxX or maxY, which the routines that clip reject.
typedef struct BlitTarget {
	MechU32 m_words[BLIT_ARENA_SIZE / 4];
	WINDOW m_buffer;
	PANE m_view;
} BlitTarget;

static void MakeBlitTarget(BlitTarget* p_target, MechU32* p_state, MechS32 p_empty)
{
	MechU8* arena = (MechU8*) p_target->m_words;
	MechS32 width = RandomRange(p_state, 1, BLIT_WIDTH);
	MechS32 height = RandomRange(p_state, 1, BLIT_HEIGHT);
	MechU32 kind = AsmNext(p_state);

	Fill(arena, BLIT_ARENA_SIZE, p_state);
	p_target->m_buffer.m_buffer = arena + BLIT_GUARD + AsmNext(p_state) % 4;
	p_target->m_buffer.m_xMax = width - 1;
	p_target->m_buffer.m_yMax = height - 1;
	p_target->m_buffer.m_bitmapInfo = NULL;
	p_target->m_buffer.m_shadow = 0;
	if (p_empty && kind % 32 == 0) {
		p_target->m_buffer.m_xMax = -1 - (MechS32) (AsmNext(p_state) % 3);
	}
	else if (p_empty && kind % 32 == 1) {
		p_target->m_buffer.m_yMax = -1;
	}

	p_target->m_view.m_window = &p_target->m_buffer;
	if ((kind >> 5) % 4 == 0) {
		p_target->m_view.m_x0 = 0;
		p_target->m_view.m_y0 = 0;
		p_target->m_view.m_x1 = width - 1;
		p_target->m_view.m_y1 = height - 1;
	}
	else {
		p_target->m_view.m_x0 = RandomRange(p_state, -8, width + 4);
		p_target->m_view.m_y0 = RandomRange(p_state, -8, height + 4);
		p_target->m_view.m_x1 = RandomRange(p_state, -4, width + 8);
		p_target->m_view.m_y1 = RandomRange(p_state, -4, height + 8);
	}
}

static void OutputBlitTarget(AsmOutput* p_output, const BlitTarget* p_target)
{
	AsmOutputBytes(p_output, (const MechU8*) p_target->m_words, BLIT_ARENA_SIZE);
}

// Whether the routines' clipping rejects the target: -1 for an empty buffer, -2 for a view that
// leaves nothing of it, else 0.
static MechS32 BlitTargetEmpty(const BlitTarget* p_target)
{
	const WINDOW* buffer = &p_target->m_buffer;
	const PANE* view = &p_target->m_view;
	MechS32 left = view->m_x0 > 0 ? view->m_x0 : 0;
	MechS32 top = view->m_y0 > 0 ? view->m_y0 : 0;
	MechS32 right = view->m_x1 < buffer->m_xMax ? view->m_x1 : buffer->m_xMax;
	MechS32 bottom = view->m_y1 < buffer->m_yMax ? view->m_y1 : buffer->m_yMax;

	if (buffer->m_xMax < 0 || buffer->m_yMax < 0) {
		return -1;
	}

	return right < left || bottom < top ? -2 : 0;
}

// A coordinate for a view p_extent pixels wide: mostly around it, sometimes far.
static MechS32 RandomCoordinate(MechU32* p_state, MechS32 p_extent)
{
	MechU32 kind = AsmNext(p_state) % 16;

	if (kind == 0) {
		return RandomRange(p_state, -0x4000, 0x4000);
	}

	if (kind == 1) {
		return RandomRange(p_state, -0x400, 0x400);
	}

	return RandomRange(p_state, -p_extent / 2 - 4, p_extent + p_extent / 2 + 4);
}

// The table a mode-1 VFX_line_draw reads, and the function a mode-2 one calls.
static MechU8 g_lineTable[0x100];
static MechS32 g_lineCalls;

static void LineCallback(void)
{
	g_lineCalls++;
}

// The display driver's entry points: the name, and the color and wait callbacks VFX_window_fade
// uses, which log what they're given (a count and a hash: a fade makes thousands of calls).
static char g_driverName[16];
static MechU8 g_driverColors[0x300];
static MechU32 g_driverLogCount;
static AsmHash g_driverLog;

static void LogDriver(MechU32 p_word)
{
	AsmHashWord(&g_driverLog, p_word);
	g_driverLogCount++;
}

static MechChar* DriverName(void)
{
	return g_driverName;
}

static MechChar* DriverOther(void)
{
	LogDriver(0xdddddddd);
	return NULL;
}

static void DriverWait(void)
{
	LogDriver(0xeeeeeeee);
}

static void DriverGetColor(MechS32 p_index, MechU8* p_color)
{
	MechU32 index = (MechU32) p_index & 0xff;

	LogDriver(0x10000000 | index);
	p_color[0] = g_driverColors[index * 3];
	p_color[1] = g_driverColors[index * 3 + 1];
	p_color[2] = g_driverColors[index * 3 + 2];
}

static void DriverSetColor(MechS32 p_index, MechU8* p_color)
{
	LogDriver(0x20000000 | ((MechU32) p_index & 0xff) << 16 | p_color[0]);
	LogDriver((MechU32) p_color[1] | (MechU32) p_color[2] << 8);
}

typedef MechChar* (*DriverEntry)(void);

static DriverEntry g_driverTable[0xd];

static void MakeDriverTable(void)
{
	MechS32 i;

	for (i = 0; i < 0xd; i++) {
		g_driverTable[i] = DriverOther;
	}

	g_driverTable[0] = DriverName;
	g_driverTable[5] = (DriverEntry) (void (*)(void)) DriverWait;
	g_driverTable[8] = (DriverEntry) (void (*)(void)) DriverGetColor;
	g_driverTable[9] = (DriverEntry) (void (*)(void)) DriverSetColor;
	g_driverLogCount = 0;
	AsmHashInit(&g_driverLog);
}

static void OutputDriverLog(AsmOutput* p_output)
{
	AsmOutputWord(p_output, g_driverLogCount);
	AsmOutputWord(p_output, (MechU32) g_driverLog.m_value);
	AsmOutputWord(p_output, (MechU32) (g_driverLog.m_value >> 32));
}

typedef MechChar* (*GetDisplayDriverNameFn)(DriverEntry* p_driver);
typedef void (*SetDisplayDriverFn)(DriverEntry* p_driver);

// Arguments: the name's length (modulo 13) and its seed. The buffer starts out random.
static void RunGetDisplayDriverName(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	MechU8* name = (MechU8*) Data(p_module, "driver_name");
	MechU32 state = AsmSeed(p_args, 2);
	MechU32 length = (MechU32) p_args[0] % 13;
	MechU8 copy[16];
	MechChar* result;
	MechU32 i;

	for (i = 0; i < length; i++) {
		g_driverName[i] = (char) (1 + AsmNext(&state) % 255);
	}

	g_driverName[length] = 0;
	Fill(name, 13, &state);
	MakeDriverTable();
	result = ((GetDisplayDriverNameFn) Function(p_module, "VFX_driver_name"))(g_driverTable);
	AsmOutputWord(p_output, (MechU8*) result == name);
	memset(copy, 0, sizeof(copy));
	memcpy(copy, name, 13);
	AsmOutputBytes(p_output, copy, sizeof(copy));
}

// Arguments: the table's seed. The table is the harness's entry points in a shuffled order.
// VFX's driver entry points, each a public pointer, in the order of the driver's table.
static const char* const g_driverPointers[0xd] = {
	"VFX_describe_driver",
	"VFX_init_driver",
	"VFX_shutdown_driver",
	"VFX_area_wipe",
	"VFX_wait_vblank",
	"VFX_wait_vblank_leading",
	"VFX_window_refresh",
	"VFX_window_read",
	"VFX_DAC_read",
	"VFX_DAC_write",
	"VFX_bank_reset",
	"VFX_pane_refresh",
	"VFX_line_address",
};

static void RunSetDisplayDriver(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	DriverEntry* installed[0xd];
	DriverEntry shuffled[0xd];
	MechU32 state = AsmSeed(p_args, 2);
	MechS32 i;
	MechS32 j;

	MakeDriverTable();
	for (i = 0; i < 0xd; i++) {
		installed[i] = (DriverEntry*) Data(p_module, g_driverPointers[i]);
		shuffled[i] = g_driverTable[AsmNext(&state) % 0xd];
		*installed[i] = NULL;
	}

	((SetDisplayDriverFn) Function(p_module, "VFX_register_driver"))(shuffled);
	for (i = 0; i < 0xd; i++) {
		for (j = 0; j < 0xd && *installed[i] != g_driverTable[j]; j++) {
		}

		AsmOutputWord(p_output, (MechU32) j);
	}
}

typedef MechS32 (*PutViewPixelFn)(PANE* p_view, MechS32 p_x, MechS32 p_y, MechU32 p_color);
typedef MechS32 (*GetViewPixelFn)(PANE* p_view, MechS32 p_x, MechS32 p_y);

// Arguments: four words that seed the target, the point and the color.
static void RunPutViewPixel(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget target;
	MechU32 state = AsmSeed(p_args, 4);
	MechS32 x;
	MechS32 y;

	MakeBlitTarget(&target, &state, 1);
	x = RandomCoordinate(&state, BLIT_WIDTH);
	y = RandomCoordinate(&state, BLIT_HEIGHT);
	AsmOutputWord(
		p_output,
		(MechU32) ((PutViewPixelFn) Function(p_module, "VFX_pixel_write"))(&target.m_view, x, y, AsmNext(&state))
	);
	OutputBlitTarget(p_output, &target);
}

static void RunGetViewPixel(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget target;
	MechU32 state = AsmSeed(p_args, 4);
	MechS32 x;
	MechS32 y;

	MakeBlitTarget(&target, &state, 1);
	x = RandomCoordinate(&state, BLIT_WIDTH);
	y = RandomCoordinate(&state, BLIT_HEIGHT);
	AsmOutputWord(p_output, (MechU32) ((GetViewPixelFn) Function(p_module, "VFX_pixel_read"))(&target.m_view, x, y));
}

typedef MechS32 (*BlitLineFn)(
	PANE* p_view,
	MechS32 p_x1,
	MechS32 p_y1,
	MechS32 p_x2,
	MechS32 p_y2,
	MechS32 p_mode,
	MechS32 p_color
);

// VFX_line_draw's mode, from the first argument: mostly plotting (0, or a negative mode that plots
// too), sometimes the table or the callback, which take a pointer in the color word.
static MechS32 LineMode(const MechS32* p_args)
{
	switch ((MechU32) p_args[0] % 32) {
	case 0:
		return 1;
	case 1:
		return 2 + (MechS32) ((MechU32) p_args[1] % 4);
	case 2:
		return -1 - (MechS32) ((MechU32) p_args[1] % 4);
	default:
		return 0;
	}
}

static MechS32 BlitLineDomain(const MechS32* p_args)
{
	return LineMode(p_args) >= 1 ? c_domainPointers : c_domainIn;
}

// Arguments: the mode (see LineMode), and three words that seed the target, the ends and the
// color. Horizontal and vertical lines' results aren't compared: the assembly returns a flag it
// never set.
static void RunBlitLine(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget target;
	MechU32 state = AsmSeed(p_args, 4);
	MechS32 mode = LineMode(p_args);
	MechS32 x1;
	MechS32 y1;
	MechS32 x2;
	MechS32 y2;
	MechS32 color;
	MechS32 result;

	MakeBlitTarget(&target, &state, 1);
	x1 = RandomCoordinate(&state, BLIT_WIDTH);
	y1 = RandomCoordinate(&state, BLIT_HEIGHT);
	switch (AsmNext(&state) % 8) {
	case 0:
		x2 = x1;
		y2 = RandomCoordinate(&state, BLIT_HEIGHT);
		break;
	case 1:
		y2 = y1;
		x2 = RandomCoordinate(&state, BLIT_WIDTH);
		break;
	case 2:
		// Diagonal
		x2 = x1 + RandomRange(&state, -60, 60);
		y2 = y1 + (AsmNext(&state) & 1 ? x2 - x1 : x1 - x2);
		break;
	default:
		x2 = RandomCoordinate(&state, BLIT_WIDTH);
		y2 = RandomCoordinate(&state, BLIT_HEIGHT);
		break;
	}

	color = PortableS32(AsmNext(&state));
	Fill(g_lineTable, sizeof(g_lineTable), &state);
	g_lineCalls = 0;
	if (mode == 1) {
		color = (MechS32) (size_t) g_lineTable;
	}
	else if (mode > 1) {
		color = (MechS32) (size_t) LineCallback;
	}

	result = ((BlitLineFn) Function(p_module, "VFX_line_draw"))(&target.m_view, x1, y1, x2, y2, mode, color);
	AsmOutputWord(p_output, x1 == x2 || y1 == y2 ? 0 : (MechU32) result);
	AsmOutputWord(p_output, (MechU32) g_lineCalls);
	OutputBlitTarget(p_output, &target);
}

typedef void (*CheckerFn)(
	PANE* p_view,
	MechS32 p_left,
	MechS32 p_top,
	MechS32 p_right,
	MechS32 p_bottom,
	MechU8 p_color
);

// Arguments: four words that seed the target, the rectangle and the color.
static void Run10032e4b(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget target;
	MechU32 state = AsmSeed(p_args, 4);
	MechS32 left;
	MechS32 top;
	MechS32 right;
	MechS32 bottom;

	MakeBlitTarget(&target, &state, 1);
	left = RandomCoordinate(&state, BLIT_WIDTH);
	top = RandomCoordinate(&state, BLIT_HEIGHT);
	right = AsmNext(&state) % 4 ? left + RandomRange(&state, -2, 30) : RandomCoordinate(&state, BLIT_WIDTH);
	bottom = AsmNext(&state) % 4 ? top + RandomRange(&state, -2, 30) : RandomCoordinate(&state, BLIT_HEIGHT);
	((CheckerFn)
		 Function(p_module, "VFX_rectangle_hash"))(&target.m_view, left, top, right, bottom, (MechU8) AsmNext(&state));
	OutputBlitTarget(p_output, &target);
}

// --- SHP shapes ---

#define SHAPE_SIZE 0x1800
#define SHAPE_FRAMES 4

// An SHP animation: up to four frames of run-length rows (some sharing an offset), each with
// bounds around the origin, and some with palette entries. The frame bounds may be inverted (a
// frame without pixels). Rows fill their frame's width at most.
typedef struct Shp {
	MechU32 m_words[SHAPE_SIZE / 4];
	MechS32 m_frames;
	MechU32 m_offsets[SHAPE_FRAMES];
	MechS32 m_pixels[SHAPE_FRAMES]; // whether the frame's bounds hold pixels
} Shp;

static void PutWord(MechU8* p_at, MechU32 p_value)
{
	p_at[0] = (MechU8) p_value;
	p_at[1] = (MechU8) (p_value >> 8);
	p_at[2] = (MechU8) (p_value >> 16);
	p_at[3] = (MechU8) (p_value >> 24);
}

// The codes of a frame's rows; returns the bytes written.
static MechU32 MakeShapeRows(MechU8* p_codes, MechS32 p_width, MechS32 p_rows, MechU32* p_state)
{
	MechU8* code = p_codes;
	MechS32 row;

	for (row = 0; row < p_rows; row++) {
		MechS32 left = p_width;

		if (AsmNext(p_state) % 8 == 0) {
			left = RandomRange(p_state, 0, p_width);
		}

		while (left > 0) {
			MechU32 kind = AsmNext(p_state) % 8;
			MechS32 count;
			MechS32 i;

			if (kind < 2) {
				count = RandomRange(p_state, 1, left < 255 ? left : 255);
				*code++ = 1;
				*code++ = (MechU8) count;
			}
			else if (kind < 5) {
				count = RandomRange(p_state, 1, left < 127 ? left : 127);
				*code++ = (MechU8) (count * 2);
				*code++ = (MechU8) AsmNext(p_state);
			}
			else {
				count = RandomRange(p_state, 1, left < 127 ? left : 127);
				*code++ = (MechU8) (count * 2 + 1);
				for (i = 0; i < count; i++) {
					*code++ = (MechU8) AsmNext(p_state);
				}
			}

			left -= count;
		}

		*code++ = 0;
	}

	return (MechU32) (code - p_codes);
}

static void MakeShp(Shp* p_shape, MechU32* p_state, MechS32 p_maxWidth, MechS32 p_maxHeight)
{
	MechU8* bytes = (MechU8*) p_shape->m_words;
	MechU32 at;
	MechS32 i;

	Fill(bytes, SHAPE_SIZE, p_state);
	p_shape->m_frames = 1 + (MechS32) (AsmNext(p_state) % SHAPE_FRAMES);
	PutWord(bytes + 4, (MechU32) p_shape->m_frames);
	at = 8 + (MechU32) p_shape->m_frames * 8 + AsmNext(p_state) % 4;
	for (i = 0; i < p_shape->m_frames; i++) {
		MechU8* entry = bytes + 8 + i * 8;
		MechS32 left;
		MechS32 top;
		MechS32 width;
		MechS32 height;

		if (i > 0 && AsmNext(p_state) % 4 == 0) {
			// The same frame as an earlier one
			MechS32 earlier = (MechS32) (AsmNext(p_state) % (MechU32) i);

			memcpy(entry, bytes + 8 + earlier * 8, 8);
			p_shape->m_offsets[i] = p_shape->m_offsets[earlier];
			p_shape->m_pixels[i] = p_shape->m_pixels[earlier];
			continue;
		}

		left = RandomRange(p_state, -12, 12);
		top = RandomRange(p_state, -12, 12);
		width = RandomRange(p_state, 1, p_maxWidth);
		height = RandomRange(p_state, 1, p_maxHeight);

		// At most what fits: a row takes up to two bytes a pixel and its end, the palette 28
		if ((MechS32) (SHAPE_SIZE - at) - 0x18 - 28 < height * (width * 2 + 1)) {
			height = ((MechS32) (SHAPE_SIZE - at) - 0x18 - 28) / (width * 2 + 1);
		}

		if (height < 1) {
			memcpy(entry, bytes + 8, 8);
			p_shape->m_offsets[i] = p_shape->m_offsets[0];
			p_shape->m_pixels[i] = p_shape->m_pixels[0];
			continue;
		}

		p_shape->m_offsets[i] = at;
		p_shape->m_pixels[i] = 1;
		PutWord(entry, at);
		PutWord(bytes + at + 8, (MechU32) left);
		PutWord(bytes + at + 0xc, (MechU32) top);
		switch (AsmNext(p_state) % 16) {
		case 0:
			// Inverted: no pixels
			PutWord(bytes + at + 0x10, (MechU32) (left - RandomRange(p_state, 1, 4)));
			PutWord(bytes + at + 0x14, (MechU32) (top + height - 1));
			p_shape->m_pixels[i] = 0;
			break;
		case 1:
			PutWord(bytes + at + 0x10, (MechU32) (left + width - 1));
			PutWord(bytes + at + 0x14, (MechU32) (top - RandomRange(p_state, 1, 4)));
			p_shape->m_pixels[i] = 0;
			height = 0;
			break;
		default:
			PutWord(bytes + at + 0x10, (MechU32) (left + width - 1));
			PutWord(bytes + at + 0x14, (MechU32) (top + height - 1));
			break;
		}

		at += 0x18;
		at += MakeShapeRows(bytes + at, width, height, p_state);

		// A palette: a count, then an index and three components each
		if (AsmNext(p_state) % 2) {
			MechU32 count = 1 + AsmNext(p_state) % 6;

			PutWord(entry + 4, at);
			PutWord(bytes + at, count);
			at += 4 + count * 4;
		}
		else {
			PutWord(entry + 4, 0);
		}
	}
}

// A frame index: one of the shape's.
static MechS32 RandomFrame(const Shp* p_shape, MechU32* p_state)
{
	return (MechS32) (AsmNext(p_state) % (MechU32) p_shape->m_frames);
}

static void OutputShp(AsmOutput* p_output, const Shp* p_shape)
{
	AsmOutputBytes(p_output, (const MechU8*) p_shape->m_words, SHAPE_SIZE);
}

typedef void (*BlitShpFrameFn)(PANE* p_view, void* p_shape, MechS32 p_frame, MechS32 p_x, MechS32 p_y);
typedef MechS32 (*BlitShpFrameRemappedFn)(PANE* p_view, void* p_shape, MechS32 p_frame, MechS32 p_x, MechS32 p_y);
typedef void (*BlitShpFrameUnclippedFn)(PANE* p_view, void* p_frame, MechS32 p_x, MechS32 p_y, MechU32 p_unused);
typedef MechS32 (*BlitShpFrameRemappedUnclippedFn)(
	PANE* p_view,
	void* p_frame,
	MechS32 p_x,
	MechS32 p_y,
	MechU32 p_unused
);
typedef void (*SetRemapTableFn)(MechU8* p_table);

// Loads a random remap table, through VFX_shape_lookaside.
static void SetRandomRemap(const AsmModule* p_module, MechU32* p_state)
{
	MechU8 table[0x100];

	Fill(table, sizeof(table), p_state);
	((SetRemapTableFn) Function(p_module, "VFX_shape_lookaside"))(table);
}

// Arguments: four words that seed the target, the shape, the frame and its position, around
// the view so that it's clipped on any side.
static void RunBlitShp(const AsmModule* p_module, const char* p_name, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget target;
	Shp shape;
	MechU32 state = AsmSeed(p_args, 4);
	MechS32 frame;
	MechS32 x;
	MechS32 y;

	MakeBlitTarget(&target, &state, 1);
	MakeShp(&shape, &state, 48, 32);
	frame = RandomFrame(&shape, &state);
	x = RandomRange(&state, -BLIT_WIDTH, BLIT_WIDTH * 2);
	y = RandomRange(&state, -BLIT_HEIGHT, BLIT_HEIGHT * 2);
	SetRandomRemap(p_module, &state);
	if (!strcmp(p_name, "VFX_shape_draw")) {
		((BlitShpFrameFn) Function(p_module, p_name))(&target.m_view, shape.m_words, frame, x, y);
	}
	else {
		AsmOutputWord(
			p_output,
			(MechU32) ((BlitShpFrameRemappedFn) Function(p_module, p_name))(&target.m_view, shape.m_words, frame, x, y)
		);
	}

	OutputBlitTarget(p_output, &target);
}

static void RunBlitShpFrame(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunBlitShp(p_module, "VFX_shape_draw", p_args, p_output);
}

static void RunBlitShpFrameRemapped(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunBlitShp(p_module, "VFX_shape_translate_draw", p_args, p_output);
}

// Arguments: four words that seed the target, the shape and the frame, placed where it lies
// wholly inside the buffer (the routines don't clip).
static void RunBlitShpUnclipped(
	const AsmModule* p_module,
	const char* p_name,
	const MechS32* p_args,
	AsmOutput* p_output
)
{
	BlitTarget target;
	Shp shape;
	MechU32 state = AsmSeed(p_args, 4);
	const MechU8* frame;
	MechS32 left;
	MechS32 top;
	MechS32 right;
	MechS32 bottom;
	MechS32 x;
	MechS32 y;
	MechS32 result;

	MakeBlitTarget(&target, &state, 0);
	MakeShp(&shape, &state, BLIT_WIDTH / 2, BLIT_HEIGHT / 2);
	frame = (const MechU8*) shape.m_words + shape.m_offsets[RandomFrame(&shape, &state)];
	left = (MechS32) (frame[8] | frame[9] << 8 | frame[10] << 16 | (MechU32) frame[11] << 24);
	top = (MechS32) (frame[0xc] | frame[0xd] << 8 | frame[0xe] << 16 | (MechU32) frame[0xf] << 24);
	right = (MechS32) (frame[0x10] | frame[0x11] << 8 | frame[0x12] << 16 | (MechU32) frame[0x13] << 24);
	bottom = (MechS32) (frame[0x14] | frame[0x15] << 8 | frame[0x16] << 16 | (MechU32) frame[0x17] << 24);

	// A wide buffer, and the frame inside it from the view's origin
	target.m_buffer.m_xMax = BLIT_WIDTH - 1;
	target.m_buffer.m_yMax = BLIT_HEIGHT - 1;
	target.m_view.m_x0 = RandomRange(&state, -4, 4);
	target.m_view.m_y0 = RandomRange(&state, -4, 4);
	x = RandomRange(&state, 0, BLIT_WIDTH - 1 - (right > left ? right - left : 0)) - left - target.m_view.m_x0;
	y = RandomRange(&state, 0, BLIT_HEIGHT - 1 - (bottom > top ? bottom - top : 0)) - top - target.m_view.m_y0;
	SetRandomRemap(p_module, &state);
	if (strcmp(p_name, "DrawShapeUnclipped")) {
		result = ((BlitShpFrameRemappedUnclippedFn)
					  Function(p_module, p_name))(&target.m_view, (void*) frame, x, y, AsmNext(&state));
		AsmOutputWord(p_output, (MechU32) result);
	}
	else {
		((BlitShpFrameUnclippedFn) Function(p_module, p_name))(&target.m_view, (void*) frame, x, y, AsmNext(&state));
	}

	OutputBlitTarget(p_output, &target);
}

static void RunBlitShpFrameUnclipped(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunBlitShpUnclipped(p_module, "DrawShapeUnclipped", p_args, p_output);
}

static void RunBlitShpFrameRemappedUnclipped(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunBlitShpUnclipped(p_module, "XlatShapeUnclipped", p_args, p_output);
}

// Arguments: the table's seed.
static void RunSetRemapTable(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	MechU32 state = AsmSeed(p_args, 2);

	SetRandomRemap(p_module, &state);
	AsmOutputBytes(p_output, (const MechU8*) Data(p_module, "lookaside"), 0x100);
}

typedef MechS32 (*ShapeQueryFn)(void* p_shape, MechS32 p_frame);

// Arguments: two words that seed the shape and the frame.
static void RunShapeQuery(const AsmModule* p_module, const char* p_name, const MechS32* p_args, AsmOutput* p_output)
{
	Shp shape;
	MechU32 state = AsmSeed(p_args, 2);
	MechS32 frame;

	MakeShp(&shape, &state, 48, 32);
	frame = RandomFrame(&shape, &state);
	SetRandomRemap(p_module, &state);
	AsmOutputWord(p_output, (MechU32) ((ShapeQueryFn) Function(p_module, p_name))(shape.m_words, frame));
	if (!strcmp(p_name, "VFX_shape_remap_colors")) {
		OutputShp(p_output, &shape);
	}
}

static void RunRemapShpFrame(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunShapeQuery(p_module, "VFX_shape_remap_colors", p_args, p_output);
}

static void RunGetShpFrameSize(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunShapeQuery(p_module, "VFX_shape_bounds", p_args, p_output);
}

static void Run10037526(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunShapeQuery(p_module, "VFX_shape_origin", p_args, p_output);
}

static void RunGetShpFrameExtent(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunShapeQuery(p_module, "VFX_shape_resolution", p_args, p_output);
}

static void RunGetShpFrameOrigin(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunShapeQuery(p_module, "VFX_shape_minxy", p_args, p_output);
}

typedef MechS32 (*ShapeCountFn)(void* p_shape);

static void RunGetShpFrameCount(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Shp shape;
	MechU32 state = AsmSeed(p_args, 2);

	MakeShp(&shape, &state, 48, 32);
	AsmOutputWord(p_output, (MechU32) ((ShapeCountFn) Function(p_module, "VFX_shape_count"))(shape.m_words));
}

typedef MechS32 (*ShapeBoundsFn)(
	void* p_shape,
	MechS32 p_frame,
	MechS32 p_x,
	MechS32 p_y,
	MechU32 p_flags,
	MechS32* p_bounds
);

// Arguments: two words that seed the shape, the frame, the position and the flags.
static void Run10034622(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Shp shape;
	MechU32 state = AsmSeed(p_args, 2);
	MechS32 bounds[6];
	MechS32 frame;
	MechS32 x;
	MechS32 y;
	MechS32 result;

	MakeShp(&shape, &state, 48, 32);
	frame = RandomFrame(&shape, &state);
	x = RandomCoordinate(&state, BLIT_WIDTH);
	y = RandomCoordinate(&state, BLIT_HEIGHT);
	FillWords(bounds, 6, &state);
	result = ((
		ShapeBoundsFn
	) Function(p_module, "VFX_shape_visible_rectangle"))(shape.m_words, frame, x, y, AsmNext(&state), &bounds[1]);
	AsmOutputWord(p_output, (MechU32) result);
	OutputWords(p_output, bounds, 6);
}

typedef void (*ShapePaletteFn)(void* p_shape, MechS32 p_frame, MechU8* p_palette);
typedef MechS32 (*ShapeEntriesFn)(void* p_shape, MechS32 p_frame, MechU32* p_entries);

// Arguments: two words that seed the shape, the frame and the palette.
static void Run100375a7(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Shp shape;
	MechU8 palette[0x300];
	MechU32 state = AsmSeed(p_args, 2);
	MechS32 frame;

	MakeShp(&shape, &state, 48, 32);
	frame = RandomFrame(&shape, &state);
	Fill(palette, sizeof(palette), &state);
	((ShapePaletteFn) Function(p_module, "VFX_shape_palette"))(shape.m_words, frame, palette);
	AsmOutputBytes(p_output, palette, sizeof(palette));
}

// Arguments: two words that seed the shape, the frame and the buffer, NULL in one case in four.
static void RunShapeEntries(const AsmModule* p_module, const char* p_name, const MechS32* p_args, AsmOutput* p_output)
{
	Shp shape;
	MechU32 entries[8];
	MechU32 state = AsmSeed(p_args, 2);
	MechS32 frame;
	MechS32 result;

	MakeShp(&shape, &state, 48, 32);
	frame = RandomFrame(&shape, &state);
	FillWords((MechS32*) entries, 8, &state);
	result =
		((ShapeEntriesFn) Function(p_module, p_name))(shape.m_words, frame, AsmNext(&state) % 4 ? &entries[1] : NULL);
	AsmOutputWord(p_output, (MechU32) result);
	OutputWords(p_output, (const MechS32*) entries, 8);
	OutputShp(p_output, &shape);
}

static void Run100375f2(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunShapeEntries(p_module, "VFX_shape_colors", p_args, p_output);
}

static void Run1003763a(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunShapeEntries(p_module, "VFX_shape_set_colors", p_args, p_output);
}

typedef MechS32 (*ShapeUniqueFn)(void* p_shape, MechS32* p_out);

// Arguments: two words that seed the shape and the buffer, NULL in one case in four.
static void RunShapeUnique(const AsmModule* p_module, const char* p_name, const MechS32* p_args, AsmOutput* p_output)
{
	Shp shape;
	MechS32 out[SHAPE_FRAMES + 2];
	MechU32 state = AsmSeed(p_args, 2);
	MechS32 result;

	MakeShp(&shape, &state, 48, 32);
	FillWords(out, SHAPE_FRAMES + 2, &state);
	result = ((ShapeUniqueFn) Function(p_module, p_name))(shape.m_words, AsmNext(&state) % 4 ? &out[1] : NULL);
	AsmOutputWord(p_output, (MechU32) result);
	OutputWords(p_output, out, SHAPE_FRAMES + 2);
}

static void RunCountShpUniqueFrames(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunShapeUnique(p_module, "VFX_shape_list", p_args, p_output);
}

static void Run100376f9(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunShapeUnique(p_module, "VFX_shape_palette_list", p_args, p_output);
}

#define SCRATCH_SIZE (32 * 24 + 64)

typedef MechS32 (*BlitRotatedFn)(
	PANE* p_view,
	void* p_shape,
	MechS32 p_frame,
	MechS32 p_x,
	MechS32 p_y,
	MechU8* p_scratch,
	MechS32 p_angle,
	MechS32 p_scaleX,
	MechS32 p_scaleY,
	MechU32 p_flags
);

// Arguments: four words that seed the target, the shape (its frames at most 32 by 24, the size
// of the scratch buffer), the frame, its position, the rotation and scales (one case in eight
// without either, VFX_shape_draw's path, which a frame without pixels always takes: the mapping
// sizes the scratch buffer from its bounds) and the flags. The mapping's result isn't compared
// but for an empty buffer or view: the assembly leaves what it last computed.
static void RunBlitRotated(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget target;
	Shp shape;
	MechU8 scratch[SCRATCH_SIZE];
	MechU32 state = AsmSeed(p_args, 4);
	MechS32 frame;
	MechS32 x;
	MechS32 y;
	MechS32 angle;
	MechS32 scaleX;
	MechS32 scaleY;
	MechU32 flags;
	MechS32 plain;
	MechS32 result;

	MakeBlitTarget(&target, &state, 1);
	MakeShp(&shape, &state, 32, 24);
	frame = RandomFrame(&shape, &state);
	x = RandomRange(&state, -BLIT_WIDTH / 2, BLIT_WIDTH + BLIT_WIDTH / 2);
	y = RandomRange(&state, -BLIT_HEIGHT / 2, BLIT_HEIGHT + BLIT_HEIGHT / 2);
	Fill(scratch, sizeof(scratch), &state);
	plain = AsmNext(&state) % 8 == 0 || !shape.m_pixels[frame];
	angle = plain ? 0 : RandomRange(&state, -7200, 7200);
	scaleX = plain ? 0x10000 : RandomRange(&state, -0x30000, 0x30000);
	scaleY = plain ? 0x10000 : AsmNext(&state) % 2 ? scaleX : RandomRange(&state, -0x30000, 0x30000);
	flags = AsmNext(&state) % 4;
	SetRandomRemap(p_module, &state);
	result = ((BlitRotatedFn) Function(p_module, "VFX_shape_transform"))(
		&target.m_view,
		shape.m_words,
		frame,
		x,
		y,
		scratch,
		angle,
		scaleX,
		scaleY,
		flags
	);
	AsmOutputWord(p_output, plain || BlitTargetEmpty(&target) ? (MechU32) result : 0);
	OutputBlitTarget(p_output, &target);
	AsmOutputBytes(p_output, scratch, sizeof(scratch));
}

typedef void (*FillViewFn)(PANE* p_view, MechS32 p_color);

// Arguments: two words that seed the target and the color.
static void RunFillView(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget target;
	MechU32 state = AsmSeed(p_args, 2);

	MakeBlitTarget(&target, &state, 1);
	((FillViewFn) Function(p_module, "VFX_pane_wipe"))(&target.m_view, PortableS32(AsmNext(&state)));
	OutputBlitTarget(p_output, &target);
}

typedef MechS32 (*BlitViewFn)(
	PANE* p_source,
	MechS32 p_sourceX,
	MechS32 p_sourceY,
	PANE* p_dest,
	MechS32 p_destX,
	MechS32 p_destY,
	MechS32 p_fillColor
);

// Arguments: four words that seed the two targets (one time in two, two views of one buffer,
// which overlap), the positions and the fill color (mostly -1, a copy).
static void RunBlitView(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget source;
	BlitTarget dest;
	PANE* destView = &dest.m_view;
	MechU32 state = AsmSeed(p_args, 4);
	MechS32 sourceX;
	MechS32 sourceY;
	MechS32 destX;
	MechS32 destY;
	MechS32 fill;
	MechS32 result;

	MakeBlitTarget(&source, &state, 1);
	MakeBlitTarget(&dest, &state, 1);
	if (AsmNext(&state) % 2) {
		// A second view of the source's buffer
		dest.m_view.m_window = &source.m_buffer;
		destView = &dest.m_view;
	}

	switch (AsmNext(&state) % 8) {
	case 0:
		fill = (MechS32) (AsmNext(&state) % 0x100);
		break;
	case 1:
		fill = PortableS32(AsmNext(&state));
		break;
	default:
		fill = -1;
		break;
	}

	sourceX = RandomRange(&state, -20, 20);
	sourceY = RandomRange(&state, -20, 20);
	destX = RandomRange(&state, -20, 20);
	destY = RandomRange(&state, -20, 20);
	result = ((BlitViewFn)
				  Function(p_module, "VFX_pane_copy"))(&source.m_view, sourceX, sourceY, destView, destX, destY, fill);
	AsmOutputWord(p_output, (MechU32) result);
	OutputBlitTarget(p_output, &source);
	OutputBlitTarget(p_output, &dest);
}

#define SCROLL_SCRATCH ((BLIT_WIDTH + 16) * (BLIT_HEIGHT + 16))

typedef MechS32 (*ScrollViewFn)(PANE* p_view, MechS32 p_dx, MechS32 p_dy, MechS32 p_mode, MechS32 p_color);

static MechU8 g_scrollScratch[SCROLL_SCRATCH];

// The mode from the first argument: 1 (wrapping, through a scratch buffer, or measuring with 0)
// one time in two, else a fill.
static MechS32 ScrollMode(const MechS32* p_args)
{
	return (MechU32) p_args[0] % 2 ? 1 : (MechS32) ((MechU32) p_args[0] % 8) - 2;
}

static MechS32 ScrollViewDomain(const MechS32* p_args)
{
	return ScrollMode(p_args) == 1 && (MechU32) p_args[1] % 8 ? c_domainPointers : c_domainIn;
}

// Arguments: the mode (see ScrollMode), whether a wrap gets its scratch buffer (0 measures),
// and two words that seed the target, the scroll and the fill color.
static void RunScrollView(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget target;
	MechU32 state = AsmSeed(p_args, 4);
	MechS32 mode = ScrollMode(p_args);
	MechS32 color;
	MechS32 dx;
	MechS32 dy;
	MechS32 result;
	AsmHash hash;

	MakeBlitTarget(&target, &state, 1);
	Fill(g_scrollScratch, sizeof(g_scrollScratch), &state);
	dx = RandomRange(&state, -BLIT_WIDTH - 20, BLIT_WIDTH + 20);
	dy = RandomRange(&state, -BLIT_HEIGHT - 20, BLIT_HEIGHT + 20);
	if (AsmNext(&state) % 8 == 0) {
		dx = 0;
	}

	if (AsmNext(&state) % 8 == 0) {
		dy = 0;
	}

	color = PortableS32(AsmNext(&state));
	if (mode == 1) {
		color = (MechU32) p_args[1] % 8 ? (MechS32) (size_t) g_scrollScratch : 0;
	}

	result = ((ScrollViewFn) Function(p_module, "VFX_pane_scroll"))(&target.m_view, dx, dy, mode, color);
	AsmOutputWord(p_output, (MechU32) result);
	OutputBlitTarget(p_output, &target);
	AsmHashInit(&hash);
	AsmHashWord(&hash, 0);
	{
		MechU32 i;

		for (i = 0; i < SCROLL_SCRATCH; i++) {
			AsmHashWord(&hash, g_scrollScratch[i]);
		}
	}
	AsmOutputWord(p_output, (MechU32) hash.m_value);
	AsmOutputWord(p_output, (MechU32) (hash.m_value >> 32));
}

typedef void (*EllipseFn)(
	PANE* p_view,
	MechS32 p_x,
	MechS32 p_y,
	MechS32 p_radiusX,
	MechS32 p_radiusY,
	MechS32 p_color
);

// A radius: mostly up to the view's size, sometimes 0 or large.
static MechS32 RandomRadius(MechU32* p_state)
{
	switch (AsmNext(p_state) % 16) {
	case 0:
		return 0;
	case 1:
		return RandomRange(p_state, 100, 2000);
	default:
		return RandomRange(p_state, 1, 50);
	}
}

// Arguments: four words that seed the target, the center, the radii and the color.
static void RunEllipse(const AsmModule* p_module, const char* p_name, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget target;
	MechU32 state = AsmSeed(p_args, 4);
	MechS32 x;
	MechS32 y;
	MechS32 radiusX;
	MechS32 radiusY;

	MakeBlitTarget(&target, &state, 1);
	x = RandomRange(&state, -30, BLIT_WIDTH + 30);
	y = RandomRange(&state, -30, BLIT_HEIGHT + 30);
	radiusX = RandomRadius(&state);
	radiusY = RandomRadius(&state);
	((EllipseFn) Function(p_module, p_name))(&target.m_view, x, y, radiusX, radiusY, PortableS32(AsmNext(&state)));
	OutputBlitTarget(p_output, &target);
}

static void RunDrawEllipse(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunEllipse(p_module, "VFX_ellipse_draw", p_args, p_output);
}

static void RunFillEllipse(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunEllipse(p_module, "VFX_ellipse_fill", p_args, p_output);
}

typedef void (*GetCosSinFn)(MechS32 p_angle, MechS32* p_cos, MechS32* p_sin);
typedef void (*BlitFixedMul16Fn)(MechS32 p_a, MechS32 p_b, MechS32* p_result);
typedef void (*RotateScalePointFn)(
	MechS32* p_point,
	MechS32* p_result,
	MechS32* p_origin,
	MechS32 p_angle,
	MechS32 p_scaleX,
	MechS32 p_scaleY
);

// An angle within +-0x100000 tenths of a degree: the routine's loops take it into a turn 3600
// at a time.
static MechS32 BoundedAngle(MechS32 p_angle)
{
	return p_angle % 0x100000;
}

// Arguments: the angle.
static void RunGetCosSin(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	MechS32 values[4];
	MechU32 state = AsmSeed(p_args, 1);

	FillWords(values, 4, &state);
	((GetCosSinFn) Function(p_module, "VFX_Cos_Sin"))(BoundedAngle(p_args[0]), &values[0], &values[2]);
	OutputWords(p_output, values, 4);
}

// Arguments: the factors.
static void RunBlitFixedMul16(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	MechS32 values[3];
	MechU32 state = AsmSeed(p_args, 2);

	FillWords(values, 3, &state);
	((BlitFixedMul16Fn) Function(p_module, "VFX_fixed_mul"))(p_args[0], p_args[1], &values[1]);
	OutputWords(p_output, values, 3);
}

// Arguments: the point, the origin, the angle and the scales.
static void RunRotateScalePoint(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	MechS32 point[2];
	MechS32 origin[2];
	MechS32 result[4];
	MechU32 state = AsmSeed(p_args, 7);

	point[0] = p_args[0];
	point[1] = p_args[1];
	origin[0] = p_args[2];
	origin[1] = p_args[3];
	FillWords(result, 4, &state);
	((RotateScalePointFn) Function(p_module, "VFX_point_transform"))(
		point,
		&result[1],
		origin,
		BoundedAngle(p_args[4]),
		p_args[5],
		p_args[6]
	);
	OutputWords(p_output, result, 4);
}

// --- Fonts and text ---

#define FONT_SIZE 0x1000
#define FONT_GLYPHS 12

// A font: its height (1 to 12), and the 256 characters' offsets into a pool of glyphs, each
// up to 10 pixels wide (some 0), some pixels 0xff.
typedef struct Font {
	MechU32 m_words[FONT_SIZE / 4];
} Font;

static void MakeFont(Font* p_font, MechU32* p_state)
{
	MechU8* bytes = (MechU8*) p_font->m_words;
	MechU32 glyphs[FONT_GLYPHS];
	MechS32 height = RandomRange(p_state, 1, 12);
	MechU32 at = 0x10 + 0x100 * 4;
	MechS32 i;
	MechS32 j;

	Fill(bytes, FONT_SIZE, p_state);
	PutWord(bytes + 8, (MechU32) height);
	for (i = 0; i < FONT_GLYPHS; i++) {
		MechS32 width = AsmNext(p_state) % 8 ? RandomRange(p_state, 1, 10) : 0;

		glyphs[i] = at;
		PutWord(bytes + at, (MechU32) width);
		at += 4;
		for (j = 0; j < width * height; j++) {
			MechU32 bits = AsmNext(p_state);

			bytes[at++] = (MechU8) (bits % 8 ? bits >> 8 : 0xff);
		}
	}

	for (i = 0; i < 0x100; i++) {
		PutWord(bytes + 0x10 + i * 4, glyphs[AsmNext(p_state) % FONT_GLYPHS]);
	}
}

// A palette for text: a 256-byte table, some entries 0xff (left out), or NULL in one case in
// three.
static MechU8 g_textPalette[0x100];

static MechU8* RandomTextPalette(MechU32* p_state)
{
	MechS32 i;

	if (AsmNext(p_state) % 3 == 0) {
		return NULL;
	}

	for (i = 0; i < 0x100; i++) {
		MechU32 bits = AsmNext(p_state);

		g_textPalette[i] = (MechU8) (bits % 4 ? bits >> 8 : 0xff);
	}

	return g_textPalette;
}

typedef MechS32 (*FontGetHeightFn)(void* p_font);
typedef MechS32 (*FontGetCharWidthFn)(void* p_font, MechS32 p_char);
typedef MechS32 (*BlitCharFn)(PANE* p_view, MechS32 p_x, MechS32 p_y, void* p_font, MechS32 p_char, void* p_palette);
typedef void (*BlitStringFn)(PANE* p_view, MechS32 p_x, MechS32 p_y, void* p_font, MechChar* p_text, void* p_palette);

// Arguments: the font's seed.
static void RunFontGetHeight(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Font font;
	MechU32 state = AsmSeed(p_args, 1);

	MakeFont(&font, &state);
	AsmOutputWord(p_output, (MechU32) ((FontGetHeightFn) Function(p_module, "VFX_font_height"))(font.m_words));
}

// Arguments: the character (its low byte) and the font's seed.
static void RunFontGetCharWidth(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Font font;
	MechU32 state = AsmSeed(p_args, 2);

	MakeFont(&font, &state);
	AsmOutputWord(
		p_output,
		(MechU32) ((FontGetCharWidthFn) Function(p_module, "VFX_character_width"))(font.m_words, p_args[0] & 0xff)
	);
}

// Arguments: four words that seed the target, the font, the character, its position and the
// palette.
static void RunBlitChar(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget target;
	Font font;
	MechU32 state = AsmSeed(p_args, 4);
	MechS32 x;
	MechS32 y;
	MechU8* palette;
	MechS32 result;

	MakeBlitTarget(&target, &state, 1);
	MakeFont(&font, &state);
	x = RandomRange(&state, -14, BLIT_WIDTH + 4);
	y = RandomRange(&state, -14, BLIT_HEIGHT + 4);
	palette = RandomTextPalette(&state);
	result = ((
		BlitCharFn
	) Function(p_module, "VFX_character_draw"))(&target.m_view, x, y, font.m_words, AsmNext(&state) % 0x100, palette);
	AsmOutputWord(p_output, (MechU32) result);
	OutputBlitTarget(p_output, &target);
}

// Arguments: four words that seed the target, the font, the text (up to 12 characters, the first
// sometimes the end) and its position and palette.
static void RunBlitString(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget target;
	Font font;
	MechU8 text[16];
	MechU32 state = AsmSeed(p_args, 4);
	MechU32 length;
	MechU32 i;
	MechS32 x;
	MechS32 y;
	MechU8* palette;

	MakeBlitTarget(&target, &state, 1);
	MakeFont(&font, &state);
	Fill(text, sizeof(text), &state);
	length = AsmNext(&state) % 13;
	for (i = 0; i < length; i++) {
		text[i] = (MechU8) (1 + AsmNext(&state) % 255);
	}

	text[length] = 0;
	text[sizeof(text) - 1] = 0;
	x = RandomRange(&state, -30, BLIT_WIDTH);
	y = RandomRange(&state, -14, BLIT_HEIGHT + 4);
	palette = RandomTextPalette(&state);
	((BlitStringFn)
		 Function(p_module, "VFX_string_draw"))(&target.m_view, x, y, font.m_words, (MechChar*) text, palette);
	OutputBlitTarget(p_output, &target);
}

typedef MechS32 (*WriteViewRowFn)(PANE* p_view, MechS32 p_row, MechU8* p_src, MechS32 p_width);

// Arguments: four words that seed the target, the row, the pixels and their count (a negative
// count that the clipping leaves faults). Only the empty buffer's and view's results are
// compared: otherwise the assembly leaves what it last computed.
static void RunWriteViewRow(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget target;
	MechU8 pixels[0xa0];
	MechU32 state = AsmSeed(p_args, 4);
	MechS32 row;
	MechS32 width;
	MechS32 result;

	MakeBlitTarget(&target, &state, 1);
	Fill(pixels, sizeof(pixels), &state);
	row = RandomRange(&state, -12, BLIT_HEIGHT + 12);
	width = RandomRange(&state, 0, 0x8c);
	result = ((WriteViewRowFn) Function(p_module, "VFX_line_to_pane"))(&target.m_view, row, &pixels[0x10], width);
	AsmOutputWord(p_output, BlitTargetEmpty(&target) ? (MechU32) result : 0);
	OutputBlitTarget(p_output, &target);
}

// --- Pictures ---

#define PICTURE_SIZE 0x1800

typedef struct Picture {
	MechU32 m_words[PICTURE_SIZE / 4];
	MechU32 m_size;
	MechU32 m_chunks[4]; // the IFF chunks' offsets: BMHD, CMAP, BODY, and another (0 for none)
	MechS32 m_masking;
} Picture;

static void PutBigWord(MechU8* p_at, MechU32 p_value)
{
	p_at[0] = (MechU8) (p_value >> 8);
	p_at[1] = (MechU8) p_value;
}

static void PutBigDword(MechU8* p_at, MechU32 p_value)
{
	PutBigWord(p_at, p_value >> 16);
	PutBigWord(p_at + 2, p_value);
}

// ByteRun1: a literal of n + 1 bytes is n, then the bytes; a run of n bytes, 257 - n, then the
// byte; 0x80 does nothing. Rows are encoded on their own, but a run may now and then end past
// the row.
static MechU32 EncodeByteRun(MechU8* p_out, const MechU8* p_row, MechU32 p_size, MechU32* p_state)
{
	MechU8* out = p_out;
	MechU32 at = 0;

	while (at < p_size) {
		MechU32 left = p_size - at;
		MechU32 kind = AsmNext(p_state) % 16;
		MechU32 count;

		if (kind == 0) {
			*out++ = 0x80;
			continue;
		}

		if (kind < 6) {
			count = 1 + AsmNext(p_state) % (left < 128 ? left : 128);
			*out++ = (MechU8) (count - 1);
			memcpy(out, p_row + at, count);
			out += count;
		}
		else {
			count = 2 + AsmNext(p_state) % 127;
			if (count > left && AsmNext(p_state) % 4) {
				count = left < 2 ? 2 : left;
			}

			*out++ = (MechU8) (0x101 - count);
			*out++ = p_row[at];
		}

		at += count;
	}

	return (MechU32) (out - p_out);
}

// An IFF picture: ILBM (planar, a multiple of 16 pixels wide) or PBM (up to 100 pixels wide),
// up to 10 rows, raw or ByteRun1-coded, its chunks in any order, padded with zero bytes, with
// another chunk among them sometimes.
static void MakeIff(Picture* p_picture, MechU32* p_state)
{
	MechU8* bytes = (MechU8*) p_picture->m_words;
	MechS32 planar = AsmNext(p_state) % 2;
	MechU32 width = planar ? 16 * (1 + AsmNext(p_state) % 6) : 1 + AsmNext(p_state) % 100;
	MechU32 height = 1 + AsmNext(p_state) % 10;
	MechU32 rowBytes = width + (width & 1);
	MechU32 compression = AsmNext(p_state) % 2;
	MechU32 order = AsmNext(p_state) % 6;
	MechU32 at = 12;
	MechU32 kinds[4];
	MechS32 i;

	Fill(bytes, PICTURE_SIZE, p_state);
	memcpy(bytes, "FORM", 4);
	memcpy(bytes + 8, planar ? "ILBM" : "PBM ", 4);
	p_picture->m_masking = AsmNext(p_state) % 8 == 0 ? 1 : (MechS32) (AsmNext(p_state) % 4 == 0) * 2;
	p_picture->m_chunks[3] = 0;
	kinds[0] = 3;
	kinds[1] = order % 3;
	kinds[2] = (order % 3 + 1 + order / 3) % 3;
	kinds[3] = 3 - kinds[1] - kinds[2];
	for (i = 0; i < 4; i++) {
		MechU32 kind = kinds[i];
		MechU32 pad = AsmNext(p_state) % 4 == 0 ? 1 + AsmNext(p_state) % 2 : 0;

		memset(bytes + at, 0, pad);
		at += pad;
		p_picture->m_chunks[kind] = at;
		if (kind == 0) {
			memcpy(bytes + at, "BMHD", 4);
			PutBigDword(bytes + at + 4, 20);
			PutBigWord(bytes + at + 8, width);
			PutBigWord(bytes + at + 10, height);
			bytes[at + 8 + 8] = (MechU8) (planar ? 8 : AsmNext(p_state));
			bytes[at + 8 + 9] = (MechU8) p_picture->m_masking;
			bytes[at + 8 + 10] = (MechU8) compression;
			at += 8 + 20;
		}
		else if (kind == 1) {
			memcpy(bytes + at, "CMAP", 4);
			PutBigDword(bytes + at + 4, 0x300);
			at += 8 + 0x300;
		}
		else if (kind == 2) {
			MechU8 row[0x80];
			MechU32 start = at + 8;
			MechU32 y;

			memcpy(bytes + at, "BODY", 4);
			at = start;
			for (y = 0; y < height; y++) {
				Fill(row, rowBytes, p_state);
				if (compression) {
					at += EncodeByteRun(bytes + at, row, rowBytes, p_state);
				}
				else {
					memcpy(bytes + at, row, rowBytes);
					at += rowBytes;
				}
			}

			PutBigDword(bytes + start - 4, at - start);
		}
		else if (AsmNext(p_state) % 2) {
			// Another chunk, of an odd size and padded
			MechU32 size = 1 + 2 * (AsmNext(p_state) % 6);

			memcpy(bytes + at, "ANNO", 4);
			PutBigDword(bytes + at + 4, size);
			at += 8 + size;
			bytes[at++] = 0;
			p_picture->m_chunks[3] = at - size - 9;
		}
		else {
			p_picture->m_chunks[3] = 0;
		}
	}

	p_picture->m_size = at;
}

typedef MechU8* (*FindIffChunkFn)(MechChar* p_tag, MechU8* p_iff);
typedef MechS32 (*BlitPictureFn)(PANE* p_view, MechU8* p_data);
typedef void (*ReadPaletteFn)(MechU8* p_data, MechU8* p_palette);
typedef MechS32 (*PictureSizeFn)(MechU8* p_data);

// Arguments: two words that seed the picture and the chunk to find.
static void RunFindIffChunk(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	static const char* const c_tags[4] = {"BMHD", "CMAP", "BODY", "ANNO"};
	Picture picture;
	MechU32 state = AsmSeed(p_args, 2);
	MechU32 kind;
	MechChar tag[4];
	MechU8* result;

	MakeIff(&picture, &state);
	kind = AsmNext(&state) % (picture.m_chunks[3] ? 4 : 3);
	memcpy(tag, c_tags[kind], 4);
	result = ((FindIffChunkFn) Function(p_module, "find_ILBM_property"))(tag, (MechU8*) picture.m_words);
	AsmOutputWord(p_output, (MechU32) (result - (MechU8*) picture.m_words));
}

// A view for the pictures: the routines size the picture by the view's own width and height,
// so it isn't empty.
static void MakePictureTarget(BlitTarget* p_target, MechU32* p_state)
{
	MakeBlitTarget(p_target, p_state, 1);
	if (p_target->m_view.m_y1 < p_target->m_view.m_y0) {
		p_target->m_view.m_y1 = p_target->m_view.m_y0 + RandomRange(p_state, 0, 8);
	}
}

// Arguments: two words that seed the target and the picture. A masking of 1's result isn't
// compared: the assembly returns a local it never set.
static void RunBlitIff(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget target;
	Picture picture;
	MechU32 state = AsmSeed(p_args, 2);
	MechS32 result;

	MakePictureTarget(&target, &state);
	MakeIff(&picture, &state);
	result = ((BlitPictureFn) Function(p_module, "VFX_ILBM_draw"))(&target.m_view, (MechU8*) picture.m_words);
	AsmOutputWord(p_output, picture.m_masking == 1 ? 0 : (MechU32) result);
	OutputBlitTarget(p_output, &target);
}

// Arguments: two words that seed the picture and the palette.
static void RunReadIffPalette(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Picture picture;
	MechU8 palette[0x300];
	MechU32 state = AsmSeed(p_args, 2);

	MakeIff(&picture, &state);
	Fill(palette, sizeof(palette), &state);
	((ReadPaletteFn) Function(p_module, "VFX_ILBM_palette"))((MechU8*) picture.m_words, palette);
	AsmOutputBytes(p_output, palette, sizeof(palette));
}

static void RunGetIffSize(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Picture picture;
	MechU32 state = AsmSeed(p_args, 2);

	MakeIff(&picture, &state);
	AsmOutputWord(
		p_output,
		(MechU32) ((PictureSizeFn) Function(p_module, "VFX_ILBM_resolution"))((MechU8*) picture.m_words)
	);
}

// A PCX picture: a 0x80-byte header, up to 10 rows of up to 100 bytes, run-length coded (a byte
// of 0xc0 and up is a run of its low six bits, sometimes 0, of the next byte; a run may end
// past the row), and the palette's 0x300 bytes at the end.
static void MakePcx(Picture* p_picture, MechU32* p_state)
{
	MechU8* bytes = (MechU8*) p_picture->m_words;
	MechU32 rows = 1 + AsmNext(p_state) % 10;
	MechU32 width = 1 + AsmNext(p_state) % 100;
	MechU32 top = AsmNext(p_state) & 0xffff;
	MechU32 at = 0x80;
	MechU32 y;

	Fill(bytes, PICTURE_SIZE, p_state);
	bytes[6] = (MechU8) top;
	bytes[7] = (MechU8) (top >> 8);
	bytes[0xa] = (MechU8) (top + rows - 1);
	bytes[0xb] = (MechU8) ((top + rows - 1) >> 8);
	bytes[0x42] = (MechU8) width;
	bytes[0x43] = 0;
	for (y = 0; y < rows; y++) {
		MechU32 x = 0;

		do {
			MechU32 bits = AsmNext(p_state);

			if (bits % 4 == 0) {
				MechU32 count = (bits >> 8) % 0x40;

				bytes[at++] = (MechU8) (0xc0 | count);
				bytes[at++] = (MechU8) (bits >> 16);
				x += count;
			}
			else {
				bytes[at++] = (MechU8) ((bits >> 8) % 0xc0);
				x++;
			}
		} while (x < width);
	}

	p_picture->m_size = at + 0x300;
}

// Arguments: two words that seed the target and the picture.
static void RunBlitPicture(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget target;
	Picture picture;
	MechU32 state = AsmSeed(p_args, 2);
	MechS32 result;

	MakePictureTarget(&target, &state);
	MakePcx(&picture, &state);
	result = ((BlitPictureFn) Function(p_module, "VFX_PCX_draw"))(&target.m_view, (MechU8*) picture.m_words);
	AsmOutputWord(p_output, (MechU32) result);
	OutputBlitTarget(p_output, &target);
}

typedef void (*ReadPicturePaletteFn)(MechU8* p_data, MechS32 p_size, void* p_palette);

static void RunReadPicturePalette(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Picture picture;
	MechU8 palette[0x300];
	MechU32 state = AsmSeed(p_args, 2);

	MakePcx(&picture, &state);
	Fill(palette, sizeof(palette), &state);
	((ReadPicturePaletteFn)
		 Function(p_module, "VFX_PCX_palette"))((MechU8*) picture.m_words, (MechS32) picture.m_size, palette);
	AsmOutputBytes(p_output, palette, sizeof(palette));
}

static void RunGetPictureSize(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Picture picture;
	MechU32 state = AsmSeed(p_args, 2);

	MakePcx(&picture, &state);
	AsmOutputWord(
		p_output,
		(MechU32) ((PictureSizeFn) Function(p_module, "VFX_PCX_resolution"))((MechU8*) picture.m_words)
	);
}

// --- GIF ---

#define GIF_STATE_SIZE 0x5030
#define GIF_TABLE 0x1000

// A GIF's LZW code stream, a code at a time, low bits first.
typedef struct GifWriter {
	MechU8* m_out;
	MechU32 m_size;
	MechU32 m_bits;
	MechU32 m_bitCount;
} GifWriter;

static void PutGifCode(GifWriter* p_writer, MechU32 p_code, MechU32 p_size)
{
	p_writer->m_bits |= p_code << p_writer->m_bitCount;
	p_writer->m_bitCount += p_size;
	while (p_writer->m_bitCount >= 8) {
		p_writer->m_out[p_writer->m_size++] = (MechU8) p_writer->m_bits;
		p_writer->m_bits >>= 8;
		p_writer->m_bitCount -= 8;
	}
}

// Encodes p_count pixels of p_bits each with LZW, as GIF encoders do (the code size grows when
// the table outgrows it, up to 12 bits), with a clear code now and then. The table finds a
// string's extensions through lists of each code's children.
static MechU32 EncodeGif(MechU8* p_out, const MechU8* p_pixels, MechU32 p_count, MechU32 p_bits, MechU32* p_state)
{
	static MechU16 children[GIF_TABLE];
	static MechU16 siblings[GIF_TABLE];
	static MechU8 suffixes[GIF_TABLE];
	GifWriter writer;
	MechU32 clear = 1u << p_bits;
	MechU32 next = clear + 2;
	MechU32 size = p_bits + 1;
	MechU32 current;
	MechU32 i;

	writer.m_out = p_out;
	writer.m_size = 0;
	writer.m_bits = 0;
	writer.m_bitCount = 0;
	memset(children, 0, sizeof(children));
	PutGifCode(&writer, clear, size);
	current = p_pixels[0];
	for (i = 1; i < p_count; i++) {
		MechU32 pixel = p_pixels[i];
		MechU32 code;

		for (code = children[current]; code && suffixes[code] != pixel; code = siblings[code]) {
		}

		if (code) {
			current = code;
			continue;
		}

		PutGifCode(&writer, current, size);
		if (next >= (1u << size) && size < 12) {
			size++;
		}

		if (next < GIF_TABLE && AsmNext(p_state) % 64) {
			suffixes[next] = (MechU8) pixel;
			siblings[next] = children[current];
			children[current] = (MechU16) next;
			children[next] = 0;
			next++;
		}
		else {
			PutGifCode(&writer, clear, size);
			memset(children, 0, sizeof(children));
			next = clear + 2;
			size = p_bits + 1;
		}

		current = pixel;
	}

	PutGifCode(&writer, current, size);
	if (next >= (1u << size) && size < 12) {
		size++;
	}

	PutGifCode(&writer, clear + 1, size);
	PutGifCode(&writer, 0, 7);
	return writer.m_size;
}

// A GIF: a header, a global palette or not, an image (up to 80 by 10 pixels, interlaced or not)
// with a local palette or not, and its LZW data in sub-blocks of any size, sometimes followed by
// another before the end.
static void MakeGif(Picture* p_picture, MechU32* p_state)
{
	MechU8* bytes = (MechU8*) p_picture->m_words;
	MechU8 pixels[80 * 10];
	MechU8 stream[0x800];
	MechU32 width = 1 + AsmNext(p_state) % 80;
	MechU32 height = 1 + AsmNext(p_state) % 10;
	MechU32 bits = 2 + AsmNext(p_state) % 7;
	MechU32 flags = AsmNext(p_state) & 0x7f;
	MechU32 imageFlags = AsmNext(p_state) & 0x7f;
	MechU32 at = 0xd;
	MechU32 length;
	MechU32 sent;
	MechU32 i;

	Fill(bytes, PICTURE_SIZE, p_state);
	memcpy(bytes, "GIF89a", 6);
	if (AsmNext(p_state) % 2) {
		flags |= 0x80;
		at += 3u << ((flags & 7) + 1);
	}

	bytes[0xa] = (MechU8) flags;
	bytes[at] = 0x2c;
	bytes[at + 5] = (MechU8) width;
	bytes[at + 6] = 0;
	bytes[at + 7] = (MechU8) height;
	bytes[at + 8] = 0;
	if (AsmNext(p_state) % 2) {
		imageFlags |= 0x80;
	}

	bytes[at + 9] = (MechU8) imageFlags;
	at += 0xa;
	if (imageFlags & 0x80) {
		at += 3u << ((imageFlags & 7) + 1);
	}

	bytes[at++] = (MechU8) bits;
	for (i = 0; i < width * height; i++) {
		MechU32 range = AsmNext(p_state) % 4 ? 4u : 1u << bits;

		pixels[i] = (MechU8) (AsmNext(p_state) % range);
	}

	length = EncodeGif(stream, pixels, width * height, bits, p_state);
	for (sent = 0; sent < length;) {
		MechU32 block = 1 + AsmNext(p_state) % 255;

		if (block > length - sent) {
			block = length - sent;
		}

		bytes[at++] = (MechU8) block;
		memcpy(bytes + at, stream + sent, block);
		at += block;
		sent += block;
	}

	if (AsmNext(p_state) % 4 == 0) {
		// A sub-block after the end code
		MechU32 block = 1 + AsmNext(p_state) % 20;

		bytes[at] = (MechU8) block;
		at += 1 + block;
	}

	bytes[at++] = 0;
	p_picture->m_size = at;
}

typedef MechS32 (*BlitGifFn)(PANE* p_view, MechU8* p_gif, MechU8* p_state);

static MechU8 g_gifState[GIF_STATE_SIZE];

// Arguments: two words that seed the target, the picture and the decoder's state (random); the
// state is compared by its hash.
static void RunBlitGif(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget target;
	Picture picture;
	MechU32 state = AsmSeed(p_args, 2);
	MechS32 result;
	AsmHash hash;
	MechU32 i;

	MakeBlitTarget(&target, &state, 1);
	MakeGif(&picture, &state);
	Fill(g_gifState, sizeof(g_gifState), &state);
	result = ((BlitGifFn) Function(p_module, "VFX_GIF_draw"))(&target.m_view, (MechU8*) picture.m_words, g_gifState);
	AsmOutputWord(p_output, (MechU32) result);
	OutputBlitTarget(p_output, &target);
	AsmHashInit(&hash);
	for (i = 0; i < GIF_STATE_SIZE; i += 4) {
		AsmHashWord(
			&hash,
			g_gifState[i] | (MechU32) g_gifState[i + 1] << 8 | (MechU32) g_gifState[i + 2] << 16 |
				(MechU32) g_gifState[i + 3] << 24
		);
	}

	AsmOutputWord(p_output, (MechU32) hash.m_value);
	AsmOutputWord(p_output, (MechU32) (hash.m_value >> 32));
}

static void RunReadGifPalette(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Picture picture;
	MechU8 palette[0x300];
	MechU32 state = AsmSeed(p_args, 2);

	MakeGif(&picture, &state);
	Fill(palette, sizeof(palette), &state);
	((ReadPaletteFn) Function(p_module, "VFX_GIF_palette"))((MechU8*) picture.m_words, palette);
	AsmOutputBytes(p_output, palette, sizeof(palette));
}

typedef MechS32 (*GetGifSizeFn)(void* p_gif);

static void RunGetGifSize(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Picture picture;
	MechU32 state = AsmSeed(p_args, 2);

	MakeGif(&picture, &state);
	AsmOutputWord(p_output, (MechU32) ((GetGifSizeFn) Function(p_module, "VFX_GIF_resolution"))(picture.m_words));
}

// --- Run-length encoding ---

#define RLE_SIZE 0x1800

typedef MechS32 (*EncodeViewRleFn)(PANE* p_view, MechU8 p_transparent, MechS32 p_x, MechS32 p_y, MechU8* p_out);

static MechU8 g_rleOut[RLE_SIZE];

// Arguments: four words that seed the target (one time in two with rows of up to 320 pixels, for
// skips and runs longer than a code holds), its pixels (blobs on the transparent color, a few
// pixels on it, long runs of a few colors, or noise), the position and the output, NULL in one
// case in four.
static void RunEncodeViewRle(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget target;
	MechU32 state = AsmSeed(p_args, 4);
	MechU8 transparent = (MechU8) AsmNext(&state);
	MechU8* pixels;
	MechU32 pattern = AsmNext(&state) % 4;
	MechU32 density = AsmNext(&state) % 4;
	MechU32 i;
	MechS32 x;
	MechS32 y;
	MechS32 result;
	MechU8* out;

	MakeBlitTarget(&target, &state, 1);
	if (AsmNext(&state) % 2) {
		MechS32 width = RandomRange(&state, 200, 320);

		target.m_buffer.m_xMax = width - 1;
		target.m_buffer.m_yMax = RandomRange(&state, 0, BLIT_WIDTH * BLIT_HEIGHT / width - 1);
		target.m_view.m_x0 = RandomRange(&state, -4, 8);
		target.m_view.m_y0 = RandomRange(&state, -2, 2);
		target.m_view.m_x1 = RandomRange(&state, width - 8, width + 4);
		target.m_view.m_y1 = target.m_buffer.m_yMax + RandomRange(&state, -1, 2);
	}

	pixels = (MechU8*) target.m_words;
	for (i = 0; i < BLIT_ARENA_SIZE;) {
		MechU32 bits = AsmNext(&state);
		MechU32 count = 1 + (bits >> 8) % (pattern == 2 ? 200 : 12);
		MechU8 color = (MechU8) (bits >> 16);

		for (; count && i < BLIT_ARENA_SIZE; count--, i++) {
			MechU32 kind = bits % (4 + density);

			if (pattern == 1) {
				kind = AsmNext(&state) % 128 ? 2 : 1;
			}
			else if (pattern == 2) {
				kind = bits % 8 ? 0 : 2;
			}
			else if (pattern == 3) {
				kind = AsmNext(&state) % 64 ? 1 : 2;
			}

			switch (kind) {
			case 0:
				pixels[i] = color;
				break;
			case 1:
				pixels[i] = (MechU8) AsmNext(&state);
				break;
			default:
				pixels[i] = transparent;
				break;
			}
		}
	}

	Fill(g_rleOut, sizeof(g_rleOut), &state);
	out = AsmNext(&state) % 4 ? g_rleOut : NULL;
	x = RandomCoordinate(&state, BLIT_WIDTH);
	y = RandomCoordinate(&state, BLIT_HEIGHT);
	result = ((EncodeViewRleFn) Function(p_module, "VFX_shape_scan"))(&target.m_view, transparent, x, y, out);
	AsmOutputWord(p_output, (MechU32) result);
	AsmOutputBytes(p_output, g_rleOut, RLE_SIZE);
}

// --- Dissolve and colors ---

typedef MechS32 (*DissolveViewFn)(PANE* p_src, PANE* p_dest, MechS32 p_count, MechS32 p_state);

// Whether two views have a pixel in common, from their origins, other than their origins: the
// LFSR never reaches 0, the state of (0, 0).
static MechS32 DissolveOverlaps(const BlitTarget* p_src, const BlitTarget* p_dest)
{
	MechS32 width = p_src->m_view.m_x1 - p_src->m_view.m_x0 + 1;
	MechS32 height = p_src->m_view.m_y1 - p_src->m_view.m_y0 + 1;
	MechS32 x;
	MechS32 y;

	if (p_dest->m_view.m_x1 - p_dest->m_view.m_x0 + 1 < width) {
		width = p_dest->m_view.m_x1 - p_dest->m_view.m_x0 + 1;
	}

	if (p_dest->m_view.m_y1 - p_dest->m_view.m_y0 + 1 < height) {
		height = p_dest->m_view.m_y1 - p_dest->m_view.m_y0 + 1;
	}

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			const PANE* views[2];
			MechS32 i;
			MechS32 in = 1;

			if (!x && !y) {
				continue;
			}

			views[0] = &p_src->m_view;
			views[1] = &p_dest->m_view;
			for (i = 0; i < 2; i++) {
				MechS32 px = x + views[i]->m_x0;
				MechS32 py = y + views[i]->m_y0;

				if (px < 0 || px > views[i]->m_window->m_xMax || px > views[i]->m_x1 || py < 0 ||
					py > views[i]->m_window->m_yMax || py > views[i]->m_y1) {
					in = 0;
				}
			}

			if (in) {
				return 1;
			}
		}
	}

	return 0;
}

// Arguments: four words that seed the views (of two buffers, or of one: they must share a
// pixel, the assembly looks for one forever otherwise), the count and the state (0 starts, or
// one the LFSR can be in).
static void RunDissolveView(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget src;
	BlitTarget dest;
	MechU32 state = AsmSeed(p_args, 4);
	MechU32 empty;
	MechS32 width;
	MechS32 height;
	MechU32 bits;
	MechU32 widthBits;
	MechS32 count;
	MechS32 seed;
	MechS32 result;

	MakeBlitTarget(&src, &state, 0);
	MakeBlitTarget(&dest, &state, 0);
	if (AsmNext(&state) % 4 == 0) {
		dest.m_view.m_window = &src.m_buffer;
	}

	empty = AsmNext(&state) % 16;

	if (src.m_view.m_x1 < src.m_view.m_x0 || src.m_view.m_y1 < src.m_view.m_y0 || dest.m_view.m_x1 < dest.m_view.m_x0 ||
		dest.m_view.m_y1 < dest.m_view.m_y0 || !DissolveOverlaps(&src, &dest)) {
		src.m_view.m_x0 = 0;
		src.m_view.m_y0 = 0;
		src.m_view.m_x1 = src.m_buffer.m_xMax;
		src.m_view.m_y1 = src.m_buffer.m_yMax;
		dest.m_view.m_x0 = 0;
		dest.m_view.m_y0 = 0;
		dest.m_view.m_x1 = dest.m_view.m_window->m_xMax;
		dest.m_view.m_y1 = dest.m_view.m_window->m_yMax;
		if (!DissolveOverlaps(&src, &dest)) {
			// Single pixels: a second one
			src.m_buffer.m_xMax++;
			src.m_view.m_x1++;
			dest.m_view.m_window->m_xMax++;
			dest.m_view.m_x1++;
			if (dest.m_view.m_window != &src.m_buffer) {
				dest.m_view.m_x1 = dest.m_view.m_window->m_xMax;
			}
		}
	}

	// An empty buffer or view, which it returns -1 or -2 for (the destination's after it reads the
	// source's)
	switch (empty) {
	case 0:
		src.m_buffer.m_xMax = -1;
		break;
	case 1:
		src.m_view.m_x0 = src.m_buffer.m_xMax + 1;
		src.m_view.m_x1 = src.m_view.m_x0 + 2;
		break;
	case 2:
		dest.m_view.m_window = &dest.m_buffer;
		dest.m_buffer.m_yMax = -1;
		break;
	case 3:
		dest.m_view.m_y0 = dest.m_view.m_window->m_yMax + 1;
		dest.m_view.m_y1 = dest.m_view.m_y0 + 2;
		break;
	}

	// A state of the LFSR, which runs over the bits of the views' common height and width (0
	// starts; from beyond them, it can reach 0, and stay there)
	width = src.m_view.m_x1 - src.m_view.m_x0 + 1;
	height = src.m_view.m_y1 - src.m_view.m_y0 + 1;
	if (dest.m_view.m_x1 - dest.m_view.m_x0 + 1 < width) {
		width = dest.m_view.m_x1 - dest.m_view.m_x0 + 1;
	}

	if (dest.m_view.m_y1 - dest.m_view.m_y0 + 1 < height) {
		height = dest.m_view.m_y1 - dest.m_view.m_y0 + 1;
	}

	for (bits = 0; (MechU32) height >> bits; bits++) {
	}

	for (widthBits = 0; (MechU32) width >> widthBits; widthBits++) {
	}

	count = RandomRange(&state, 0, 2 * BLIT_WIDTH * BLIT_HEIGHT);
	seed = AsmNext(&state) % 4 ? (MechS32) (1 + AsmNext(&state) % ((1u << (bits + widthBits)) - 1)) : 0;
	result = ((DissolveViewFn) Function(p_module, "VFX_pixel_fade"))(&src.m_view, &dest.m_view, count, seed);
	AsmOutputWord(p_output, (MechU32) result);
	OutputBlitTarget(p_output, &src);
	OutputBlitTarget(p_output, &dest);
}

typedef void (*FadeViewColorsFn)(WINDOW* p_buffer, MechU8* p_palette, MechS32 p_steps);

// Arguments: four words that seed the buffer (up to 6 by 6 pixels, of a few colors), the colors
// the driver reads, the palette (6-bit components mostly), the steps and the error terms.
static void RunFadeViewColors(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	WINDOW buffer;
	MechU8 pixels[36];
	MechU8 colors[8];
	MechU8 palette[0x300];
	MechU32 state = AsmSeed(p_args, 4);
	MechU32 wide = AsmNext(&state) % 4 == 0;
	MechU32 i;

	Fill(colors, sizeof(colors), &state);
	for (i = 0; i < sizeof(pixels); i++) {
		pixels[i] = colors[AsmNext(&state) % 8];
	}

	buffer.m_buffer = pixels;
	buffer.m_xMax = RandomRange(&state, 0, 5);
	buffer.m_yMax = RandomRange(&state, 0, 5);
	buffer.m_bitmapInfo = NULL;
	buffer.m_shadow = 0;
	for (i = 0; i < 0x300; i++) {
		MechU32 bits = AsmNext(&state);

		g_driverColors[i] = (MechU8) (wide ? bits : bits % 0x40);
		palette[i] = (MechU8) (wide ? bits >> 8 : (bits >> 8) % 0x40);
	}

	Fill((MechU8*) Data(p_module, "color_error"), 0x300, &state);
	MakeDriverTable();
	((SetDisplayDriverFn) Function(p_module, "VFX_register_driver"))(g_driverTable);
	((FadeViewColorsFn) Function(p_module, "VFX_window_fade"))(&buffer, palette, RandomRange(&state, 0, 400));
	AsmOutputBytes(p_output, (const MechU8*) Data(p_module, "color_error"), 0x300);
	OutputDriverLog(p_output);
}

typedef MechS32 (*CountViewColorsFn)(PANE* p_view, MechU32* p_out);

// Arguments: two words that seed the target (its view inside the buffer: the routine doesn't
// clip) and the output, NULL in one case in four.
static void RunCountViewColors(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	BlitTarget target;
	MechU32 colors[0x102];
	MechU32 state = AsmSeed(p_args, 2);
	MechU8* pixels;
	MechU32 palette = 1 + AsmNext(&state) % 40;
	MechS32 result;
	MechU32 i;

	MakeBlitTarget(&target, &state, 0);
	pixels = (MechU8*) target.m_words;
	for (i = 0; i < BLIT_ARENA_SIZE; i++) {
		pixels[i] = (MechU8) (AsmNext(&state) % palette * 7);
	}

	target.m_view.m_x0 = RandomRange(&state, 0, target.m_buffer.m_xMax);
	target.m_view.m_y0 = RandomRange(&state, 0, target.m_buffer.m_yMax);
	target.m_view.m_x1 = RandomRange(&state, target.m_view.m_x0, target.m_buffer.m_xMax);
	target.m_view.m_y1 = RandomRange(&state, target.m_view.m_y0, target.m_buffer.m_yMax);
	FillWords((MechS32*) colors, 0x102, &state);
	result = ((CountViewColorsFn)
				  Function(p_module, "VFX_color_scan"))(&target.m_view, AsmNext(&state) % 4 ? &colors[1] : NULL);
	AsmOutputWord(p_output, (MechU32) result);
	OutputWords(p_output, (const MechS32*) colors, 0x102);
}

// --- The table ---

const AsmRoutine g_asmRoutines[] = {
	// The fixed-point helpers
	{"FixedMul16", 2, NULL, NULL, NULL},
	{"FixedMul30", 2, NULL, NULL, NULL},
	{"FixedDiv16", 2, FixedDiv16Domain, NULL, NULL},
	{"FixedDiv29", 2, FixedDiv29Domain, NULL, NULL},
	{"FixedDivU16", 2, FixedDivU16Domain, NULL, NULL},
	{"FixedDot27", 6, NULL, NULL, NULL},
	{"FixedDot29", 6, NULL, NULL, NULL},
	{"MulDiv64", 3, MulDiv64Domain, NULL, NULL},
	// The other __asm functions
	{"ApproximateVectorLength", 3, NULL, NULL, NULL},
	{"FixedSqrtGuess", 1, FixedSqrtGuessDomain, RunFixedSqrtGuess, NULL},
	{"HashName", 2, NULL, RunNameHash, NULL},
	{"IntegrateMidpoint", 4, NULL, RunIntegrateMidpoint, NULL},
	{"IsWithinRadius", 4, NULL, NULL, NULL},
	{"ProjectRadius", 3, Domain10013340, NULL, NULL},
	{"FixedMul29", 2, NULL, NULL, NULL},
	{"UnprojectCoordinate", 4, NULL, NULL, NULL},
	{"CrossDiv", 5, Domain100349c0, NULL, NULL},
	{"Lerp", 5, Domain100349f0, NULL, NULL},
	{"ProjectCoordinate", 4, Domain10042740, NULL, NULL},
	{"MulRatio", 3, Domain1004c800, NULL, NULL},
	{"MulNormalize16", 5, NULL, Run1004c820, NULL},
	{"MulAddDiv", 4, Domain1004c860, NULL, NULL},
	{"MemCopy", 4, NULL, RunMemCopy, NULL},
	{"MemSet", 4, NULL, RunMemSet, NULL},
	{"FixedSin", 2, NULL, RunSine, HashSineInputs},
	{"FixedAsin", 2, ArcsineDomain, RunArcsine, HashArcsineInputs},
	{"FixedAtan2", 3, BearingDomain, RunBearing, HashBearingInputs},
	{"IsAboveHorizon", 9, Domain10071930, Run10071930, NULL},
	{"HorizonYAtX", 9, Domain100719ca, Run100719ca, NULL},
	{"HorizonXAtY", 9, Domain10071a4c, Run10071a4c, NULL},
	{"AllocProjectedVertex", 3, NULL, Run1007d248, NULL},
	{"AllocQueuedPolygon", 3, NULL, Run1007d296, NULL},
	// ticks.asm
	{"GameTickTimerCallback", 3, NULL, RunGameTickTimerCallback, NULL},
	{"AllocTicks", 3, NULL, RunAllocTicks, NULL},
	{"GetTicks", 3, NULL, RunGetTicks, NULL},
	{"ResetTicks", 3, NULL, RunResetTicks, NULL},
	{"SetTicks", 3, NULL, RunSetTicks, NULL},
	{"FreeTicks", 3, NULL, RunFreeTicks, NULL},
	{"PauseTimer", 3, NULL, RunPauseTimer, NULL},
	// sndunpack.asm
	{"DecodeSoundFrames", 4, NULL, RunSoundUnpack, NULL},
	// transform.c
	{"TransformPoint", 3, NULL, Run1000d650, NULL},
	{"RotatePoint", 3, NULL, Run1000d708, NULL},
	{"OrthogonalizeMatrixColumn", 2, NULL, Run1000d7c0, NULL},
	{"MatrixMul29", 2, NULL, NULL, NULL},
	{"MatrixDot29", 6, NULL, NULL, NULL},
	{"MultiplyRotations", 2, NULL, Run1000da0c, NULL},
	{"BuildMatrixEx", 8, NULL, Run1000de3b, HashMatrixInputs},
	// shapegeom.c
	{"TransformModel", 3, NULL, Run10039a30, NULL},
	{"TransformShapeCenter", 4, NULL, Run10039b94, NULL},
	{"SolvePlaneY", 6, Domain10039c96, NULL, NULL},
	{"ApproximateShapeDistance", 5, NULL, Run10039ccc, NULL},
	{"ComputeTriangleNormal", 9, Domain10039dda, Run10039dda, NULL},
	{"DivDifference17", 3, Domain1003a05d, NULL, NULL},
	{"RayShapeDistance", 9, Domain1003a096, Run1003a096, NULL},
	// objectanim.c
	{"GetViewVertex", 4, NULL, Run10048c50, NULL},
	{"ClipEdgeToNearPlane", 9, Domain10048d46, Run10048d46, NULL},
	{"ProjectVertex", 6, Domain10048ebe, Run10048ebe, NULL},
	{"GetFaceShade", 9, Domain10048faf, Run10048faf, NULL},
	{"QueueFace", 3, NULL, Run10049155, NULL},
	// VFX3D.ASM
	{"VFX_flat_polygon", 4, NULL, RunFillPolygonFlat, NULL},
	{"VFX_Gouraud_polygon", 4, NULL, Run1002ae41, NULL},
	{"VFX_dithered_Gouraud_polygon", 4, NULL, Run1002b68b, NULL},
	{"VFX_translate_polygon", 4, NULL, Run1002bf39, NULL},
	{"VFX_illuminate_polygon", 4, NULL, Run1002c48d, NULL},
	{"VFX_map_lookaside", 2, NULL, RunSetLumaTable, NULL},
	{"VFX_map_polygon", 4, NULL, RunFillPolygonTextured, NULL},
	// VFXREND.ASM
	{"VFX_set_Gouraud_dither_level", 2, NULL, RunSetDitherLevel, NULL},
	{"GetCodeBlock", 1, GetCodeBlockDomain, RunGetCodeBlock, NULL},
	{"VFX_polygon_render", 4, NULL, RunPolygonRender, NULL},
	{"F16_div_to_F30", 2, F16DivToF30Domain, NULL, NULL},
	{"F30_reciprocal", 1, F30ReciprocalDomain, NULL, NULL},
	{"mul_F30", 2, NULL, NULL, NULL},
	{"VFX_polygon_clip_XY_and_render", 4, NULL, RunPolygonClip, NULL},
	// VFXA.ASM
	{"VFX_driver_name", 2, NULL, RunGetDisplayDriverName, NULL},
	{"VFX_register_driver", 2, NULL, RunSetDisplayDriver, NULL},
	{"VFX_pixel_write", 4, NULL, RunPutViewPixel, NULL},
	{"VFX_pixel_read", 4, NULL, RunGetViewPixel, NULL},
	{"VFX_line_draw", 4, BlitLineDomain, RunBlitLine, NULL},
	{"VFX_rectangle_hash", 4, NULL, Run10032e4b, NULL},
	{"VFX_shape_draw", 4, NULL, RunBlitShpFrame, NULL},
	{"DrawShapeUnclipped", 4, NULL, RunBlitShpFrameUnclipped, NULL},
	{"VFX_shape_lookaside", 2, NULL, RunSetRemapTable, NULL},
	{"VFX_shape_translate_draw", 4, NULL, RunBlitShpFrameRemapped, NULL},
	{"XlatShapeUnclipped", 4, NULL, RunBlitShpFrameRemappedUnclipped, NULL},
	{"VFX_shape_transform", 4, NULL, RunBlitRotated, NULL},
	{"VFX_shape_visible_rectangle", 2, NULL, Run10034622, NULL},
	{"VFX_shape_scan", 4, NULL, RunEncodeViewRle, NULL},
	{"VFX_shape_remap_colors", 2, NULL, RunRemapShpFrame, NULL},
	{"VFX_pane_wipe", 2, NULL, RunFillView, NULL},
	{"VFX_pane_copy", 4, NULL, RunBlitView, NULL},
	{"VFX_pane_scroll", 4, ScrollViewDomain, RunScrollView, NULL},
	{"VFX_ellipse_draw", 4, NULL, RunDrawEllipse, NULL},
	{"VFX_ellipse_fill", 4, NULL, RunFillEllipse, NULL},
	{"VFX_Cos_Sin", 1, NULL, RunGetCosSin, NULL},
	{"VFX_fixed_mul", 2, NULL, RunBlitFixedMul16, NULL},
	{"VFX_point_transform", 7, NULL, RunRotateScalePoint, NULL},
	{"VFX_font_height", 1, NULL, RunFontGetHeight, NULL},
	{"VFX_character_width", 2, NULL, RunFontGetCharWidth, NULL},
	{"VFX_character_draw", 4, NULL, RunBlitChar, NULL},
	{"VFX_string_draw", 4, NULL, RunBlitString, NULL},
	{"VFX_line_to_pane", 4, NULL, RunWriteViewRow, NULL},
	{"find_ILBM_property", 2, NULL, RunFindIffChunk, NULL},
	{"VFX_ILBM_draw", 2, NULL, RunBlitIff, NULL},
	{"VFX_ILBM_palette", 2, NULL, RunReadIffPalette, NULL},
	{"VFX_ILBM_resolution", 2, NULL, RunGetIffSize, NULL},
	{"VFX_PCX_draw", 2, NULL, RunBlitPicture, NULL},
	{"VFX_PCX_palette", 2, NULL, RunReadPicturePalette, NULL},
	{"VFX_PCX_resolution", 2, NULL, RunGetPictureSize, NULL},
	{"VFX_GIF_draw", 2, NULL, RunBlitGif, NULL},
	{"VFX_GIF_palette", 2, NULL, RunReadGifPalette, NULL},
	{"VFX_GIF_resolution", 2, NULL, RunGetGifSize, NULL},
	{"VFX_shape_bounds", 2, NULL, RunGetShpFrameSize, NULL},
	{"VFX_shape_origin", 2, NULL, Run10037526, NULL},
	{"VFX_shape_resolution", 2, NULL, RunGetShpFrameExtent, NULL},
	{"VFX_shape_minxy", 2, NULL, RunGetShpFrameOrigin, NULL},
	{"VFX_shape_palette", 2, NULL, Run100375a7, NULL},
	{"VFX_shape_colors", 2, NULL, Run100375f2, NULL},
	{"VFX_shape_set_colors", 2, NULL, Run1003763a, NULL},
	{"VFX_shape_count", 2, NULL, RunGetShpFrameCount, NULL},
	{"VFX_shape_list", 2, NULL, RunCountShpUniqueFrames, NULL},
	{"VFX_shape_palette_list", 2, NULL, Run100376f9, NULL},
	{"VFX_pixel_fade", 4, NULL, RunDissolveView, NULL},
	{"VFX_window_fade", 4, NULL, RunFadeViewColors, NULL},
	{"VFX_color_scan", 2, NULL, RunCountViewColors, NULL},
};

const MechS32 g_asmRoutineCount = sizeof(g_asmRoutines) / sizeof(g_asmRoutines[0]);
