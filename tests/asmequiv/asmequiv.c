// asmequiv: runs every case through the reference (the matched hand-written assembly, asmref.dll)
// and a candidate (the portable C, asmport.dll), and compares the results. Each DLL is loaded
// separately, so each has its own copy of every global. Optionally writes the golden vectors that
// asmgolden checks a candidate against on other platforms.
//
//   asmequiv REF.dll CANDIDATE.dll [-golden FILE] [-blocks N] [-routine NAME] [-case SET INDEX]
//
// -blocks sets the number of random blocks (ASM_RANDOM_BLOCKS by default), -case runs and prints
// one case. Outside a routine's domain, the reference must fault (and is the only one called);
// inside it, it must not.

#include "asmequiv.h"

#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define MAX_REPORTS 10

static DWORD CallGuarded(AsmFn p_fn, MechS32 p_arity, const MechS32* p_args, MechS32* p_result)
{
	DWORD code = 0;

	__try {
		*p_result = AsmCall(p_fn, p_arity, p_args);
	}
	__except (code = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER) {
		*p_result = 0;
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

typedef struct Totals {
	MechU32 m_cases;
	MechU32 m_outOfDomain;
	MechU32 m_failures;
} Totals;

// Runs one case; returns whether it passed.
static MechS32 RunCase(
	const AsmRoutine* p_routine,
	AsmFn p_ref,
	AsmFn p_cand,
	MechS32 p_set,
	MechU32 p_index,
	MechS32 p_verbose,
	Totals* p_totals,
	AsmHash* p_inputs,
	AsmHash* p_outputs
)
{
	MechS32 args[ASM_MAX_ARGS];
	MechS32 inDomain;
	MechS32 expected;
	MechS32 actual;
	DWORD refCode;
	DWORD candCode;
	MechS32 i;
	MechS32 report;

	AsmMakeCase(p_routine, p_set, p_index, args);
	for (i = 0; i < p_routine->m_arity; i++) {
		AsmHashWord(p_inputs, (MechU32) args[i]);
	}

	inDomain = p_routine->m_inDomain ? p_routine->m_inDomain(args) : 1;
	refCode = CallGuarded(p_ref, p_routine->m_arity, args, &expected);
	p_totals->m_cases++;
	report = p_totals->m_failures < MAX_REPORTS || p_verbose;

	if (!inDomain) {
		p_totals->m_outOfDomain++;
		AsmHashWord(p_outputs, 1);
		if (!IsDivideFault(refCode)) {
			p_totals->m_failures++;
			if (report) {
				PrintCase(p_routine, p_set, p_index, args);
				printf(
					"out of domain, but the reference returns 0x%08lx (exception 0x%08lx)\n",
					(unsigned long) (MechU32) expected,
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

	AsmHashWord(p_outputs, 0);
	AsmHashWord(p_outputs, (MechU32) expected);
	if (refCode) {
		p_totals->m_failures++;
		if (report) {
			PrintCase(p_routine, p_set, p_index, args);
			printf("in domain, but the reference faults (exception 0x%08lx)\n", (unsigned long) refCode);
		}
		return 0;
	}

	candCode = CallGuarded(p_cand, p_routine->m_arity, args, &actual);
	if (candCode || actual != expected) {
		p_totals->m_failures++;
		if (report) {
			PrintCase(p_routine, p_set, p_index, args);
			if (candCode) {
				printf(
					"reference 0x%08lx, candidate faults (exception 0x%08lx)\n",
					(unsigned long) (MechU32) expected,
					(unsigned long) candCode
				);
			}
			else {
				printf(
					"reference 0x%08lx, candidate 0x%08lx\n",
					(unsigned long) (MechU32) expected,
					(unsigned long) (MechU32) actual
				);
			}
		}
		return 0;
	}

	if (p_verbose) {
		PrintCase(p_routine, p_set, p_index, args);
		printf("0x%08lx\n", (unsigned long) (MechU32) expected);
	}

	return 1;
}

static AsmFn Resolve(HMODULE p_module, const char* p_dll, const char* p_name)
{
	AsmFn fn = (AsmFn) GetProcAddress(p_module, p_name);

	if (!fn) {
		printf("%s: no export %s\n", p_dll, p_name);
		exit(2);
	}

	return fn;
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
	MechS32 randomBlocks = ASM_RANDOM_BLOCKS;
	MechS32 caseSet = -1;
	MechU32 caseIndex = 0;
	HMODULE ref;
	HMODULE cand;
	FILE* golden = NULL;
	MechS32 failed = 0;
	MechS32 r;
	MechS32 i;

	if (p_argc < 3) {
		printf("usage: asmequiv REF.dll CANDIDATE.dll [-golden FILE] [-blocks N] [-routine NAME] [-case SET INDEX]\n");
		return 2;
	}

	refDll = p_argv[1];
	candDll = p_argv[2];
	for (i = 3; i < p_argc; i++) {
		if (!strcmp(p_argv[i], "-golden") && i + 1 < p_argc) {
			goldenPath = p_argv[++i];
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

	if (caseSet >= 0 && (!only || goldenPath)) {
		printf("-case needs -routine, and doesn't write golden vectors\n");
		return 2;
	}

	ref = Load(refDll);
	cand = Load(candDll);

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
		AsmFn refFn;
		AsmFn candFn;
		Totals totals;
		MechS32 set;

		if (only && strcmp(only, routine->m_name)) {
			continue;
		}

		refFn = Resolve(ref, refDll, routine->m_name);
		candFn = Resolve(cand, candDll, routine->m_name);
		memset(&totals, 0, sizeof(totals));

		if (caseSet >= 0) {
			AsmHash inputs;
			AsmHash outputs;

			AsmHashInit(&inputs);
			AsmHashInit(&outputs);
			if (!RunCase(routine, refFn, candFn, caseSet, caseIndex, 1, &totals, &inputs, &outputs)) {
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
					RunCase(routine, refFn, candFn, set, index, 0, &totals, &inputs, &outputs);
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
			"%-12s %8lu cases, %8lu out of domain, %lu failed\n",
			routine->m_name,
			(unsigned long) totals.m_cases,
			(unsigned long) totals.m_outOfDomain,
			(unsigned long) totals.m_failures
		);
		if (totals.m_failures) {
			failed = 1;
		}
	}

	if (golden) {
		fclose(golden);
	}

	printf(failed ? "FAILED\n" : "OK\n");
	return failed;
}
