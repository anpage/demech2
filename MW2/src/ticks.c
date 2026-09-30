/* Stubs for ticks.asm's routines and its data, for builds with other compilers (COMPAT_MODE): the
   VC++ 4.1 build assembles ticks.asm with MASM 6.11 instead. */
#include "ticks.h"

#include "decomp.h"
#include "types.h"

MechU32 g_ticksPaused = 0;
MechS32 g_ticks1Bases[64] = {0};
MechS32 g_ticks2Bases[64] = {0};
MechS32 g_ticks1 = 0;
MechS32 g_ticks2 = 0;

void GameTickTimerCallback(void)
{
	STUB(0x10067ed8);
}

MechS16 AllocTicks(MechU32 p_flags)
{
	STUB(0x10067f01);
	return 0;
}

MechS32 GetTicks(MechU32 p_handle)
{
	STUB(0x10067f6f);
	return 0;
}

void ResetTicks(MechU32 p_handle)
{
	STUB(0x10067faa);
}

void SetTicks(MechU32 p_handle, MechS32 p_ticks)
{
	STUB(0x10067fe5);
}

void FreeTicks(MechU32 p_handle)
{
	STUB(0x10068023);
}

void PauseTimer(MechS32 p_flags, MechS32 p_paused)
{
	STUB(0x10068058);
}
