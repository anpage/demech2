#ifndef VFX3D_H
#define VFX3D_H

#include "decomp.h"
#include "pane.h"
#include "types.h"
#include "window.h"

// VFX3D, Miles Design VFX's polygon fillers (3rdparty/vfx/VFX3D.ASM; its portable C,
// common/src/vfx3d.c, in COMPAT_MODE), which both DLLs link.
#ifdef __cplusplus
extern "C"
{
#endif

	void VFX_flat_polygon(PANE* p_pane, MechS32 p_vcnt, MechU32* p_vlist);
	void VFX_Gouraud_polygon(PANE* p_pane, MechS32 p_vcnt, MechU32* p_vlist);
	void VFX_dithered_Gouraud_polygon(PANE* p_pane, MechS32 p_ditherAmount, MechS32 p_vcnt, MechU32* p_vlist);
	void VFX_illuminate_polygon(PANE* p_pane, MechS32 p_ditherAmount, MechS32 p_vcnt, MechU32* p_vlist);
	void VFX_translate_polygon(PANE* p_pane, MechS32 p_vcnt, MechU32* p_vlist, MechU8* p_lookaside);
	void VFX_map_lookaside(MechU16* p_table);
	void VFX_map_polygon(PANE* p_pane, MechS32 p_vcnt, MechU32* p_vlist, WINDOW* p_texture, MechS32 p_flags);

#ifdef __cplusplus
}
#endif

// reccmp reads annotations from C sources only, so VFX3D's are here, by name.

// FUNCTION: MW2SHELL 0x1002a968
// FUNCTION: MW2 0x10036918
// VFX_flat_polygon

// FUNCTION: MW2SHELL 0x1002ae41
// FUNCTION: MW2 0x10036df1
// VFX_Gouraud_polygon

// FUNCTION: MW2SHELL 0x1002b68b
// FUNCTION: MW2 0x1003763b
// FUNCTION: MW2MATROX 0x100134f7
// VFX_dithered_Gouraud_polygon

// FUNCTION: MW2SHELL 0x1002bf39
// FUNCTION: MW2 0x10037ee9
// FUNCTION: MW2MATROX 0x10013da5
// VFX_translate_polygon

// FUNCTION: MW2SHELL 0x1002c48d
// FUNCTION: MW2 0x1003843d
// FUNCTION: MW2MATROX 0x100142f9
// VFX_illuminate_polygon

// FUNCTION: MW2SHELL 0x1002cd3d
// FUNCTION: MW2 0x10038ced
// FUNCTION: MW2MATROX 0x10014ba9
// VFX_map_lookaside

// About 93%: VFX keeps the table of span routines (__map_logic) and the span routines inside
// the procedure, and reccmp reads them as data from the table on, so their relocated operands
// compare by value. The object is identical to the game's.
// FUNCTION: MW2SHELL 0x1002cd5d
// FUNCTION: MW2 0x10038d0d
// FUNCTION: MW2MATROX 0x10014bc9
// VFX_map_polygon

#endif // VFX3D_H
