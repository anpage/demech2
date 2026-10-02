#ifndef ASMEQUIV_H
#define ASMEQUIV_H

#include "portable.h"
#include "types.h"

// The cases both test programs run: asmequiv (the reference assembly against a candidate, x86
// Windows) and asmgolden (a candidate against the golden vectors asmequiv writes, any
// platform). Both generate every case from its routine, set and index alone, with integer
// arithmetic that is the same on every platform, so a case can be rerun by itself anywhere.

#define ASM_MAX_ARGS 9
#define ASM_MAX_OUTPUTS 2048
#define ASM_BLOCK_SIZE 4096 // cases per golden-vector line
#define ASM_RANDOM_BLOCKS 16

typedef void (*AsmFn)(void);

// A build of the routines: asmref.dll, asmport.dll, or the routines linked into asmgolden. A
// lookup that fails ends the program.
typedef struct AsmModule {
	AsmFn (*m_function)(void* p_handle, const char* p_name);
	void* (*m_data)(void* p_handle, const char* p_name);
	void* m_handle;
} AsmModule;

// What a case returns and writes, in 32-bit words.
typedef struct AsmOutput {
	MechU32 m_words[ASM_MAX_OUTPUTS];
	MechS32 m_count;
} AsmOutput;

void AsmOutputWord(AsmOutput* p_output, MechU32 p_word);
void AsmOutputBytes(AsmOutput* p_output, const MechU8* p_bytes, MechU32 p_size); // p_size % 4 == 0

// Whether the original computes a result for a case's arguments.
enum {
	c_domainIn,       // it does: the candidate must compute the same
	c_domainFault,    // it faults (idiv): callers never get there, the candidate may do anything
	c_domainUndefined // its result isn't defined (bsr of 0): neither is called
};

// 64-bit FNV-1a over 32-bit words, little-endian whatever the host's byte order.
typedef struct AsmHash {
	MechU64 m_value;
} AsmHash;

// An entry point.
typedef struct AsmRoutine {
	const char* m_name;
	MechS32 m_arity; // the argument words AsmMakeCase generates for a case
	// NULL: c_domainIn for all arguments.
	MechS32 (*m_domain)(const MechS32* p_args);
	// Sets up the state a case reads in a module, calls the routine and records what it returns
	// and writes. NULL: the routine takes the m_arity words as MechS32 arguments and returns a
	// MechS32, its one output (exactly: a call through another function type is undefined).
	void (*m_run)(const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output);
	// Adds the inputs a case derives from its arguments other than with integer arithmetic
	// (tables computed with the C library's sin) to the input hash. NULL: none.
	void (*m_hashInputs)(const MechS32* p_args, AsmHash* p_hash);
} AsmRoutine;

enum {
	c_setEdge,   // boundary values, exhaustively combined where the arity allows
	c_setRandom, // seeded random values, ASM_RANDOM_BLOCKS blocks by default
	c_setCount
};

extern const AsmRoutine g_asmRoutines[];
extern const MechS32 g_asmRoutineCount;
extern const char* const g_asmSetNames[c_setCount];

const AsmRoutine* AsmFindRoutine(const char* p_name);
MechS32 AsmFindSet(const char* p_name);
MechU32 AsmEdgeCaseCount(const AsmRoutine* p_routine);
void AsmMakeCase(const AsmRoutine* p_routine, MechS32 p_set, MechU32 p_index, MechS32* p_args);
MechS32 AsmDomain(const AsmRoutine* p_routine, const MechS32* p_args);
void AsmHashCase(const AsmRoutine* p_routine, const MechS32* p_args, AsmHash* p_hash);
void AsmRun(const AsmRoutine* p_routine, const AsmModule* p_module, const MechS32* p_args, AsmOutput* p_output);
MechS32 AsmCall(AsmFn p_fn, MechS32 p_arity, const MechS32* p_args);

// The case generator's integer hash and the random numbers runners draw from it.
MechU32 AsmMix(MechU32 p_x);
MechU32 AsmNext(MechU32* p_state);
MechU32 AsmSeed(const MechS32* p_args, MechS32 p_count);

void AsmHashInit(AsmHash* p_hash);
void AsmHashWord(AsmHash* p_hash, MechU32 p_word);
void AsmHashFormat(const AsmHash* p_hash, char* p_buffer); // 17 bytes

#endif // ASMEQUIV_H
