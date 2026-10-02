#ifndef ASMEQUIV_H
#define ASMEQUIV_H

#include "portable.h"
#include "types.h"

// The cases both test programs run: asmequiv (the reference assembly against a candidate, x86
// Windows) and asmgolden (a candidate against the golden vectors asmequiv writes, any
// platform). Both generate every case from its routine, set and index alone, with integer
// arithmetic that is the same on every platform, so a case can be rerun by itself anywhere.

#define ASM_MAX_ARGS 6
#define ASM_BLOCK_SIZE 4096 // cases per golden-vector line
#define ASM_RANDOM_BLOCKS 16

typedef void (*AsmFn)(void);

// An entry point: every routine so far takes and returns MechS32 values.
typedef struct AsmRoutine {
	const char* m_name;
	MechS32 m_arity;
	// Whether the original computes a result for the arguments; NULL: for all. Outside the
	// domain the original faults, and the portable C may do anything (callers never get there).
	MechS32 (*m_inDomain)(const MechS32* p_args);
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
MechS32 AsmCall(AsmFn p_fn, MechS32 p_arity, const MechS32* p_args);

// 64-bit FNV-1a over 32-bit words, little-endian whatever the host's byte order.
typedef struct AsmHash {
	MechU64 m_value;
} AsmHash;

void AsmHashInit(AsmHash* p_hash);
void AsmHashWord(AsmHash* p_hash, MechU32 p_word);
void AsmHashFormat(const AsmHash* p_hash, char* p_buffer); // 17 bytes

#endif // ASMEQUIV_H
