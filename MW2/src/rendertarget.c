#include "rendertarget.h"

#include "decomp.h"
#include "players.h"
#include "types.h"

DECOMP_SIZE_ASSERT(PixelBuffer, 0x14)
DECOMP_SIZE_ASSERT(RenderTarget, 0x14)
DECOMP_SIZE_ASSERT(NavPoint, 0x54)

// GLOBAL: MW2 0x100aaba4
MechS32 g_navCount = 0;

// GLOBAL: MW2 0x10177160
NavPoint g_navTable[128];

// STUB: MW2 0x1005ec80
MechS32 FUN_1005ec80(MechU32 p_owner, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	STUB(0x1005ec80);
	return 0;
}

// STUB: MW2 0x1005ed4f
void FUN_1005ed4f(MechU32 p_owner, MechU32 p_nav)
{
	STUB(0x1005ed4f);
}

// STUB: MW2 0x1005fa22
MechS32 FUN_1005fa22(Player* p_player)
{
	STUB(0x1005fa22);
	return 0;
}

// STUB: MW2 0x10060197
void FUN_10060197(
	MechS32 p_dx,
	MechS32 p_dy,
	MechS32 p_dz,
	MechS32* p_unk0x0c,
	MechS32* p_unk0x10,
	MechU32* p_distance,
	MechS32* p_unk0x18
)
{
	STUB(0x10060197);
}

// STUB: MW2 0x100602b2
void FUN_100602b2(Player* p_player, MechS32 p_unk0x04, MechS32 p_unk0x08)
{
	STUB(0x100602b2);
}

// Returns the pixel at (p_x, p_y) of a render target, relative to its top left, or a negative
// value outside it.
// STUB: MW2 0x10060617
MechS32 FUN_10060617(RenderTarget* p_target, MechS32 p_x, MechS32 p_y)
{
	STUB(0x10060617);
	return 0;
}

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

// The shape accessors' bodies are __asm blocks. A shape starts with its frame count at 0x04 and a
// table of frame offsets (8 bytes each) at 0x08; a frame has its bounds at 0x08-0x14.
#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// Returns the value at 0x04 of a shape frame.
// FUNCTION: MW2 0x100657ca
MechS32 FUN_100657ca(void* p_shape, MechS32 p_frame)
{
	__asm {
		push es
		mov esi, p_shape
		add esi, 8
		mov eax, p_frame
		shl eax, 3
		add esi, eax
		mov esi, [esi]
		add esi, p_shape
		mov eax, [esi + 4]
		pop es
	}
}

// Returns a shape frame's size from its bounds: the width in the high word, the height in the
// low word.
// FUNCTION: MW2 0x100657ed
MechS32 FUN_100657ed(void* p_shape, MechS32 p_frame)
{
	__asm {
		push es
		mov esi, p_shape
		add esi, 8
		mov eax, p_frame
		shl eax, 3
		add esi, eax
		mov esi, [esi]
		add esi, p_shape
		mov eax, [esi + 0x10]
		sub eax, [esi + 8]
		inc eax
		mov ebx, [esi + 0x14]
		sub ebx, [esi + 0xc]
		inc ebx
		shl eax, 16
		mov ax, bx
		pop es
	}
}

// Returns a shape frame's origin: x (0x08) in the high word, y (0x0c) in the low word.
// FUNCTION: MW2 0x10065821
MechS32 FUN_10065821(void* p_shape, MechS32 p_frame)
{
	__asm {
		push es
		mov esi, p_shape
		add esi, 8
		mov eax, p_frame
		shl eax, 3
		add esi, eax
		mov esi, [esi]
		add esi, p_shape
		mov eax, [esi + 8]
		shl eax, 16
		mov ax, [esi + 0xc]
		pop es
	}
}

// Returns a shape's frame count.

// FUNCTION: MW2 0x10065928
MechS32 GetShapeFrameCount(void* p_shape)
{
	__asm {
		push es
		mov esi, p_shape
		mov eax, [esi + 4]
		pop es
	}
}

#pragma warning(default : 4035)

// STUB: MW2 0x10065a7b
MechS32 FUN_10065a7b(RenderTarget* p_dst, RenderTarget* p_src, MechS32 p_unk0x08, MechS32 p_unk0x0c)
{
	STUB(0x10065a7b);
	return 0;
}
