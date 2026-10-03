// asmequiv: runs every case through the reference (the matched hand-written assembly, asmref.dll)
// and a candidate (the portable C, asmport.dll), and compares the results. Each DLL is loaded
// separately, so each has its own copy of every global. Optionally writes the golden vectors that
// asmgolden checks a candidate against on other platforms.
//
//   asmequiv REF.dll CANDIDATE.dll [-golden FILE] [-coverage FILE] [-exhaustive] [-blocks N]
//            [-routine NAME] [-case SET INDEX]
//
// -blocks sets the number of random blocks (ASM_RANDOM_BLOCKS by default), -case runs and prints
// one case. Outside a routine's domain, the reference must fault (and is the only one called);
// inside it, it must not. Where the original's result is undefined, neither is called.
//
// -exhaustive runs the routines with one argument word on every input instead of the sets.
//
// -coverage reads the reference's basic blocks (tools/asmblocks.py) and fails unless the cases
// reach every block of the routines that ran, but for the few that no input can (c_deadBlocks).

#include "asmequiv.h"

#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define MAX_REPORTS 10
#define MAX_BLOCKS 8192
#define MAX_ROUTINES 128

// --- Modules ---

static AsmFn Export(HMODULE p_module, const char* p_name)
{
	AsmFn fn = (AsmFn) GetProcAddress(p_module, p_name);

	if (!fn) {
		printf("no export %s\n", p_name);
		exit(2);
	}

	return fn;
}

#ifdef _M_IX86
// The assembly doesn't always keep the registers C callers expect it to: ResetTicks returns with
// ebx and ecx swapped (it pops them in the wrong order), and VFX_pane_scroll's fill returns with es
// loaded from the wrong stack slot (0, after pushing a word too many), which the game's /Od
// callers never notice. Every call into a module goes through this thunk, which calls the
// routine with the caller's arguments and restores ebx, esi, edi, ebp and es. One call at a
// time: the state is static.
static AsmFn g_thunkTarget;
static DWORD g_thunkReturn;
static DWORD g_thunkEbx;
static DWORD g_thunkEsi;
static DWORD g_thunkEdi;
static DWORD g_thunkEbp;
static WORD g_thunkEs;

static __declspec(naked) void PreservingThunk(void)
{
	__asm {
		pop g_thunkReturn
		mov g_thunkEbx, ebx
		mov g_thunkEsi, esi
		mov g_thunkEdi, edi
		mov g_thunkEbp, ebp
		mov g_thunkEs, es
		call g_thunkTarget
		mov ebx, g_thunkEbx
		mov esi, g_thunkEsi
		mov edi, g_thunkEdi
		mov ebp, g_thunkEbp
		mov es, g_thunkEs
		push g_thunkReturn
		ret
	}
}
#endif

// A routine to call: runners call it right away, so the thunk's target is the last one looked up.
static AsmFn DllFunction(void* p_handle, const char* p_name)
{
	AsmFn fn = Export((HMODULE) p_handle, p_name);

#ifdef _M_IX86
	g_thunkTarget = fn;
	return PreservingThunk;
#else
	// The driver is x86 only, but clang-tidy parses it for the host
	return fn;
#endif
}

static void* DllData(void* p_handle, const char* p_name)
{
	void* data = (void*) GetProcAddress((HMODULE) p_handle, p_name);

	if (!data) {
		printf("no export %s\n", p_name);
		exit(2);
	}

	return data;
}

// A routine's code: the export, or the target of the jmp an incremental link exports instead.
static MechU8* RoutineCode(HMODULE p_module, const char* p_name)
{
	MechU8* code = (MechU8*) Export(p_module, p_name);

	if (code[0] == 0xe9) {
		code += 5 + (code[1] | (code[2] << 8) | (code[3] << 16) | ((MechU32) code[4] << 24));
	}

	return code;
}

// --- Coverage ---

// The blocks no input can reach, which the coverage check leaves out: offsets from the routine's
// start, the same in the original.
typedef struct DeadBlock {
	const char* m_routine;
	MechS32 m_offset;
} DeadBlock;

static const DeadBlock c_deadBlocks[] = {
	// VFX_line_draw's clipping, past its eight tests of the ends' outcodes: it only gets there with a
	// bit of one set
	{"VFX_line_draw", 0x1da},
	{"VFX_line_draw", 0x223},
	// VFX_shape_scan's helper FlushPacket, where a run ends past the bounds' right: it measures the
	// run from the clipped view's left, which lies no further right than the bounds' left, so the
	// run ends at most at their right
	{"VFX_shape_scan", 0x567},
	{"VFX_shape_scan", 0x60e},
	// VFX_GIF_draw's two pixels a byte, for a pixel size of 1 (cmp edx, 1), which it never sets: edx is
	// always 8 there
	{"VFX_GIF_draw", 0x1ca},
	{"VFX_GIF_draw", 0x1e9},
};

static MechS32 IsDeadBlock(const char* p_routine, MechS32 p_offset)
{
	MechS32 i;

	for (i = 0; i < (MechS32) (sizeof(c_deadBlocks) / sizeof(c_deadBlocks[0])); i++) {
		if (!strcmp(c_deadBlocks[i].m_routine, p_routine) && c_deadBlocks[i].m_offset == p_offset) {
			return 1;
		}
	}

	return 0;
}

// The reference's basic blocks (written by tools/asmblocks.py), each with an int3 in its first
// byte until a case reaches it.
typedef struct Block {
	MechS32 m_routine; // in g_asmRoutines
	MechS32 m_offset;  // from the routine's start (a helper before it is negative)
	MechU8* m_address;
	MechU8 m_byte;
	MechS32 m_reached;
} Block;

static Block g_blocks[MAX_BLOCKS];
static MechS32 g_blockCount = 0;

static MechS32 LoadBlocks(const char* p_path, HMODULE p_module)
{
	FILE* file = fopen(p_path, "r");
	char line[4096];
	MechS32 lineNumber = 0;

	if (!file) {
		printf("%s: can't read\n", p_path);
		return 0;
	}

	while (fgets(line, sizeof(line), file)) {
		const AsmRoutine* routine;
		MechU8* code;
		char* token;

		lineNumber++;
		token = strtok(line, " \r\n");
		if (!token || token[0] == '#') {
			continue;
		}

		routine = AsmFindRoutine(token);
		if (!routine) {
			printf("%s:%ld: unknown routine %s\n", p_path, (long) lineNumber, token);
			fclose(file);
			return 0;
		}

		code = RoutineCode(p_module, routine->m_name);
		while ((token = strtok(NULL, " \r\n")) != NULL) {
			Block* block = &g_blocks[g_blockCount];
			char* end;
			long offset = strtol(token, &end, 16);
			unsigned long byte;
			DWORD protection;

			if (g_blockCount == MAX_BLOCKS || *end != ':' || sscanf(end + 1, "%lx", &byte) != 1) {
				printf("%s:%ld: too many blocks, or a malformed one\n", p_path, (long) lineNumber);
				fclose(file);
				return 0;
			}

			if (IsDeadBlock(routine->m_name, (MechS32) offset)) {
				continue;
			}

			block->m_routine = (MechS32) (routine - g_asmRoutines);
			block->m_offset = (MechS32) offset;
			block->m_address = code + offset;
			block->m_byte = (MechU8) byte;
			block->m_reached = 0;
			if (*block->m_address != block->m_byte) {
				printf(
					"%s: %+ld starts with 0x%02x, not 0x%02lx: the reference isn't the one %s describes\n",
					routine->m_name,
					offset,
					*block->m_address,
					byte,
					p_path
				);
				fclose(file);
				return 0;
			}

			VirtualProtect(block->m_address, 1, PAGE_EXECUTE_READWRITE, &protection);
			*block->m_address = 0xcc;
			g_blockCount++;
		}
	}

	fclose(file);
	FlushInstructionCache(GetCurrentProcess(), NULL, 0);
	return 1;
}

// A block's int3: restores the block's first byte and resumes there.
static MechS32 ReachBlock(EXCEPTION_POINTERS* p_info)
{
	MechU8* address = (MechU8*) p_info->ExceptionRecord->ExceptionAddress;
	MechS32 i;

	for (i = 0; i < g_blockCount; i++) {
		if (g_blocks[i].m_address == address && !g_blocks[i].m_reached) {
			*address = g_blocks[i].m_byte;
			g_blocks[i].m_reached = 1;
			FlushInstructionCache(GetCurrentProcess(), address, 1);
#ifdef _M_IX86
			p_info->ContextRecord->Eip = (DWORD) address;
#else
			// The driver is x86 only, but clang-tidy parses it for the host
			p_info->ContextRecord->Rip = (DWORD64) address;
#endif
			return 1;
		}
	}

	return 0;
}

static MechS32 Abs32(MechS32 p_value)
{
	return p_value < 0 ? -p_value : p_value;
}

// Reports the blocks of the routines that ran which no case reached.
static MechS32 ReportCoverage(const MechS32* p_ran)
{
	MechS32 failed = 0;
	MechS32 r;
	MechS32 i;

	for (r = 0; r < g_asmRoutineCount; r++) {
		MechS32 blocks = 0;
		MechS32 reached = 0;

		if (!p_ran[r]) {
			continue;
		}

		for (i = 0; i < g_blockCount; i++) {
			if (g_blocks[i].m_routine == r) {
				blocks++;
				reached += g_blocks[i].m_reached;
			}
		}

		if (!blocks) {
			printf("%-24s no blocks: not in the coverage file\n", g_asmRoutines[r].m_name);
			failed = 1;
			continue;
		}

		printf("%-24s %3ld/%3ld blocks reached", g_asmRoutines[r].m_name, (long) reached, (long) blocks);
		if (reached < blocks) {
			printf(", not:");
			for (i = 0; i < g_blockCount; i++) {
				if (g_blocks[i].m_routine == r && !g_blocks[i].m_reached) {
					printf(
						g_blocks[i].m_offset < 0 ? " -0x%lx" : " +0x%lx",
						(unsigned long) Abs32(g_blocks[i].m_offset)
					);
				}
			}

			failed = 1;
		}

		printf("\n");
	}

	return !failed;
}

// --- Cases ---

static int Filter(EXCEPTION_POINTERS* p_info, DWORD* p_code)
{
	if (p_info->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT && ReachBlock(p_info)) {
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	*p_code = p_info->ExceptionRecord->ExceptionCode;
	return EXCEPTION_EXECUTE_HANDLER;
}

static DWORD RunGuarded(
	const AsmRoutine* p_routine,
	const AsmModule* p_module,
	const MechS32* p_args,
	AsmOutput* p_output
)
{
	DWORD code = 0;

	__try {
		AsmRun(p_routine, p_module, p_args, p_output);
	}
	__except (Filter(GetExceptionInformation(), &code)) {
		p_output->m_count = 0;
	}

	return code;
}

static MechS32 IsDivideFault(DWORD p_code)
{
	return p_code == EXCEPTION_INT_DIVIDE_BY_ZERO || p_code == EXCEPTION_INT_OVERFLOW;
}

static void PrintCase(const AsmRoutine* p_routine, MechS32 p_set, MechU32 p_index, const MechS32* p_args)
{
	MechS32 i;

	printf("  %s %s %lu (", p_routine->m_name, g_asmSetNames[p_set], (unsigned long) p_index);
	for (i = 0; i < p_routine->m_arity; i++) {
		printf(i ? ", 0x%08lx" : "0x%08lx", (unsigned long) (MechU32) p_args[i]);
	}
	printf("): ");
}

// The first differing word of two outputs, or -1.
static MechS32 FirstDifference(const AsmOutput* p_a, const AsmOutput* p_b)
{
	MechS32 i;

	for (i = 0; i < p_a->m_count && i < p_b->m_count; i++) {
		if (p_a->m_words[i] != p_b->m_words[i]) {
			return i;
		}
	}

	return p_a->m_count == p_b->m_count ? -1 : i;
}

typedef struct Totals {
	MechU32 m_cases;
	MechU32 m_outOfDomain;
	MechU32 m_undefined;
	MechU32 m_failures;
} Totals;

// Runs one case; returns whether it passed.
static MechS32 RunCase(
	const AsmRoutine* p_routine,
	const AsmModule* p_ref,
	const AsmModule* p_cand,
	MechS32 p_set,
	MechU32 p_index,
	MechS32 p_verbose,
	Totals* p_totals,
	AsmHash* p_inputs,
	AsmHash* p_outputs
)
{
	MechS32 args[ASM_MAX_ARGS];
	MechS32 domain;
	AsmOutput expected;
	AsmOutput actual;
	DWORD refCode;
	DWORD candCode;
	MechS32 difference;
	MechS32 i;
	MechS32 report;

	AsmMakeCase(p_routine, p_set, p_index, args);
	AsmHashCase(p_routine, args, p_inputs);
	domain = AsmDomain(p_routine, args);
	p_totals->m_cases++;
	report = p_totals->m_failures < MAX_REPORTS || p_verbose;

	if (domain == c_domainUndefined) {
		p_totals->m_undefined++;
		AsmHashWord(p_outputs, 2);
		if (p_verbose) {
			PrintCase(p_routine, p_set, p_index, args);
			printf("undefined\n");
		}
		return 1;
	}

	refCode = RunGuarded(p_routine, p_ref, args, &expected);
	if (expected.m_count > ASM_MAX_OUTPUTS) {
		printf("%s: %ld outputs, more than ASM_MAX_OUTPUTS\n", p_routine->m_name, (long) expected.m_count);
		exit(2);
	}

	if (domain == c_domainFault) {
		p_totals->m_outOfDomain++;
		AsmHashWord(p_outputs, 1);
		if (!IsDivideFault(refCode)) {
			p_totals->m_failures++;
			if (report) {
				PrintCase(p_routine, p_set, p_index, args);
				printf(
					"out of domain, but the reference returns 0x%08lx (exception 0x%08lx)\n",
					(unsigned long) (expected.m_count ? expected.m_words[0] : 0),
					(unsigned long) refCode
				);
			}
			return 0;
		}
		if (p_verbose) {
			PrintCase(p_routine, p_set, p_index, args);
			printf("out of domain (reference exception 0x%08lx)\n", (unsigned long) refCode);
		}
		return 1;
	}

	if (domain == c_domainPointers) {
		AsmHashWord(p_outputs, 3);
	}
	else {
		AsmHashWord(p_outputs, 0);
		for (i = 0; i < expected.m_count; i++) {
			AsmHashWord(p_outputs, expected.m_words[i]);
		}
	}

	if (refCode) {
		p_totals->m_failures++;
		if (report) {
			PrintCase(p_routine, p_set, p_index, args);
			printf("in domain, but the reference faults (exception 0x%08lx)\n", (unsigned long) refCode);
		}
		return 0;
	}

	candCode = RunGuarded(p_routine, p_cand, args, &actual);
	difference = candCode ? 0 : FirstDifference(&expected, &actual);
	if (difference >= 0) {
		p_totals->m_failures++;
		if (report) {
			PrintCase(p_routine, p_set, p_index, args);
			if (candCode) {
				printf("the candidate faults (exception 0x%08lx)\n", (unsigned long) candCode);
			}
			else if (difference >= expected.m_count || difference >= actual.m_count) {
				printf("%ld outputs, the candidate %ld\n", (long) expected.m_count, (long) actual.m_count);
			}
			else {
				printf(
					"output %ld: reference 0x%08lx, candidate 0x%08lx\n",
					(long) difference,
					(unsigned long) expected.m_words[difference],
					(unsigned long) actual.m_words[difference]
				);
			}
		}
		return 0;
	}

	if (p_verbose) {
		PrintCase(p_routine, p_set, p_index, args);
		for (i = 0; i < expected.m_count; i++) {
			printf(i ? " %08lx" : "%08lx", (unsigned long) expected.m_words[i]);
		}
		printf("\n");
	}

	return 1;
}

// Whether the reference has the routine's assembly. Newer compilers' references have neither
// the MASM objects' (their builds have no MASM: ticks.asm, sndunpack.asm, VFX3D.ASM, VFXA.ASM),
// nor those whose __asm
// blocks jump to C labels (FUN_10071930, and four each in unk10039a30.c and unk10046750.c): they
// compile the portable C instead. The VC++ 4.1 reference has every routine's.
static MechS32 HasReference(const AsmRoutine* p_routine)
{
#if defined(_MSC_VER) && _MSC_VER >= 1100
	static const char* const c_portableOnly[] = {
		"FUN_10071930",
		"FUN_10039a30",
		"FUN_10039ccc",
		"FUN_10039dda",
		"FUN_1003a096",
		"FUN_10048c50",
		"FUN_10048ebe",
		"FUN_10048faf",
		"FUN_10049155",
		"GameTickTimerCallback",
		"AllocTicks",
		"GetTicks",
		"ResetTicks",
		"SetTicks",
		"FreeTicks",
		"PauseTimer",
		"FUN_1001a63c",
		"VFX_flat_polygon",
		"VFX_Gouraud_polygon",
		"VFX_dithered_Gouraud_polygon",
		"VFX_translate_polygon",
		"VFX_illuminate_polygon",
		"VFX_map_lookaside",
		"VFX_map_polygon",
		"VFX_driver_name",
		"VFX_register_driver",
		"VFX_pixel_write",
		"VFX_pixel_read",
		"VFX_line_draw",
		"VFX_rectangle_hash",
		"VFX_shape_draw",
		"DrawShapeUnclipped",
		"VFX_shape_lookaside",
		"VFX_shape_translate_draw",
		"XlatShapeUnclipped",
		"VFX_shape_transform",
		"VFX_shape_visible_rectangle",
		"VFX_shape_scan",
		"VFX_shape_remap_colors",
		"VFX_pane_wipe",
		"VFX_pane_copy",
		"VFX_pane_scroll",
		"VFX_ellipse_draw",
		"VFX_ellipse_fill",
		"VFX_Cos_Sin",
		"VFX_fixed_mul",
		"VFX_point_transform",
		"VFX_font_height",
		"VFX_character_width",
		"VFX_character_draw",
		"VFX_string_draw",
		"VFX_line_to_pane",
		"find_ILBM_property",
		"VFX_ILBM_draw",
		"VFX_ILBM_palette",
		"VFX_ILBM_resolution",
		"VFX_PCX_draw",
		"VFX_PCX_palette",
		"VFX_PCX_resolution",
		"VFX_GIF_draw",
		"VFX_GIF_palette",
		"VFX_GIF_resolution",
		"VFX_shape_bounds",
		"VFX_shape_origin",
		"VFX_shape_resolution",
		"VFX_shape_minxy",
		"VFX_shape_palette",
		"VFX_shape_colors",
		"VFX_shape_set_colors",
		"VFX_shape_count",
		"VFX_shape_list",
		"VFX_shape_palette_list",
		"VFX_pixel_fade",
		"VFX_window_fade",
		"VFX_color_scan",
	};
	MechS32 i;

	for (i = 0; i < (MechS32) (sizeof(c_portableOnly) / sizeof(c_portableOnly[0])); i++) {
		if (!strcmp(p_routine->m_name, c_portableOnly[i])) {
			return 0;
		}
	}

	return 1;
#else
	(void) p_routine;
	return 1;
#endif
}

// -exhaustive: every input of each routine with one argument word (FixedSqrtGuess), called
// directly. Where the original faults, neither is called (the other sets check the reference's
// faults); where its result is undefined, neither is either.
typedef MechU32 (*Exhaustive1Fn)(MechU32 p_value);

static MechS32 RunExhaustive(const AsmRoutine* p_routine, HMODULE p_ref, HMODULE p_cand)
{
	Exhaustive1Fn ref = (Exhaustive1Fn) Export(p_ref, p_routine->m_name);
	Exhaustive1Fn cand = (Exhaustive1Fn) Export(p_cand, p_routine->m_name);
	MechU32 skipped = 0;
	MechU32 failures = 0;
	MechU32 value = 0;

	do {
		MechS32 arg = PortableS32(value);
		MechU32 expected;
		MechU32 actual;

		if (AsmDomain(p_routine, &arg) != c_domainIn) {
			skipped++;
			continue;
		}

		expected = ref(value);
		actual = cand(value);
		if (actual != expected) {
			if (failures < MAX_REPORTS) {
				printf(
					"  %s (0x%08lx): reference 0x%08lx, candidate 0x%08lx\n",
					p_routine->m_name,
					(unsigned long) value,
					(unsigned long) expected,
					(unsigned long) actual
				);
			}
			failures++;
		}
	} while (++value != 0);

	printf(
		"%-24s every input, %8lu skipped (out of domain or undefined), %lu failed\n",
		p_routine->m_name,
		(unsigned long) skipped,
		(unsigned long) failures
	);
	return failures == 0;
}

static HMODULE Load(const char* p_dll)
{
	HMODULE module = LoadLibrary(p_dll);

	if (!module) {
		printf("%s: can't load (error %lu)\n", p_dll, (unsigned long) GetLastError());
		exit(2);
	}

	return module;
}

int main(int p_argc, char** p_argv)
{
	const char* refDll;
	const char* candDll;
	const char* goldenPath = NULL;
	const char* only = NULL;
	const char* coveragePath = NULL;
	MechS32 exhaustive = 0;
	MechS32 randomBlocks = ASM_RANDOM_BLOCKS;
	MechS32 caseSet = -1;
	MechU32 caseIndex = 0;
	HMODULE ref;
	HMODULE cand;
	AsmModule refModule;
	AsmModule candModule;
	MechS32 ran[MAX_ROUTINES];
	FILE* golden = NULL;
	MechS32 failed = 0;
	MechS32 r;
	MechS32 i;

	if (p_argc < 3) {
		printf(
			"usage: asmequiv REF.dll CANDIDATE.dll [-golden FILE] [-coverage FILE] [-exhaustive] [-blocks N] [-routine "
			"NAME] [-case SET INDEX]\n"
		);
		return 2;
	}

	if (g_asmRoutineCount > MAX_ROUTINES) {
		printf("more routines than MAX_ROUTINES\n");
		return 2;
	}

	refDll = p_argv[1];
	candDll = p_argv[2];
	for (i = 3; i < p_argc; i++) {
		if (!strcmp(p_argv[i], "-golden") && i + 1 < p_argc) {
			goldenPath = p_argv[++i];
		}
		else if (!strcmp(p_argv[i], "-exhaustive")) {
			exhaustive = 1;
		}
		else if (!strcmp(p_argv[i], "-coverage") && i + 1 < p_argc) {
			coveragePath = p_argv[++i];
		}
		else if (!strcmp(p_argv[i], "-blocks") && i + 1 < p_argc) {
			randomBlocks = atoi(p_argv[++i]);
		}
		else if (!strcmp(p_argv[i], "-routine") && i + 1 < p_argc) {
			only = p_argv[++i];
		}
		else if (!strcmp(p_argv[i], "-case") && i + 2 < p_argc) {
			caseSet = AsmFindSet(p_argv[i + 1]);
			caseIndex = (MechU32) strtoul(p_argv[i + 2], NULL, 0);
			i += 2;
			if (caseSet < 0) {
				printf("unknown set %s\n", p_argv[i - 1]);
				return 2;
			}
		}
		else {
			printf("unknown argument %s\n", p_argv[i]);
			return 2;
		}
	}

	if (only && !AsmFindRoutine(only)) {
		printf("unknown routine %s\n", only);
		return 2;
	}

	if (exhaustive && (goldenPath || coveragePath || caseSet >= 0)) {
		printf("-exhaustive runs alone\n");
		return 2;
	}

	if (caseSet >= 0 && (!only || goldenPath)) {
		printf("-case needs -routine, and doesn't write golden vectors\n");
		return 2;
	}

	ref = Load(refDll);
	cand = Load(candDll);
	refModule.m_function = DllFunction;
	refModule.m_data = DllData;
	refModule.m_handle = ref;
	candModule = refModule;
	candModule.m_handle = cand;
	memset(ran, 0, sizeof(ran));

	if (coveragePath && !LoadBlocks(coveragePath, ref)) {
		return 2;
	}

	if (goldenPath) {
		golden = fopen(goldenPath, "w");
		if (!golden) {
			printf("%s: can't write\n", goldenPath);
			return 2;
		}

		fprintf(golden, "# Golden vectors for tests/asmequiv, written by asmequiv from the reference.\n");
		fprintf(golden, "# routine set block cases input-hash output-hash\n");
	}

	for (r = 0; r < g_asmRoutineCount; r++) {
		const AsmRoutine* routine = &g_asmRoutines[r];
		Totals totals;
		MechS32 set;

		if (only && strcmp(only, routine->m_name)) {
			continue;
		}

		if (!HasReference(routine)) {
			printf("%-24s skipped: the reference has no assembly for it\n", routine->m_name);
			continue;
		}

		if (exhaustive) {
			if (routine->m_arity == 1 && !RunExhaustive(routine, ref, cand)) {
				failed = 1;
			}
			continue;
		}

		memset(&totals, 0, sizeof(totals));
		ran[r] = 1;

		if (caseSet >= 0) {
			AsmHash inputs;
			AsmHash outputs;

			AsmHashInit(&inputs);
			AsmHashInit(&outputs);
			if (!RunCase(routine, &refModule, &candModule, caseSet, caseIndex, 1, &totals, &inputs, &outputs)) {
				failed = 1;
			}
			continue;
		}

		for (set = 0; set < c_setCount; set++) {
			MechU32 count = set == c_setEdge ? AsmEdgeCaseCount(routine) : (MechU32) randomBlocks * ASM_BLOCK_SIZE;
			MechU32 start;

			for (start = 0; start < count; start += ASM_BLOCK_SIZE) {
				MechU32 end = count - start < ASM_BLOCK_SIZE ? count : start + ASM_BLOCK_SIZE;
				AsmHash inputs;
				AsmHash outputs;
				MechU32 index;

				AsmHashInit(&inputs);
				AsmHashInit(&outputs);
				for (index = start; index < end; index++) {
					RunCase(routine, &refModule, &candModule, set, index, 0, &totals, &inputs, &outputs);
				}

				if (golden) {
					char inputText[17];
					char outputText[17];

					AsmHashFormat(&inputs, inputText);
					AsmHashFormat(&outputs, outputText);
					fprintf(
						golden,
						"%s %s %lu %lu %s %s\n",
						routine->m_name,
						g_asmSetNames[set],
						(unsigned long) (start / ASM_BLOCK_SIZE),
						(unsigned long) (end - start),
						inputText,
						outputText
					);
				}
			}
		}

		printf(
			"%-24s %8lu cases, %8lu out of domain, %6lu undefined, %lu failed\n",
			routine->m_name,
			(unsigned long) totals.m_cases,
			(unsigned long) totals.m_outOfDomain,
			(unsigned long) totals.m_undefined,
			(unsigned long) totals.m_failures
		);
		if (totals.m_failures) {
			failed = 1;
		}
	}

	if (golden) {
		fclose(golden);
	}

	if (coveragePath && caseSet < 0 && !ReportCoverage(ran)) {
		failed = 1;
	}

	printf(failed ? "FAILED\n" : "OK\n");
	return failed;
}
