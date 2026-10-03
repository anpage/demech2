/* Stubs for VFXREND's routines (3rdparty/vfx/VFXREND.ASM), for builds with other compilers
   (COMPAT_MODE): the VC++ 4.1 build assembles VFXREND.ASM with MASM 6.11 instead. The values the
   stubs pass are MW2SHELL's addresses. */
#include "vfxrend.h"

#include "compat.h"
#include "decomp.h"
#include "pane.h"
#include "types.h"

void VFX_set_Gouraud_dither_level(MechS32 p_dither1, MechS32 p_dither2)
{
	STUB(0x100286a6);
}

MechS32 GetCodeBlock(undefined4* p_start, undefined4* p_selector)
{
	STUB(0x100286c3);
	return 0;
}

void VFX_polygon_render(
	PANE* p_pane,
	MechU32* p_vlist,
	MechS32 p_nvertices,
	MechS32 p_operation,
	undefined4 p_color,
	VFX_TEXTURE* p_texture,
	MechU16* p_cueing,
	undefined4 p_translucency
)
{
	STUB(0x100286eb);
}

MechS32 F16_div_to_F30(MechS32 p_dividend, MechS32 p_divisor)
{
	STUB(0x1002875c);
	return 0;
}

MechS32 F30_reciprocal(MechS32 p_value)
{
	STUB(0x10028794);
	return 0;
}

MechS32 mul_F30(MechS32 p_m1, MechS32 p_m2)
{
	STUB(0x100287c4);
	return 0;
}

void VFX_polygon_clip_XY_and_render(
	PANE* p_pane,
	MechU32* p_vlist,
	MechS32 p_nvertices,
	MechS32 p_operation,
	undefined4 p_color,
	VFX_TEXTURE* p_texture,
	MechU16* p_cueing,
	undefined4 p_translucency
)
{
	STUB(0x100287e0);
}
