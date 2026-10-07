#ifndef CLOCK_H
#define CLOCK_H

#include "fixedfloat.h"
#include "transform.h"
#include "types.h"

#include <windows.h>

// The sizes of the sine and arctangent tables: a quarter wave in 256 steps and two copies of the
// last value in 1.1, 512 steps in the Matrox edition (floats).
#ifdef MW2_MATROX
#define TRIG_TABLE_SIZE 0x200
#else
#define TRIG_TABLE_SIZE 0x102
#endif

// ScaleVectorToLength's parameters: the Matrox edition only normalizes, to length 1.
#ifdef MW2_MATROX
#define SCALE_VECTOR_PARAMS MechScalar *p_x, MechScalar *p_y, MechScalar *p_z
#else
#define SCALE_VECTOR_PARAMS MechS32 p_length, MechS32 *p_x, MechS32 *p_y, MechS32 *p_z
#endif

// The functions and globals of clock.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechScalar g_slopeSines[800];
	extern MechScalar g_slopeCosines[800];
	extern MechS32 g_currentClock;
	extern MechS32 g_framerateLimit;
	extern MechS32 g_timeExpansionEnabled;
	extern MechS32 g_timeCompressionEnabled;
	extern MechS32 g_realClock;
	extern MechS32 g_clockMode;
	extern MechS32 g_deltaTime;
	extern BOOL g_ticksTimerInitialized;
	extern MechS16* g_sqrtTable;
	extern MechScalar g_sinTable[TRIG_TABLE_SIZE];
	extern MechScalar g_atanTable[TRIG_TABLE_SIZE];
	extern MechS16 g_sqrtTableData[0x400];

	MechS32 InitSinAtanTables(void);
	MechS32 InitSlopeTables(void);
	MechS32 InitSqrtTable(void);
	MechScalar Hypot2D(MechScalar p_x, MechScalar p_y);
	void BuildMatrixFromDirection(Matrix* p_matrix, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	void NormalizeRotation(Matrix* p_matrix);
	void ScaleVectorToLength(SCALE_VECTOR_PARAMS);
	void FirstClock(void);
	void NextClock(void);
	void StopTimers(void);
	MechS32 GetGameClock(void);
	MechS32 GetTicksSinceSync(void);
	void ResetSyncTicks(void);
	void ResetClocks(void);
	MechS32 GetRealClock(void);

#ifdef __cplusplus
}
#endif

#endif // CLOCK_H
