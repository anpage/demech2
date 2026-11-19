#ifndef TICKS_H
#define TICKS_H

#include "compat.h"
#include "types.h"

// The tick counters of ticks.asm (ticks.c in COMPAT_MODE).
#ifdef __cplusplus
extern "C"
{
#endif

	void __cdecl GameTickTimerCallback(void);
	MechS16 __cdecl AllocTicks(MechU32 p_flags);
	MechS32 __cdecl GetTicks(MechU32 p_handle);
	void __cdecl ResetTicks(MechU32 p_handle);
	void __cdecl SetTicks(MechU32 p_handle, MechS32 p_ticks);
	void __cdecl FreeTicks(MechU32 p_handle);
	void __cdecl PauseTimer(MechS32 p_flags, MechS32 p_paused);

#ifdef __cplusplus
}
#endif

// ticks.asm's routines and data. reccmp reads annotations from C sources only, so they're here, by
// name.

// FUNCTION: MW2 0x10067ed8
// FUNCTION: MW2MATROX 0x10007de4
// FUNCTION: MW23DFX 0x1003daec
// FUNCTION: MW2ATI 0x1003b84c
// GameTickTimerCallback

// FUNCTION: MW2 0x10067f01
// FUNCTION: MW2MATROX 0x10007e0d
// FUNCTION: MW23DFX 0x1003db15
// FUNCTION: MW2ATI 0x1003b875
// AllocTicks

// FUNCTION: MW2 0x10067f6f
// FUNCTION: MW2MATROX 0x10007e7b
// FUNCTION: MW23DFX 0x1003db83
// FUNCTION: MW2ATI 0x1003b8e3
// GetTicks

// FUNCTION: MW2 0x10067faa
// FUNCTION: MW2MATROX 0x10007eb6
// FUNCTION: MW23DFX 0x1003dbbe
// FUNCTION: MW2ATI 0x1003b91e
// ResetTicks

// FUNCTION: MW2 0x10067fe5
// FUNCTION: MW2MATROX 0x10007ef1
// FUNCTION: MW23DFX 0x1003dbf9
// FUNCTION: MW2ATI 0x1003b959
// SetTicks

// FUNCTION: MW2 0x10068023
// FUNCTION: MW2MATROX 0x10007f2f
// FUNCTION: MW23DFX 0x1003dc37
// FUNCTION: MW2ATI 0x1003b997
// FreeTicks

// FUNCTION: MW2 0x10068058
// FUNCTION: MW2MATROX 0x10007f64
// FUNCTION: MW23DFX 0x1003dc6c
// FUNCTION: MW2ATI 0x1003b9cc
// PauseTimer

// GLOBAL: MW2 0x100ad008
// GLOBAL: MW2MATROX 0x100a2b34
// GLOBAL: MW23DFX 0x10259984
// GLOBAL: MW2ATI 0x1018dc84
// g_ticksPaused

// GLOBAL: MW2 0x100ad00c
// GLOBAL: MW2MATROX 0x100a2b38
// GLOBAL: MW23DFX 0x10259988
// GLOBAL: MW2ATI 0x1018dc88
// g_ticks1Bases

// GLOBAL: MW2 0x100ad10c
// GLOBAL: MW2MATROX 0x100a2c38
// GLOBAL: MW23DFX 0x10259a88
// GLOBAL: MW2ATI 0x1018dd88
// g_ticks2Bases

// GLOBAL: MW2 0x100ad20c
// GLOBAL: MW2MATROX 0x100a2d38
// GLOBAL: MW23DFX 0x10259b88
// GLOBAL: MW2ATI 0x1018de88
// g_ticks1

// GLOBAL: MW2 0x100ad210
// GLOBAL: MW2MATROX 0x100a2d3c
// GLOBAL: MW23DFX 0x10259b8c
// GLOBAL: MW2ATI 0x1018de8c
// g_ticks2

#endif // TICKS_H
