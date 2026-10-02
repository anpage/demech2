#ifndef POLYFILL_H
#define POLYFILL_H

#include "decomp.h"
#include "pane.h"
#include "types.h"

// The polygon fillers of polyfill.asm (common/src; its portable C, polyfill.c, in COMPAT_MODE).
// MW2's polyfill.h declares them with the same types, for polyfill.c.
#ifdef __cplusplus
extern "C"
{
#endif

	extern undefined4 g_polyVars[0x74];
	extern void (*g_polySpanRoutines[4])(void);

	void FillPolygonFlat(Pane* p_view, MechS32 p_count, MechU32* p_vertices);
	void FUN_1002ae41(Pane* p_view, MechS32 p_count, MechU32* p_vertices);
	void FUN_1002b68b(Pane* p_view, MechS32 p_dither, MechS32 p_count, MechU32* p_vertices);
	void FUN_1002bf39(Pane* p_view, MechS32 p_count, MechU32* p_vertices, MechU8* p_table);
	void FUN_1002c48d(Pane* p_view, MechS32 p_dither, MechS32 p_count, MechU32* p_vertices);
	void SetLumaTable(MechU16* p_table);
	void FillPolygonTextured(Pane* p_view, MechS32 p_count, MechU32* p_vertices, PixelBuffer* p_source, MechS32 p_mode);
	void FUN_1002d2b0(void);
	void FUN_1002d457(void);
	void FUN_1002d5c3(void);
	void FUN_1002d724(void);

#ifdef __cplusplus
}
#endif

// reccmp reads annotations from C sources only, so polyfill.asm's are here, by name.

// GLOBAL: MW2SHELL 0x100666a0
// g_polyVars

// FUNCTION: MW2SHELL 0x1002a968
// FillPolygonFlat

// FUNCTION: MW2SHELL 0x1002ae41
// FUN_1002ae41

// FUNCTION: MW2SHELL 0x1002b68b
// FUN_1002b68b

// FUNCTION: MW2SHELL 0x1002bf39
// FUN_1002bf39

// FUNCTION: MW2SHELL 0x1002c48d
// FUN_1002c48d

// FUNCTION: MW2SHELL 0x1002cd3d
// SetLumaTable

// FUNCTION: MW2SHELL 0x1002cd5d
// FillPolygonTextured

// GLOBAL: MW2SHELL 0x1002d2a0
// g_polySpanRoutines

// FUNCTION: MW2SHELL 0x1002d2b0
// FUN_1002d2b0

// FUNCTION: MW2SHELL 0x1002d457
// FUN_1002d457

// FUNCTION: MW2SHELL 0x1002d5c3
// FUN_1002d5c3

// FUNCTION: MW2SHELL 0x1002d724
// FUN_1002d724

#endif // POLYFILL_H
