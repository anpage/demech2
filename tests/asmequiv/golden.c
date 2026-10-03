// asmgolden: checks the portable C, linked in, against the golden vectors asmequiv wrote from the
// reference. Builds on any platform (no assembly, no Windows); meant to run under UBSan and ASan.
//
//   asmgolden GOLDEN.txt [-shard K N] [-case ROUTINE SET INDEX]
//
// -shard checks the vector lines whose index is K modulo N, so that N processes can share the
// work (ctest runs them in parallel); every shard still counts every routine's lines.
//
// A block whose input hash differs means the case generator computes differently on this
// platform (a harness bug); one whose output hash differs has a failing case, which
// `asmequiv ... -routine ROUTINE -case SET INDEX` shows against the reference, and -case here
// against this build.

#include "approxlen.h"
#include "asmequiv.h"
#include "clock.h"
#include "eyepoint.h"
#include "fixeddiv.h"
#include "fixeddiv29.h"
#include "fixeddivu.h"
#include "fixeddot27.h"
#include "fixeddot29.h"
#include "fixedmul.h"
#include "fixedmul30.h"
#include "integrate.h"
#include "loadres.h"
#include "muldiv.h"
#include "namehash.h"
#include "sndunpack.h"
#include "sqrtguess.h"
#include "ticks.h"
#include "transform.h"
#include "types.h"
#include "unk10004ec0.h"
#include "unk10013340.h"
#include "unk10019ad0.h"
#include "unk100335d0.h"
#include "unk10034990.h"
#include "unk100349c0.h"
#include "unk100349f0.h"
#include "unk10039a30.h"
#include "unk10042740.h"
#include "unk10042e00.h"
#include "unk10046750.h"
#include "unk1004b980.h"
#include "unk1004c800.h"
#include "unk1004c820.h"
#include "unk1004c860.h"
#include "unk100696c0.h"
#include "unk10071930.h"
#include "unk1007d120.h"
#include "vfx3d.h"
#include "vfxa.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROUTINES 128

// Functions and globals only their own units use: unk1007d120.c's, ticks.asm's, sndunpack.asm's,
// transform.c's, unk10039a30.c's, unk10046750.c's and VFXA's.
void FUN_1000d7c0(Matrix* p_matrix, MechS32 p_column);
MechS32 FUN_1003a05d(MechS32 p_a, MechS32 p_b, MechS32 p_value);
extern MechU8* g_unk0x100c1a70;
extern MechU32 g_ticksPaused;
extern MechS32 g_ticks1Bases[64];
extern MechS32 g_ticks2Bases[64];
extern MechS32 g_ticks1;
extern MechS32 g_ticks2;
extern MechU8 g_unk0x100a2f04[0x400];
extern MechU8 g_unk0x100a3304[0x401];
extern MechU8 g_unk0x100a3705[0x43];
extern MechU8* g_unk0x1010b534;
extern MechS32 g_unk0x1010b5bc;
extern MechS32 g_unk0x1010b538;
extern MechS32 g_unk0x1010b5b4;
extern MechS32 g_unk0x1010b5a8;
struct WINDOW;
MechS32 VFX_shape_transform(
	struct PANE* p_pane,
	void* p_shapeTable,
	MechS32 p_shapeNumber,
	MechS32 p_hotX,
	MechS32 p_hotY,
	MechU8* p_buffer,
	MechS32 p_rot,
	MechS32 p_xScale,
	MechS32 p_yScale,
	MechU32 p_flags
);
MechS32 VFX_shape_scan(
	struct PANE* p_pane,
	MechU8 p_transparentColor,
	MechS32 p_hotX,
	MechS32 p_hotY,
	MechU8* p_buffer
);
void VFX_window_fade(struct WINDOW* p_buffer, MechU8* p_palette, MechS32 p_intervals);
extern MechChar driver_name[0xd];
extern MechU8 lookaside[0x100];
extern MechU8 color_error[0x300];

// The routines and globals linked in, by name.
typedef struct Symbol {
	const char* m_name;
	AsmFn m_function;
	void* m_data;
} Symbol;

#define FUNCTION(p_name) {#p_name, (AsmFn) p_name, NULL}
#define DATA(p_name) {#p_name, NULL, (void*) &p_name}

static const Symbol g_symbols[] = {
	FUNCTION(FixedMul16),
	FUNCTION(FixedMul30),
	FUNCTION(FixedDiv16),
	FUNCTION(FixedDiv29),
	FUNCTION(FixedDivU16),
	FUNCTION(FixedDot27),
	FUNCTION(FixedDot29),
	FUNCTION(MulDiv64),
	FUNCTION(ApproximateVectorLength),
	FUNCTION(FixedSqrtGuess),
	FUNCTION(FUN_100074e0),
	FUNCTION(IntegrateMidpoint),
	FUNCTION(FUN_10004ec0),
	FUNCTION(FUN_10013340),
	FUNCTION(FUN_10019ad0),
	FUNCTION(FUN_10034990),
	FUNCTION(FUN_100349c0),
	FUNCTION(FUN_100349f0),
	FUNCTION(FUN_10042740),
	FUNCTION(FUN_1004c800),
	FUNCTION(FUN_1004c820),
	FUNCTION(FUN_1004c860),
	FUNCTION(MemCopy),
	FUNCTION(MemSet),
	FUNCTION(FUN_100696c0),
	FUNCTION(FUN_1006975b),
	FUNCTION(FUN_100698de),
	FUNCTION(FUN_10071930),
	FUNCTION(FUN_100719ca),
	FUNCTION(FUN_10071a4c),
	FUNCTION(FUN_1007d248),
	FUNCTION(FUN_1007d296),
	FUNCTION(GameTickTimerCallback),
	FUNCTION(AllocTicks),
	FUNCTION(GetTicks),
	FUNCTION(ResetTicks),
	FUNCTION(SetTicks),
	FUNCTION(FreeTicks),
	FUNCTION(PauseTimer),
	FUNCTION(FUN_1001a63c),
	DATA(g_sinTable),
	DATA(g_atanTable),
	DATA(g_unk0x100c1a70),
	DATA(g_unk0x100c2698),
	DATA(g_unk0x1010b5ac),
	DATA(g_ticksPaused),
	DATA(g_ticks1Bases),
	DATA(g_ticks2Bases),
	DATA(g_ticks1),
	DATA(g_ticks2),
	DATA(g_unk0x100a2f04),
	DATA(g_unk0x100a3304),
	DATA(g_unk0x100a3705),
	FUNCTION(FUN_1000d650),
	FUNCTION(FUN_1000d708),
	FUNCTION(FUN_1000d7c0),
	FUNCTION(FUN_1000d9a8),
	FUNCTION(FUN_1000d9ce),
	FUNCTION(FUN_1000da0c),
	FUNCTION(FUN_1000de3b),
	FUNCTION(FUN_10039a30),
	FUNCTION(FUN_10039b94),
	FUNCTION(FUN_10039c96),
	FUNCTION(FUN_10039ccc),
	FUNCTION(FUN_10039dda),
	FUNCTION(FUN_1003a05d),
	FUNCTION(FUN_1003a096),
	FUNCTION(FUN_10048c50),
	FUNCTION(FUN_10048d46),
	FUNCTION(FUN_10048ebe),
	FUNCTION(FUN_10048faf),
	FUNCTION(FUN_10049155),
	DATA(g_sqrtTable),
	DATA(g_unk0x100ea820),
	DATA(g_unk0x100ea824),
	DATA(g_unk0x100ea828),
	DATA(g_unk0x100ea82c),
	DATA(g_unk0x100ea830),
	DATA(g_unk0x100ea834),
	DATA(g_unk0x100ea840),
	DATA(g_unk0x100ea84c),
	DATA(g_unk0x100ea850),
	DATA(g_unk0x100ea858),
	DATA(g_unk0x100ea864),
	DATA(g_unk0x100ea868),
	DATA(g_unk0x100ea86c),
	DATA(g_unk0x100ea870),
	DATA(g_unk0x100ea874),
	DATA(g_unk0x100ea878),
	DATA(g_unk0x100ea87c),
	DATA(g_unk0x100ea880),
	DATA(g_unk0x100ea884),
	DATA(g_unk0x100ea8b4),
	DATA(g_unk0x100ea8b8),
	DATA(g_unk0x100ea8bc),
	DATA(g_unk0x100ea8c0),
	DATA(g_unk0x100ea8c4),
	DATA(g_unk0x100ea8c8),
	DATA(g_unk0x1010b530),
	DATA(g_unk0x1010b53c),
	DATA(g_unk0x1010b5b8),
	DATA(g_unk0x1010b550),
	DATA(g_unk0x1010b5b0),
	DATA(g_unk0x1010b534),
	DATA(g_unk0x1010b5bc),
	DATA(g_unk0x1010b538),
	DATA(g_unk0x1010b5b4),
	DATA(g_unk0x1010b5a8),
	DATA(g_unk0x100c1a68),
	DATA(g_unk0x100a54b0),
	DATA(g_unk0x100a54b4),
	DATA(g_unk0x1010b5c4),
	DATA(g_unk0x1010b5c8),
	DATA(g_unk0x100a6cc8),
	FUNCTION(VFX_flat_polygon),
	FUNCTION(VFX_Gouraud_polygon),
	FUNCTION(VFX_dithered_Gouraud_polygon),
	FUNCTION(VFX_translate_polygon),
	FUNCTION(VFX_illuminate_polygon),
	FUNCTION(VFX_map_lookaside),
	FUNCTION(VFX_map_polygon),
	FUNCTION(VFX_driver_name),
	FUNCTION(VFX_register_driver),
	FUNCTION(VFX_pixel_write),
	FUNCTION(VFX_pixel_read),
	FUNCTION(VFX_line_draw),
	FUNCTION(VFX_rectangle_hash),
	FUNCTION(VFX_shape_draw),
	FUNCTION(DrawShapeUnclipped),
	FUNCTION(VFX_shape_lookaside),
	FUNCTION(VFX_shape_translate_draw),
	FUNCTION(XlatShapeUnclipped),
	FUNCTION(VFX_shape_transform),
	FUNCTION(VFX_shape_visible_rectangle),
	FUNCTION(VFX_shape_scan),
	FUNCTION(VFX_shape_remap_colors),
	FUNCTION(VFX_pane_wipe),
	FUNCTION(VFX_pane_copy),
	FUNCTION(VFX_pane_scroll),
	FUNCTION(VFX_ellipse_draw),
	FUNCTION(VFX_ellipse_fill),
	FUNCTION(VFX_Cos_Sin),
	FUNCTION(VFX_fixed_mul),
	FUNCTION(VFX_point_transform),
	FUNCTION(VFX_font_height),
	FUNCTION(VFX_character_width),
	FUNCTION(VFX_character_draw),
	FUNCTION(VFX_string_draw),
	FUNCTION(VFX_line_to_pane),
	FUNCTION(find_ILBM_property),
	FUNCTION(VFX_ILBM_draw),
	FUNCTION(VFX_ILBM_palette),
	FUNCTION(VFX_ILBM_resolution),
	FUNCTION(VFX_PCX_draw),
	FUNCTION(VFX_PCX_palette),
	FUNCTION(VFX_PCX_resolution),
	FUNCTION(VFX_GIF_draw),
	FUNCTION(VFX_GIF_palette),
	FUNCTION(VFX_GIF_resolution),
	FUNCTION(VFX_shape_bounds),
	FUNCTION(VFX_shape_origin),
	FUNCTION(VFX_shape_resolution),
	FUNCTION(VFX_shape_minxy),
	FUNCTION(VFX_shape_palette),
	FUNCTION(VFX_shape_colors),
	FUNCTION(VFX_shape_set_colors),
	FUNCTION(VFX_shape_count),
	FUNCTION(VFX_shape_list),
	FUNCTION(VFX_shape_palette_list),
	FUNCTION(VFX_pixel_fade),
	FUNCTION(VFX_window_fade),
	FUNCTION(VFX_color_scan),
	DATA(VFX_describe_driver),
	DATA(VFX_init_driver),
	DATA(VFX_shutdown_driver),
	DATA(VFX_area_wipe),
	DATA(VFX_wait_vblank),
	DATA(VFX_wait_vblank_leading),
	DATA(VFX_window_refresh),
	DATA(VFX_window_read),
	DATA(VFX_DAC_read),
	DATA(VFX_DAC_write),
	DATA(VFX_bank_reset),
	DATA(VFX_pane_refresh),
	DATA(VFX_line_address),
	DATA(driver_name),
	DATA(lookaside),
	DATA(color_error),
};

#define SYMBOL_COUNT ((MechS32) (sizeof(g_symbols) / sizeof(g_symbols[0])))

static const Symbol* FindSymbol(const char* p_name)
{
	MechS32 i;

	for (i = 0; i < SYMBOL_COUNT; i++) {
		if (!strcmp(g_symbols[i].m_name, p_name)) {
			return &g_symbols[i];
		}
	}

	printf("%s isn't linked in\n", p_name);
	exit(2);
}

static AsmFn LinkedFunction(void* p_handle, const char* p_name)
{
	(void) p_handle;
	return FindSymbol(p_name)->m_function;
}

static void* LinkedData(void* p_handle, const char* p_name)
{
	(void) p_handle;
	return FindSymbol(p_name)->m_data;
}

static const AsmModule g_linked = {LinkedFunction, LinkedData, NULL};

static void RunCase(
	const AsmRoutine* p_routine,
	MechS32 p_set,
	MechU32 p_index,
	AsmHash* p_inputs,
	AsmHash* p_outputs,
	MechS32 p_print
)
{
	MechS32 args[ASM_MAX_ARGS];
	MechS32 domain;
	AsmOutput output;
	MechS32 i;

	AsmMakeCase(p_routine, p_set, p_index, args);
	AsmHashCase(p_routine, args, p_inputs);

	if (p_print) {
		printf("%s %s %lu (", p_routine->m_name, g_asmSetNames[p_set], (unsigned long) p_index);
		for (i = 0; i < p_routine->m_arity; i++) {
			printf(i ? ", 0x%08lx" : "0x%08lx", (unsigned long) (MechU32) args[i]);
		}
		printf("): ");
	}

	domain = AsmDomain(p_routine, args);
	if (domain != c_domainIn) {
		AsmHashWord(p_outputs, domain == c_domainFault ? 1 : domain == c_domainUndefined ? 2 : 3);
		if (p_print) {
			printf(
				domain == c_domainFault       ? "out of domain\n"
				: domain == c_domainUndefined ? "undefined\n"
											  : "x86 only: asmequiv checks it\n"
			);
		}
		return;
	}

	AsmRun(p_routine, &g_linked, args, &output);
	if (output.m_count > ASM_MAX_OUTPUTS) {
		printf("%s: %ld outputs, more than ASM_MAX_OUTPUTS\n", p_routine->m_name, (long) output.m_count);
		exit(2);
	}

	AsmHashWord(p_outputs, 0);
	for (i = 0; i < output.m_count; i++) {
		AsmHashWord(p_outputs, output.m_words[i]);
		if (p_print) {
			printf(i ? " %08lx" : "%08lx", (unsigned long) output.m_words[i]);
		}
	}

	if (p_print) {
		printf("\n");
	}
}

// Whether the routine runs on this platform. FUN_10049155 copies pointers into 32-bit records
// (the polygon records of unk1007d120.c's buffer, 0xc bytes and a dword per point), which
// overlap where pointers are wider: it's checked on x86 only.
static MechS32 RunsHere(const AsmRoutine* p_routine)
{
	return sizeof(void*) == 4 || strcmp(p_routine->m_name, "FUN_10049155");
}

static int RunOneCase(char** p_argv)
{
	const AsmRoutine* routine = AsmFindRoutine(p_argv[0]);
	MechS32 set = AsmFindSet(p_argv[1]);
	AsmHash inputs;
	AsmHash outputs;

	if (!routine || set < 0) {
		printf("unknown routine or set\n");
		return 2;
	}

	if (!RunsHere(routine)) {
		printf("%s doesn't run on this platform\n", routine->m_name);
		return 2;
	}

	AsmHashInit(&inputs);
	AsmHashInit(&outputs);
	RunCase(routine, set, (MechU32) strtoul(p_argv[2], NULL, 0), &inputs, &outputs, 1);
	return 0;
}

int main(int p_argc, char** p_argv)
{
	FILE* file;
	char line[256];
	MechS32 lineNumber = 0;
	MechS32 failed = 0;
	MechS32 checked[MAX_ROUTINES];
	MechU32 shard = 0;
	MechU32 shardCount = 1;
	MechU32 vectorLine = 0;
	MechS32 i;

	if (p_argc == 6 && !strcmp(p_argv[2], "-case")) {
		return RunOneCase(p_argv + 3);
	}

	if (p_argc == 5 && !strcmp(p_argv[2], "-shard")) {
		shard = (MechU32) strtoul(p_argv[3], NULL, 0);
		shardCount = (MechU32) strtoul(p_argv[4], NULL, 0);
		if (!shardCount || shard >= shardCount) {
			printf("-shard K N needs K < N\n");
			return 2;
		}
	}
	else if (p_argc != 2) {
		printf("usage: asmgolden GOLDEN.txt [-shard K N] [-case ROUTINE SET INDEX]\n");
		return 2;
	}

	if (g_asmRoutineCount > MAX_ROUTINES) {
		printf("more routines than the checked blocks' array\n");
		return 2;
	}

	memset(checked, 0, sizeof(checked));
	file = fopen(p_argv[1], "r");
	if (!file) {
		printf("%s: can't read\n", p_argv[1]);
		return 2;
	}

	while (fgets(line, sizeof(line), file)) {
		char name[64];
		char setName[16];
		unsigned long block;
		unsigned long count;
		char inputText[32];
		char outputText[32];
		char actualInputs[17];
		char actualOutputs[17];
		const AsmRoutine* routine;
		MechS32 set;
		AsmHash inputs;
		AsmHash outputs;
		MechU32 index;

		lineNumber++;
		if (line[0] == '#' || line[0] == '\n') {
			continue;
		}

		if (sscanf(line, "%63s %15s %lu %lu %31s %31s", name, setName, &block, &count, inputText, outputText) != 6) {
			printf("%s:%ld: malformed line\n", p_argv[1], (long) lineNumber);
			failed = 1;
			continue;
		}

		routine = AsmFindRoutine(name);
		set = AsmFindSet(setName);
		if (!routine || set < 0) {
			printf("%s:%ld: unknown routine or set\n", p_argv[1], (long) lineNumber);
			failed = 1;
			continue;
		}

		checked[routine - g_asmRoutines]++;
		if (vectorLine++ % shardCount != shard || !RunsHere(routine)) {
			continue;
		}

		AsmHashInit(&inputs);
		AsmHashInit(&outputs);
		for (index = 0; index < count; index++) {
			RunCase(routine, set, block * ASM_BLOCK_SIZE + index, &inputs, &outputs, 0);
		}

		AsmHashFormat(&inputs, actualInputs);
		AsmHashFormat(&outputs, actualOutputs);
		if (strcmp(actualInputs, inputText)) {
			printf("%s %s block %lu: the inputs differ (case generator)\n", name, setName, block);
			failed = 1;
		}
		else if (strcmp(actualOutputs, outputText)) {
			printf(
				"%s %s block %lu: the outputs differ (cases %lu to %lu)\n",
				name,
				setName,
				block,
				block * ASM_BLOCK_SIZE,
				block * ASM_BLOCK_SIZE + count - 1
			);
			failed = 1;
		}
	}

	fclose(file);

	for (i = 0; i < g_asmRoutineCount; i++) {
		if (!RunsHere(&g_asmRoutines[i])) {
			printf("%-24s skipped: it only runs on x86\n", g_asmRoutines[i].m_name);
			continue;
		}

		printf("%-24s %3ld blocks\n", g_asmRoutines[i].m_name, (long) checked[i]);
		if (!checked[i]) {
			printf("%s: no golden vectors\n", g_asmRoutines[i].m_name);
			failed = 1;
		}
	}

	printf(failed ? "FAILED\n" : "OK\n");
	return failed;
}
