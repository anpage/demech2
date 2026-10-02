// The routines under test: how each one's cases are set up, called and compared, and for which
// arguments the original computes a result.

#include "asmequiv.h"
#include "eyepoint.h"
#include "portable.h"
#include "types.h"

#include <math.h>
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

// Whether idiv computes p_dividend / p_divisor without faulting.
static MechS32 Divides(MechS64 p_dividend, MechS32 p_divisor)
{
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

typedef void* (*PopRecordFn)(void);

// Arguments: the top's offset in the arena (0x20 to 0x200), the bottom's (0 to 0x1f4), both
// dword-aligned as the game's are, and the flag the routines clear.
static void RunRecordStack(const AsmModule* p_module, const char* p_name, const MechS32* p_args, AsmOutput* p_output)
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
	result = ((PopRecordFn) Function(p_module, p_name))();

	AsmOutputWord(p_output, (MechU32) (*top - arena));
	AsmOutputWord(p_output, (MechU32) (*bottom - arena));
	AsmOutputWord(p_output, (MechU32) *flag);
	OutputArena(p_output, arena, result);
}

static void Run1007d248(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunRecordStack(p_module, "FUN_1007d248", p_args, p_output);
}

static void Run1007d296(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	RunRecordStack(p_module, "FUN_1007d296", p_args, p_output);
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
	{"FixedSqrtGuess", 1, FixedSqrtGuessDomain, NULL, NULL},
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
};

const MechS32 g_asmRoutineCount = sizeof(g_asmRoutines) / sizeof(g_asmRoutines[0]);
