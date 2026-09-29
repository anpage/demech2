#include "rendertarget.h"

#include "decomp.h"
#include "types.h"

DECOMP_SIZE_ASSERT(PixelBuffer, 0x14)
DECOMP_SIZE_ASSERT(RenderTarget, 0x14)

// STUB: MW2 0x100630b9
void FillRenderTargetRect(RenderTarget* p_target, MechS32 p_color)
{
	STUB(0x100630b9);
}

// STUB: MW2 0x10064d60
MechS32 FUN_10064d60(undefined4 p_font, MechS32 p_char)
{
	STUB(0x10064d60);
	return 0;
}

// STUB: MW2 0x10065a7b
MechS32 FUN_10065a7b(RenderTarget* p_dst, RenderTarget* p_src, MechS32 p_unk0x08, MechS32 p_unk0x0c)
{
	STUB(0x10065a7b);
	return 0;
}
