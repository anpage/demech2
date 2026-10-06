#include "random.h"

#include "types.h"

#include <math.h>
#include <stdlib.h>

// Two tables of 127 random numbers, read round-robin: rand() values, and values roughly normally
// distributed around 0 (the sum of 30 rand() calls, scaled to +-sqrt(90) * 1024).

// GLOBAL: MW2 0x100ae74c
// GLOBAL: MW2MATROX 0x100a4570
MechS32 g_randomIndex = 0;

// GLOBAL: MW2 0x100ae750
// GLOBAL: MW2MATROX 0x100a4574
MechS32 g_normalRandomIndex = 0;

// A second pair of indices, so a second stream of callers doesn't disturb the first.

// GLOBAL: MW2 0x100ae754
// GLOBAL: MW2MATROX 0x100a4578
MechS32 g_randomIndex2 = 0;

// GLOBAL: MW2 0x100ae758
// GLOBAL: MW2MATROX 0x100a457c
MechS32 g_normalRandomIndex2 = 0;

// GLOBAL: MW2 0x100c2d20
// GLOBAL: MW2MATROX 0x10215920
MechS32 g_normalRandomInts[127];

// GLOBAL: MW2 0x100c2f20
// GLOBAL: MW2MATROX 0x10215b20
MechS32 g_randomInts[127];

// Stack-slot permutation: sum, scale, i, j and range.
// FUNCTION: MW2 0x100735e0
void InitRandom(MechU32 p_seed)
{
	MechS32 sum;
	MechDouble scale;
	MechS32 i;
	MechS32 j;
	MechDouble range;

	srand(p_seed);
	for (i = 0; i < 127; i++) {
		g_randomInts[i] = rand();
	}

	scale = (range = sqrt(90.0)) * 2.0 / (30 * RAND_MAX);
	for (i = 0; i < 127; i++) {
		sum = 0;
		for (j = 0; j < 30; j++) {
			sum += rand();
		}

		g_normalRandomInts[i] = (MechS32) ((sum * scale - range) * 1024.0);
	}
}

// FUNCTION: MW2 0x100736b3
// FUNCTION: MW2MATROX 0x1000bfdf
MechS32 RandomIntBelow(MechS32 p_max)
{
	MechS32 value;

	value = g_randomInts[g_randomIndex] % p_max;
	g_randomIndex++;
	g_randomIndex %= 127;
	return value;
}

// FUNCTION: MW2 0x100736f5
// FUNCTION: MW2MATROX 0x1000c021
MechS32 RandomNormal(void)
{
	MechS32 value;

	value = g_normalRandomInts[g_normalRandomIndex];
	g_normalRandomIndex++;
	g_normalRandomIndex %= 127;
	return value;
}

// FUNCTION: MW2 0x10073733
// FUNCTION: MW2MATROX 0x1000c05f
MechS32 RandomIntBelow2(MechS32 p_max)
{
	MechS32 value;

	value = g_randomInts[g_randomIndex2] % p_max;
	g_randomIndex2++;
	g_randomIndex2 %= 127;
	return value;
}

// FUNCTION: MW2 0x10073775
// FUNCTION: MW2MATROX 0x1000c0a1
MechS32 RandomNormal2(void)
{
	MechS32 value;

	value = g_normalRandomInts[g_normalRandomIndex2];
	g_normalRandomIndex2++;
	g_normalRandomIndex2 %= 127;
	return value;
}

// FUNCTION: MW2 0x100737b3
// FUNCTION: MW2MATROX 0x1000c0df
MechS32 RandomNormalMean(void)
{
	return 0;
}

// FUNCTION: MW2 0x100737c5
// FUNCTION: MW2MATROX 0x1000c0f1
MechS32 RandomNormalDeviation(void)
{
	return 0x400;
}
