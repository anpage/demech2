// The routines under test: how each one's cases are set up, called and compared, and for which
// arguments the original computes a result.

#include "asmequiv.h"
#include "copperwren.h"
#include "duskmoth.h"
#include "emberfern.h"
#include "eyepoint.h"
#include "ivorydelta.h"
#include "portable.h"
#include "ray.h"
#include "slateheron.h"
#include "transform.h"
#include "types.h"
#include "unk100335d0.h"
#include "unk10039a30.h"
#include "unk1003a530.h"

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
	AsmOutputWord(p_output, ((NameHashFn) Function(p_module, "FUN_100074e0"))((const MechChar*) name));
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
	) Function(p_module, "FUN_1004c820"))(&cells[1], &cells[3], &shifts[1], (MechU32) p_args[3], (MechU32) p_args[4]);
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

// --- The record stacks (unk1007d120.c) ---

typedef CopperWren0x20* (*PopVertexFn)(void);
typedef MechU8* (*PopRecordFn)(void);

// Arguments: the top's offset in the arena (0x20 to 0x200), the bottom's (0 to 0x1f4), both
// dword-aligned as the game's are, and the flag the routines clear.
static void RunRecordStack(const AsmModule* p_module, MechS32 p_top, const MechS32* p_args, AsmOutput* p_output)
{
	MechU32 words[ARENA_SIZE / 4];
	MechU8* arena = (MechU8*) words;
	MechU8** top = (MechU8**) Data(p_module, "g_unk0x100c1a70");
	MechU8** bottom = (MechU8**) Data(p_module, "g_unk0x100c2698");
	MechS32* flag = (MechS32*) Data(p_module, "g_unk0x1010b5ac");
	MechU32 state = AsmSeed(p_args, 3);
	void* result;

	Fill(arena, ARENA_SIZE, &state);
	*top = arena + 0x20 + (MechU32) p_args[0] % ((ARENA_SIZE - 0x20) / 4 + 1) * 4;
	*bottom = arena + (MechU32) p_args[1] % ((ARENA_SIZE - 0xc) / 4 + 1) * 4;
	*flag = p_args[2];
	if (p_top) {
		result = ((PopVertexFn) Function(p_module, "FUN_1007d248"))();
	}
	else {
		result = ((PopRecordFn) Function(p_module, "FUN_1007d296"))();
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

// --- Trigonometry (unk100696c0.c, over clock.c's tables) ---

#define TABLE_SIZE 0x102

typedef MechS32 (*TrigFn)(MechS32 p_value);
typedef MechS32 (*BearingFn)(MechS32 p_x, MechS32 p_z);

enum {
	c_tableGame,       // the game's, as FUN_1007c930 computes it
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
	AsmOutputWord(p_output, (MechU32) ((TrigFn) Function(p_module, "FUN_100696c0"))(p_args[0]));
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
	AsmOutputWord(p_output, (MechU32) ((TrigFn) Function(p_module, "FUN_1006975b"))(ArcsineArgument(p_args, table)));
}

// The search and the idiv of FUN_1006975b, over the case's table: the idiv faults when the
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
	AsmOutputWord(p_output, (MechU32) ((BearingFn) Function(p_module, "FUN_100698de"))(p_args[0], p_args[1]));
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

// --- The horizon (unk10071930.c) ---

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
	p_eyepoint->m_unk0x54.m_rows[0][1] = p_args[c_argA];
	p_eyepoint->m_unk0x94 = p_args[c_argB];
	p_eyepoint->m_unk0x54.m_rows[1][1] = p_args[c_argC];
	p_eyepoint->m_unk0x98 = p_args[c_argD];
	p_eyepoint->m_unk0x54.m_rows[2][1] = p_args[c_argE];
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
	result = ((HorizonTestFn) Function(p_module, "FUN_10071930"))(p_args[c_argX], p_args[c_argY], &eyepoint);
	OutputEyepoint(p_output, result, &eyepoint);
}

static void Run100719ca(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Eyepoint eyepoint;
	MechS32 result;

	MakeEyepoint(&eyepoint, p_args);
	result = ((HorizonFn) Function(p_module, "FUN_100719ca"))(p_args[c_argX], &eyepoint);
	OutputEyepoint(p_output, result, &eyepoint);
}

static void Run10071a4c(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	Eyepoint eyepoint;
	MechS32 result;

	MakeEyepoint(&eyepoint, p_args);
	result = ((HorizonFn) Function(p_module, "FUN_10071a4c"))(p_args[c_argY], &eyepoint);
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
	MechU8* scratch = (MechU8*) Data(p_module, "g_unk0x100a2f04");
	MechU8* frame = (MechU8*) Data(p_module, "g_unk0x100a3304");
	MechU8* deltas = (MechU8*) Data(p_module, "g_unk0x100a3705");
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

	end = ((SoundUnpackFn) Function(p_module, "FUN_1001a63c"))(src, dst, count, frameSize, &value);

	AsmOutputWord(p_output, end ? (MechU32) (end - src) : 0xffffffff);
	AsmOutputWord(p_output, (MechU32) value);
	AsmOutputBytes(p_output, dst, SOUND_DST_SIZE);
	memset(globals, 0, sizeof(globals));
	memcpy(globals, scratch, SOUND_SCRATCH_SIZE);
	memcpy(globals + SOUND_SCRATCH_SIZE, frame, SOUND_FRAME_BUFFER_SIZE);
	memcpy(globals + SOUND_SCRATCH_SIZE + SOUND_FRAME_BUFFER_SIZE, deltas, SOUND_DELTAS_SIZE);
	AsmOutputBytes(p_output, globals, SOUND_GLOBALS_SIZE);
}

// --- Fixed-point transforms (transform.c, unk10039a30.c) ---

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
	RunTransformPoint(p_module, "FUN_1000d650", p_args, p_output);
}

static void Run1000d708(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunTransformPoint(p_module, "FUN_1000d708", p_args, p_output);
}

typedef void (*CrossColumnFn)(Matrix* p_matrix, MechS32 p_column);

// Arguments: the column (-1 to 4: the others store nothing), and the matrix's seed.
static void Run1000d7c0(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	GuardedMatrix matrix;
	MechU32 state = AsmSeed(p_args, 2);

	MakeMatrix(&matrix, &state);
	((CrossColumnFn) Function(p_module, "FUN_1000d7c0"))(&matrix.m_matrix, (MechS32) ((MechU32) p_args[0] % 6) - 1);
	OutputMatrix(p_output, &matrix);
}

typedef void (*ComposeFn)(Matrix* p_a, Matrix* p_b, Matrix* p_result);

// Arguments: where the product goes (a third matrix, either operand, or the one matrix all three
// are), and the matrices' seed.
static void Run1000da0c(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	GuardedMatrix matrices[3];
	MechU32 state = AsmSeed(p_args, 2);
	ComposeFn fn = (ComposeFn) Function(p_module, "FUN_1000da0c");
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

	((BuildMatrixFn) Function(p_module, "FUN_1000de3b"))(
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
static void MakeVertex(EmberFern0x2c* p_vertex, MechU32* p_state)
{
	p_vertex->m_unk0x00 = RandomWord(p_state);
	p_vertex->m_unk0x04 = RandomWord(p_state);
	p_vertex->m_unk0x08 = RandomWord(p_state);
	p_vertex->m_unk0x0c = RandomWord(p_state);
	p_vertex->m_unk0x10 = RandomWord(p_state);
	p_vertex->m_unk0x14 = RandomWord(p_state);
	p_vertex->m_unk0x18 = (MechU32) RandomWord(p_state);
	p_vertex->m_unk0x1c = (MechU32) RandomWord(p_state);
	p_vertex->m_unk0x20 = (MechU32) RandomWord(p_state);
	p_vertex->m_unk0x24 = NULL;
	Fill(&p_vertex->m_unk0x28, 1, p_state);
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

static void OutputVertex(AsmOutput* p_output, const EmberFern0x2c* p_vertex, const Region* p_regions, MechS32 p_count)
{
	AsmOutputWord(p_output, (MechU32) p_vertex->m_unk0x00);
	AsmOutputWord(p_output, (MechU32) p_vertex->m_unk0x04);
	AsmOutputWord(p_output, (MechU32) p_vertex->m_unk0x08);
	AsmOutputWord(p_output, (MechU32) p_vertex->m_unk0x0c);
	AsmOutputWord(p_output, (MechU32) p_vertex->m_unk0x10);
	AsmOutputWord(p_output, (MechU32) p_vertex->m_unk0x14);
	AsmOutputWord(p_output, p_vertex->m_unk0x18);
	AsmOutputWord(p_output, p_vertex->m_unk0x1c);
	AsmOutputWord(p_output, p_vertex->m_unk0x20);
	AsmOutputWord(p_output, Locate(p_regions, p_count, p_vertex->m_unk0x24));
	AsmOutputWord(
		p_output,
		p_vertex->m_unk0x28 | ((MechU32) p_vertex->m_unk0x29[0] << 8) | ((MechU32) p_vertex->m_unk0x29[1] << 16) |
			((MechU32) p_vertex->m_unk0x29[2] << 24)
	);
}

static void MakeFace(DuskMoth0x24* p_face, MechU32* p_state)
{
	p_face->m_unk0x00 = (MechU16) AsmNext(p_state);
	p_face->m_unk0x02 = (MechU16) AsmNext(p_state);
	p_face->m_unk0x04 = AsmNext(p_state);
	p_face->m_unk0x08 = RandomWord(p_state);
	p_face->m_unk0x0c = RandomWord(p_state);
	p_face->m_unk0x10 = RandomWord(p_state);
	p_face->m_normal[0] = RandomWord(p_state);
	p_face->m_normal[1] = RandomWord(p_state);
	p_face->m_normal[2] = RandomWord(p_state);
	p_face->m_unk0x20 = NULL;
}

static void OutputFace(AsmOutput* p_output, const DuskMoth0x24* p_face)
{
	AsmOutputWord(p_output, p_face->m_unk0x00 | ((MechU32) p_face->m_unk0x02 << 16));
	AsmOutputWord(p_output, p_face->m_unk0x04);
	AsmOutputWord(p_output, (MechU32) p_face->m_unk0x08);
	AsmOutputWord(p_output, (MechU32) p_face->m_unk0x0c);
	AsmOutputWord(p_output, (MechU32) p_face->m_unk0x10);
	OutputWords(p_output, p_face->m_normal, 3);
	AsmOutputWord(p_output, p_face->m_unk0x20 ? 1 : 0);
}

#define MODEL_VERTICES_MAX 6
#define MODEL_FACES_MAX 6

// A model as the game lays it out: the header, the vertices right after it and the faces at an
// offset, here after a vertex past the last one, and with a face past the last one.
typedef struct ModelBuffer {
	GraniteLattice0x18 m_header;
	EmberFern0x2c m_vertices[MODEL_VERTICES_MAX + 1];
	DuskMoth0x24 m_faces[MODEL_FACES_MAX + 1];
} ModelBuffer;

typedef void (*TransformModelFn)(GraniteLattice0x18* p_model, Matrix* p_matrix);

// Arguments: the vertex count (1 to MODEL_VERTICES_MAX), the face count (1 to MODEL_FACES_MAX),
// and the seed of the rest. A count of 0 runs 0x10000 times.
static void Run10039a30(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	ModelBuffer model;
	GuardedMatrix matrix;
	MechU32 state = AsmSeed(p_args, 3);
	MechS32 i;

	model.m_header.m_unk0x00 = PortableS32(AsmNext(&state));
	model.m_header.m_unk0x04 = (MechS16) (1 + (MechU32) p_args[0] % MODEL_VERTICES_MAX);
	model.m_header.m_unk0x06 = (MechS16) (1 + (MechU32) p_args[1] % MODEL_FACES_MAX);
	model.m_header.m_unk0x08 = (MechU32) offsetof(ModelBuffer, m_faces);
	model.m_header.m_unk0x0c = NULL;
	model.m_header.m_unk0x10 = AsmNext(&state);
	model.m_header.m_unk0x14 = (MechU16) AsmNext(&state);
	Fill(model.m_header.m_unk0x16, sizeof(model.m_header.m_unk0x16), &state);
	for (i = 0; i <= MODEL_VERTICES_MAX; i++) {
		MakeVertex(&model.m_vertices[i], &state);
	}

	for (i = 0; i <= MODEL_FACES_MAX; i++) {
		MakeFace(&model.m_faces[i], &state);
	}

	MakeMatrix(&matrix, &state);
	((TransformModelFn) Function(p_module, "FUN_10039a30"))(&model.m_header, &matrix.m_matrix);

	AsmOutputWord(p_output, (MechU32) model.m_header.m_unk0x00);
	AsmOutputWord(p_output, (MechU16) model.m_header.m_unk0x04 | ((MechU32) (MechU16) model.m_header.m_unk0x06 << 16));
	AsmOutputWord(p_output, model.m_header.m_unk0x08 == offsetof(ModelBuffer, m_faces) && !model.m_header.m_unk0x0c);
	AsmOutputWord(p_output, model.m_header.m_unk0x10);
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
static void MakeShape(ScarletOrchid0x4c* p_shape, MechU32* p_state)
{
	p_shape->m_unk0x00 = (MechU16) AsmNext(p_state);
	p_shape->m_unk0x02 = (MechU16) AsmNext(p_state);
	p_shape->m_unk0x04 = NULL;
	p_shape->m_unk0x08 = NULL;
	p_shape->m_unk0x0c = NULL;
	p_shape->m_unk0x10 = NULL;
	p_shape->m_unk0x14 = (MechU16) AsmNext(p_state);
	p_shape->m_unk0x16 = (MechU16) AsmNext(p_state);
	p_shape->m_unk0x18 = NULL;
	p_shape->m_unk0x1c = NULL;
	p_shape->m_unk0x20 = NULL;
	p_shape->m_unk0x24 = RandomWord(p_state);
	p_shape->m_unk0x28 = RandomWord(p_state);
	p_shape->m_unk0x2c = RandomWord(p_state);
	p_shape->m_unk0x30 = RandomWord(p_state);
	p_shape->m_unk0x34 = RandomWord(p_state);
	p_shape->m_unk0x38 = RandomWord(p_state);
	p_shape->m_unk0x3c = RandomWord(p_state);
	p_shape->m_unk0x40 = RandomWord(p_state);
	p_shape->m_unk0x44 = NULL;
	p_shape->m_unk0x48 = (MechU32) RandomWord(p_state);
}

static void OutputShape(AsmOutput* p_output, const ScarletOrchid0x4c* p_shape)
{
	AsmOutputWord(p_output, p_shape->m_unk0x00 | ((MechU32) p_shape->m_unk0x02 << 16));
	AsmOutputWord(p_output, p_shape->m_unk0x14 | ((MechU32) p_shape->m_unk0x16 << 16));
	AsmOutputWord(p_output, (MechU32) p_shape->m_unk0x24);
	AsmOutputWord(p_output, (MechU32) p_shape->m_unk0x28);
	AsmOutputWord(p_output, (MechU32) p_shape->m_unk0x2c);
	AsmOutputWord(p_output, (MechU32) p_shape->m_unk0x30);
	AsmOutputWord(p_output, (MechU32) p_shape->m_unk0x34);
	AsmOutputWord(p_output, (MechU32) p_shape->m_unk0x38);
	AsmOutputWord(p_output, (MechU32) p_shape->m_unk0x3c);
	AsmOutputWord(p_output, (MechU32) p_shape->m_unk0x40);
	AsmOutputWord(p_output, p_shape->m_unk0x48);
}

typedef void (*TransformShapeFn)(ScarletOrchid0x4c* p_shape, Matrix* p_matrix);

// Arguments: the shape's position, and the seed of the rest.
static void Run10039b94(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	ScarletOrchid0x4c shape;
	GuardedMatrix matrix;
	MechU32 state = AsmSeed(p_args, 4);

	MakeShape(&shape, &state);
	MakeMatrix(&matrix, &state);
	shape.m_unk0x28 = p_args[0];
	shape.m_unk0x2c = p_args[1];
	shape.m_unk0x30 = p_args[2];
	((TransformShapeFn) Function(p_module, "FUN_10039b94"))(&shape, &matrix.m_matrix);
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

typedef MechS32 (*ShapeDistanceFn)(ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z);

// Arguments: the point, the shape's radius, and where its center is (NearValue, from bits 0-5).
static void Run10039ccc(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	ScarletOrchid0x4c shape;
	MechU32 state = AsmSeed(p_args, 5);
	MechS32 result;

	MakeShape(&shape, &state);
	shape.m_unk0x40 = p_args[3];
	shape.m_unk0x34 = NearValue(p_args[0], p_args[3], (MechU32) p_args[4], &state);
	shape.m_unk0x38 = NearValue(p_args[1], p_args[3], (MechU32) p_args[4] >> 2, &state);
	shape.m_unk0x3c = NearValue(p_args[2], p_args[3], (MechU32) p_args[4] >> 4, &state);
	result = ((ShapeDistanceFn) Function(p_module, "FUN_10039ccc"))(&shape, p_args[0], p_args[1], p_args[2]);
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

// FUN_10039dda's scaled normal (its words before the division), and the divisor; returns 0 for
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
	result = ((TriangleNormalFn) Function(p_module, "FUN_10039dda"))(
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
static void MakeRayCase(const MechS32* p_args, ScarletOrchid0x4c* p_shape, Ray* p_ray)
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

	p_shape->m_unk0x40 = p_args[6];
	p_shape->m_unk0x34 = NearValue(p_args[0], p_args[6], selector >> 3, &state);
	p_shape->m_unk0x38 = NearValue(p_args[1], p_args[6], selector >> 5, &state);
	p_shape->m_unk0x3c = NearValue(p_args[2], p_args[6], selector >> 7, &state);
}

static MechU64 Square(MechS32 p_value)
{
	return (MechU64) ((MechS64) p_value * p_value);
}

// FUN_1003a096 up to its idiv.
static MechS32 Domain1003a096(const MechS32* p_args)
{
	ScarletOrchid0x4c shape;
	Ray ray;
	MechS32 deltaX;
	MechS32 deltaY;
	MechS32 deltaZ;
	MechU64 partial;
	MechU64 dot;

	MakeRayCase(p_args, &shape, &ray);
	if (shape.m_unk0x40 <= 0) {
		return c_domainIn;
	}

	deltaX = Difference(shape.m_unk0x34, ray.m_x0);
	deltaY = Difference(shape.m_unk0x38, ray.m_y0);
	deltaZ = Difference(shape.m_unk0x3c, ray.m_z0);
	if (Square(deltaX) + Square(deltaY) + Square(deltaZ) < Square(shape.m_unk0x40)) {
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

typedef MechS32 (*RayDistanceFn)(ScarletOrchid0x4c* p_shape, Ray* p_ray);

// Arguments: MakeRayCase's.
static void Run1003a096(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	ScarletOrchid0x4c shape;
	Ray ray;
	MechS32 result;

	MakeRayCase(p_args, &shape, &ray);
	result = ((RayDistanceFn) Function(p_module, "FUN_1003a096"))(&shape, &ray);
	AsmOutputWord(p_output, (MechU32) result);
	OutputShape(p_output, &shape);
	OutputRay(p_output, &ray);
}

// --- Projection and clipping (unk10046750.c) ---

// The view globals (unk1004b980.c) the routines read.
enum {
	c_viewNear,                      // g_unk0x100ea820
	c_viewShiftX,                    // g_unk0x100ea824
	c_viewShiftY,                    // g_unk0x100ea828
	c_viewFar,                       // g_unk0x100ea82c
	c_viewLeft,                      // g_unk0x100ea830
	c_viewCenterX,                   // g_unk0x100ea834
	c_viewBottom,                    // g_unk0x100ea840
	c_viewRight,                     // g_unk0x100ea84c
	c_viewTop,                       // g_unk0x100ea850
	c_viewCenterY,                   // g_unk0x100ea858
	c_viewRowX,                      // g_unk0x100ea864, 868, 86c
	c_viewRowY = c_viewRowX + 3,     // g_unk0x100ea870, 874, 878
	c_viewRowDepth = c_viewRowY + 3, // g_unk0x100ea87c, 880, 884
	c_viewEyeY = c_viewRowDepth + 3, // g_unk0x100ea8b4
	c_viewEyeX,                      // g_unk0x100ea8b8
	c_viewEyeZ,                      // g_unk0x100ea8bc
	c_viewLightZ,                    // g_unk0x100ea8c0
	c_viewLightX,                    // g_unk0x100ea8c4
	c_viewLightY,                    // g_unk0x100ea8c8
	c_viewCount
};

static const char* const g_viewNames[c_viewCount] = {
	"g_unk0x100ea820", "g_unk0x100ea824", "g_unk0x100ea828", "g_unk0x100ea82c", "g_unk0x100ea830",
	"g_unk0x100ea834", "g_unk0x100ea840", "g_unk0x100ea84c", "g_unk0x100ea850", "g_unk0x100ea858",
	"g_unk0x100ea864", "g_unk0x100ea868", "g_unk0x100ea86c", "g_unk0x100ea870", "g_unk0x100ea874",
	"g_unk0x100ea878", "g_unk0x100ea87c", "g_unk0x100ea880", "g_unk0x100ea884", "g_unk0x100ea8b4",
	"g_unk0x100ea8b8", "g_unk0x100ea8bc", "g_unk0x100ea8c0", "g_unk0x100ea8c4", "g_unk0x100ea8c8",
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

// FUN_10048c50's view-space x, y (p_row c_viewRowX, c_viewRowY) or depth (c_viewRowDepth) of a
// vertex.
static MechS32 ViewValue(const MechS32* p_view, MechS32 p_row, const EmberFern0x2c* p_vertex)
{
	MechU64 sum = (MechU64) ((MechS64) p_view[p_row] * Difference(p_vertex->m_unk0x0c, p_view[c_viewEyeX])) +
				  (MechU64) ((MechS64) p_view[p_row + 1] * Difference(p_vertex->m_unk0x10, p_view[c_viewEyeY])) +
				  (MechU64) ((MechS64) p_view[p_row + 2] * Difference(p_vertex->m_unk0x14, p_view[c_viewEyeZ]));

	return PortableS32(PortableShrdRound(sum, 27));
}

#define RECORD_ARENA_SIZE 0x600

// The two record stacks' buffer (unk1007d120.c): the projected vertices from the top, the
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
	p_stacks->m_top = (MechU8**) Data(p_module, "g_unk0x100c1a70");
	p_stacks->m_bottom = (MechU8**) Data(p_module, "g_unk0x100c2698");
	p_stacks->m_flag = (MechS32*) Data(p_module, "g_unk0x1010b5ac");
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

static void FillRecords(CopperWren0x20* p_records, MechS32 p_count, MechU32* p_state)
{
	Fill((MechU8*) p_records, p_count * (MechU32) sizeof(CopperWren0x20), p_state);
}

static void OutputRecord(AsmOutput* p_output, const CopperWren0x20* p_record)
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

typedef CopperWren0x20* (*ProjectVertexFn)(EmberFern0x2c* p_vertex);

// Arguments: the vertex's position, and whether it has a projected copy already (one case in
// four, from the last).
static void Run10048c50(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RecordStacks stacks;
	EmberFern0x2c vertex;
	CopperWren0x20 copy;
	MechS32 view[c_viewCount];
	Region regions[2];
	MechU32 state = AsmSeed(p_args, 4);
	MechU32 top = 0x20 + AsmNext(&state) % ((RECORD_ARENA_SIZE - 0x20) / 4 + 1) * 4;
	CopperWren0x20* result;

	MakeView(view, &state);
	SetView(p_module, view);
	SetUpRecords(p_module, &stacks, top, AsmNext(&state) % (top / 4) * 4, &state);
	FillRecords(&copy, 1, &state);
	MakeVertex(&vertex, &state);
	vertex.m_unk0x0c = p_args[0];
	vertex.m_unk0x10 = p_args[1];
	vertex.m_unk0x14 = p_args[2];
	if (!(p_args[3] & 3)) {
		vertex.m_unk0x24 = &copy;
	}

	result = ((ProjectVertexFn) Function(p_module, "FUN_10048c50"))(&vertex);
	regions[0].m_start = stacks.m_words;
	regions[0].m_size = RECORD_ARENA_SIZE;
	regions[1].m_start = &copy;
	regions[1].m_size = sizeof(copy);
	AsmOutputWord(p_output, Locate(regions, 2, result));
	OutputVertex(p_output, &vertex, regions, 2);
	OutputRecord(p_output, &copy);
	OutputRecords(p_output, &stacks, regions, 2);
}

// An end of an edge FUN_10048d46 clips: its vertex, and the projected copy it may have already.
typedef struct ClipEnd {
	EmberFern0x2c m_vertex;
	CopperWren0x20 m_copy;
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
			end->m_vertex.m_unk0x20 = (MechU32) p_args[i];
		}
	}

	p_case->m_same = (p_args[7] >> 2) & 1;
}

// An end's position, depth and texture coordinates, as FUN_10048d46 reads them.
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
		p_values[2] = PortableS32(end->m_vertex.m_unk0x18 << 16);
		p_values[3] = PortableS32(end->m_vertex.m_unk0x1c << 16);
		p_values[4] = PortableS32(end->m_vertex.m_unk0x20);
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

typedef CopperWren0x20* (*ClipEdgeFn)(EmberFern0x2c* p_a, EmberFern0x2c* p_b);

// Arguments: MakeClipCase's (arguments 5 and 6 are unused).
static void Run10048d46(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	ClipCase clip;
	RecordStacks stacks;
	Region regions[3];
	MechU32 state = AsmSeed(p_args, 9) ^ 0x5bd1e995;
	CopperWren0x20* result;
	MechS32 i;

	MakeClipCase(p_args, &clip);
	SetView(p_module, clip.m_view);
	SetUpRecords(p_module, &stacks, RECORD_ARENA_SIZE - AsmNext(&state) % 0x40 * 4, AsmNext(&state) % 0x40 * 4, &state);
	for (i = 0; i < 2; i++) {
		if (clip.m_ends[i].m_hasCopy) {
			clip.m_ends[i].m_vertex.m_unk0x24 = &clip.m_ends[i].m_copy;
		}
	}

	result = ((
		ClipEdgeFn
	) Function(p_module, "FUN_10048d46"))(&clip.m_ends[0].m_vertex, &clip.m_ends[clip.m_same ? 0 : 1].m_vertex);
	regions[0].m_start = stacks.m_words;
	regions[0].m_size = RECORD_ARENA_SIZE;
	regions[1].m_start = &clip.m_ends[0].m_copy;
	regions[1].m_size = sizeof(CopperWren0x20);
	regions[2].m_start = &clip.m_ends[1].m_copy;
	regions[2].m_size = sizeof(CopperWren0x20);
	AsmOutputWord(p_output, Locate(regions, 3, result));
	for (i = 0; i < 2; i++) {
		OutputVertex(p_output, &clip.m_ends[i].m_vertex, regions, 3);
		OutputRecord(p_output, &clip.m_ends[i].m_copy);
	}

	OutputRecords(p_output, &stacks, regions, 3);
}

// The polygon list FUN_10048ebe adds to, and its outcodes.
typedef struct PolygonPoints {
	MechU8* m_or;
	MechU8* m_and;
	CopperWren0x20** m_points;
	MechS32* m_count;
} PolygonPoints;

static void FindPolygonPoints(const AsmModule* p_module, PolygonPoints* p_points)
{
	p_points->m_or = (MechU8*) Data(p_module, "g_unk0x1010b53c");
	p_points->m_and = (MechU8*) Data(p_module, "g_unk0x1010b5b8");
	p_points->m_points = (CopperWren0x20**) Data(p_module, "g_unk0x1010b550");
	p_points->m_count = (MechS32*) Data(p_module, "g_unk0x1010b5b0");
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
static void MakeProjectCase(const MechS32* p_args, CopperWren0x20* p_record, MechS32* p_view, MechU32* p_state)
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
	CopperWren0x20 record;
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

typedef CopperWren0x20* (*AddPointFn)(CopperWren0x20* p_vertex);

// Arguments: MakeProjectCase's (argument 5 is unused). The list has 0 to 21 points.
static void Run10048ebe(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	CopperWren0x20 record;
	CopperWren0x20 others[20];
	MechS32 view[c_viewCount];
	PolygonPoints points;
	MechS32* flag = (MechS32*) Data(p_module, "g_unk0x1010b5ac");
	Region regions[2];
	MechU32 state = AsmSeed(p_args, 6);
	CopperWren0x20* result;
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

	result = ((AddPointFn) Function(p_module, "FUN_10048ebe"))(&record);
	regions[0].m_start = &record;
	regions[0].m_size = sizeof(record);
	regions[1].m_start = others;
	regions[1].m_size = sizeof(others);
	AsmOutputWord(p_output, Locate(regions, 2, result));
	OutputRecord(p_output, &record);
	OutputPolygonPoints(p_output, &points, regions, 2);
	AsmOutputWord(p_output, (MechU32) *flag);
}

// A face and the indices of its vertices, at m_unk0x04 from it.
typedef struct FaceBuffer {
	DuskMoth0x24 m_face;
	MechU8 m_indices[24];
} FaceBuffer;

#define SHADE_VERTICES 4

// A shading case: the face's first vertex (its position is arguments 0-2), the light (3-5; one
// case in eight on the vertex instead), the face's normal (6-8), whether the vertex is taken to be
// at the origin, and the square root table.
typedef struct ShadeCase {
	FaceBuffer m_face;
	EmberFern0x2c m_vertices[SHADE_VERTICES];
	MechS32 m_view[c_viewCount];
	MechS32 m_atOrigin;
	MechS16 m_table[SQRT_TABLE_SIZE];
} ShadeCase;

static void MakeShadeCase(const MechS32* p_args, ShadeCase* p_case)
{
	MechU32 state = AsmSeed(p_args, 9);
	EmberFern0x2c* vertex;
	MechS32 i;

	for (i = 0; i < SHADE_VERTICES; i++) {
		MakeVertex(&p_case->m_vertices[i], &state);
	}

	MakeFace(&p_case->m_face.m_face, &state);
	Fill(p_case->m_face.m_indices, sizeof(p_case->m_face.m_indices), &state);
	p_case->m_face.m_face.m_unk0x04 = (MechU32) offsetof(FaceBuffer, m_indices);
	p_case->m_face.m_indices[0] %= SHADE_VERTICES;
	vertex = &p_case->m_vertices[p_case->m_face.m_indices[0]];
	vertex->m_unk0x0c = p_args[0];
	vertex->m_unk0x10 = p_args[1];
	vertex->m_unk0x14 = p_args[2];
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

// The magnitude FUN_10048faf takes of a light coordinate's difference from the vertex's: negated
// when the light's is the smaller one.
static MechU32 ShadeMagnitude(MechS32 p_light, MechS32 p_vertex)
{
	MechU32 difference = (MechU32) p_light - (MechU32) p_vertex;

	return p_light < p_vertex ? 0 - difference : difference;
}

// FUN_10048faf up to its idiv.
static MechS32 Domain10048faf(const MechS32* p_args)
{
	ShadeCase shade;
	EmberFern0x2c* vertex;
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
	position[0] = shade.m_atOrigin ? 0 : vertex->m_unk0x0c;
	position[1] = shade.m_atOrigin ? 0 : vertex->m_unk0x10;
	position[2] = shade.m_atOrigin ? 0 : vertex->m_unk0x14;
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

typedef MechS32 (*ShadeFaceFn)(DuskMoth0x24* p_face, EmberFern0x2c* p_vertices);

// Arguments: MakeShadeCase's.
static void Run10048faf(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	ShadeCase shade;
	MechS32 result;

	MakeShadeCase(p_args, &shade);
	SetView(p_module, shade.m_view);
	*(MechS32*) Data(p_module, "g_unk0x1010b530") = shade.m_atOrigin;
	*(MechS16**) Data(p_module, "g_sqrtTable") = shade.m_table;
	result = ((ShadeFaceFn) Function(p_module, "FUN_10048faf"))(&shade.m_face.m_face, shade.m_vertices);
	AsmOutputWord(p_output, (MechU32) result);
}

// --- Queueing a face (FUN_10049155) ---

#define QUEUE_VERTICES 8
#define QUEUE_POLYGONS 8
#define QUEUE_LOG 64
#define POLYGON_RECORD_SIZE (0xc + 20 * 4) // a polygon and its points, on x86

// The hooks FUN_10049155 calls through g_unk0x100a6cc8, and what they're passed. The projection
// hook stands in for FUN_10048ebe: it adds the point to the polygon (up to 20) and ands random
// outcodes, mostly 0, into g_unk0x1010b5b8, so that most polygons get queued.
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

static CopperWren0x20* QueueProjectHook(CopperWren0x20* p_vertex)
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

static MechS32 QueueDrawHook(DuskMoth0x24* p_face, EmberFern0x2c* p_vertices, MechS32 p_flags, MechS32 p_depth)
{
	LogHook(Locate(g_queueHooks.m_regions, g_queueHooks.m_regionCount, p_face));
	LogHook(Locate(g_queueHooks.m_regions, g_queueHooks.m_regionCount, p_vertices));
	LogHook((MechU32) p_flags);
	LogHook((MechU32) p_depth);
	return PortableS32(AsmNext(&g_queueHooks.m_state));
}

// FUN_10049155's counters, and the polygon list's count and capacity.
static const char* const g_queueCounterNames[] = {
	"g_unk0x1010b5bc",
	"g_unk0x1010b538",
	"g_unk0x1010b5b4",
	"g_unk0x1010b5a8",
	"g_unk0x100a54b0",
	"g_unk0x100a54b4",
	"g_unk0x100c1a68",
};

#define QUEUE_COUNTERS ((MechS32) (sizeof(g_queueCounterNames) / sizeof(g_queueCounterNames[0])))

// The polygon FUN_10049155 takes from the bottom of the record buffer, and the points it copies
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
	const IvoryDelta0xc* poly = (const IvoryDelta0xc*) p_record;
	const MechU8* cursor = p_record;

	if (p_end > p_record) {
		AsmOutputWord(p_output, (MechU16) poly->m_count | ((MechU32) poly->m_unk0x02 << 16));
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

typedef void (*QueueFaceFn)(DuskMoth0x24* p_face, EmberFern0x2c* p_vertices);

// Arguments: the face's vertex count (1 to QUEUE_VERTICES), the depth rule (g_unk0x1010b5c8),
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
	EmberFern0x2c vertices[QUEUE_VERTICES];
	CopperWren0x20 copies[QUEUE_VERTICES];
	CopperWren0x20 others[20];
	AmberDune0x8 polygons[QUEUE_POLYGONS];
	RecordStacks stacks;
	SlateHeron0x68* hooks = (SlateHeron0x68*) Data(p_module, "g_unk0x100a6cc8");
	MechU8** cursor = (MechU8**) Data(p_module, "g_unk0x1010b534");
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
		EmberFern0x2c* vertex = &vertices[i];
		MechU32 kind = AsmNext(&state) % 4;

		MakeVertex(vertex, &state);
		if (wide) {
			vertex->m_unk0x0c = ExtremeWord(&state);
			vertex->m_unk0x10 = ExtremeWord(&state);
			vertex->m_unk0x14 = ExtremeWord(&state);
		}
		else {
			vertex->m_unk0x0c = PortableS32((MechU32) view[c_viewEyeX] + (AsmNext(&state) & 0x1ffffff) - 0x1000000);
			vertex->m_unk0x10 = PortableS32((MechU32) view[c_viewEyeY] + (AsmNext(&state) & 0x1ffffff) - 0x1000000);
			vertex->m_unk0x14 = PortableS32((MechU32) view[c_viewEyeZ] + (AsmNext(&state) & 0x1ffffff) - 0x1000000);
		}

		if (kind < 2) {
			vertex->m_unk0x28 &= ~4;
		}
		else {
			MechS32 depth = (MechS32) (AsmNext(&state) % 0x4400000) - 0x400000;

			vertex->m_unk0x20 = (MechU32) depth;
			vertex->m_unk0x28 =
				(MechU8) ((vertex->m_unk0x28 & ~3) | 4 | (depth < view[c_viewNear]) | ((depth > view[c_viewFar]) << 1));
			if (kind == 3) {
				copies[i].m_z = depth;
				vertex->m_unk0x24 = &copies[i];
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
	face.m_face.m_unk0x02 = (MechU16) count;
	face.m_face.m_unk0x04 = (MechU32) offsetof(FaceBuffer, m_indices);
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

	*(AmberDune0x8**) Data(p_module, "g_unk0x1010b5c4") = polygons;
	*(MechU32*) Data(p_module, "g_unk0x1010b5c8") = (MechU32) p_args[1];
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
	hooks->m_unk0x5c = QueueProjectHook;
	hooks->m_unk0x60 = QueueDrawHook;

	((QueueFaceFn) Function(p_module, "FUN_10049155"))(&face.m_face, vertices);

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
	{"FUN_100074e0", 2, NULL, RunNameHash, NULL},
	{"IntegrateMidpoint", 4, NULL, RunIntegrateMidpoint, NULL},
	{"FUN_10004ec0", 4, NULL, NULL, NULL},
	{"FUN_10013340", 3, Domain10013340, NULL, NULL},
	{"FUN_10019ad0", 2, NULL, NULL, NULL},
	{"FUN_10034990", 4, NULL, NULL, NULL},
	{"FUN_100349c0", 5, Domain100349c0, NULL, NULL},
	{"FUN_100349f0", 5, Domain100349f0, NULL, NULL},
	{"FUN_10042740", 4, Domain10042740, NULL, NULL},
	{"FUN_1004c800", 3, Domain1004c800, NULL, NULL},
	{"FUN_1004c820", 5, NULL, Run1004c820, NULL},
	{"FUN_1004c860", 4, Domain1004c860, NULL, NULL},
	{"MemCopy", 4, NULL, RunMemCopy, NULL},
	{"MemSet", 4, NULL, RunMemSet, NULL},
	{"FUN_100696c0", 2, NULL, RunSine, HashSineInputs},
	{"FUN_1006975b", 2, ArcsineDomain, RunArcsine, HashArcsineInputs},
	{"FUN_100698de", 3, BearingDomain, RunBearing, HashBearingInputs},
	{"FUN_10071930", 9, Domain10071930, Run10071930, NULL},
	{"FUN_100719ca", 9, Domain100719ca, Run100719ca, NULL},
	{"FUN_10071a4c", 9, Domain10071a4c, Run10071a4c, NULL},
	{"FUN_1007d248", 3, NULL, Run1007d248, NULL},
	{"FUN_1007d296", 3, NULL, Run1007d296, NULL},
	// ticks.asm
	{"GameTickTimerCallback", 3, NULL, RunGameTickTimerCallback, NULL},
	{"AllocTicks", 3, NULL, RunAllocTicks, NULL},
	{"GetTicks", 3, NULL, RunGetTicks, NULL},
	{"ResetTicks", 3, NULL, RunResetTicks, NULL},
	{"SetTicks", 3, NULL, RunSetTicks, NULL},
	{"FreeTicks", 3, NULL, RunFreeTicks, NULL},
	{"PauseTimer", 3, NULL, RunPauseTimer, NULL},
	// sndunpack.asm
	{"FUN_1001a63c", 4, NULL, RunSoundUnpack, NULL},
	// transform.c
	{"FUN_1000d650", 3, NULL, Run1000d650, NULL},
	{"FUN_1000d708", 3, NULL, Run1000d708, NULL},
	{"FUN_1000d7c0", 2, NULL, Run1000d7c0, NULL},
	{"FUN_1000d9a8", 2, NULL, NULL, NULL},
	{"FUN_1000d9ce", 6, NULL, NULL, NULL},
	{"FUN_1000da0c", 2, NULL, Run1000da0c, NULL},
	{"FUN_1000de3b", 8, NULL, Run1000de3b, HashMatrixInputs},
	// unk10039a30.c
	{"FUN_10039a30", 3, NULL, Run10039a30, NULL},
	{"FUN_10039b94", 4, NULL, Run10039b94, NULL},
	{"FUN_10039c96", 6, Domain10039c96, NULL, NULL},
	{"FUN_10039ccc", 5, NULL, Run10039ccc, NULL},
	{"FUN_10039dda", 9, Domain10039dda, Run10039dda, NULL},
	{"FUN_1003a05d", 3, Domain1003a05d, NULL, NULL},
	{"FUN_1003a096", 9, Domain1003a096, Run1003a096, NULL},
	// unk10046750.c
	{"FUN_10048c50", 4, NULL, Run10048c50, NULL},
	{"FUN_10048d46", 9, Domain10048d46, Run10048d46, NULL},
	{"FUN_10048ebe", 6, Domain10048ebe, Run10048ebe, NULL},
	{"FUN_10048faf", 9, Domain10048faf, Run10048faf, NULL},
	{"FUN_10049155", 3, NULL, Run10049155, NULL},
};

const MechS32 g_asmRoutineCount = sizeof(g_asmRoutines) / sizeof(g_asmRoutines[0]);
