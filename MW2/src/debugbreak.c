#include "debugbreak.h"

/* Hand-written assembly: DebugBreakpoint's body is an inline int 3 (its own object). */

#ifndef MW2_3DFX /* the 3Dfx edition expands it inline: see debugbreak.h */
// FUNCTION: MW2 0x1000a9b0
// FUNCTION: MW2MATROX 0x1002f830
void DebugBreakpoint(void){
#if defined(_MSC_VER) && defined(_M_IX86)
	__asm int 3
#endif
}
#endif
