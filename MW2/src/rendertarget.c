/* Hand-written assembly: the font and shape accessors (FUN_10064d4d through GetShapeFrameCount)
   are C functions with __asm bodies. Their short jumps are _emit pairs: the inline assembler
   encodes them as rel32. */
#include "rendertarget.h"

#include "decomp.h"
#include "geocache.h"
#include "object.h"
#include "players.h"
#include "simmain.h"
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

// STUB: MW2 0x1005ef5e
void FUN_1005ef5e(Player* p_player, MechS32 p_step, MechU32 p_flags)
{
	STUB(0x1005ef5e);
}

// Marks the local player's target (bit 0x1000).
// FUNCTION: MW2 0x1005f284
void FUN_1005f284(void)
{
	Player* player;

	player = g_players[g_localPlayerId];
	player->m_targetInfo.m_target |= 0x1000;
}

// STUB: MW2 0x1005fa22
MechS32 FUN_1005fa22(Player* p_player)
{
	STUB(0x1005fa22);
	return 0;
}

// Returns the player the local player targets, or -1.
// The only diff is a stack-slot permutation of index, player and kind.
// FUNCTION: MW2 0x1005fe63
MechS32 FUN_1005fe63(void)
{
	MechS32 index;
	Player* player;
	MechS32 kind;

	player = g_players[g_localPlayerId];
	kind = player->m_targetInfo.m_target & 0xf00;
	index = player->m_targetInfo.m_target & 0xff;
	if (kind != 0x200) {
		index = -1;
	}

	return index;
}

// Returns the game thing the local player targets, or -1.
// The only diff is a stack-slot permutation of index, player and kind.
// FUNCTION: MW2 0x1005febe
MechS32 FUN_1005febe(void)
{
	MechS32 index;
	Player* player;
	MechS32 kind;

	player = g_players[g_localPlayerId];
	kind = player->m_targetInfo.m_target & 0xf00;
	index = player->m_targetInfo.m_target & 0xff;
	if (kind != 0x400) {
		index = -1;
	}

	return index;
}

// Returns the shape of the local player's target, or NULL.
// FUNCTION: MW2 0x1005ff19
ScarletOrchid0x4c* FUN_1005ff19(void)
{
	AmberWillow0x7c* obj;

	obj = FUN_1005ff56();
	if (obj) {
		return FUN_1000154d(obj);
	}
	else {
		return NULL;
	}
}

// Returns the scene object of the local player's target: a player's or a game thing's.
// The only diff is a stack-slot permutation of index, player, obj, kind and id.
// FUNCTION: MW2 0x1005ff56
AmberWillow0x7c* FUN_1005ff56(void)
{
	MechS32 index;
	Player* player;
	AmberWillow0x7c* obj;
	MechS32 kind;
	MechS32 id;

	obj = NULL;
	player = g_players[g_localPlayerId];
	kind = player->m_targetInfo.m_target & 0xf00;
	index = player->m_targetInfo.m_target & 0xff;
	switch (kind) {
	case 0x200:
		obj = g_players[index]->m_obj;
		break;
	case 0x400:
		id = g_gameThings[index].m_unk0x04;
		obj = FUN_10020bdd(id);
		break;
	default:
		break;
	}

	return obj;
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

// Steps p_player's selected target by p_step, with bit 0x100 of the flags set when p_unk0x08.
// FUNCTION: MW2 0x100602b2
void FUN_100602b2(Player* p_player, MechS32 p_step, MechS32 p_unk0x08)
{
	MechU32 flags;

	flags = 1;
	if (p_unk0x08) {
		flags |= 0x100;
	}

	FUN_1005ef5e(p_player, p_step, flags);
}

// FUNCTION: MW2 0x100602ec
void FUN_100602ec(MechS32 p_step)
{
	Player* player;

	player = g_players[g_localPlayerId];
	FUN_1005ef5e(player, p_step, 4);
}

// FUNCTION: MW2 0x1006031b
void FUN_1006031b(MechS32 p_step)
{
	Player* player;

	player = g_players[g_localPlayerId];
	FUN_1005ef5e(player, p_step, 2);
}

// FUNCTION: MW2 0x1006034a
void FUN_1006034a(MechS32 p_step)
{
	Player* player;

	player = g_players[g_localPlayerId];
	FUN_1005ef5e(player, p_step, 0x20008);
}

// FUNCTION: MW2 0x1006037c
void FUN_1006037c(MechS32 p_step)
{
	Player* player;

	player = g_players[g_localPlayerId];
	FUN_1005ef5e(player, p_step, 0x40008);
}

// Sets the pixel at (p_x, p_y) of a render target, relative to its top left.
// STUB: MW2 0x1006053c
MechS32 FUN_1006053c(RenderTarget* p_target, MechS32 p_x, MechS32 p_y, MechU32 p_color)
{
	STUB(0x1006053c);
	return 0;
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

// The font and shape accessors' bodies are __asm blocks. A font has its height at 0x08 and a
// table of glyph offsets (4 bytes each) at 0x10, each glyph starting with its width. A shape
// starts with its frame count at 0x04 and a table of frame offsets (8 bytes each) at 0x08; a
// frame has its bounds at 0x08-0x14.
#pragma warning(disable : 4035) /* no return value: the result is left in eax */
#pragma warning(disable : 4102) /* the labels mark the targets of the _emit short jumps */

// Returns a font's height.
// FUNCTION: MW2 0x10064d4d
MechS32 FUN_10064d4d(void* p_font)
{
	__asm {
		push es
		mov esi, p_font
		mov eax, [esi + 8]
		pop es
	}
}

// Returns the width of glyph p_char of a font.
// FUNCTION: MW2 0x10064d60
MechS32 FUN_10064d60(void* p_font, MechS32 p_char)
{
	__asm {
		push es
		mov eax, p_char
		shl eax, 2
		add eax, p_font
		add eax, 0x10
		mov esi, [eax]
		add esi, p_font
		mov eax, [esi]
		pop es
	}
}

// STUB: MW2 0x10064d80
MechS32 FUN_10064d80(RenderTarget* p_target, MechS32 p_x, MechS32 p_y, void* p_font, MechS32 p_char, void* p_unk0x14)
{
	STUB(0x10064d80);
	return 0;
}

// Draws the non-empty string p_text, each glyph through FUN_10064d80, which returns its width.
// FUNCTION: MW2 0x10064f0b
void FUN_10064f0b(RenderTarget* p_target, MechS32 p_x, MechS32 p_y, void* p_font, MechChar* p_text, void* p_unk0x14)
{
	__asm {
		push es
		cld
		push ds
		pop es
		mov esi, p_text
		mov edi, p_x
jmp_10064f1b:
		movzx eax, byte ptr [esi]
		push p_unk0x14
		push eax
		push p_font
		push p_y
		push edi
		push p_target
		call FUN_10064d80
		add esp, 0x18
		add edi, eax
		inc esi
		cmp byte ptr [esi], 0
		_emit 0x75 /* jne jmp_10064f1b */
		_emit 0xdf
		pop es
	}
}

// Returns a shape's size from its header: the width in the high word, the height in the low
// word. The header is followed by a palette of 2^(n+1) entries when bit 7 of its byte 0x0a is set.
// FUNCTION: MW2 0x10065770
MechS32 FUN_10065770(void* p_shape)
{
	__asm {
		push es
		mov esi, p_shape
		mov al, [esi + 0xa]
		mov cl, al
		and cl, 7
		inc cl
		mov ebx, 1
		shl ebx, cl
		add esi, 0xd
		test al, 0x80
		_emit 0x74 /* je jmp_10065797 */
		_emit 0x05
		imul ebx, ebx, 3
		add esi, ebx
jmp_10065797:
		mov ax, [esi + 5]
		shl eax, 16
		mov ax, [esi + 7]
		pop es
	}
}

// Returns a shape frame's size, stored at its start: the width in the high word, the height in
// the low word.
// FUNCTION: MW2 0x100657a8
MechS32 FUN_100657a8(void* p_shape, MechS32 p_frame)
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
		mov eax, [esi]
		pop es
	}
}

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
