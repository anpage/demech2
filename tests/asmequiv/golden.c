// asmgolden: checks the portable C, linked in, against the golden vectors asmequiv wrote from the
// reference. Builds on any platform (no assembly, no Windows); meant to run under UBSan and ASan.
//
//   asmgolden GOLDEN.txt [-case ROUTINE SET INDEX]
//
// A block whose input hash differs means the case generator computes differently on this
// platform (a harness bug); one whose output hash differs has a failing case, which
// `asmequiv ... -routine ROUTINE -case SET INDEX` shows against the reference, and -case here
// against this build.

#include "asmequiv.h"
#include "fixeddiv.h"
#include "fixeddiv29.h"
#include "fixeddivu.h"
#include "fixeddot27.h"
#include "fixeddot29.h"
#include "fixedmul.h"
#include "fixedmul30.h"
#include "muldiv.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Candidate {
	const char* m_name;
	AsmFn m_fn;
	MechS32 m_blocks; // golden lines checked
} Candidate;

static Candidate g_candidates[] = {
	{"FixedMul16", (AsmFn) FixedMul16, 0},
	{"FixedMul30", (AsmFn) FixedMul30, 0},
	{"FixedDiv16", (AsmFn) FixedDiv16, 0},
	{"FixedDiv29", (AsmFn) FixedDiv29, 0},
	{"FixedDivU16", (AsmFn) FixedDivU16, 0},
	{"FixedDot27", (AsmFn) FixedDot27, 0},
	{"FixedDot29", (AsmFn) FixedDot29, 0},
	{"MulDiv64", (AsmFn) MulDiv64, 0},
};

#define CANDIDATE_COUNT ((MechS32) (sizeof(g_candidates) / sizeof(g_candidates[0])))

static Candidate* FindCandidate(const char* p_name)
{
	MechS32 i;

	for (i = 0; i < CANDIDATE_COUNT; i++) {
		if (!strcmp(g_candidates[i].m_name, p_name)) {
			return &g_candidates[i];
		}
	}

	return NULL;
}

static void RunCase(
	const AsmRoutine* p_routine,
	AsmFn p_fn,
	MechS32 p_set,
	MechU32 p_index,
	AsmHash* p_inputs,
	AsmHash* p_outputs,
	MechS32 p_print
)
{
	MechS32 args[ASM_MAX_ARGS];
	MechS32 i;

	AsmMakeCase(p_routine, p_set, p_index, args);
	for (i = 0; i < p_routine->m_arity; i++) {
		AsmHashWord(p_inputs, (MechU32) args[i]);
	}

	if (p_print) {
		printf("%s %s %lu (", p_routine->m_name, g_asmSetNames[p_set], (unsigned long) p_index);
		for (i = 0; i < p_routine->m_arity; i++) {
			printf(i ? ", 0x%08lx" : "0x%08lx", (unsigned long) (MechU32) args[i]);
		}
		printf("): ");
	}

	if (p_routine->m_inDomain && !p_routine->m_inDomain(args)) {
		AsmHashWord(p_outputs, 1);
		if (p_print) {
			printf("out of domain\n");
		}
		return;
	}

	{
		MechS32 result = AsmCall(p_fn, p_routine->m_arity, args);

		AsmHashWord(p_outputs, 0);
		AsmHashWord(p_outputs, (MechU32) result);
		if (p_print) {
			printf("0x%08lx\n", (unsigned long) (MechU32) result);
		}
	}
}

static int RunOneCase(char** p_argv)
{
	const AsmRoutine* routine = AsmFindRoutine(p_argv[0]);
	Candidate* candidate = FindCandidate(p_argv[0]);
	MechS32 set = AsmFindSet(p_argv[1]);
	AsmHash inputs;
	AsmHash outputs;

	if (!routine || !candidate || set < 0) {
		printf("unknown routine or set\n");
		return 2;
	}

	AsmHashInit(&inputs);
	AsmHashInit(&outputs);
	RunCase(routine, candidate->m_fn, set, (MechU32) strtoul(p_argv[2], NULL, 0), &inputs, &outputs, 1);
	return 0;
}

int main(int p_argc, char** p_argv)
{
	FILE* file;
	char line[256];
	MechS32 lineNumber = 0;
	MechS32 failed = 0;
	MechS32 i;

	if (p_argc == 6 && !strcmp(p_argv[2], "-case")) {
		return RunOneCase(p_argv + 3);
	}

	if (p_argc != 2) {
		printf("usage: asmgolden GOLDEN.txt [-case ROUTINE SET INDEX]\n");
		return 2;
	}

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
		Candidate* candidate;
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
		candidate = FindCandidate(name);
		set = AsmFindSet(setName);
		if (!routine || !candidate || set < 0) {
			printf("%s:%ld: unknown routine or set\n", p_argv[1], (long) lineNumber);
			failed = 1;
			continue;
		}

		AsmHashInit(&inputs);
		AsmHashInit(&outputs);
		for (index = 0; index < count; index++) {
			RunCase(routine, candidate->m_fn, set, block * ASM_BLOCK_SIZE + index, &inputs, &outputs, 0);
		}

		AsmHashFormat(&inputs, actualInputs);
		AsmHashFormat(&outputs, actualOutputs);
		candidate->m_blocks++;
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

	for (i = 0; i < CANDIDATE_COUNT; i++) {
		printf("%-12s %3ld blocks\n", g_candidates[i].m_name, (long) g_candidates[i].m_blocks);
		if (!g_candidates[i].m_blocks) {
			printf("%s: no golden vectors\n", g_candidates[i].m_name);
			failed = 1;
		}
	}

	printf(failed ? "FAILED\n" : "OK\n");
	return failed;
}
