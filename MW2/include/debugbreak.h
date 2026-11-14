#ifndef DEBUGBREAK_H
#define DEBUGBREAK_H

// The functions and globals of debugbreak.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef MW2_3DFX
	// The 3Dfx edition expands DebugBreakpoint inline in its callers (its /Ox implies /Ob1), so the
	// header defines it there; the other targets call debugbreak.c's.
	// clang-format off
	__inline void DebugBreakpoint(void)
	{
#if defined(_MSC_VER) && defined(_M_IX86)
		__asm int 3
#endif
	}
	// clang-format on
#else
void DebugBreakpoint(void);
#endif

#ifdef __cplusplus
}
#endif

#endif // DEBUGBREAK_H
