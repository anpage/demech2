#ifndef POLYFILL_H
#define POLYFILL_H

#include "decomp.h"
#include "pixelbuffer.h"
#include "rendertarget.h"
#include "types.h"

// The polygon fillers of polyfill.asm (common/src; polyfill.c in COMPAT_MODE). The placeholder
// names are MW2SHELL's.
#ifdef __cplusplus
extern "C"
{
#endif

	extern undefined4 g_polyVars[0x74];
	extern void (*g_polySpanRoutines[4])(void);

	void FillPolygonFlat(RenderTarget* p_target, MechS32 p_count, MechU32* p_points);
	void FUN_1002ae41(RenderTarget* p_target, MechS32 p_count, MechU32* p_points);
	void FUN_1002b68b(RenderTarget* p_target, MechS32 p_unk0x04, MechS32 p_count, MechU32* p_points);
	void FUN_1002bf39(RenderTarget* p_target, MechS32 p_count, MechU32* p_points, undefined4 p_unk0x0c);
	void FUN_1002c48d(RenderTarget* p_target, MechS32 p_unk0x04, MechS32 p_count, MechU32* p_points);
	void SetLumaTable(MechU16* p_table);
	void FillPolygonTextured(
		RenderTarget* p_target,
		MechS32 p_count,
		MechU32* p_points,
		PixelBuffer* p_source,
		MechS32 p_mode
	);
	void FUN_1002d2b0(void);
	void FUN_1002d457(void);
	void FUN_1002d5c3(void);
	void FUN_1002d724(void);

#ifdef __cplusplus
}
#endif

// reccmp reads annotations from C sources only, so polyfill.asm's are here, by name.

// GLOBAL: MW2 0x100a5560
// g_polyVars

// FUNCTION: MW2 0x10036918
// FillPolygonFlat

// FUNCTION: MW2 0x10036df1
// FUN_1002ae41

// FUNCTION: MW2 0x1003763b
// FUN_1002b68b

// FUNCTION: MW2 0x10037ee9
// FUN_1002bf39

// FUNCTION: MW2 0x1003843d
// FUN_1002c48d

// FUNCTION: MW2 0x10038ced
// SetLumaTable

// FUNCTION: MW2 0x10038d0d
// FillPolygonTextured

// GLOBAL: MW2 0x10039250
// g_polySpanRoutines

// FUNCTION: MW2 0x10039260
// FUN_1002d2b0

// FUNCTION: MW2 0x10039407
// FUN_1002d457

// FUNCTION: MW2 0x10039573
// FUN_1002d5c3

// FUNCTION: MW2 0x100396d4
// FUN_1002d724

#endif // POLYFILL_H
