#include "asmequiv.h"
#include "portable.h"
#include "types.h"

#include <string.h>

#define INT_MIN32 (-0x7fffffff - 1)

const char* const g_asmSetNames[c_setCount] = {"edge", "random"};

const AsmRoutine* AsmFindRoutine(const char* p_name)
{
	MechS32 i;

	for (i = 0; i < g_asmRoutineCount; i++) {
		if (!strcmp(g_asmRoutines[i].m_name, p_name)) {
			return &g_asmRoutines[i];
		}
	}

	return NULL;
}

MechS32 AsmFindSet(const char* p_name)
{
	MechS32 i;

	for (i = 0; i < c_setCount; i++) {
		if (!strcmp(g_asmSetNames[i], p_name)) {
			return i;
		}
	}

	return -1;
}

// --- Values ---

// Boundaries for every argument: 0, +-1, +-2^k and their neighbours, the extremes. Built on
// first use.
#define WIDE_COUNT (3 + 30 * 6 + 4)
static MechS32 g_wideValues[WIDE_COUNT];
static MechS32 g_wideReady = 0;

// A smaller set for arity 3, where the exhaustive product of the wide set is too large: the
// 16.16 and 2.30 units, the extremes, and a few unremarkable values.
static const MechS32 g_smallValues[] = {
	0,           1,          -1,         2,           -2,          3,          0x7fff,    0x8000,        -0x8000,
	0xffff,      0x10000,    -0x10000,   0x10001,     -0x10001,    0xb504,     0xb505,    0x1fffffff,    0x20000000,
	-0x20000000, 0x3fffffff, 0x40000000, -0x40000000, 0x7ffffffe,  0x7fffffff, INT_MIN32, INT_MIN32 + 1, 100,
	-100,        0x12345,    -0x12345,   0x55555555,  -0x55555555,
};

#define SMALL_COUNT ((MechS32) (sizeof(g_smallValues) / sizeof(g_smallValues[0])))

static void InitWideValues(void)
{
	MechS32 n = 0;
	MechS32 k;

	if (g_wideReady) {
		return;
	}

	g_wideValues[n++] = 0;
	g_wideValues[n++] = 1;
	g_wideValues[n++] = -1;
	for (k = 1; k <= 30; k++) {
		MechS32 power = (MechS32) ((MechU32) 1 << k);

		g_wideValues[n++] = power;
		g_wideValues[n++] = power - 1;
		g_wideValues[n++] = power + 1;
		g_wideValues[n++] = -power;
		g_wideValues[n++] = -power + 1;
		g_wideValues[n++] = -power - 1;
	}
	g_wideValues[n++] = 0x7fffffff;
	g_wideValues[n++] = 0x7ffffffe;
	g_wideValues[n++] = INT_MIN32;
	g_wideValues[n++] = INT_MIN32 + 1;
	g_wideReady = 1;
}

// A 32-bit integer hash (lowbias32, by Chris Wellons): the case generator is counter-based.
MechU32 AsmMix(MechU32 p_x)
{
	p_x ^= p_x >> 16;
	p_x *= 0x7feb352d;
	p_x ^= p_x >> 15;
	p_x *= 0x846ca68b;
	p_x ^= p_x >> 16;
	return p_x;
}

MechU32 AsmNext(MechU32* p_state)
{
	*p_state = AsmMix(*p_state + 0x9e3779b9);
	return *p_state;
}

// A state for a runner's own random numbers, from a case's arguments.
MechU32 AsmSeed(const MechS32* p_args, MechS32 p_count)
{
	MechU32 state = 0x2545f491;
	MechS32 i;

	for (i = 0; i < p_count; i++) {
		state = AsmMix(state ^ (MechU32) p_args[i]);
	}

	return state;
}

static MechU32 NameSeed(const char* p_name)
{
	MechU32 seed = 0x811c9dc5;

	while (*p_name) {
		seed = (seed ^ (MechU8) *p_name++) * 0x01000193;
	}

	return seed;
}

// One argument: a mix of distributions, so that every routine sees full-range values as well
// as the small, fixed-point-sized and boundary ones its callers pass.
static MechS32 RandomValue(MechU32* p_state)
{
	MechU32 kind = AsmNext(p_state);
	MechU32 bits = AsmNext(p_state);
	MechS32 negative = (kind >> 8) & 1;
	MechS32 value;

	switch (kind & 7) {
	case 0:
	case 1:
		return PortableS32(bits);
	case 2:
		return (MechS32) (bits % 513) - 256;
	case 3: {
		// A magnitude of 1 to 31 bits
		MechU32 width = 1 + (kind >> 3) % 31;

		value = (MechS32) (bits & (((MechU32) 1 << width) - 1));
		return negative ? -value : value;
	}
	case 4:
		// A power of two and its neighbours
		value = (MechS32) ((MechU32) 1 << ((kind >> 3) % 31)) + (MechS32) (bits % 5) - 2;
		return negative ? -value : value;
	case 5:
		return g_wideValues[bits % WIDE_COUNT];
	case 6:
		// 16.16 within +-256.0
		return (MechS32) (bits & 0x1ffffff) - 0x1000000;
	default:
		// 2.30 within +-1.0
		return (MechS32) (bits & 0x7fffffff) - 0x40000000;
	}
}

MechU32 AsmEdgeCaseCount(const AsmRoutine* p_routine)
{
	switch (p_routine->m_arity) {
	case 1:
		return WIDE_COUNT;
	case 2:
		return WIDE_COUNT * WIDE_COUNT;
	case 3:
		return SMALL_COUNT * SMALL_COUNT * SMALL_COUNT;
	default:
		// Random combinations of the wide set
		return ASM_BLOCK_SIZE * 16;
	}
}

void AsmMakeCase(const AsmRoutine* p_routine, MechS32 p_set, MechU32 p_index, MechS32* p_args)
{
	MechU32 state = AsmMix(NameSeed(p_routine->m_name) ^ AsmMix(p_index + (MechU32) p_set * 0x9e3779b9));
	MechS32 i;

	InitWideValues();
	if (p_set == c_setRandom) {
		for (i = 0; i < p_routine->m_arity; i++) {
			p_args[i] = RandomValue(&state);
		}
	}
	else if (p_routine->m_arity <= 2) {
		// Every combination, the last argument varying fastest
		for (i = p_routine->m_arity - 1; i >= 0; i--) {
			p_args[i] = g_wideValues[p_index % WIDE_COUNT];
			p_index /= WIDE_COUNT;
		}
	}
	else if (p_routine->m_arity == 3) {
		for (i = 2; i >= 0; i--) {
			p_args[i] = g_smallValues[p_index % SMALL_COUNT];
			p_index /= SMALL_COUNT;
		}
	}
	else {
		for (i = 0; i < p_routine->m_arity; i++) {
			p_args[i] = g_wideValues[AsmNext(&state) % WIDE_COUNT];
		}
	}
}

typedef MechS32 (*AsmFn1)(MechS32);
typedef MechS32 (*AsmFn2)(MechS32, MechS32);
typedef MechS32 (*AsmFn3)(MechS32, MechS32, MechS32);
typedef MechS32 (*AsmFn4)(MechS32, MechS32, MechS32, MechS32);
typedef MechS32 (*AsmFn5)(MechS32, MechS32, MechS32, MechS32, MechS32);
typedef MechS32 (*AsmFn6)(MechS32, MechS32, MechS32, MechS32, MechS32, MechS32);

MechS32 AsmCall(AsmFn p_fn, MechS32 p_arity, const MechS32* p_args)
{
	switch (p_arity) {
	case 1:
		return ((AsmFn1) p_fn)(p_args[0]);
	case 2:
		return ((AsmFn2) p_fn)(p_args[0], p_args[1]);
	case 3:
		return ((AsmFn3) p_fn)(p_args[0], p_args[1], p_args[2]);
	case 4:
		return ((AsmFn4) p_fn)(p_args[0], p_args[1], p_args[2], p_args[3]);
	case 5:
		return ((AsmFn5) p_fn)(p_args[0], p_args[1], p_args[2], p_args[3], p_args[4]);
	default:
		return ((AsmFn6) p_fn)(p_args[0], p_args[1], p_args[2], p_args[3], p_args[4], p_args[5]);
	}
}

MechS32 AsmDomain(const AsmRoutine* p_routine, const MechS32* p_args)
{
	return p_routine->m_domain ? p_routine->m_domain(p_args) : c_domainIn;
}

// The case's inputs: its arguments, and what it derives from them other than with integer
// arithmetic.
void AsmHashCase(const AsmRoutine* p_routine, const MechS32* p_args, AsmHash* p_hash)
{
	MechS32 i;

	for (i = 0; i < p_routine->m_arity; i++) {
		AsmHashWord(p_hash, (MechU32) p_args[i]);
	}

	if (p_routine->m_hashInputs) {
		p_routine->m_hashInputs(p_args, p_hash);
	}
}

void AsmRun(const AsmRoutine* p_routine, const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output)
{
	p_output->m_count = 0;
	if (p_routine->m_run) {
		p_routine->m_run(p_module, p_args, p_output);
	}
	else {
		AsmFn fn = p_module->m_function(p_module->m_handle, p_routine->m_name);

		AsmOutputWord(p_output, (MechU32) AsmCall(fn, p_routine->m_arity, p_args));
	}
}

void AsmOutputWord(AsmOutput* p_output, MechU32 p_word)
{
	if (p_output->m_count < ASM_MAX_OUTPUTS) {
		p_output->m_words[p_output->m_count] = p_word;
	}

	p_output->m_count++;
}

void AsmOutputBytes(AsmOutput* p_output, const MechU8* p_bytes, MechU32 p_size)
{
	MechU32 i;

	for (i = 0; i + 4 <= p_size; i += 4) {
		AsmOutputWord(
			p_output,
			p_bytes[i] | ((MechU32) p_bytes[i + 1] << 8) | ((MechU32) p_bytes[i + 2] << 16) |
				((MechU32) p_bytes[i + 3] << 24)
		);
	}
}

// --- Hash ---

void AsmHashInit(AsmHash* p_hash)
{
	p_hash->m_value = ((MechU64) 0xcbf29ce4 << 32) | 0x84222325;
}

void AsmHashWord(AsmHash* p_hash, MechU32 p_word)
{
	MechU64 prime = ((MechU64) 0x100 << 32) | 0x1b3;
	MechS32 i;

	for (i = 0; i < 4; i++) {
		p_hash->m_value ^= (p_word >> (i * 8)) & 0xff;
		p_hash->m_value *= prime;
	}
}

void AsmHashFormat(const AsmHash* p_hash, char* p_buffer)
{
	static const char digits[] = "0123456789abcdef";
	MechS32 i;

	for (i = 0; i < 16; i++) {
		p_buffer[i] = digits[(MechU32) (p_hash->m_value >> (60 - i * 4)) & 0xf];
	}
	p_buffer[16] = '\0';
}
