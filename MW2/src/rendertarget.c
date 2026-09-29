#include "rendertarget.h"

#include "decomp.h"
#include "types.h"

DECOMP_SIZE_ASSERT(PixelBuffer, 0x14)
DECOMP_SIZE_ASSERT(RenderTarget, 0x14)

// STUB: MW2 0x100606ed
MechS32 FUN_100606ed(
	RenderTarget* p_target,
	MechS32 p_x1,
	MechS32 p_y1,
	MechS32 p_x2,
	MechS32 p_y2,
	MechS32 p_mode,
	MechS32 p_color
)
{
	STUB(0x100606ed);
	return 0;
}

// STUB: MW2 0x100610ef
void FUN_100610ef(
	RenderTarget* p_target,
	MechS32 p_left,
	MechS32 p_top,
	MechS32 p_right,
	MechS32 p_bottom,
	MechU8 p_color
)
{
	STUB(0x100610ef);
}

// STUB: MW2 0x10061228
void DrawShapeFrame(RenderTarget* p_target, void* p_shape, MechS32 p_frame, MechS32 p_x, MechS32 p_y)
{
	STUB(0x10061228);
}

// STUB: MW2 0x100630b9
void FillRenderTargetRect(RenderTarget* p_target, MechS32 p_color)
{
	STUB(0x100630b9);
}

// STUB: MW2 0x10063755
void FUN_10063755(
	RenderTarget* p_target,
	MechS32 p_centerX,
	MechS32 p_centerY,
	MechS32 p_radiusX,
	MechS32 p_radiusY,
	MechS32 p_color
)
{
	STUB(0x10063755);
}

// STUB: MW2 0x10063a96
void FUN_10063a96(
	RenderTarget* p_target,
	MechS32 p_centerX,
	MechS32 p_centerY,
	MechS32 p_radiusX,
	MechS32 p_radiusY,
	MechS32 p_color
)
{
	STUB(0x10063a96);
}

// STUB: MW2 0x10064d4d
MechS32 FUN_10064d4d(void* p_font)
{
	STUB(0x10064d4d);
	return 0;
}

// STUB: MW2 0x10064d60
MechS32 FUN_10064d60(void* p_font, MechS32 p_char)
{
	STUB(0x10064d60);
	return 0;
}

// STUB: MW2 0x10064f0b
void FUN_10064f0b(RenderTarget* p_target, MechS32 p_x, MechS32 p_y, void* p_font, MechChar* p_text, void* p_unk0x14)
{
	STUB(0x10064f0b);
}

// Returns a shape's size: the width in the high word, the height in the low word.
// STUB: MW2 0x10065770
MechS32 FUN_10065770(void* p_shape)
{
	STUB(0x10065770);
	return 0;
}

// Returns a shape frame's size: the width in the high word, the height in the low word.
// STUB: MW2 0x100657a8
MechS32 FUN_100657a8(void* p_shape, MechS32 p_frame)
{
	STUB(0x100657a8);
	return 0;
}

// STUB: MW2 0x10065a7b
MechS32 FUN_10065a7b(RenderTarget* p_dst, RenderTarget* p_src, MechS32 p_unk0x08, MechS32 p_unk0x0c)
{
	STUB(0x10065a7b);
	return 0;
}
