/* Hand-written assembly: the routines from GetDisplayDriverName (0x100604f4) to CountViewColors
   (0x10065e76) are MW2's copy of the MASM object MW2SHELL's blit.c transcribes, routine for
   routine; they take the shell's names. Those with the /Od frame and no locals are C functions
   with __asm bodies (FUN_10064d4d through GetShapeFrameCount, FindIffChunk...); the rest (a
   MASM frame with locals, several exits, registers for arguments, or a `push offset`, which
   libclang can't parse) are __declspec(naked), with STUB() bodies in COMPAT_MODE. Their short
   jumps are _emit pairs: the inline assembler encodes them as rel32. The IFF chunk tags sit in
   the code (0x10065045) and are defined here as globals. */
#include "rendertarget.h"

#include "compat.h"
#include "decomp.h"
#include "fixeddiv29.h"
#include "geocache.h"
#include "object.h"
#include "players.h"
#include "simmain.h"
#include "types.h"
#include "unk100696c0.h"

#include <stdlib.h>

DECOMP_SIZE_ASSERT(PixelBuffer, 0x14)
DECOMP_SIZE_ASSERT(RenderTarget, 0x14)
DECOMP_SIZE_ASSERT(NavPoint, 0x54)

// GLOBAL: MW2 0x100aaba4
MechS32 g_navCount = 0;

// GLOBAL: MW2 0x10177160
NavPoint g_navTable[128];

// Set by SetDisplayDriver.
// GLOBAL: MW2 0x100ab100
MechU32 g_displayDriver[0xd] = {0};

// The name GetDisplayDriverName returns.
// GLOBAL: MW2 0x100ab134
MechChar g_displayDriverName[0xd] = "MCGA.DLL";

// The image decoders' row buffer (BlitPicture, GifPutPixel) and bit-plane buffer (BlitIff).
// GLOBAL: MW2 0x100ab679
MechU8 g_scanline[0x300] = {0};

// GLOBAL: MW2 0x100ac379
MechU8 g_unk0x100ac379[0x300] = {0};

// The GIF decoder's tables: the masks of 0 to 8 low bits, and the interlaced passes' row steps and
// first rows.
// GLOBAL: MW2 0x100ac979
MechU8 g_gifCodeMasks[9] = {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};

// GLOBAL: MW2 0x100ac982
MechU8 g_gifPassSteps[5] = {8, 8, 4, 2, 0};

// GLOBAL: MW2 0x100ac987
MechU8 g_gifPassStarts[5] = {0, 4, 2, 1, 0};

// The render target BlitGif draws into.
// GLOBAL: MW2 0x100ac98c
RenderTarget* g_gifView = NULL;

// A colour map SetRemapTable sets and the shape drawers translate through.
// GLOBAL: MW2 0x100ac990
MechU8 g_remapTable[0x100] = {0};

// Places a nav point for player p_owner at (p_x, p_y, p_z), named "!". Returns its index, or
// -1 if the table is full.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1005ec80
MechS32 FUN_1005ec80(MechU32 p_owner, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	MechS32 index;
	NavPoint* nav;

	index = -1;
	if (g_navCount < 0x80 && g_navCount != -1) {
		nav = &g_navTable[g_navCount];
		nav->m_position[0] = p_x;
		nav->m_position[1] = p_y;
		nav->m_position[2] = p_z;
		nav->m_unk0x00 = 1;
		nav->m_flags = 0x401;
		nav->m_owner = p_owner | 0x200;
		nav->m_team = g_players[p_owner]->m_team;
		nav->m_radius = 3000;
		nav->m_obj = NULL;
		nav->m_name[0] = '!';
		nav->m_name[1] = '\0';
		index = g_navCount;
		g_navCount++;
	}

	return index;
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

// Turns the vector (p_dx, p_dy, p_dz) into its heading (*p_unk0x0c), its length along the
// ground (*p_distance), its full length (*p_unk0x10) and its pitch (*p_unk0x18).
// The abs(p_dx)/abs(p_dz) comparison evaluates its operands in the opposite order (one attempt at
// swapping them didn't flip it), and stack-slot permutation: every local.
// FUNCTION: MW2 0x10060197
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
	MechS32 pitch;
	MechS32 cosine;
	MechS32 ground;
	MechS32 heading;
	MechS32 sine;
	MechS32 length;
	MechS32 pitchCosine;

	heading = FUN_100698de(p_dx, p_dz);
	if (abs(p_dx) > abs(p_dz)) {
		sine = FUN_100696c0(heading);
		if (sine) {
			ground = FixedDiv29(p_dx, sine);
		}
		else {
			ground = 0;
		}
	}
	else {
		cosine = FUN_1006973a(heading);
		if (cosine) {
			ground = FixedDiv29(p_dz, cosine);
		}
		else {
			ground = 0;
		}
	}

	pitch = FUN_100698de(p_dy, ground);
	pitchCosine = FUN_1006973a(pitch);
	if (pitchCosine) {
		length = FixedDiv29(ground, pitchCosine);
	}
	else {
		length = 0;
	}

	*p_unk0x0c = heading;
	*p_distance = ground;
	*p_unk0x10 = length;
	*p_unk0x18 = pitch;
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

// Selects the local player's nearest target within 0x2ab98, cycling through them all; keeps the
// current one if there is none or FUN_1005fa22 refuses it, and then clears the autopilot's
// steering flag (PlayerSteering::m_unk0x42).
// The distance/bestDistance comparison loads its operands in the opposite order (one attempt at
// swapping them didn't flip it), and stack-slot permutation: every local.
// FUNCTION: MW2 0x100603ae
void FUN_100603ae(void)
{
	MechS32 autopilot;
	MechS32 target;
	Player* player;
	MechS32 best;
	MechS32 saved;
	MechS32 distance;
	MechS32 bestDistance;

	target = 0;
	best = -1;
	bestDistance = 0x7fffffff;
	autopilot = FALSE;
	player = g_players[g_localPlayerId];
	saved = player->m_targetInfo.m_target;
	if (player->m_mech->m_unk0xbc == 1) {
		autopilot = TRUE;
	}

	FUN_1006037c(0);
	while (best != target) {
		target = player->m_targetInfo.m_target;
		if (target & 0x1000) {
			break;
		}

		distance = player->m_targetInfo.m_unk0x04;
		if (distance < bestDistance) {
			best = target;
			bestDistance = distance;
			target = 0;
		}

		FUN_1006037c(1);
	}

	if (best == -1 || bestDistance > 0x2ab98) {
		player->m_targetInfo.m_target = saved;
		if (autopilot) {
			player->m_steering->m_unk0x42 = 0;
		}
	}
	else {
		player->m_targetInfo.m_target = best;
		if (!FUN_1005fa22(player)) {
			player->m_targetInfo.m_target = saved;
			if (autopilot) {
				player->m_steering->m_unk0x42 = 0;
			}
		}
	}
}

// Copies the name the first function of p_driver returns to g_displayDriverName and returns it.
// FUNCTION: MW2 0x100604f4
MechChar* GetDisplayDriverName(MechChar* (**p_driver)(void) )
{
	__asm {
		push es
		cld
		push ds
		pop es
		mov esi, p_driver
		call dword ptr [esi]
		mov edi, offset g_displayDriverName
jmp_10060508:
		mov bl, byte ptr [eax]
		mov byte ptr [edi], bl
		inc edi
		inc eax
		or bl, bl
		_emit 0x75 /* jne jmp_10060508 */
		_emit 0xf6
		mov eax, offset g_displayDriverName
		pop es
	}
}

// Copies 0xd dwords from p_src to g_displayDriver.
// FUNCTION: MW2 0x1006051d
void SetDisplayDriver(MechU32* p_src)
{
	__asm {
		push es
		cld
		push ds
		pop es
		mov esi, p_src
		mov edi, offset g_displayDriver
		mov ecx, 0xd
		rep movsd
		pop es
	}
}

// Sets the pixel at (p_x, p_y) of a render target, relative to its top left.
#ifdef COMPAT_MODE
MechS32 PutViewPixel(RenderTarget* p_target, MechS32 p_x, MechS32 p_y, MechU32 p_color)
{
	STUB(0x1006053c);
	return 0;
}
#else
// FUNCTION: MW2 0x1006053c
__declspec(naked) MechS32 PutViewPixel(RenderTarget* p_target, MechS32 p_x, MechS32 p_y, MechU32 p_color)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x20
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp + 8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx + 4]
		inc eax
		mov dword ptr [ebp - 0x18], eax
		_emit 0x7e /* jle jmp_100605bb */
		_emit 0x64
		mov eax, dword ptr [ebx + 8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_100605bb */
		_emit 0x5c
		mov eax, dword ptr [esi + 4]
		mov dword ptr [ebp - 0x1c], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_1006056f */
		_emit 0x05
		mov eax, 0
jmp_1006056f:
		mov dword ptr [ebp - 4], eax
		mov eax, dword ptr [esi + 8]
		mov dword ptr [ebp - 0x20], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_10060582 */
		_emit 0x05
		mov eax, 0
jmp_10060582:
		mov dword ptr [ebp - 8], eax
		mov eax, dword ptr [esi + 0xc]
		mov edx, dword ptr [ebp - 0x18]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10060592 */
		_emit 0x02
		mov eax, edx
jmp_10060592:
		mov dword ptr [ebp - 0xc], eax
		mov eax, dword ptr [esi + 0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_100605a1 */
		_emit 0x02
		mov eax, edx
jmp_100605a1:
		mov dword ptr [ebp - 0x10], eax
		mov eax, dword ptr [ebp - 0xc]
		cmp eax, dword ptr [ebp - 4]
		_emit 0x7c /* jl jmp_100605c6 */
		_emit 0x1a
		mov eax, dword ptr [ebp - 0x10]
		cmp eax, dword ptr [ebp - 8]
		_emit 0x7c /* jl jmp_100605c6 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp - 0x14], eax
		jmp short jmp_100605d1
jmp_100605bb:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_100605c6:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_100605d1:
		mov ecx, dword ptr [ebp + 0xc]
		mov ebx, dword ptr [ebp + 0x10]
		add ecx, dword ptr [ebp - 0x1c]
		add ebx, dword ptr [ebp - 0x20]
		cmp ecx, dword ptr [ebp - 4]
		_emit 0x7c /* jl jmp_1006060c */
		_emit 0x2a
		cmp ecx, dword ptr [ebp - 0xc]
		_emit 0x7f /* jg jmp_1006060c */
		_emit 0x25
		cmp ebx, dword ptr [ebp - 8]
		_emit 0x7c /* jl jmp_1006060c */
		_emit 0x20
		cmp ebx, dword ptr [ebp - 0x10]
		_emit 0x7f /* jg jmp_1006060c */
		_emit 0x1b
		mov eax, ebx
		imul dword ptr [ebp - 0x18]
		add eax, dword ptr [ebp - 0x14]
		add eax, ecx
		mov ebx, eax
		xor eax, eax
		mov al, byte ptr [ebx]
		mov dl, byte ptr [ebp + 0x14]
		mov byte ptr [ebx], dl
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1006060c:
		mov eax, 0xfffffffd
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Returns the pixel at (p_x, p_y) of a render target, relative to its top left, or a negative
// value outside it.
#ifdef COMPAT_MODE
MechS32 GetViewPixel(RenderTarget* p_target, MechS32 p_x, MechS32 p_y)
{
	STUB(0x10060617);
	return 0;
}
#else
// FUNCTION: MW2 0x10060617
__declspec(naked) MechS32 GetViewPixel(RenderTarget* p_target, MechS32 p_x, MechS32 p_y)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x20
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp + 8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx + 4]
		inc eax
		mov dword ptr [ebp - 0x18], eax
		_emit 0x7e /* jle jmp_10060696 */
		_emit 0x64
		mov eax, dword ptr [ebx + 8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10060696 */
		_emit 0x5c
		mov eax, dword ptr [esi + 4]
		mov dword ptr [ebp - 0x1c], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_1006064a */
		_emit 0x05
		mov eax, 0
jmp_1006064a:
		mov dword ptr [ebp - 4], eax
		mov eax, dword ptr [esi + 8]
		mov dword ptr [ebp - 0x20], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_1006065d */
		_emit 0x05
		mov eax, 0
jmp_1006065d:
		mov dword ptr [ebp - 8], eax
		mov eax, dword ptr [esi + 0xc]
		mov edx, dword ptr [ebp - 0x18]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_1006066d */
		_emit 0x02
		mov eax, edx
jmp_1006066d:
		mov dword ptr [ebp - 0xc], eax
		mov eax, dword ptr [esi + 0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_1006067c */
		_emit 0x02
		mov eax, edx
jmp_1006067c:
		mov dword ptr [ebp - 0x10], eax
		mov eax, dword ptr [ebp - 0xc]
		cmp eax, dword ptr [ebp - 4]
		_emit 0x7c /* jl jmp_100606a1 */
		_emit 0x1a
		mov eax, dword ptr [ebp - 0x10]
		cmp eax, dword ptr [ebp - 8]
		_emit 0x7c /* jl jmp_100606a1 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp - 0x14], eax
		jmp short jmp_100606ac
jmp_10060696:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_100606a1:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_100606ac:
		mov ecx, dword ptr [ebp + 0xc]
		mov ebx, dword ptr [ebp + 0x10]
		add ecx, dword ptr [ebp - 0x1c]
		add ebx, dword ptr [ebp - 0x20]
		cmp ecx, dword ptr [ebp - 4]
		_emit 0x7c /* jl jmp_100606e2 */
		_emit 0x25
		cmp ecx, dword ptr [ebp - 0xc]
		_emit 0x7f /* jg jmp_100606e2 */
		_emit 0x20
		cmp ebx, dword ptr [ebp - 8]
		_emit 0x7c /* jl jmp_100606e2 */
		_emit 0x1b
		cmp ebx, dword ptr [ebp - 0x10]
		_emit 0x7f /* jg jmp_100606e2 */
		_emit 0x16
		mov eax, ebx
		imul dword ptr [ebp - 0x18]
		add eax, dword ptr [ebp - 0x14]
		add eax, ecx
		mov ebx, eax
		xor eax, eax
		mov al, byte ptr [ebx]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_100606e2:
		mov eax, 0xfffffffd
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

#ifdef COMPAT_MODE
MechS32 BlitLine(
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
#else
// FUNCTION: MW2 0x100606ed
__declspec(naked) MechS32 BlitLine(
	RenderTarget* p_target,
	MechS32 p_x1,
	MechS32 p_y1,
	MechS32 p_x2,
	MechS32 p_y2,
	MechS32 p_mode,
	MechS32 p_color
)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x58
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp + 8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx + 4]
		inc eax
		mov dword ptr [ebp - 0x50], eax
		_emit 0x7e /* jle jmp_1006076c */
		_emit 0x64
		mov eax, dword ptr [ebx + 8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_1006076c */
		_emit 0x5c
		mov eax, dword ptr [esi + 4]
		mov dword ptr [ebp - 0x54], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_10060720 */
		_emit 0x05
		mov eax, 0
jmp_10060720:
		mov dword ptr [ebp - 0x3c], eax
		mov eax, dword ptr [esi + 8]
		mov dword ptr [ebp - 0x58], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_10060733 */
		_emit 0x05
		mov eax, 0
jmp_10060733:
		mov dword ptr [ebp - 0x40], eax
		mov eax, dword ptr [esi + 0xc]
		mov edx, dword ptr [ebp - 0x50]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10060743 */
		_emit 0x02
		mov eax, edx
jmp_10060743:
		mov dword ptr [ebp - 0x44], eax
		mov eax, dword ptr [esi + 0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10060752 */
		_emit 0x02
		mov eax, edx
jmp_10060752:
		mov dword ptr [ebp - 0x48], eax
		mov eax, dword ptr [ebp - 0x44]
		cmp eax, dword ptr [ebp - 0x3c]
		_emit 0x7c /* jl jmp_10060777 */
		_emit 0x1a
		mov eax, dword ptr [ebp - 0x48]
		cmp eax, dword ptr [ebp - 0x40]
		_emit 0x7c /* jl jmp_10060777 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp - 0x4c], eax
		jmp short jmp_10060782
jmp_1006076c:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10060777:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10060782:
		mov eax, dword ptr [ebp - 0x54]
		add dword ptr [ebp + 0xc], eax
		add dword ptr [ebp + 0x14], eax
		mov eax, dword ptr [ebp - 0x58]
		add dword ptr [ebp + 0x10], eax
		add dword ptr [ebp + 0x18], eax
		mov eax, dword ptr [ebp + 0x14]
		sub eax, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 4], eax
		cdq
		mov dword ptr [ebp - 0xc], edx
		xor eax, edx
		sub eax, edx
		mov dword ptr [ebp - 8], eax
		mov eax, dword ptr [ebp + 0x18]
		sub eax, dword ptr [ebp + 0x10]
		mov dword ptr [ebp - 0x10], eax
		cdq
		mov dword ptr [ebp - 0x18], edx
		xor eax, edx
		sub eax, edx
		mov dword ptr [ebp - 0x14], eax
		mov eax, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 0x24], eax
		mov eax, dword ptr [ebp + 0x14]
		mov dword ptr [ebp - 0x2c], eax
		mov eax, dword ptr [ebp + 0x10]
		mov dword ptr [ebp - 0x28], eax
		mov eax, dword ptr [ebp + 0x18]
		mov dword ptr [ebp - 0x30], eax
		cmp dword ptr [ebp - 4], 0
		je jmp_10060f03
		cmp dword ptr [ebp - 0x10], 0
		je jmp_10060f8e
		mov eax, dword ptr [ebp - 0xc]
		xor eax, dword ptr [ebp - 0x18]
		mov dword ptr [ebp - 0x1c], eax
		mov edx, dword ptr [ebp - 8]
		mov ebx, dword ptr [ebp - 0x14]
		mov eax, 0xffffffff
		cmp edx, ebx
		_emit 0x74 /* je jmp_10060808 */
		_emit 0x08
		_emit 0x7c /* jl jmp_10060804 */
		_emit 0x02
		_emit 0x87 /* xchg ebx, edx */
		_emit 0xd3
jmp_10060804:
		xor eax, eax
		div ebx
jmp_10060808:
		mov dword ptr [ebp - 0x20], eax
		mov dword ptr [ebp - 0x34], 0
jmp_10060812:
		xor edx, edx
		mov eax, dword ptr [ebp - 0x24]
		sub eax, dword ptr [ebp - 0x3c]
		shl eax, 1
		adc dl, dl
		mov eax, dword ptr [ebp - 0x44]
		sub eax, dword ptr [ebp - 0x24]
		shl eax, 1
		adc dl, dl
		mov eax, dword ptr [ebp - 0x28]
		sub eax, dword ptr [ebp - 0x40]
		shl eax, 1
		adc dl, dl
		mov eax, dword ptr [ebp - 0x48]
		sub eax, dword ptr [ebp - 0x28]
		shl eax, 1
		adc dl, dl
		mov eax, dword ptr [ebp - 0x2c]
		sub eax, dword ptr [ebp - 0x3c]
		shl eax, 1
		adc dh, dh
		mov eax, dword ptr [ebp - 0x44]
		sub eax, dword ptr [ebp - 0x2c]
		shl eax, 1
		adc dh, dh
		mov eax, dword ptr [ebp - 0x30]
		sub eax, dword ptr [ebp - 0x40]
		shl eax, 1
		adc dh, dh
		mov eax, dword ptr [ebp - 0x48]
		sub eax, dword ptr [ebp - 0x30]
		shl eax, 1
		adc dh, dh
		or dword ptr [ebp - 0x34], edx
		or edx, edx
		je jmp_10060bd1
		_emit 0x84 /* test dh, dl */
		_emit 0xd6
		jne jmp_100610e4
		mov ebx, dword ptr [ebp - 8]
		cmp ebx, dword ptr [ebp - 0x14]
		_emit 0x7c /* jl jmp_100608cc */
		_emit 0x4d
		test dl, 8
		jne jmp_10060915
		test dl, 4
		jne jmp_10060969
		test dl, 2
		jne jmp_100609ed
		test dl, 1
		jne jmp_10060a45
		test dh, 8
		jne jmp_10060a75
		test dh, 4
		jne jmp_10060ad0
		test dh, 2
		jne jmp_10060b4f
		test dh, 1
		jne jmp_10060ba6
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0x46
		_emit 0xff
		_emit 0xff
		_emit 0xff
jmp_100608cc:
		test dl, 8
		_emit 0x75 /* jne jmp_1006093d */
		_emit 0x6c
		test dl, 4
		jne jmp_10060995
		test dl, 2
		jne jmp_100609c5
		test dl, 1
		jne jmp_10060a19
		test dh, 8
		jne jmp_10060aa1
		test dh, 4
		jne jmp_10060af8
		test dh, 2
		jne jmp_10060b23
		test dh, 1
		jne jmp_10060b7e
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0xfd
		_emit 0xfe
		_emit 0xff
		_emit 0xff
jmp_10060915:
		mov eax, dword ptr [ebp - 0x3c]
		mov dword ptr [ebp - 0x24], eax
		sub eax, dword ptr [ebp + 0xc]
		mul dword ptr [ebp - 0x20]
		add eax, 0x80000000
		adc edx, 0
		mov eax, edx
		mov edx, dword ptr [ebp - 0x1c]
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp + 0x10]
		mov dword ptr [ebp - 0x28], eax
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0xd5
		_emit 0xfe
		_emit 0xff
		_emit 0xff
jmp_1006093d:
		mov eax, dword ptr [ebp - 0x3c]
		mov dword ptr [ebp - 0x24], eax
		sub eax, dword ptr [ebp + 0xc]
		mov edx, eax
		dec edx
		mov eax, 0x80000000
		div dword ptr [ebp - 0x20]
		cmp edx, 1
		sbb eax, -1
		mov edx, dword ptr [ebp - 0x1c]
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp + 0x10]
		mov dword ptr [ebp - 0x28], eax
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0xa9
		_emit 0xfe
		_emit 0xff
		_emit 0xff
jmp_10060969:
		mov eax, dword ptr [ebp - 0x44]
		mov dword ptr [ebp - 0x24], eax
		sub eax, dword ptr [ebp + 0xc]
		neg eax
		mul dword ptr [ebp - 0x20]
		add eax, 0x80000000
		adc edx, 0
		mov eax, edx
		mov edx, dword ptr [ebp - 0x1c]
		not edx
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp + 0x10]
		mov dword ptr [ebp - 0x28], eax
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0x7d
		_emit 0xfe
		_emit 0xff
		_emit 0xff
jmp_10060995:
		mov eax, dword ptr [ebp - 0x44]
		mov dword ptr [ebp - 0x24], eax
		sub eax, dword ptr [ebp + 0xc]
		neg eax
		mov edx, eax
		dec edx
		mov eax, 0x80000000
		div dword ptr [ebp - 0x20]
		cmp edx, 1
		sbb eax, -1
		mov edx, dword ptr [ebp - 0x1c]
		not edx
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp + 0x10]
		mov dword ptr [ebp - 0x28], eax
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0x4d
		_emit 0xfe
		_emit 0xff
		_emit 0xff
jmp_100609c5:
		mov eax, dword ptr [ebp - 0x40]
		mov dword ptr [ebp - 0x28], eax
		sub eax, dword ptr [ebp + 0x10]
		mul dword ptr [ebp - 0x20]
		add eax, 0x80000000
		adc edx, 0
		mov eax, edx
		mov edx, dword ptr [ebp - 0x1c]
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 0x24], eax
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0x25
		_emit 0xfe
		_emit 0xff
		_emit 0xff
jmp_100609ed:
		mov eax, dword ptr [ebp - 0x40]
		mov dword ptr [ebp - 0x28], eax
		sub eax, dword ptr [ebp + 0x10]
		mov edx, eax
		dec edx
		mov eax, 0x80000000
		div dword ptr [ebp - 0x20]
		cmp edx, 1
		sbb eax, -1
		mov edx, dword ptr [ebp - 0x1c]
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 0x24], eax
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0xf9
		_emit 0xfd
		_emit 0xff
		_emit 0xff
jmp_10060a19:
		mov eax, dword ptr [ebp - 0x48]
		mov dword ptr [ebp - 0x28], eax
		sub eax, dword ptr [ebp + 0x10]
		neg eax
		mul dword ptr [ebp - 0x20]
		add eax, 0x80000000
		adc edx, 0
		mov eax, edx
		mov edx, dword ptr [ebp - 0x1c]
		not edx
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 0x24], eax
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0xcd
		_emit 0xfd
		_emit 0xff
		_emit 0xff
jmp_10060a45:
		mov eax, dword ptr [ebp - 0x48]
		mov dword ptr [ebp - 0x28], eax
		sub eax, dword ptr [ebp + 0x10]
		neg eax
		mov edx, eax
		dec edx
		mov eax, 0x80000000
		div dword ptr [ebp - 0x20]
		cmp edx, 1
		sbb eax, -1
		mov edx, dword ptr [ebp - 0x1c]
		not edx
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 0x24], eax
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0x9d
		_emit 0xfd
		_emit 0xff
		_emit 0xff
jmp_10060a75:
		mov eax, dword ptr [ebp - 0x3c]
		mov dword ptr [ebp - 0x2c], eax
		sub eax, dword ptr [ebp + 0xc]
		neg eax
		mul dword ptr [ebp - 0x20]
		add eax, 0x80000000
		adc edx, 0
		mov eax, edx
		mov edx, dword ptr [ebp - 0x1c]
		not edx
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp + 0x10]
		mov dword ptr [ebp - 0x30], eax
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0x71
		_emit 0xfd
		_emit 0xff
		_emit 0xff
jmp_10060aa1:
		mov eax, dword ptr [ebp - 0x3c]
		mov dword ptr [ebp - 0x2c], eax
		sub eax, dword ptr [ebp + 0xc]
		neg eax
		mov edx, eax
		mov eax, 0x80000000
		div dword ptr [ebp - 0x20]
		cmp edx, 1
		sbb eax, 0
		mov edx, dword ptr [ebp - 0x1c]
		not edx
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp + 0x10]
		mov dword ptr [ebp - 0x30], eax
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0x42
		_emit 0xfd
		_emit 0xff
		_emit 0xff
jmp_10060ad0:
		mov eax, dword ptr [ebp - 0x44]
		mov dword ptr [ebp - 0x2c], eax
		sub eax, dword ptr [ebp + 0xc]
		mul dword ptr [ebp - 0x20]
		add eax, 0x80000000
		adc edx, 0
		mov eax, edx
		mov edx, dword ptr [ebp - 0x1c]
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp + 0x10]
		mov dword ptr [ebp - 0x30], eax
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0x1a
		_emit 0xfd
		_emit 0xff
		_emit 0xff
jmp_10060af8:
		mov eax, dword ptr [ebp - 0x44]
		mov dword ptr [ebp - 0x2c], eax
		sub eax, dword ptr [ebp + 0xc]
		mov edx, eax
		mov eax, 0x80000000
		div dword ptr [ebp - 0x20]
		cmp edx, 1
		sbb eax, 0
		mov edx, dword ptr [ebp - 0x1c]
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp + 0x10]
		mov dword ptr [ebp - 0x30], eax
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0xef
		_emit 0xfc
		_emit 0xff
		_emit 0xff
jmp_10060b23:
		mov eax, dword ptr [ebp - 0x40]
		mov dword ptr [ebp - 0x30], eax
		sub eax, dword ptr [ebp + 0x10]
		neg eax
		mul dword ptr [ebp - 0x20]
		add eax, 0x80000000
		adc edx, 0
		mov eax, edx
		mov edx, dword ptr [ebp - 0x1c]
		not edx
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 0x2c], eax
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0xc3
		_emit 0xfc
		_emit 0xff
		_emit 0xff
jmp_10060b4f:
		mov eax, dword ptr [ebp - 0x40]
		mov dword ptr [ebp - 0x30], eax
		sub eax, dword ptr [ebp + 0x10]
		neg eax
		mov edx, eax
		mov eax, 0x80000000
		div dword ptr [ebp - 0x20]
		cmp edx, 1
		sbb eax, 0
		mov edx, dword ptr [ebp - 0x1c]
		not edx
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 0x2c], eax
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0x94
		_emit 0xfc
		_emit 0xff
		_emit 0xff
jmp_10060b7e:
		mov eax, dword ptr [ebp - 0x48]
		mov dword ptr [ebp - 0x30], eax
		sub eax, dword ptr [ebp + 0x10]
		mul dword ptr [ebp - 0x20]
		add eax, 0x80000000
		adc edx, 0
		mov eax, edx
		mov edx, dword ptr [ebp - 0x1c]
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 0x2c], eax
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0x6c
		_emit 0xfc
		_emit 0xff
		_emit 0xff
jmp_10060ba6:
		mov eax, dword ptr [ebp - 0x48]
		mov dword ptr [ebp - 0x30], eax
		sub eax, dword ptr [ebp + 0x10]
		mov edx, eax
		mov eax, 0x80000000
		div dword ptr [ebp - 0x20]
		cmp edx, 1
		sbb eax, 0
		mov edx, dword ptr [ebp - 0x1c]
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 0x2c], eax
		_emit 0xe9 /* jmp jmp_10060812 */
		_emit 0x41
		_emit 0xfc
		_emit 0xff
		_emit 0xff
jmp_10060bd1:
		mov eax, dword ptr [ebp - 0x28]
		imul dword ptr [ebp - 0x50]
		add eax, dword ptr [ebp - 0x4c]
		add eax, dword ptr [ebp - 0x24]
		mov edi, eax
		mov esi, dword ptr [ebp - 0x50]
		xor esi, dword ptr [ebp - 0x18]
		sub esi, dword ptr [ebp - 0x18]
		mov ebx, dword ptr [ebp - 0x20]
		mov eax, dword ptr [ebp - 8]
		cmp eax, dword ptr [ebp - 0x14]
		je jmp_10061014
		jg jmp_10060d65
		mov eax, dword ptr [ebp - 0x30]
		sub eax, dword ptr [ebp - 0x28]
		cdq
		xor eax, edx
		sub eax, edx
		inc eax
		mov ecx, eax
		mov eax, dword ptr [ebp - 0x28]
		sub eax, dword ptr [ebp + 0x10]
		cdq
		xor eax, edx
		sub eax, edx
		mul ebx
		add eax, 0x80000000
		mov edx, eax
		cmp dword ptr [ebp - 0xc], -1
		je jmp_10060cc6
		cmp dword ptr [ebp + 0x1c], 1
		_emit 0x74 /* je jmp_10060c5d */
		_emit 0x2e
		_emit 0x7f /* jg jmp_10060c9b */
		_emit 0x6a
		mov eax, dword ptr [ebp + 0x20]
jmp_10060c34:
		mov byte ptr [edi], al
		add edx, ebx
		adc edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10060c58 */
		_emit 0x1b
		mov byte ptr [edi], al
		add edx, ebx
		adc edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10060c58 */
		_emit 0x12
		mov byte ptr [edi], al
		add edx, ebx
		adc edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10060c58 */
		_emit 0x09
		mov byte ptr [edi], al
		add edx, ebx
		adc edi, esi
		dec ecx
		_emit 0x75 /* jne jmp_10060c34 */
		_emit 0xdc
jmp_10060c58:
		jmp jmp_100610d5
jmp_10060c5d:
		mov eax, dword ptr [ebp + 0x20]
		push ebp
		mov ebp, ebx
		mov ebx, eax
jmp_10060c65:
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edx, ebp
		adc edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10060c95 */
		_emit 0x24
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edx, ebp
		adc edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10060c95 */
		_emit 0x18
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edx, ebp
		adc edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10060c95 */
		_emit 0x0c
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edx, ebp
		adc edi, esi
		dec ecx
		_emit 0x75 /* jne jmp_10060c65 */
		_emit 0xd0
jmp_10060c95:
		pop ebp
		jmp jmp_100610d5
jmp_10060c9b:
		mov esi, dword ptr [ebp + 8]
		mov edi, dword ptr [ebp - 0x24]
		sub edi, dword ptr [esi + 4]
		mov eax, dword ptr [ebp - 0x28]
		sub eax, dword ptr [esi + 8]
		mov esi, eax
		mov eax, dword ptr [ebp - 0x18]
		add eax, eax
		inc eax
jmp_10060cb2:
		pushad
		call dword ptr [ebp + 0x20]
		popad
		add edx, ebx
		_emit 0x73 /* jae jmp_10060cbc */
		_emit 0x01
		inc edi
jmp_10060cbc:
		add esi, eax
		dec ecx
		_emit 0x75 /* jne jmp_10060cb2 */
		_emit 0xf1
		jmp jmp_100610d5
jmp_10060cc6:
		neg esi
		cmp dword ptr [ebp + 0x1c], 1
		_emit 0x74 /* je jmp_10060cfc */
		_emit 0x2e
		_emit 0x7f /* jg jmp_10060d3a */
		_emit 0x6a
		mov eax, dword ptr [ebp + 0x20]
jmp_10060cd3:
		mov byte ptr [edi], al
		add edx, ebx
		sbb edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10060cf7 */
		_emit 0x1b
		mov byte ptr [edi], al
		add edx, ebx
		sbb edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10060cf7 */
		_emit 0x12
		mov byte ptr [edi], al
		add edx, ebx
		sbb edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10060cf7 */
		_emit 0x09
		mov byte ptr [edi], al
		add edx, ebx
		sbb edi, esi
		dec ecx
		_emit 0x75 /* jne jmp_10060cd3 */
		_emit 0xdc
jmp_10060cf7:
		jmp jmp_100610d5
jmp_10060cfc:
		mov eax, dword ptr [ebp + 0x20]
		push ebp
		mov ebp, ebx
		mov ebx, eax
jmp_10060d04:
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edx, ebp
		sbb edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10060d34 */
		_emit 0x24
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edx, ebp
		sbb edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10060d34 */
		_emit 0x18
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edx, ebp
		sbb edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10060d34 */
		_emit 0x0c
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edx, ebp
		sbb edi, esi
		dec ecx
		_emit 0x75 /* jne jmp_10060d04 */
		_emit 0xd0
jmp_10060d34:
		pop ebp
		jmp jmp_100610d5
jmp_10060d3a:
		mov esi, dword ptr [ebp + 8]
		mov edi, dword ptr [ebp - 0x24]
		sub edi, dword ptr [esi + 4]
		mov eax, dword ptr [ebp - 0x28]
		sub eax, dword ptr [esi + 8]
		mov esi, eax
		mov eax, dword ptr [ebp - 0x18]
		add eax, eax
		inc eax
jmp_10060d51:
		pushad
		call dword ptr [ebp + 0x20]
		popad
		add edx, ebx
		_emit 0x73 /* jae jmp_10060d5b */
		_emit 0x01
		dec edi
jmp_10060d5b:
		add esi, eax
		dec ecx
		_emit 0x75 /* jne jmp_10060d51 */
		_emit 0xf1
		jmp jmp_100610d5
jmp_10060d65:
		mov eax, dword ptr [ebp - 0x2c]
		sub eax, dword ptr [ebp - 0x24]
		cdq
		xor eax, edx
		sub eax, edx
		inc eax
		mov ecx, eax
		mov eax, dword ptr [ebp - 0x24]
		sub eax, dword ptr [ebp + 0xc]
		cdq
		xor eax, edx
		sub eax, edx
		mul ebx
		add eax, 0x80000000
		mov edx, eax
		cmp dword ptr [ebp - 0xc], -1
		je jmp_10060e4a
		cmp dword ptr [ebp + 0x1c], 1
		_emit 0x74 /* je jmp_10060dd5 */
		_emit 0x3e
		jg jmp_10060e1f
		mov eax, dword ptr [ebp + 0x20]
jmp_10060da0:
		mov byte ptr [edi], al
		inc edi
		add edx, ebx
		_emit 0x73 /* jae jmp_10060da9 */
		_emit 0x02
		add edi, esi
jmp_10060da9:
		dec ecx
		_emit 0x74 /* je jmp_10060dd0 */
		_emit 0x24
		mov byte ptr [edi], al
		inc edi
		add edx, ebx
		_emit 0x73 /* jae jmp_10060db5 */
		_emit 0x02
		add edi, esi
jmp_10060db5:
		dec ecx
		_emit 0x74 /* je jmp_10060dd0 */
		_emit 0x18
		mov byte ptr [edi], al
		inc edi
		add edx, ebx
		_emit 0x73 /* jae jmp_10060dc1 */
		_emit 0x02
		add edi, esi
jmp_10060dc1:
		dec ecx
		_emit 0x74 /* je jmp_10060dd0 */
		_emit 0x0c
		mov byte ptr [edi], al
		inc edi
		add edx, ebx
		_emit 0x73 /* jae jmp_10060dcd */
		_emit 0x02
		add edi, esi
jmp_10060dcd:
		dec ecx
		_emit 0x75 /* jne jmp_10060da0 */
		_emit 0xd0
jmp_10060dd0:
		jmp jmp_100610d5
jmp_10060dd5:
		mov eax, dword ptr [ebp + 0x20]
		push ebp
		mov ebp, ebx
		mov ebx, eax
jmp_10060ddd:
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		inc edi
		add edx, ebp
		_emit 0x73 /* jae jmp_10060de9 */
		_emit 0x02
		add edi, esi
jmp_10060de9:
		dec ecx
		_emit 0x74 /* je jmp_10060e19 */
		_emit 0x2d
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		inc edi
		add edx, ebp
		_emit 0x73 /* jae jmp_10060df8 */
		_emit 0x02
		add edi, esi
jmp_10060df8:
		dec ecx
		_emit 0x74 /* je jmp_10060e19 */
		_emit 0x1e
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		inc edi
		add edx, ebp
		_emit 0x73 /* jae jmp_10060e07 */
		_emit 0x02
		add edi, esi
jmp_10060e07:
		dec ecx
		_emit 0x74 /* je jmp_10060e19 */
		_emit 0x0f
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		inc edi
		add edx, ebp
		_emit 0x73 /* jae jmp_10060e16 */
		_emit 0x02
		add edi, esi
jmp_10060e16:
		dec ecx
		_emit 0x75 /* jne jmp_10060ddd */
		_emit 0xc4
jmp_10060e19:
		pop ebp
		jmp jmp_100610d5
jmp_10060e1f:
		mov esi, dword ptr [ebp + 8]
		mov edi, dword ptr [ebp - 0x24]
		sub edi, dword ptr [esi + 4]
		mov eax, dword ptr [ebp - 0x28]
		sub eax, dword ptr [esi + 8]
		mov esi, eax
		mov eax, dword ptr [ebp - 0x18]
		add eax, eax
		inc eax
jmp_10060e36:
		pushad
		call dword ptr [ebp + 0x20]
		popad
		add edx, ebx
		_emit 0x73 /* jae jmp_10060e40 */
		_emit 0x01
		inc esi
jmp_10060e40:
		add edi, eax
		dec ecx
		_emit 0x75 /* jne jmp_10060e36 */
		_emit 0xf1
		jmp jmp_100610d5
jmp_10060e4a:
		cmp dword ptr [ebp + 0x1c], 1
		_emit 0x74 /* je jmp_10060e8e */
		_emit 0x3e
		jg jmp_10060ed8
		mov eax, dword ptr [ebp + 0x20]
jmp_10060e59:
		mov byte ptr [edi], al
		dec edi
		add edx, ebx
		_emit 0x73 /* jae jmp_10060e62 */
		_emit 0x02
		add edi, esi
jmp_10060e62:
		dec ecx
		_emit 0x74 /* je jmp_10060e89 */
		_emit 0x24
		mov byte ptr [edi], al
		dec edi
		add edx, ebx
		_emit 0x73 /* jae jmp_10060e6e */
		_emit 0x02
		add edi, esi
jmp_10060e6e:
		dec ecx
		_emit 0x74 /* je jmp_10060e89 */
		_emit 0x18
		mov byte ptr [edi], al
		dec edi
		add edx, ebx
		_emit 0x73 /* jae jmp_10060e7a */
		_emit 0x02
		add edi, esi
jmp_10060e7a:
		dec ecx
		_emit 0x74 /* je jmp_10060e89 */
		_emit 0x0c
		mov byte ptr [edi], al
		dec edi
		add edx, ebx
		_emit 0x73 /* jae jmp_10060e86 */
		_emit 0x02
		add edi, esi
jmp_10060e86:
		dec ecx
		_emit 0x75 /* jne jmp_10060e59 */
		_emit 0xd0
jmp_10060e89:
		jmp jmp_100610d5
jmp_10060e8e:
		mov eax, dword ptr [ebp + 0x20]
		push ebp
		mov ebp, ebx
		mov ebx, eax
jmp_10060e96:
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		dec edi
		add edx, ebp
		_emit 0x73 /* jae jmp_10060ea2 */
		_emit 0x02
		add edi, esi
jmp_10060ea2:
		dec ecx
		_emit 0x74 /* je jmp_10060ed2 */
		_emit 0x2d
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		dec edi
		add edx, ebp
		_emit 0x73 /* jae jmp_10060eb1 */
		_emit 0x02
		add edi, esi
jmp_10060eb1:
		dec ecx
		_emit 0x74 /* je jmp_10060ed2 */
		_emit 0x1e
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		dec edi
		add edx, ebp
		_emit 0x73 /* jae jmp_10060ec0 */
		_emit 0x02
		add edi, esi
jmp_10060ec0:
		dec ecx
		_emit 0x74 /* je jmp_10060ed2 */
		_emit 0x0f
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		dec edi
		add edx, ebp
		_emit 0x73 /* jae jmp_10060ecf */
		_emit 0x02
		add edi, esi
jmp_10060ecf:
		dec ecx
		_emit 0x75 /* jne jmp_10060e96 */
		_emit 0xc4
jmp_10060ed2:
		pop ebp
		jmp jmp_100610d5
jmp_10060ed8:
		mov esi, dword ptr [ebp + 8]
		mov edi, dword ptr [ebp - 0x24]
		sub edi, dword ptr [esi + 4]
		mov eax, dword ptr [ebp - 0x28]
		sub eax, dword ptr [esi + 8]
		mov esi, eax
		mov eax, dword ptr [ebp - 0x18]
		add eax, eax
		inc eax
jmp_10060eef:
		pushad
		call dword ptr [ebp + 0x20]
		popad
		add edx, ebx
		_emit 0x73 /* jae jmp_10060ef9 */
		_emit 0x01
		dec esi
jmp_10060ef9:
		add edi, eax
		dec ecx
		_emit 0x75 /* jne jmp_10060eef */
		_emit 0xf1
		jmp jmp_100610d5
jmp_10060f03:
		mov eax, dword ptr [ebp + 0xc]
		cmp eax, dword ptr [ebp - 0x3c]
		jl jmp_100610e4
		cmp eax, dword ptr [ebp - 0x44]
		jg jmp_100610e4
		mov eax, dword ptr [ebp + 0x10]
		cmp eax, dword ptr [ebp + 0x18]
		_emit 0x7f /* jg jmp_10060f23 */
		_emit 0x03
		mov eax, dword ptr [ebp + 0x18]
jmp_10060f23:
		cmp eax, dword ptr [ebp - 0x40]
		jl jmp_100610e4
		mov eax, dword ptr [ebp + 0x10]
		cmp eax, dword ptr [ebp + 0x18]
		_emit 0x7c /* jl jmp_10060f37 */
		_emit 0x03
		mov eax, dword ptr [ebp + 0x18]
jmp_10060f37:
		cmp eax, dword ptr [ebp - 0x48]
		jg jmp_100610e4
		mov eax, dword ptr [ebp + 0x10]
		cmp eax, dword ptr [ebp - 0x40]
		_emit 0x7f /* jg jmp_10060f4b */
		_emit 0x03
		mov eax, dword ptr [ebp - 0x40]
jmp_10060f4b:
		cmp eax, dword ptr [ebp - 0x48]
		_emit 0x7c /* jl jmp_10060f53 */
		_emit 0x03
		mov eax, dword ptr [ebp - 0x48]
jmp_10060f53:
		mov dword ptr [ebp - 0x28], eax
		mov eax, dword ptr [ebp + 0x18]
		cmp eax, dword ptr [ebp - 0x40]
		_emit 0x7f /* jg jmp_10060f61 */
		_emit 0x03
		mov eax, dword ptr [ebp - 0x40]
jmp_10060f61:
		cmp eax, dword ptr [ebp - 0x48]
		_emit 0x7c /* jl jmp_10060f69 */
		_emit 0x03
		mov eax, dword ptr [ebp - 0x48]
jmp_10060f69:
		mov dword ptr [ebp - 0x30], eax
		mov eax, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 0x24], eax
		mov esi, dword ptr [ebp - 0x50]
		xor esi, dword ptr [ebp - 0x18]
		sub esi, dword ptr [ebp - 0x18]
		mov eax, dword ptr [ebp - 0x30]
		sub eax, dword ptr [ebp - 0x28]
		cdq
		xor eax, edx
		sub eax, edx
		mov ecx, eax
		inc ecx
		jmp jmp_10061034
jmp_10060f8e:
		mov eax, dword ptr [ebp + 0x10]
		cmp eax, dword ptr [ebp - 0x40]
		jl jmp_100610e4
		cmp eax, dword ptr [ebp - 0x48]
		jg jmp_100610e4
		mov eax, dword ptr [ebp + 0xc]
		cmp eax, dword ptr [ebp + 0x14]
		_emit 0x7f /* jg jmp_10060fae */
		_emit 0x03
		mov eax, dword ptr [ebp + 0x14]
jmp_10060fae:
		cmp eax, dword ptr [ebp - 0x3c]
		jl jmp_100610e4
		mov eax, dword ptr [ebp + 0xc]
		cmp eax, dword ptr [ebp + 0x14]
		_emit 0x7c /* jl jmp_10060fc2 */
		_emit 0x03
		mov eax, dword ptr [ebp + 0x14]
jmp_10060fc2:
		cmp eax, dword ptr [ebp - 0x44]
		jg jmp_100610e4
		mov eax, dword ptr [ebp + 0xc]
		cmp eax, dword ptr [ebp - 0x3c]
		_emit 0x7f /* jg jmp_10060fd6 */
		_emit 0x03
		mov eax, dword ptr [ebp - 0x3c]
jmp_10060fd6:
		cmp eax, dword ptr [ebp - 0x44]
		_emit 0x7c /* jl jmp_10060fde */
		_emit 0x03
		mov eax, dword ptr [ebp - 0x44]
jmp_10060fde:
		mov dword ptr [ebp - 0x24], eax
		mov eax, dword ptr [ebp + 0x14]
		cmp eax, dword ptr [ebp - 0x3c]
		_emit 0x7f /* jg jmp_10060fec */
		_emit 0x03
		mov eax, dword ptr [ebp - 0x3c]
jmp_10060fec:
		cmp eax, dword ptr [ebp - 0x44]
		_emit 0x7c /* jl jmp_10060ff4 */
		_emit 0x03
		mov eax, dword ptr [ebp - 0x44]
jmp_10060ff4:
		mov dword ptr [ebp - 0x2c], eax
		mov eax, dword ptr [ebp + 0x10]
		mov dword ptr [ebp - 0x28], eax
		mov esi, dword ptr [ebp - 0xc]
		inc esi
		or esi, dword ptr [ebp - 0xc]
		mov eax, dword ptr [ebp - 0x2c]
		sub eax, dword ptr [ebp - 0x24]
		cdq
		xor eax, edx
		sub eax, edx
		mov ecx, eax
		inc ecx
		jmp short jmp_10061034
jmp_10061014:
		mov esi, dword ptr [ebp - 0x50]
		xor esi, dword ptr [ebp - 0x18]
		sub esi, dword ptr [ebp - 0x18]
		mov eax, dword ptr [ebp - 0xc]
		inc eax
		or eax, dword ptr [ebp - 0xc]
		add esi, eax
		mov eax, dword ptr [ebp - 0x2c]
		sub eax, dword ptr [ebp - 0x24]
		cdq
		xor eax, edx
		sub eax, edx
		mov ecx, eax
		inc ecx
jmp_10061034:
		mov eax, dword ptr [ebp - 0x28]
		imul dword ptr [ebp - 0x50]
		add eax, dword ptr [ebp - 0x4c]
		add eax, dword ptr [ebp - 0x24]
		mov edi, eax
		cmp dword ptr [ebp + 0x1c], 1
		_emit 0x74 /* je jmp_1006106b */
		_emit 0x23
		_emit 0x7f /* jg jmp_10061098 */
		_emit 0x4e
		mov eax, dword ptr [ebp + 0x20]
jmp_1006104d:
		mov byte ptr [edi], al
		add edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10061069 */
		_emit 0x15
		mov byte ptr [edi], al
		add edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10061069 */
		_emit 0x0e
		mov byte ptr [edi], al
		add edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10061069 */
		_emit 0x07
		mov byte ptr [edi], al
		add edi, esi
		dec ecx
		_emit 0x75 /* jne jmp_1006104d */
		_emit 0xe4
jmp_10061069:
		jmp short jmp_100610d5
jmp_1006106b:
		mov ebx, dword ptr [ebp + 0x20]
jmp_1006106e:
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10061096 */
		_emit 0x1e
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10061096 */
		_emit 0x14
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edi, esi
		dec ecx
		_emit 0x74 /* je jmp_10061096 */
		_emit 0x0a
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edi, esi
		dec ecx
		_emit 0x75 /* jne jmp_1006106e */
		_emit 0xd8
jmp_10061096:
		jmp short jmp_100610d5
jmp_10061098:
		mov esi, dword ptr [ebp + 8]
		mov edi, dword ptr [ebp - 0x24]
		sub edi, dword ptr [esi + 4]
		mov eax, dword ptr [ebp - 0x28]
		sub eax, dword ptr [esi + 8]
		mov esi, eax
		xor eax, eax
		test dword ptr [ebp - 0x10], 0xffffffff
		setne al
		or eax, dword ptr [ebp - 0x18]
		xor ebx, ebx
		test dword ptr [ebp - 4], 0xffffffff
		setne bl
		or ebx, dword ptr [ebp - 0xc]
jmp_100610c7:
		pushad
		call dword ptr [ebp + 0x20]
		popad
		add esi, eax
		add edi, ebx
		dec ecx
		_emit 0x75 /* jne jmp_100610c7 */
		_emit 0xf4
		jmp short jmp_100610d5
jmp_100610d5:
		xor eax, eax
		cmp dword ptr [ebp - 0x34], 1
		setae al
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_100610e4:
		mov eax, 2
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

#ifdef COMPAT_MODE
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
#else
// FUNCTION: MW2 0x100610ef
__declspec(naked) void FUN_100610ef(
	RenderTarget* p_target,
	MechS32 p_left,
	MechS32 p_top,
	MechS32 p_right,
	MechS32 p_bottom,
	MechU8 p_color
)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x20
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp + 8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx + 4]
		inc eax
		mov dword ptr [ebp - 0x18], eax
		_emit 0x7e /* jle jmp_1006116e */
		_emit 0x64
		mov eax, dword ptr [ebx + 8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_1006116e */
		_emit 0x5c
		mov eax, dword ptr [esi + 4]
		mov dword ptr [ebp - 0x1c], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_10061122 */
		_emit 0x05
		mov eax, 0
jmp_10061122:
		mov dword ptr [ebp - 4], eax
		mov eax, dword ptr [esi + 8]
		mov dword ptr [ebp - 0x20], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_10061135 */
		_emit 0x05
		mov eax, 0
jmp_10061135:
		mov dword ptr [ebp - 8], eax
		mov eax, dword ptr [esi + 0xc]
		mov edx, dword ptr [ebp - 0x18]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10061145 */
		_emit 0x02
		mov eax, edx
jmp_10061145:
		mov dword ptr [ebp - 0xc], eax
		mov eax, dword ptr [esi + 0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10061154 */
		_emit 0x02
		mov eax, edx
jmp_10061154:
		mov dword ptr [ebp - 0x10], eax
		mov eax, dword ptr [ebp - 0xc]
		cmp eax, dword ptr [ebp - 4]
		_emit 0x7c /* jl jmp_10061179 */
		_emit 0x1a
		mov eax, dword ptr [ebp - 0x10]
		cmp eax, dword ptr [ebp - 8]
		_emit 0x7c /* jl jmp_10061179 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp - 0x14], eax
		jmp short jmp_10061184
jmp_1006116e:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10061179:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10061184:
		mov eax, dword ptr [ebp - 0x1c]
		add dword ptr [ebp + 0xc], eax
		add dword ptr [ebp + 0x14], eax
		mov eax, dword ptr [ebp - 0x20]
		add dword ptr [ebp + 0x10], eax
		add dword ptr [ebp + 0x18], eax
		mov eax, dword ptr [ebp - 4]
		cmp dword ptr [ebp + 0xc], eax
		_emit 0x7f /* jg jmp_100611a1 */
		_emit 0x03
		mov dword ptr [ebp + 0xc], eax
jmp_100611a1:
		mov eax, dword ptr [ebp - 8]
		cmp dword ptr [ebp + 0x10], eax
		_emit 0x7f /* jg jmp_100611ac */
		_emit 0x03
		mov dword ptr [ebp + 0x10], eax
jmp_100611ac:
		mov eax, dword ptr [ebp - 0xc]
		cmp dword ptr [ebp + 0x14], eax
		_emit 0x7c /* jl jmp_100611b7 */
		_emit 0x03
		mov dword ptr [ebp + 0x14], eax
jmp_100611b7:
		mov eax, dword ptr [ebp - 0x10]
		cmp dword ptr [ebp + 0x18], eax
		_emit 0x7c /* jl jmp_100611c2 */
		_emit 0x03
		mov dword ptr [ebp + 0x18], eax
jmp_100611c2:
		mov ecx, dword ptr [ebp + 0x14]
		sub ecx, dword ptr [ebp + 0xc]
		_emit 0x7c /* jl jmp_1006121d */
		_emit 0x53
		inc ecx
		mov eax, dword ptr [ebp + 0x10]
		imul dword ptr [ebp - 0x18]
		add eax, dword ptr [ebp - 0x14]
		add eax, dword ptr [ebp + 0xc]
		mov edi, eax
		mov edx, dword ptr [ebp + 0x18]
		sub edx, dword ptr [ebp + 0x10]
		_emit 0x7c /* jl jmp_1006121d */
		_emit 0x3c
		mov eax, dword ptr [ebp + 0x1c]
		mov esi, edi
		mov ebx, ecx
		jmp short jmp_100611f1
jmp_100611ea:
		add esi, dword ptr [ebp - 0x18]
		mov edi, esi
		mov ecx, ebx
jmp_100611f1:
		push edx
		and edx, 1
		_emit 0x74 /* je jmp_100611fc */
		_emit 0x05
		pop edx
		inc edi
		dec ecx
		jmp short jmp_100611fd
jmp_100611fc:
		pop edx
jmp_100611fd:
		mov byte ptr [edi], al
		add edi, 2
		sub ecx, 2
		_emit 0x7f /* jg jmp_100611fd */
		_emit 0xf6
		dec edx
		_emit 0x79 /* jns jmp_100611ea */
		_emit 0xe0
		xor eax, eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
		mov eax, 0xfffffffd
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1006121d:
		mov eax, 0xfffffffc
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

#ifdef COMPAT_MODE
void BlitShpFrame(RenderTarget* p_target, void* p_shape, MechS32 p_frame, MechS32 p_x, MechS32 p_y)
{
	STUB(0x10061228);
}
#else
// FUNCTION: MW2 0x10061228
__declspec(naked) void BlitShpFrame(RenderTarget* p_target, void* p_shape, MechS32 p_frame, MechS32 p_x, MechS32 p_y)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x50
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp + 8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx + 4]
		inc eax
		mov dword ptr [ebp - 0x48], eax
		_emit 0x7e /* jle jmp_100612a7 */
		_emit 0x64
		mov eax, dword ptr [ebx + 8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_100612a7 */
		_emit 0x5c
		mov eax, dword ptr [esi + 4]
		mov dword ptr [ebp - 0x4c], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_1006125b */
		_emit 0x05
		mov eax, 0
jmp_1006125b:
		mov dword ptr [ebp - 0x34], eax
		mov eax, dword ptr [esi + 8]
		mov dword ptr [ebp - 0x50], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_1006126e */
		_emit 0x05
		mov eax, 0
jmp_1006126e:
		mov dword ptr [ebp - 0x38], eax
		mov eax, dword ptr [esi + 0xc]
		mov edx, dword ptr [ebp - 0x48]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_1006127e */
		_emit 0x02
		mov eax, edx
jmp_1006127e:
		mov dword ptr [ebp - 0x3c], eax
		mov eax, dword ptr [esi + 0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_1006128d */
		_emit 0x02
		mov eax, edx
jmp_1006128d:
		mov dword ptr [ebp - 0x40], eax
		mov eax, dword ptr [ebp - 0x3c]
		cmp eax, dword ptr [ebp - 0x34]
		_emit 0x7c /* jl jmp_100612b2 */
		_emit 0x1a
		mov eax, dword ptr [ebp - 0x40]
		cmp eax, dword ptr [ebp - 0x38]
		_emit 0x7c /* jl jmp_100612b2 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp - 0x44], eax
		jmp short jmp_100612bd
jmp_100612a7:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_100612b2:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_100612bd:
		mov eax, dword ptr [ebp - 0x4c]
		add dword ptr [ebp + 0x14], eax
		mov eax, dword ptr [ebp - 0x50]
		add dword ptr [ebp + 0x18], eax
		mov esi, dword ptr [ebp + 0x10]
		shl esi, 3
		add esi, 8
		add esi, dword ptr [ebp + 0xc]
		mov esi, dword ptr [esi]
		add esi, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 0x30], esi
		mov eax, dword ptr [esi + 8]
		add eax, dword ptr [ebp + 0x14]
		mov dword ptr [ebp - 0xc], eax
		mov eax, dword ptr [esi + 0xc]
		add eax, dword ptr [ebp + 0x18]
		mov dword ptr [ebp - 0x10], eax
		mov eax, dword ptr [esi + 0x10]
		add eax, dword ptr [ebp + 0x14]
		mov dword ptr [ebp - 0x14], eax
		mov eax, dword ptr [esi + 0x14]
		add eax, dword ptr [ebp + 0x18]
		mov dword ptr [ebp - 0x18], eax
		add esi, 0x18
		mov eax, dword ptr [ebp - 0x14]
		cmp eax, dword ptr [ebp - 0xc]
		jl jmp_10061691
		mov eax, dword ptr [ebp - 0x18]
		cmp eax, dword ptr [ebp - 0x10]
		jl jmp_10061691
		xor edx, edx
		mov eax, dword ptr [ebp - 0xc]
		sub eax, dword ptr [ebp - 0x34]
		shl eax, 1
		adc dl, dl
		mov eax, dword ptr [ebp - 0x3c]
		sub eax, dword ptr [ebp - 0xc]
		shl eax, 1
		adc dl, dl
		mov eax, dword ptr [ebp - 0x10]
		sub eax, dword ptr [ebp - 0x38]
		shl eax, 1
		adc dl, dl
		mov eax, dword ptr [ebp - 0x40]
		sub eax, dword ptr [ebp - 0x10]
		shl eax, 1
		adc dl, dl
		mov eax, dword ptr [ebp - 0x14]
		sub eax, dword ptr [ebp - 0x34]
		shl eax, 1
		adc dh, dh
		mov eax, dword ptr [ebp - 0x3c]
		sub eax, dword ptr [ebp - 0x14]
		shl eax, 1
		adc dh, dh
		mov eax, dword ptr [ebp - 0x18]
		sub eax, dword ptr [ebp - 0x38]
		shl eax, 1
		adc dh, dh
		mov eax, dword ptr [ebp - 0x40]
		sub eax, dword ptr [ebp - 0x18]
		shl eax, 1
		adc dh, dh
		mov dword ptr [ebp - 0x1c], edx
		_emit 0x84 /* test dh, dl */
		_emit 0xd6
		jne jmp_10061686
		or dl, dh
		_emit 0x75 /* jne jmp_100613a8 */
		_emit 0x2b
		mov esi, dword ptr [ebp + 8]
		mov eax, dword ptr [esi + 4]
		sub dword ptr [ebp + 0x14], eax
		mov eax, dword ptr [esi + 8]
		sub dword ptr [ebp + 0x18], eax
		push dword ptr [ebp - 0x48]
		push dword ptr [ebp + 0x18]
		push dword ptr [ebp + 0x14]
		push dword ptr [ebp - 0x30]
		push dword ptr [ebp + 8]
		call BlitShpFrameUnclipped
		add esp, 0x14
		jmp jmp_1006167e
jmp_100613a8:
		mov eax, dword ptr [ebp - 0x10]
		imul dword ptr [ebp - 0x48]
		add eax, dword ptr [ebp - 0x44]
		add eax, dword ptr [ebp - 0xc]
		mov edi, eax
		mov ecx, dword ptr [ebp - 0x10]
		mov dword ptr [ebp - 0x20], ecx
		jmp short jmp_100613d4
jmp_100613be:
		movzx eax, al
		add esi, eax
		dec esi
jmp_100613c4:
		inc esi
jmp_100613c5:
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_100613c4 */
		_emit 0xf8
		_emit 0x75 /* jne jmp_100613be */
		_emit 0xf0
		_emit 0x72 /* jb jmp_100613c4 */
		_emit 0xf4
		add edi, dword ptr [ebp - 0x48]
		inc ecx
jmp_100613d4:
		cmp ecx, dword ptr [ebp - 0x38]
		_emit 0x7c /* jl jmp_100613c5 */
		_emit 0xec
		mov dword ptr [ebp - 0x24], edi
		mov dword ptr [ebp - 0x20], ecx
		mov eax, edi
		sub eax, dword ptr [ebp - 0xc]
		add eax, dword ptr [ebp - 0x34]
		mov dword ptr [ebp - 0x28], eax
		mov eax, edi
		sub eax, dword ptr [ebp - 0xc]
		add eax, dword ptr [ebp - 0x3c]
		mov dword ptr [ebp - 0x2c], eax
		jmp jmp_10061672
jmp_100613fa:
		mov eax, dword ptr [ebp - 0x20]
		cmp eax, dword ptr [ebp - 0x40]
		jg jmp_1006167e
		mov edi, dword ptr [ebp - 0x24]
		test dword ptr [ebp - 0x1c], 8
		jne jmp_100614bf
		test dword ptr [ebp - 0x1c], 0x400
		jne jmp_1006154d
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061447 */
		_emit 0x1d
		_emit 0x75 /* jne jmp_10061486 */
		_emit 0x5a
		jae jmp_100614ba
jmp_10061432:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061447 */
		_emit 0x06
		_emit 0x75 /* jne jmp_10061486 */
		_emit 0x43
		_emit 0x72 /* jb jmp_10061432 */
		_emit 0xed
		_emit 0x73 /* jae jmp_100614ba */
		_emit 0x73
jmp_10061447:
		movzx ecx, al
jmp_1006144a:
		mov al, byte ptr [esi]
		inc esi
		cmp ecx, 4
		_emit 0x7e /* jle jmp_10061479 */
		_emit 0x27
		push ebx
		mov ah, al
		mov ebx, ecx
		mov ecx, edi
		rol eax, 8
		neg ecx
		and ecx, 3
		mov al, ah
		sub ebx, ecx
		rep stosb
		rol eax, 8
		mov ecx, ebx
		mov al, ah
		and ebx, 3
		shr ecx, 2
		rep stosd
		mov ecx, ebx
		pop ebx
jmp_10061479:
		rep stosb
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061447 */
		_emit 0xc5
		_emit 0x73 /* jae jmp_100614ba */
		_emit 0x36
		_emit 0x74 /* je jmp_10061432 */
		_emit 0xac
jmp_10061486:
		movzx ecx, al
jmp_10061489:
		cmp ecx, 4
		_emit 0x7e /* jle jmp_100614a9 */
		_emit 0x1b
		push ebx
		mov ebx, ecx
		mov ecx, edi
		neg ecx
		and ecx, 3
		sub ebx, ecx
		rep movsb
		mov ecx, ebx
		and ebx, 3
		shr ecx, 2
		rep movsd
		mov ecx, ebx
		pop ebx
jmp_100614a9:
		rep movsb
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061447 */
		_emit 0x95
		_emit 0x75 /* jne jmp_10061486 */
		_emit 0xd2
		jb jmp_10061432
jmp_100614ba:
		jmp jmp_10061663
jmp_100614bf:
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_100614df */
		_emit 0x19
		_emit 0x75 /* jne jmp_10061510 */
		_emit 0x48
		_emit 0x73 /* jae jmp_10061548 */
		_emit 0x7e
jmp_100614ca:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_100614df */
		_emit 0x06
		_emit 0x75 /* jne jmp_10061510 */
		_emit 0x35
		_emit 0x72 /* jb jmp_100614ca */
		_emit 0xed
		_emit 0x73 /* jae jmp_10061548 */
		_emit 0x69
jmp_100614df:
		movzx ecx, al
		mov eax, dword ptr [ebp - 0x28]
		sub eax, edi
		cmp eax, ecx
		_emit 0x7d /* jge jmp_10061502 */
		_emit 0x17
		or eax, eax
		_emit 0x78 /* js jmp_100614f3 */
		_emit 0x04
		add edi, eax
		sub ecx, eax
jmp_100614f3:
		test dword ptr [ebp - 0x1c], 0x400
		je jmp_1006144a
		_emit 0x75 /* jne jmp_10061578 */
		_emit 0x76
jmp_10061502:
		add edi, ecx
		inc esi
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_100614df */
		_emit 0xd3
		_emit 0x73 /* jae jmp_10061548 */
		_emit 0x3a
		_emit 0x74 /* je jmp_100614ca */
		_emit 0xba
jmp_10061510:
		movzx ecx, al
		mov eax, dword ptr [ebp - 0x28]
		sub eax, edi
		cmp eax, ecx
		_emit 0x7d /* jge jmp_10061539 */
		_emit 0x1d
		or eax, eax
		_emit 0x78 /* js jmp_10061526 */
		_emit 0x06
		add edi, eax
		sub ecx, eax
		add esi, eax
jmp_10061526:
		test dword ptr [ebp - 0x1c], 0x400
		je jmp_10061489
		jne jmp_100615d1
jmp_10061539:
		add edi, ecx
		add esi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_100614df */
		_emit 0x9b
		_emit 0x75 /* jne jmp_10061510 */
		_emit 0xca
		_emit 0x72 /* jb jmp_100614ca */
		_emit 0x82
jmp_10061548:
		jmp jmp_10061663
jmp_1006154d:
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061575 */
		_emit 0x21
		_emit 0x75 /* jne jmp_100615ce */
		_emit 0x78
		jae jmp_1006161e
jmp_1006155c:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061575 */
		_emit 0x0a
		_emit 0x75 /* jne jmp_100615ce */
		_emit 0x61
		_emit 0x72 /* jb jmp_1006155c */
		_emit 0xed
		jae jmp_1006161e
jmp_10061575:
		movzx ecx, al
jmp_10061578:
		cmp edi, dword ptr [ebp - 0x2c]
		jg jmp_10061643
		mov eax, edi
		add eax, ecx
		dec eax
		sub eax, dword ptr [ebp - 0x2c]
		cdq
		not edx
		and edx, eax
		sub ecx, edx
		mov al, byte ptr [esi]
		inc esi
		cmp ecx, 4
		_emit 0x7e /* jle jmp_100615bf */
		_emit 0x27
		push ebx
		mov ah, al
		mov ebx, ecx
		mov ecx, edi
		rol eax, 8
		neg ecx
		and ecx, 3
		mov al, ah
		sub ebx, ecx
		rep stosb
		rol eax, 8
		mov ecx, ebx
		mov al, ah
		and ebx, 3
		shr ecx, 2
		rep stosd
		mov ecx, ebx
		pop ebx
jmp_100615bf:
		rep stosb
		add edi, edx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061575 */
		_emit 0xab
		_emit 0x73 /* jae jmp_1006161e */
		_emit 0x52
		_emit 0x74 /* je jmp_1006155c */
		_emit 0x8e
jmp_100615ce:
		movzx ecx, al
jmp_100615d1:
		cmp edi, dword ptr [ebp - 0x2c]
		_emit 0x7f /* jg jmp_10061654 */
		_emit 0x7e
		mov eax, edi
		add eax, ecx
		dec eax
		sub eax, dword ptr [ebp - 0x2c]
		cdq
		not edx
		and edx, eax
		sub ecx, edx
		cmp ecx, 4
		_emit 0x7e /* jle jmp_10061605 */
		_emit 0x1b
		push ebx
		mov ebx, ecx
		mov ecx, edi
		neg ecx
		and ecx, 3
		sub ebx, ecx
		rep movsb
		mov ecx, ebx
		and ebx, 3
		shr ecx, 2
		rep movsd
		mov ecx, ebx
		pop ebx
jmp_10061605:
		rep movsb
		add edi, edx
		add esi, edx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		ja jmp_10061575
		_emit 0x75 /* jne jmp_100615ce */
		_emit 0xb6
		jb jmp_1006155c
jmp_1006161e:
		jmp short jmp_10061663
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061640 */
		_emit 0x19
		_emit 0x75 /* jne jmp_10061651 */
		_emit 0x28
		_emit 0x73 /* jae jmp_10061663 */
		_emit 0x38
jmp_1006162b:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061640 */
		_emit 0x06
		_emit 0x75 /* jne jmp_10061651 */
		_emit 0x15
		_emit 0x72 /* jb jmp_1006162b */
		_emit 0xed
		_emit 0x73 /* jae jmp_10061663 */
		_emit 0x23
jmp_10061640:
		movzx ecx, al
jmp_10061643:
		add edi, ecx
		inc esi
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061640 */
		_emit 0xf3
		_emit 0x73 /* jae jmp_10061663 */
		_emit 0x14
		_emit 0x74 /* je jmp_1006162b */
		_emit 0xda
jmp_10061651:
		movzx ecx, al
jmp_10061654:
		add edi, ecx
		add esi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061640 */
		_emit 0xe1
		_emit 0x75 /* jne jmp_10061651 */
		_emit 0xf0
		_emit 0x72 /* jb jmp_1006162b */
		_emit 0xc8
jmp_10061663:
		mov eax, dword ptr [ebp - 0x48]
		add dword ptr [ebp - 0x24], eax
		add dword ptr [ebp - 0x28], eax
		add dword ptr [ebp - 0x2c], eax
		inc dword ptr [ebp - 0x20]
jmp_10061672:
		mov eax, dword ptr [ebp - 0x20]
		cmp eax, dword ptr [ebp - 0x18]
		jle jmp_100613fa
jmp_1006167e:
		xor eax, eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10061686:
		mov eax, 0xfffffffd
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10061691:
		mov eax, 0xfffffffc
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Draws the run-length encoded shape frame p_frame at p_x, p_y of p_target (unclipped).
// FUNCTION: MW2 0x1006169c
void BlitShpFrameUnclipped(RenderTarget* p_target, void* p_frame, MechS32 p_x, MechS32 p_y, undefined4 p_unk0x10)
{
	__asm {
		push es
		cld
		push ds
		pop es
		mov esi, p_target
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [esi + 4]
		add p_x, eax
		mov eax, dword ptr [esi + 8]
		add p_y, eax
		mov eax, dword ptr [ebx + 4]
		inc eax
		mov p_unk0x10, eax
		jle jmp_10061797
		mov esi, p_frame
		mov edi, dword ptr [ebx]
		mov eax, dword ptr [esi + 8]
		add eax, p_x
		add edi, eax
		mov eax, dword ptr [esi + 0xc]
		mov ebx, eax
		add eax, p_y
		mul p_unk0x10
		add edi, eax
		mov edx, edi
		mov eax, dword ptr [esi + 0x10]
		mov eax, dword ptr [esi + 0x14]
		inc eax
		sub eax, ebx
		mov ebx, eax
		jle jmp_10061797
		add esi, 0x18
jmp_100616f4:
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061718 */
		_emit 0x1d
		_emit 0x75 /* jne jmp_10061757 */
		_emit 0x5a
		jae jmp_1006178b
jmp_10061703:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061718 */
		_emit 0x06
		_emit 0x75 /* jne jmp_10061757 */
		_emit 0x43
		_emit 0x72 /* jb jmp_10061703 */
		_emit 0xed
		_emit 0x73 /* jae jmp_1006178b */
		_emit 0x73
jmp_10061718:
		movzx ecx, al
		mov al, byte ptr [esi]
		inc esi
		cmp ecx, 4
		_emit 0x7e /* jle jmp_1006174a */
		_emit 0x27
		push ebx
		mov ah, al
		mov ebx, ecx
		mov ecx, edi
		rol eax, 8
		neg ecx
		and ecx, 3
		mov al, ah
		sub ebx, ecx
		rep stosb
		rol eax, 8
		mov ecx, ebx
		mov al, ah
		and ebx, 3
		shr ecx, 2
		rep stosd
		mov ecx, ebx
		pop ebx
jmp_1006174a:
		rep stosb
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061718 */
		_emit 0xc5
		_emit 0x73 /* jae jmp_1006178b */
		_emit 0x36
		_emit 0x74 /* je jmp_10061703 */
		_emit 0xac
jmp_10061757:
		movzx ecx, al
		cmp ecx, 4
		_emit 0x7e /* jle jmp_1006177a */
		_emit 0x1b
		push ebx
		mov ebx, ecx
		mov ecx, edi
		neg ecx
		and ecx, 3
		sub ebx, ecx
		rep movsb
		mov ecx, ebx
		and ebx, 3
		shr ecx, 2
		rep movsd
		mov ecx, ebx
		pop ebx
jmp_1006177a:
		rep movsb
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061718 */
		_emit 0x95
		_emit 0x75 /* jne jmp_10061757 */
		_emit 0xd2
		jb jmp_10061703
jmp_1006178b:
		add edx, p_unk0x10
		mov edi, edx
		dec ebx
		jne jmp_100616f4
jmp_10061797:
		xor eax, eax
		pop es
	}
}

// Sets the colour map g_remapTable from p_map.
// FUNCTION: MW2 0x1006179f
void SetRemapTable(MechU8* p_map)
{
	__asm {
		push es
		cld
		push ds
		pop es
		mov esi, p_map
		mov edi, offset g_remapTable
		mov ecx, 0x40
		rep movsd
		pop es
	}
}

// BlitShpFrame through the colour map g_remapTable: clipped, or through BlitShpFrameRemappedUnclipped when the
// frame lies inside the target. Returns a negative code for an empty target or frame.
#ifdef COMPAT_MODE
MechS32 BlitShpFrameRemapped(RenderTarget* p_target, void* p_shape, MechS32 p_frame, MechS32 p_x, MechS32 p_y)
{
	STUB(0x100617be);
	return 0;
}
#else
// FUNCTION: MW2 0x100617be
__declspec(naked) MechS32
BlitShpFrameRemapped(RenderTarget* p_target, void* p_shape, MechS32 p_frame, MechS32 p_x, MechS32 p_y)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x50
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp + 8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx + 4]
		inc eax
		mov dword ptr [ebp - 0x48], eax
		_emit 0x7e /* jle jmp_1006183d */
		_emit 0x64
		mov eax, dword ptr [ebx + 8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_1006183d */
		_emit 0x5c
		mov eax, dword ptr [esi + 4]
		mov dword ptr [ebp - 0x4c], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_100617f1 */
		_emit 0x05
		mov eax, 0
jmp_100617f1:
		mov dword ptr [ebp - 0x34], eax
		mov eax, dword ptr [esi + 8]
		mov dword ptr [ebp - 0x50], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_10061804 */
		_emit 0x05
		mov eax, 0
jmp_10061804:
		mov dword ptr [ebp - 0x38], eax
		mov eax, dword ptr [esi + 0xc]
		mov edx, dword ptr [ebp - 0x48]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10061814 */
		_emit 0x02
		mov eax, edx
jmp_10061814:
		mov dword ptr [ebp - 0x3c], eax
		mov eax, dword ptr [esi + 0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10061823 */
		_emit 0x02
		mov eax, edx
jmp_10061823:
		mov dword ptr [ebp - 0x40], eax
		mov eax, dword ptr [ebp - 0x3c]
		cmp eax, dword ptr [ebp - 0x34]
		_emit 0x7c /* jl jmp_10061848 */
		_emit 0x1a
		mov eax, dword ptr [ebp - 0x40]
		cmp eax, dword ptr [ebp - 0x38]
		_emit 0x7c /* jl jmp_10061848 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp - 0x44], eax
		jmp short jmp_10061853
jmp_1006183d:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10061848:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10061853:
		mov eax, dword ptr [ebp - 0x4c]
		add dword ptr [ebp + 0x14], eax
		mov eax, dword ptr [ebp - 0x50]
		add dword ptr [ebp + 0x18], eax
		mov esi, dword ptr [ebp + 0x10]
		shl esi, 3
		add esi, 8
		add esi, dword ptr [ebp + 0xc]
		mov esi, dword ptr [esi]
		add esi, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 0x30], esi
		mov eax, dword ptr [esi + 8]
		add eax, dword ptr [ebp + 0x14]
		mov dword ptr [ebp - 0xc], eax
		mov eax, dword ptr [esi + 0xc]
		add eax, dword ptr [ebp + 0x18]
		mov dword ptr [ebp - 0x10], eax
		mov eax, dword ptr [esi + 0x10]
		add eax, dword ptr [ebp + 0x14]
		mov dword ptr [ebp - 0x14], eax
		mov eax, dword ptr [esi + 0x14]
		add eax, dword ptr [ebp + 0x18]
		mov dword ptr [ebp - 0x18], eax
		add esi, 0x18
		mov eax, dword ptr [ebp - 0x14]
		cmp eax, dword ptr [ebp - 0xc]
		jl jmp_10061c19
		mov eax, dword ptr [ebp - 0x18]
		cmp eax, dword ptr [ebp - 0x10]
		jl jmp_10061c19
		xor edx, edx
		mov eax, dword ptr [ebp - 0xc]
		sub eax, dword ptr [ebp - 0x34]
		shl eax, 1
		adc dl, dl
		mov eax, dword ptr [ebp - 0x3c]
		sub eax, dword ptr [ebp - 0xc]
		shl eax, 1
		adc dl, dl
		mov eax, dword ptr [ebp - 0x10]
		sub eax, dword ptr [ebp - 0x38]
		shl eax, 1
		adc dl, dl
		mov eax, dword ptr [ebp - 0x40]
		sub eax, dword ptr [ebp - 0x10]
		shl eax, 1
		adc dl, dl
		mov eax, dword ptr [ebp - 0x14]
		sub eax, dword ptr [ebp - 0x34]
		shl eax, 1
		adc dh, dh
		mov eax, dword ptr [ebp - 0x3c]
		sub eax, dword ptr [ebp - 0x14]
		shl eax, 1
		adc dh, dh
		mov eax, dword ptr [ebp - 0x18]
		sub eax, dword ptr [ebp - 0x38]
		shl eax, 1
		adc dh, dh
		mov eax, dword ptr [ebp - 0x40]
		sub eax, dword ptr [ebp - 0x18]
		shl eax, 1
		adc dh, dh
		mov dword ptr [ebp - 0x1c], edx
		_emit 0x84 /* test dh, dl */
		_emit 0xd6
		jne jmp_10061c0e
		or dl, dh
		_emit 0x75 /* jne jmp_1006193e */
		_emit 0x2b
		mov esi, dword ptr [ebp + 8]
		mov eax, dword ptr [esi + 4]
		sub dword ptr [ebp + 0x14], eax
		mov eax, dword ptr [esi + 8]
		sub dword ptr [ebp + 0x18], eax
		push dword ptr [ebp - 0x48]
		push dword ptr [ebp + 0x18]
		push dword ptr [ebp + 0x14]
		push dword ptr [ebp - 0x30]
		push dword ptr [ebp + 8]
		call BlitShpFrameRemappedUnclipped
		add esp, 0x14
		jmp jmp_10061c06
jmp_1006193e:
		mov eax, dword ptr [ebp - 0x10]
		imul dword ptr [ebp - 0x48]
		add eax, dword ptr [ebp - 0x44]
		add eax, dword ptr [ebp - 0xc]
		mov edi, eax
		mov ecx, dword ptr [ebp - 0x10]
		mov dword ptr [ebp - 0x20], ecx
		jmp short jmp_1006196a
jmp_10061954:
		movzx eax, al
		add esi, eax
		dec esi
jmp_1006195a:
		inc esi
jmp_1006195b:
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_1006195a */
		_emit 0xf8
		_emit 0x75 /* jne jmp_10061954 */
		_emit 0xf0
		_emit 0x72 /* jb jmp_1006195a */
		_emit 0xf4
		add edi, dword ptr [ebp - 0x48]
		inc ecx
jmp_1006196a:
		cmp ecx, dword ptr [ebp - 0x38]
		_emit 0x7c /* jl jmp_1006195b */
		_emit 0xec
		mov dword ptr [ebp - 0x24], edi
		mov dword ptr [ebp - 0x20], ecx
		mov eax, edi
		sub eax, dword ptr [ebp - 0xc]
		add eax, dword ptr [ebp - 0x34]
		mov dword ptr [ebp - 0x28], eax
		mov eax, edi
		sub eax, dword ptr [ebp - 0xc]
		add eax, dword ptr [ebp - 0x3c]
		mov dword ptr [ebp - 0x2c], eax
		jmp jmp_10061bfa
jmp_10061990:
		mov eax, dword ptr [ebp - 0x20]
		cmp eax, dword ptr [ebp - 0x40]
		jg jmp_10061c06
		mov edi, dword ptr [ebp - 0x24]
		test dword ptr [ebp - 0x1c], 8
		jne jmp_10061a48
		test dword ptr [ebp - 0x1c], 0x400
		jne jmp_10061ad6
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_100619d9 */
		_emit 0x19
		_emit 0x75 /* jne jmp_10061a20 */
		_emit 0x5e
		_emit 0x73 /* jae jmp_10061a43 */
		_emit 0x7f
jmp_100619c4:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_100619d9 */
		_emit 0x06
		_emit 0x75 /* jne jmp_10061a20 */
		_emit 0x4b
		_emit 0x72 /* jb jmp_100619c4 */
		_emit 0xed
		_emit 0x73 /* jae jmp_10061a43 */
		_emit 0x6a
jmp_100619d9:
		movzx ecx, al
jmp_100619dc:
		xor eax, eax
		mov al, byte ptr [esi]
		inc esi
		mov al, byte ptr [g_remapTable + eax]
		cmp ecx, 4
		_emit 0x7e /* jle jmp_10061a13 */
		_emit 0x27
		push ebx
		mov ah, al
		mov ebx, ecx
		mov ecx, edi
		rol eax, 8
		neg ecx
		and ecx, 3
		mov al, ah
		sub ebx, ecx
		rep stosb
		rol eax, 8
		mov ecx, ebx
		mov al, ah
		and ebx, 3
		shr ecx, 2
		rep stosd
		mov ecx, ebx
		pop ebx
jmp_10061a13:
		rep stosb
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_100619d9 */
		_emit 0xbd
		_emit 0x73 /* jae jmp_10061a43 */
		_emit 0x25
		_emit 0x74 /* je jmp_100619c4 */
		_emit 0xa4
jmp_10061a20:
		movzx ecx, al
jmp_10061a23:
		xor eax, eax
		or ecx, ecx
		_emit 0x74 /* je jmp_10061a38 */
		_emit 0x0f
jmp_10061a29:
		mov al, byte ptr [esi]
		inc esi
		mov al, byte ptr [g_remapTable + eax]
		mov byte ptr [edi], al
		inc edi
		dec ecx
		_emit 0x75 /* jne jmp_10061a29 */
		_emit 0xf1
jmp_10061a38:
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_100619d9 */
		_emit 0x9a
		_emit 0x75 /* jne jmp_10061a20 */
		_emit 0xdf
		_emit 0x72 /* jb jmp_100619c4 */
		_emit 0x81
jmp_10061a43:
		jmp jmp_10061beb
jmp_10061a48:
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061a68 */
		_emit 0x19
		_emit 0x75 /* jne jmp_10061a99 */
		_emit 0x48
		_emit 0x73 /* jae jmp_10061ad1 */
		_emit 0x7e
jmp_10061a53:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061a68 */
		_emit 0x06
		_emit 0x75 /* jne jmp_10061a99 */
		_emit 0x35
		_emit 0x72 /* jb jmp_10061a53 */
		_emit 0xed
		_emit 0x73 /* jae jmp_10061ad1 */
		_emit 0x69
jmp_10061a68:
		movzx ecx, al
		mov eax, dword ptr [ebp - 0x28]
		sub eax, edi
		cmp eax, ecx
		_emit 0x7d /* jge jmp_10061a8b */
		_emit 0x17
		or eax, eax
		_emit 0x78 /* js jmp_10061a7c */
		_emit 0x04
		add edi, eax
		sub ecx, eax
jmp_10061a7c:
		test dword ptr [ebp - 0x1c], 0x400
		je jmp_100619dc
		_emit 0x75 /* jne jmp_10061b05 */
		_emit 0x7a
jmp_10061a8b:
		add edi, ecx
		inc esi
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061a68 */
		_emit 0xd3
		_emit 0x73 /* jae jmp_10061ad1 */
		_emit 0x3a
		_emit 0x74 /* je jmp_10061a53 */
		_emit 0xba
jmp_10061a99:
		movzx ecx, al
		mov eax, dword ptr [ebp - 0x28]
		sub eax, edi
		cmp eax, ecx
		_emit 0x7d /* jge jmp_10061ac2 */
		_emit 0x1d
		or eax, eax
		_emit 0x78 /* js jmp_10061aaf */
		_emit 0x06
		add edi, eax
		sub ecx, eax
		add esi, eax
jmp_10061aaf:
		test dword ptr [ebp - 0x1c], 0x400
		je jmp_10061a23
		jne jmp_10061b66
jmp_10061ac2:
		add edi, ecx
		add esi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061a68 */
		_emit 0x9b
		_emit 0x75 /* jne jmp_10061a99 */
		_emit 0xca
		_emit 0x72 /* jb jmp_10061a53 */
		_emit 0x82
jmp_10061ad1:
		jmp jmp_10061beb
jmp_10061ad6:
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061b02 */
		_emit 0x25
		jne jmp_10061b63
		jae jmp_10061ba6
jmp_10061ae9:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061b02 */
		_emit 0x0a
		_emit 0x75 /* jne jmp_10061b63 */
		_emit 0x69
		_emit 0x72 /* jb jmp_10061ae9 */
		_emit 0xed
		jae jmp_10061ba6
jmp_10061b02:
		movzx ecx, al
jmp_10061b05:
		cmp edi, dword ptr [ebp - 0x2c]
		jg jmp_10061bcb
		mov eax, edi
		add eax, ecx
		dec eax
		sub eax, dword ptr [ebp - 0x2c]
		cdq
		not edx
		and edx, eax
		sub ecx, edx
		xor eax, eax
		mov al, byte ptr [esi]
		inc esi
		mov al, byte ptr [g_remapTable + eax]
		cmp ecx, 4
		_emit 0x7e /* jle jmp_10061b54 */
		_emit 0x27
		push ebx
		mov ah, al
		mov ebx, ecx
		mov ecx, edi
		rol eax, 8
		neg ecx
		and ecx, 3
		mov al, ah
		sub ebx, ecx
		rep stosb
		rol eax, 8
		mov ecx, ebx
		mov al, ah
		and ebx, 3
		shr ecx, 2
		rep stosd
		mov ecx, ebx
		pop ebx
jmp_10061b54:
		rep stosb
		add edi, edx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061b02 */
		_emit 0xa3
		_emit 0x73 /* jae jmp_10061ba6 */
		_emit 0x45
		_emit 0x74 /* je jmp_10061ae9 */
		_emit 0x86
jmp_10061b63:
		movzx ecx, al
jmp_10061b66:
		cmp edi, dword ptr [ebp - 0x2c]
		_emit 0x7f /* jg jmp_10061bdc */
		_emit 0x71
		mov eax, edi
		add eax, ecx
		dec eax
		sub eax, dword ptr [ebp - 0x2c]
		cdq
		not edx
		and edx, eax
		sub ecx, edx
		xor eax, eax
		or ecx, ecx
		_emit 0x74 /* je jmp_10061b8f */
		_emit 0x0f
jmp_10061b80:
		mov al, byte ptr [esi]
		inc esi
		mov al, byte ptr [g_remapTable + eax]
		mov byte ptr [edi], al
		inc edi
		dec ecx
		_emit 0x75 /* jne jmp_10061b80 */
		_emit 0xf1
jmp_10061b8f:
		add edi, edx
		add esi, edx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		ja jmp_10061b02
		_emit 0x75 /* jne jmp_10061b63 */
		_emit 0xc3
		jb jmp_10061ae9
jmp_10061ba6:
		jmp short jmp_10061beb
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061bc8 */
		_emit 0x19
		_emit 0x75 /* jne jmp_10061bd9 */
		_emit 0x28
		_emit 0x73 /* jae jmp_10061beb */
		_emit 0x38
jmp_10061bb3:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061bc8 */
		_emit 0x06
		_emit 0x75 /* jne jmp_10061bd9 */
		_emit 0x15
		_emit 0x72 /* jb jmp_10061bb3 */
		_emit 0xed
		_emit 0x73 /* jae jmp_10061beb */
		_emit 0x23
jmp_10061bc8:
		movzx ecx, al
jmp_10061bcb:
		add edi, ecx
		inc esi
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061bc8 */
		_emit 0xf3
		_emit 0x73 /* jae jmp_10061beb */
		_emit 0x14
		_emit 0x74 /* je jmp_10061bb3 */
		_emit 0xda
jmp_10061bd9:
		movzx ecx, al
jmp_10061bdc:
		add edi, ecx
		add esi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061bc8 */
		_emit 0xe1
		_emit 0x75 /* jne jmp_10061bd9 */
		_emit 0xf0
		_emit 0x72 /* jb jmp_10061bb3 */
		_emit 0xc8
jmp_10061beb:
		mov eax, dword ptr [ebp - 0x48]
		add dword ptr [ebp - 0x24], eax
		add dword ptr [ebp - 0x28], eax
		add dword ptr [ebp - 0x2c], eax
		inc dword ptr [ebp - 0x20]
jmp_10061bfa:
		mov eax, dword ptr [ebp - 0x20]
		cmp eax, dword ptr [ebp - 0x18]
		jle jmp_10061990
jmp_10061c06:
		xor eax, eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10061c0e:
		mov eax, 0xfffffffd
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10061c19:
		mov eax, 0xfffffffc
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// BlitShpFrameUnclipped through the colour map g_remapTable. Returns 0.
// FUNCTION: MW2 0x10061c24
MechS32 BlitShpFrameRemappedUnclipped(
	RenderTarget* p_target,
	void* p_frame,
	MechS32 p_x,
	MechS32 p_y,
	undefined4 p_unk0x10
)
{
	__asm {
		push es
		cld
		push ds
		pop es
		mov esi, p_target
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [esi + 4]
		add p_x, eax
		mov eax, dword ptr [esi + 8]
		add p_y, eax
		mov eax, dword ptr [ebx + 4]
		inc eax
		mov p_unk0x10, eax
		jle jmp_10061d12
		mov esi, p_frame
		mov edi, dword ptr [ebx]
		mov eax, dword ptr [esi + 8]
		add eax, p_x
		add edi, eax
		mov eax, dword ptr [esi + 0xc]
		mov ebx, eax
		add eax, p_y
		mul p_unk0x10
		add edi, eax
		mov edx, edi
		mov eax, dword ptr [esi + 0x10]
		mov eax, dword ptr [esi + 0x14]
		inc eax
		sub eax, ebx
		mov ebx, eax
		jle jmp_10061d12
		add esi, 0x18
jmp_10061c7c:
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061c9c */
		_emit 0x19
		_emit 0x75 /* jne jmp_10061ce3 */
		_emit 0x5e
		_emit 0x73 /* jae jmp_10061d06 */
		_emit 0x7f
jmp_10061c87:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061c9c */
		_emit 0x06
		_emit 0x75 /* jne jmp_10061ce3 */
		_emit 0x4b
		_emit 0x72 /* jb jmp_10061c87 */
		_emit 0xed
		_emit 0x73 /* jae jmp_10061d06 */
		_emit 0x6a
jmp_10061c9c:
		movzx ecx, al
		xor eax, eax
		mov al, byte ptr [esi]
		inc esi
		mov al, byte ptr [g_remapTable + eax]
		cmp ecx, 4
		_emit 0x7e /* jle jmp_10061cd6 */
		_emit 0x27
		push ebx
		mov ah, al
		mov ebx, ecx
		mov ecx, edi
		rol eax, 8
		neg ecx
		and ecx, 3
		mov al, ah
		sub ebx, ecx
		rep stosb
		rol eax, 8
		mov ecx, ebx
		mov al, ah
		and ebx, 3
		shr ecx, 2
		rep stosd
		mov ecx, ebx
		pop ebx
jmp_10061cd6:
		rep stosb
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061c9c */
		_emit 0xbd
		_emit 0x73 /* jae jmp_10061d06 */
		_emit 0x25
		_emit 0x74 /* je jmp_10061c87 */
		_emit 0xa4
jmp_10061ce3:
		movzx ecx, al
		xor eax, eax
		or ecx, ecx
		_emit 0x74 /* je jmp_10061cfb */
		_emit 0x0f
jmp_10061cec:
		mov al, byte ptr [esi]
		inc esi
		mov al, byte ptr [g_remapTable + eax]
		mov byte ptr [edi], al
		inc edi
		dec ecx
		_emit 0x75 /* jne jmp_10061cec */
		_emit 0xf1
jmp_10061cfb:
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10061c9c */
		_emit 0x9a
		_emit 0x75 /* jne jmp_10061ce3 */
		_emit 0xdf
		_emit 0x72 /* jb jmp_10061c87 */
		_emit 0x81
jmp_10061d06:
		add edx, p_unk0x10
		mov edi, edx
		dec ebx
		jne jmp_10061c7c
jmp_10061d12:
		xor eax, eax
		pop es
	}
}

// Measures shape frame p_frame at p_x, p_y. Returns 0.
#ifdef COMPAT_MODE
MechS32 FUN_100628c6(
	void* p_shape,
	MechS32 p_frame,
	MechS32 p_x,
	MechS32 p_y,
	undefined4 p_unk0x10,
	undefined4 p_unk0x14
)
{
	STUB(0x100628c6);
	return 0;
}
#else
// FUNCTION: MW2 0x100628c6
__declspec(naked) MechS32
FUN_100628c6(void* p_shape, MechS32 p_frame, MechS32 p_x, MechS32 p_y, undefined4 p_unk0x10, undefined4 p_unk0x14)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x24
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov dword ptr [ebp - 0x18], 0
		mov dword ptr [ebp - 0x1c], 0
		mov dword ptr [ebp - 0x20], 0
		mov dword ptr [ebp - 0x24], 0
		mov esi, dword ptr [ebp + 0xc]
		shl esi, 3
		add esi, 8
		add esi, dword ptr [ebp + 8]
		mov esi, dword ptr [esi]
		add esi, dword ptr [ebp + 8]
		mov dword ptr [ebp - 4], esi
		mov esi, dword ptr [ebp - 4]
		mov eax, dword ptr [esi + 8]
		add eax, dword ptr [ebp + 0x10]
		mov dword ptr [ebp - 8], eax
		mov eax, dword ptr [esi + 0xc]
		mov ebx, eax
		add eax, dword ptr [ebp + 0x14]
		mov dword ptr [ebp - 0xc], eax
		mov eax, dword ptr [esi + 0x10]
		add eax, dword ptr [ebp + 0x10]
		mov dword ptr [ebp - 0x10], eax
		mov eax, dword ptr [esi + 0x14]
		mov ecx, eax
		add ecx, dword ptr [ebp + 0x14]
		mov dword ptr [ebp - 0x14], ecx
		inc eax
		sub eax, ebx
		mov ebx, eax
		jle jmp_100629eb
		add esi, 0x18
		mov dword ptr [ebp - 0x18], 0x7fffffff
		mov dword ptr [ebp - 0x1c], 0x7fffffff
		mov dword ptr [ebp - 0x20], 0x80000000
		mov dword ptr [ebp - 0x24], 0x80000000
		mov edx, dword ptr [ebp - 0xc]
jmp_1006295b:
		mov edi, dword ptr [ebp - 8]
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_1006297e */
		_emit 0x19
		_emit 0x75 /* jne jmp_100629b1 */
		_emit 0x4a
		_emit 0x73 /* jae jmp_100629e3 */
		_emit 0x7a
jmp_10062969:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_1006297e */
		_emit 0x06
		_emit 0x75 /* jne jmp_100629b1 */
		_emit 0x37
		_emit 0x72 /* jb jmp_10062969 */
		_emit 0xed
		_emit 0x73 /* jae jmp_100629e3 */
		_emit 0x65
jmp_1006297e:
		movzx ecx, al
		mov al, byte ptr [esi]
		inc esi
		cmp dword ptr [ebp - 0x18], edi
		_emit 0x7c /* jl jmp_1006298c */
		_emit 0x03
		mov dword ptr [ebp - 0x18], edi
jmp_1006298c:
		add edi, ecx
		cmp dword ptr [ebp - 0x20], edi
		_emit 0x7f /* jg jmp_10062996 */
		_emit 0x03
		mov dword ptr [ebp - 0x20], edi
jmp_10062996:
		cmp dword ptr [ebp - 0x1c], edx
		_emit 0x7c /* jl jmp_1006299e */
		_emit 0x03
		mov dword ptr [ebp - 0x1c], edx
jmp_1006299e:
		cmp dword ptr [ebp - 0x24], edx
		_emit 0x7f /* jg jmp_100629a6 */
		_emit 0x03
		mov dword ptr [ebp - 0x24], edx
jmp_100629a6:
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_1006297e */
		_emit 0xd1
		_emit 0x73 /* jae jmp_100629e3 */
		_emit 0x34
		_emit 0x74 /* je jmp_10062969 */
		_emit 0xb8
jmp_100629b1:
		movzx ecx, al
		add esi, ecx
		cmp dword ptr [ebp - 0x18], edi
		_emit 0x7c /* jl jmp_100629be */
		_emit 0x03
		mov dword ptr [ebp - 0x18], edi
jmp_100629be:
		add edi, ecx
		cmp dword ptr [ebp - 0x20], edi
		_emit 0x7f /* jg jmp_100629c8 */
		_emit 0x03
		mov dword ptr [ebp - 0x20], edi
jmp_100629c8:
		cmp dword ptr [ebp - 0x1c], edx
		_emit 0x7c /* jl jmp_100629d0 */
		_emit 0x03
		mov dword ptr [ebp - 0x1c], edx
jmp_100629d0:
		cmp dword ptr [ebp - 0x24], edx
		_emit 0x7f /* jg jmp_100629d8 */
		_emit 0x03
		mov dword ptr [ebp - 0x24], edx
jmp_100629d8:
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_1006297e */
		_emit 0x9f
		_emit 0x75 /* jne jmp_100629b1 */
		_emit 0xd0
		_emit 0x72 /* jb jmp_10062969 */
		_emit 0x86
jmp_100629e3:
		inc edx
		dec ebx
		jne jmp_1006295b
jmp_100629eb:
		mov eax, dword ptr [ebp + 0x18]
		and eax, 1
		_emit 0x74 /* je jmp_10062a07 */
		_emit 0x14
		mov eax, dword ptr [ebp + 0x10]
		add eax, dword ptr [ebp + 0x10]
		mov ebx, eax
		sub eax, dword ptr [ebp - 0x18]
		sub ebx, dword ptr [ebp - 0x20]
		mov dword ptr [ebp - 0x20], eax
		mov dword ptr [ebp - 0x18], ebx
jmp_10062a07:
		mov eax, dword ptr [ebp + 0x18]
		and eax, 2
		_emit 0x74 /* je jmp_10062a23 */
		_emit 0x14
		mov eax, dword ptr [ebp + 0x14]
		add eax, dword ptr [ebp + 0x14]
		mov ebx, eax
		sub eax, dword ptr [ebp - 0x1c]
		sub ebx, dword ptr [ebp - 0x24]
		mov dword ptr [ebp - 0x24], eax
		mov dword ptr [ebp - 0x1c], ebx
jmp_10062a23:
		mov edi, dword ptr [ebp + 0x1c]
		mov eax, dword ptr [ebp - 0x18]
		stosd
		mov eax, dword ptr [ebp - 0x1c]
		stosd
		mov eax, dword ptr [ebp - 0x20]
		stosd
		mov eax, dword ptr [ebp - 0x24]
		stosd
		xor eax, eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Translates shape frame p_frame's pixels through the colour map g_remapTable, in place. Returns 0.
// FUNCTION: MW2 0x10062cc1
MechS32 RemapShpFrame(void* p_shape, MechS32 p_frame)
{
	__asm {
		push es
		cld
		push ds
		pop es
		mov esi, p_frame
		shl esi, 3
		add esi, 8
		add esi, p_shape
		mov esi, dword ptr [esi]
		add esi, p_shape
		mov ebx, dword ptr [esi + 0xc]
		mov eax, dword ptr [esi + 0x14]
		inc eax
		sub eax, ebx
		mov ebx, eax
		_emit 0x7e /* jle jmp_10062d4b */
		_emit 0x62
		add esi, 0x18
jmp_10062cec:
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10062d0a */
		_emit 0x17
		_emit 0x75 /* jne jmp_10062d28 */
		_emit 0x33
		_emit 0x73 /* jae jmp_10062d48 */
		_emit 0x51
jmp_10062cf7:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10062d0a */
		_emit 0x06
		_emit 0x75 /* jne jmp_10062d28 */
		_emit 0x22
		_emit 0x72 /* jb jmp_10062cf7 */
		_emit 0xef
		_emit 0x73 /* jae jmp_10062d48 */
		_emit 0x3e
jmp_10062d0a:
		movzx ecx, al
		mov eax, 0
		mov al, byte ptr [esi]
		mov al, byte ptr [g_remapTable + eax]
		mov byte ptr [esi], al
		inc esi
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10062d0a */
		_emit 0xe6
		_emit 0x73 /* jae jmp_10062d48 */
		_emit 0x22
		_emit 0x74 /* je jmp_10062cf7 */
		_emit 0xcf
jmp_10062d28:
		movzx ecx, al
		mov eax, 0
jmp_10062d30:
		mov al, byte ptr [esi]
		mov al, byte ptr [g_remapTable + eax]
		mov byte ptr [esi], al
		inc esi
		loop jmp_10062d30
		mov al, byte ptr [esi]
		inc esi
		shr al, 1
		_emit 0x77 /* ja jmp_10062d0a */
		_emit 0xc6
		_emit 0x75 /* jne jmp_10062d28 */
		_emit 0xe2
		_emit 0x72 /* jb jmp_10062cf7 */
		_emit 0xaf
jmp_10062d48:
		dec ebx
		_emit 0x75 /* jne jmp_10062cec */
		_emit 0xa1
jmp_10062d4b:
		xor eax, eax
		pop es
	}
}

#ifdef COMPAT_MODE
void FillView(RenderTarget* p_target, MechS32 p_color)
{
	STUB(0x100630b9);
}
#else
// FUNCTION: MW2 0x100630b9
__declspec(naked) void FillView(RenderTarget* p_target, MechS32 p_color)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x24
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp + 8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx + 4]
		inc eax
		mov dword ptr [ebp - 0x18], eax
		_emit 0x7e /* jle jmp_10063138 */
		_emit 0x64
		mov eax, dword ptr [ebx + 8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10063138 */
		_emit 0x5c
		mov eax, dword ptr [esi + 4]
		mov dword ptr [ebp - 0x1c], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_100630ec */
		_emit 0x05
		mov eax, 0
jmp_100630ec:
		mov dword ptr [ebp - 4], eax
		mov eax, dword ptr [esi + 8]
		mov dword ptr [ebp - 0x20], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_100630ff */
		_emit 0x05
		mov eax, 0
jmp_100630ff:
		mov dword ptr [ebp - 8], eax
		mov eax, dword ptr [esi + 0xc]
		mov edx, dword ptr [ebp - 0x18]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_1006310f */
		_emit 0x02
		mov eax, edx
jmp_1006310f:
		mov dword ptr [ebp - 0xc], eax
		mov eax, dword ptr [esi + 0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_1006311e */
		_emit 0x02
		mov eax, edx
jmp_1006311e:
		mov dword ptr [ebp - 0x10], eax
		mov eax, dword ptr [ebp - 0xc]
		cmp eax, dword ptr [ebp - 4]
		_emit 0x7c /* jl jmp_10063143 */
		_emit 0x1a
		mov eax, dword ptr [ebp - 0x10]
		cmp eax, dword ptr [ebp - 8]
		_emit 0x7c /* jl jmp_10063143 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp - 0x14], eax
		jmp short jmp_1006314e
jmp_10063138:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10063143:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1006314e:
		mov eax, dword ptr [ebp - 8]
		imul dword ptr [ebp - 0x18]
		add eax, dword ptr [ebp - 0x14]
		add eax, dword ptr [ebp - 4]
		mov edi, eax
		mov ebx, dword ptr [ebp - 0xc]
		inc ebx
		sub ebx, dword ptr [ebp - 4]
		mov esi, dword ptr [ebp - 0x18]
		sub esi, ebx
		mov al, byte ptr [ebp + 0xc]
		mov ah, al
		shl eax, 0x10
		mov al, byte ptr [ebp + 0xc]
		mov ah, al
		mov edx, dword ptr [ebp - 8]
		mov dword ptr [ebp - 0x24], ebx
		cmp ebx, 4
		_emit 0x7e /* jle jmp_10063189 */
		_emit 0x09
		jmp short jmp_100631af
jmp_10063182:
		mov ecx, ebx
		rep stosb
		add edi, esi
		inc edx
jmp_10063189:
		cmp edx, dword ptr [ebp - 0x10]
		_emit 0x7e /* jle jmp_10063182 */
		_emit 0xf4
		jmp short jmp_100631b4
jmp_10063190:
		mov ecx, edi
		mov ebx, dword ptr [ebp - 0x24]
		neg ecx
		and ecx, 3
		sub ebx, ecx
		rep stosb
		mov ecx, ebx
		and ebx, 3
		shr ecx, 2
		rep stosd
		mov ecx, ebx
		rep stosb
		add edi, esi
		inc edx
jmp_100631af:
		cmp edx, dword ptr [ebp - 0x10]
		_emit 0x7e /* jle jmp_10063190 */
		_emit 0xdc
jmp_100631b4:
		xor eax, eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Draws a rectangle into p_target, clipped. Returns a negative code for an empty target or rectangle.
#ifdef COMPAT_MODE
MechS32 BlitView(
	RenderTarget* p_target,
	MechS32 p_x,
	MechS32 p_y,
	MechS32 p_width,
	MechS32 p_height,
	MechS32 p_unk0x14,
	MechS32 p_unk0x18
)
{
	STUB(0x100631bc);
	return 0;
}
#else
// FUNCTION: MW2 0x100631bc
__declspec(naked) MechS32 BlitView(
	RenderTarget* p_target,
	MechS32 p_x,
	MechS32 p_y,
	MechS32 p_width,
	MechS32 p_height,
	MechS32 p_unk0x14,
	MechS32 p_unk0x18
)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, 0xffffff64
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp + 8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx + 4]
		inc eax
		mov dword ptr [ebp - 0x70], eax
		_emit 0x7e /* jle jmp_1006323e */
		_emit 0x64
		mov eax, dword ptr [ebx + 8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_1006323e */
		_emit 0x5c
		mov eax, dword ptr [esi + 4]
		mov dword ptr [ebp - 0x78], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_100631f2 */
		_emit 0x05
		mov eax, 0
jmp_100631f2:
		mov dword ptr [ebp - 0x60], eax
		mov eax, dword ptr [esi + 8]
		mov dword ptr [ebp - 0x7c], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_10063205 */
		_emit 0x05
		mov eax, 0
jmp_10063205:
		mov dword ptr [ebp - 0x64], eax
		mov eax, dword ptr [esi + 0xc]
		mov edx, dword ptr [ebp - 0x70]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10063215 */
		_emit 0x02
		mov eax, edx
jmp_10063215:
		mov dword ptr [ebp - 0x68], eax
		mov eax, dword ptr [esi + 0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10063224 */
		_emit 0x02
		mov eax, edx
jmp_10063224:
		mov dword ptr [ebp - 0x6c], eax
		mov eax, dword ptr [ebp - 0x68]
		cmp eax, dword ptr [ebp - 0x60]
		_emit 0x7c /* jl jmp_10063249 */
		_emit 0x1a
		mov eax, dword ptr [ebp - 0x6c]
		cmp eax, dword ptr [ebp - 0x64]
		_emit 0x7c /* jl jmp_10063249 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp - 0x74], eax
		jmp short jmp_10063254
jmp_1006323e:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10063249:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10063254:
		mov eax, dword ptr [ebp - 0x60]
		mov dword ptr [ebp - 0x2c], eax
		mov eax, dword ptr [ebp - 0x64]
		mov dword ptr [ebp - 0x30], eax
		mov eax, dword ptr [ebp - 0x68]
		mov dword ptr [ebp - 0x34], eax
		mov eax, dword ptr [ebp - 0x6c]
		mov dword ptr [ebp - 0x38], eax
		mov eax, dword ptr [ebp - 0x78]
		sub dword ptr [ebp - 0x2c], eax
		sub dword ptr [ebp - 0x34], eax
		mov eax, dword ptr [ebp - 0x7c]
		sub dword ptr [ebp - 0x30], eax
		sub dword ptr [ebp - 0x38], eax
		mov esi, dword ptr [ebp + 0x14]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx + 4]
		inc eax
		mov dword ptr [ebp - 0x94], eax
		jle jmp_10063315
		mov eax, dword ptr [ebx + 8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10063315 */
		_emit 0x7a
		mov eax, dword ptr [esi + 4]
		mov dword ptr [ebp - 0x98], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_100632ae */
		_emit 0x05
		mov eax, 0
jmp_100632ae:
		mov dword ptr [ebp - 0x80], eax
		mov eax, dword ptr [esi + 8]
		mov dword ptr [ebp - 0x9c], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_100632c4 */
		_emit 0x05
		mov eax, 0
jmp_100632c4:
		mov dword ptr [ebp - 0x84], eax
		mov eax, dword ptr [esi + 0xc]
		mov edx, dword ptr [ebp - 0x94]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_100632da */
		_emit 0x02
		mov eax, edx
jmp_100632da:
		mov dword ptr [ebp - 0x88], eax
		mov eax, dword ptr [esi + 0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_100632ec */
		_emit 0x02
		mov eax, edx
jmp_100632ec:
		mov dword ptr [ebp - 0x8c], eax
		mov eax, dword ptr [ebp - 0x88]
		cmp eax, dword ptr [ebp - 0x80]
		_emit 0x7c /* jl jmp_10063320 */
		_emit 0x23
		mov eax, dword ptr [ebp - 0x8c]
		cmp eax, dword ptr [ebp - 0x84]
		_emit 0x7c /* jl jmp_10063320 */
		_emit 0x15
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp - 0x90], eax
		jmp short jmp_1006332b
jmp_10063315:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10063320:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1006332b:
		mov eax, dword ptr [ebp - 0x80]
		mov dword ptr [ebp - 0x40], eax
		mov eax, dword ptr [ebp - 0x84]
		mov dword ptr [ebp - 0x44], eax
		mov eax, dword ptr [ebp - 0x88]
		mov dword ptr [ebp - 0x48], eax
		mov eax, dword ptr [ebp - 0x8c]
		mov dword ptr [ebp - 0x4c], eax
		mov eax, dword ptr [ebp - 0x98]
		sub dword ptr [ebp - 0x40], eax
		sub dword ptr [ebp - 0x48], eax
		mov eax, dword ptr [ebp - 0x9c]
		sub dword ptr [ebp - 0x44], eax
		sub dword ptr [ebp - 0x4c], eax
		mov eax, dword ptr [ebp + 0xc]
		sub eax, dword ptr [ebp + 0x18]
		mov dword ptr [ebp - 0x24], eax
		mov eax, dword ptr [ebp + 0x10]
		sub eax, dword ptr [ebp + 0x1c]
		mov dword ptr [ebp - 0x28], eax
		mov eax, dword ptr [ebp - 0x2c]
		mov edx, dword ptr [ebp - 0x40]
		add edx, dword ptr [ebp - 0x24]
		cmp eax, edx
		_emit 0x7f /* jg jmp_10063385 */
		_emit 0x02
		mov eax, edx
jmp_10063385:
		mov dword ptr [ebp - 4], eax
		mov eax, dword ptr [ebp - 0x30]
		mov edx, dword ptr [ebp - 0x44]
		add edx, dword ptr [ebp - 0x28]
		cmp eax, edx
		_emit 0x7f /* jg jmp_10063397 */
		_emit 0x02
		mov eax, edx
jmp_10063397:
		mov dword ptr [ebp - 8], eax
		mov eax, dword ptr [ebp - 0x34]
		mov edx, dword ptr [ebp - 0x48]
		add edx, dword ptr [ebp - 0x24]
		cmp eax, edx
		_emit 0x7c /* jl jmp_100633a9 */
		_emit 0x02
		mov eax, edx
jmp_100633a9:
		mov dword ptr [ebp - 0xc], eax
		mov eax, dword ptr [ebp - 0x38]
		mov edx, dword ptr [ebp - 0x4c]
		add edx, dword ptr [ebp - 0x28]
		cmp eax, edx
		_emit 0x7c /* jl jmp_100633bb */
		_emit 0x02
		mov eax, edx
jmp_100633bb:
		mov dword ptr [ebp - 0x10], eax
		mov eax, dword ptr [ebp - 0xc]
		cmp eax, dword ptr [ebp - 4]
		jl jmp_1006354d
		mov eax, dword ptr [ebp - 0x10]
		cmp eax, dword ptr [ebp - 8]
		jl jmp_1006354d
		mov eax, dword ptr [ebp - 0x40]
		mov edx, dword ptr [ebp - 0x2c]
		sub edx, dword ptr [ebp - 0x24]
		cmp eax, edx
		_emit 0x7f /* jg jmp_100633e5 */
		_emit 0x02
		mov eax, edx
jmp_100633e5:
		mov dword ptr [ebp - 0x14], eax
		mov eax, dword ptr [ebp - 0x44]
		mov edx, dword ptr [ebp - 0x30]
		sub edx, dword ptr [ebp - 0x28]
		cmp eax, edx
		_emit 0x7f /* jg jmp_100633f7 */
		_emit 0x02
		mov eax, edx
jmp_100633f7:
		mov dword ptr [ebp - 0x18], eax
		mov eax, dword ptr [ebp - 0x48]
		mov edx, dword ptr [ebp - 0x34]
		sub edx, dword ptr [ebp - 0x24]
		cmp eax, edx
		_emit 0x7c /* jl jmp_10063409 */
		_emit 0x02
		mov eax, edx
jmp_10063409:
		mov dword ptr [ebp - 0x1c], eax
		mov eax, dword ptr [ebp - 0x4c]
		mov edx, dword ptr [ebp - 0x38]
		sub edx, dword ptr [ebp - 0x28]
		cmp eax, edx
		_emit 0x7c /* jl jmp_1006341b */
		_emit 0x02
		mov eax, edx
jmp_1006341b:
		mov dword ptr [ebp - 0x20], eax
		mov eax, dword ptr [ebp - 0xc]
		inc eax
		sub eax, dword ptr [ebp - 4]
		mov dword ptr [ebp - 0x58], eax
		mov eax, dword ptr [ebp - 0x10]
		inc eax
		sub eax, dword ptr [ebp - 8]
		mov dword ptr [ebp - 0x54], eax
		mov eax, dword ptr [ebp - 0x7c]
		imul dword ptr [ebp - 0x70]
		add eax, dword ptr [ebp - 0x74]
		add eax, dword ptr [ebp - 0x78]
		mov esi, eax
		mov eax, dword ptr [ebp - 0x9c]
		imul dword ptr [ebp - 0x94]
		add eax, dword ptr [ebp - 0x90]
		add eax, dword ptr [ebp - 0x98]
		mov edi, eax
		mov eax, dword ptr [ebp - 8]
		mov ebx, dword ptr [ebp - 0x18]
		cmp eax, ebx
		_emit 0x7e /* jle jmp_10063484 */
		_emit 0x20
		mul dword ptr [ebp - 0x70]
		add esi, eax
		mov eax, ebx
		mul dword ptr [ebp - 0x94]
		add edi, eax
		mov eax, dword ptr [ebp - 0x70]
		mov dword ptr [ebp - 0x3c], eax
		mov eax, dword ptr [ebp - 0x94]
		mov dword ptr [ebp - 0x50], eax
		jmp short jmp_100634aa
jmp_10063484:
		mov eax, dword ptr [ebp - 0x10]
		mul dword ptr [ebp - 0x70]
		add esi, eax
		mov eax, dword ptr [ebp - 0x20]
		mul dword ptr [ebp - 0x94]
		add edi, eax
		mov eax, dword ptr [ebp - 0x70]
		neg eax
		mov dword ptr [ebp - 0x3c], eax
		mov eax, dword ptr [ebp - 0x94]
		neg eax
		mov dword ptr [ebp - 0x50], eax
jmp_100634aa:
		mov ecx, dword ptr [ebp - 0x58]
		mov eax, dword ptr [ebp - 4]
		mov ebx, dword ptr [ebp - 0x14]
		cmp eax, ebx
		_emit 0x7e /* jle jmp_100634cb */
		_emit 0x14
		add esi, eax
		add edi, ebx
		sub dword ptr [ebp - 0x3c], ecx
		sub dword ptr [ebp - 0x50], ecx
		cld
		mov dword ptr [ebp - 0x5c], 0
		jmp short jmp_100634df
jmp_100634cb:
		add esi, dword ptr [ebp - 0xc]
		add edi, dword ptr [ebp - 0x1c]
		add dword ptr [ebp - 0x3c], ecx
		add dword ptr [ebp - 0x50], ecx
		std
		mov dword ptr [ebp - 0x5c], 3
jmp_100634df:
		mov eax, dword ptr [ebp + 0x20]
		test eax, 0xffffff00
		_emit 0x74 /* je jmp_10063518 */
		_emit 0x2f
		mov edx, dword ptr [ebp - 0x54]
		mov eax, dword ptr [ebp - 0x3c]
		mov ebx, dword ptr [ebp - 0x50]
jmp_100634f2:
		mov ecx, dword ptr [ebp - 0x58]
		and ecx, 3
		rep movsb
		mov ecx, dword ptr [ebp - 0x58]
		shr ecx, 2
		sub esi, dword ptr [ebp - 0x5c]
		sub edi, dword ptr [ebp - 0x5c]
		rep movsd
		add esi, dword ptr [ebp - 0x5c]
		add edi, dword ptr [ebp - 0x5c]
		add esi, eax
		add edi, ebx
		dec edx
		_emit 0x75 /* jne jmp_100634f2 */
		_emit 0xdd
		cld
		jmp short jmp_10063545
jmp_10063518:
		mov dl, al
		mov ah, al
		shl eax, 0x10
		mov al, dl
		mov ah, al
		mov edx, dword ptr [ebp - 0x54]
		mov ebx, dword ptr [ebp - 0x50]
jmp_10063529:
		mov ecx, dword ptr [ebp - 0x58]
		and ecx, 3
		rep stosb
		mov ecx, dword ptr [ebp - 0x58]
		shr ecx, 2
		sub edi, dword ptr [ebp - 0x5c]
		rep stosd
		add edi, dword ptr [ebp - 0x5c]
		add edi, ebx
		dec edx
		_emit 0x75 /* jne jmp_10063529 */
		_emit 0xe5
		cld
jmp_10063545:
		xor eax, eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1006354d:
		mov eax, 0xfffffffd
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Draws through BlitView. Returns a negative code for an empty target or rectangle.
#ifdef COMPAT_MODE
MechS32 ScrollView(RenderTarget* p_target, MechS32 p_x, MechS32 p_y, MechS32 p_width, MechS32 p_height)
{
	STUB(0x10063558);
	return 0;
}
#else
// FUNCTION: MW2 0x10063558
__declspec(naked) MechS32
ScrollView(RenderTarget* p_target, MechS32 p_x, MechS32 p_y, MechS32 p_width, MechS32 p_height)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x44
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp + 8]
		mov eax, dword ptr [esi + 0xc]
		inc eax
		sub eax, dword ptr [esi + 4]
		mov dword ptr [ebp - 0x30], eax
		jle jmp_1006374a
		mov edx, dword ptr [esi + 0x10]
		inc edx
		sub edx, dword ptr [esi + 8]
		mov dword ptr [ebp - 0x34], edx
		jle jmp_1006374a
		neg eax
		mov dword ptr [ebp - 0x38], eax
		neg edx
		mov dword ptr [ebp - 0x3c], edx
		cmp dword ptr [ebp + 0x14], 1
		_emit 0x74 /* je jmp_100635df */
		_emit 0x47
		mov eax, dword ptr [ebp + 0xc]
		cdq
		xor eax, edx
		sub eax, edx
		cmp eax, dword ptr [ebp - 0x30]
		_emit 0x7d /* jge jmp_100635c3 */
		_emit 0x1e
		mov eax, dword ptr [ebp + 0x10]
		cdq
		xor eax, edx
		sub eax, edx
		cmp eax, dword ptr [ebp - 0x34]
		_emit 0x7d /* jge jmp_100635c3 */
		_emit 0x11
		mov eax, dword ptr [ebp + 8]
		mov dword ptr [ebp - 0x2c], eax
		mov eax, dword ptr [ebp + 0x18]
		mov dword ptr [ebp - 0x44], eax
		jmp jmp_1006364a
jmp_100635c3:
		mov eax, dword ptr [ebp + 0x18]
		push 0
		movzx ax, al
		push ax
		push dword ptr [ebp + 8]
		call FillView
		add esp, 8
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_100635df:
		cmp dword ptr [ebp + 0x18], 0
		je jmp_1006373e
		lea eax, [ebp - 0x28]
		mov dword ptr [ebp - 0x2c], eax
		mov eax, dword ptr [ebp + 0x18]
		mov dword ptr [ebp - 0x14], eax
		lea eax, [ebp - 0x14]
		mov dword ptr [ebp - 0x28], eax
		xor eax, eax
		mov dword ptr [ebp - 0x24], eax
		mov dword ptr [ebp - 0x20], eax
		mov eax, dword ptr [ebp - 0x30]
		dec eax
		mov dword ptr [ebp - 0x10], eax
		mov dword ptr [ebp - 0x1c], eax
		mov eax, dword ptr [ebp - 0x34]
		dec eax
		mov dword ptr [ebp - 0xc], eax
		mov dword ptr [ebp - 0x18], eax
		push -1
		push 0
		push 0
		push dword ptr [ebp - 0x2c]
		push 0
		push 0
		push dword ptr [ebp + 8]
		call BlitView
		add esp, 0x1c
		mov eax, dword ptr [ebp + 0xc]
		cdq
		idiv dword ptr [ebp - 0x30]
		mov dword ptr [ebp + 0xc], edx
		mov eax, dword ptr [ebp + 0x10]
		cdq
		idiv dword ptr [ebp - 0x34]
		mov dword ptr [ebp + 0x10], edx
		mov dword ptr [ebp - 0x44], 0xffffffff
jmp_1006364a:
		mov eax, dword ptr [ebp + 0xc]
		or eax, dword ptr [ebp + 0x10]
		je jmp_10063736
		mov esi, dword ptr [ebp - 0x2c]
		mov edi, dword ptr [ebp + 8]
		push -1
		push dword ptr [ebp + 0x10]
		push dword ptr [ebp + 0xc]
		push edi
		push 0
		push 0
		push esi
		call BlitView
		add esp, 0x1c
		push dword ptr [ebp - 0x44]
		push dword ptr [ebp + 0x10]
		push dword ptr [ebp + 0xc]
		push edi
		push dword ptr [ebp - 0x34]
		push dword ptr [ebp - 0x30]
		push esi
		call BlitView
		add esp, 0x1c
		push dword ptr [ebp - 0x44]
		push dword ptr [ebp + 0x10]
		push dword ptr [ebp + 0xc]
		push edi
		push 0
		push dword ptr [ebp - 0x30]
		push esi
		call BlitView
		add esp, 0x1c
		push dword ptr [ebp - 0x44]
		push dword ptr [ebp + 0x10]
		push dword ptr [ebp + 0xc]
		push edi
		push dword ptr [ebp - 0x3c]
		push dword ptr [ebp - 0x30]
		push esi
		call BlitView
		add esp, 0x1c
		push dword ptr [ebp - 0x44]
		push dword ptr [ebp + 0x10]
		push dword ptr [ebp + 0xc]
		push edi
		push dword ptr [ebp - 0x34]
		push 0
		push esi
		call BlitView
		add esp, 0x1c
		push dword ptr [ebp - 0x44]
		push dword ptr [ebp + 0x10]
		push dword ptr [ebp + 0xc]
		push edi
		push dword ptr [ebp - 0x3c]
		push 0
		push esi
		call BlitView
		add esp, 0x1c
		push dword ptr [ebp - 0x44]
		push dword ptr [ebp + 0x10]
		push dword ptr [ebp + 0xc]
		push edi
		push dword ptr [ebp - 0x34]
		push dword ptr [ebp - 0x38]
		push esi
		call BlitView
		add esp, 0x1c
		push dword ptr [ebp - 0x44]
		push dword ptr [ebp + 0x10]
		push dword ptr [ebp + 0xc]
		push edi
		push 0
		push dword ptr [ebp - 0x38]
		push esi
		call BlitView
		add esp, 0x1c
		push dword ptr [ebp - 0x44]
		push dword ptr [ebp + 0x10]
		push dword ptr [ebp + 0xc]
		push edi
		push dword ptr [ebp - 0x3c]
		push dword ptr [ebp - 0x38]
		push esi
		call BlitView
		add esp, 0x1c
jmp_10063736:
		xor eax, eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1006373e:
		mov eax, dword ptr [ebp - 0x30]
		mul dword ptr [ebp - 0x34]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1006374a:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

#ifdef COMPAT_MODE
void DrawEllipse(
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
#else
// FUNCTION: MW2 0x10063755
__declspec(naked) void DrawEllipse(
	RenderTarget* p_target,
	MechS32 p_centerX,
	MechS32 p_centerY,
	MechS32 p_radiusX,
	MechS32 p_radiusY,
	MechS32 p_color
)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x54
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		cmp dword ptr [ebp + 0x14], 0
		_emit 0x74 /* je jmp_1006376e */
		_emit 0x06
		cmp dword ptr [ebp + 0x18], 0
		_emit 0x75 /* jne jmp_1006379f */
		_emit 0x31
jmp_1006376e:
		mov eax, dword ptr [ebp + 0x10]
		add eax, dword ptr [ebp + 0x18]
		mov ebx, dword ptr [ebp + 0xc]
		add ebx, dword ptr [ebp + 0x14]
		mov ecx, dword ptr [ebp + 0x10]
		sub ecx, dword ptr [ebp + 0x18]
		mov edx, dword ptr [ebp + 0xc]
		sub edx, dword ptr [ebp + 0x14]
		push dword ptr [ebp + 0x1c]
		push 0
		push eax
		push ebx
		push ecx
		push edx
		push dword ptr [ebp + 8]
		call BlitLine
		add esp, 0x1c
		jmp jmp_10063a90
jmp_1006379f:
		mov esi, dword ptr [ebp + 8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx + 4]
		inc eax
		mov dword ptr [ebp - 0x4c], eax
		_emit 0x7e /* jle jmp_10063811 */
		_emit 0x64
		mov eax, dword ptr [ebx + 8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10063811 */
		_emit 0x5c
		mov eax, dword ptr [esi + 4]
		mov dword ptr [ebp - 0x50], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_100637c5 */
		_emit 0x05
		mov eax, 0
jmp_100637c5:
		mov dword ptr [ebp - 0x38], eax
		mov eax, dword ptr [esi + 8]
		mov dword ptr [ebp - 0x54], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_100637d8 */
		_emit 0x05
		mov eax, 0
jmp_100637d8:
		mov dword ptr [ebp - 0x3c], eax
		mov eax, dword ptr [esi + 0xc]
		mov edx, dword ptr [ebp - 0x4c]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_100637e8 */
		_emit 0x02
		mov eax, edx
jmp_100637e8:
		mov dword ptr [ebp - 0x40], eax
		mov eax, dword ptr [esi + 0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_100637f7 */
		_emit 0x02
		mov eax, edx
jmp_100637f7:
		mov dword ptr [ebp - 0x44], eax
		mov eax, dword ptr [ebp - 0x40]
		cmp eax, dword ptr [ebp - 0x38]
		_emit 0x7c /* jl jmp_1006381c */
		_emit 0x1a
		mov eax, dword ptr [ebp - 0x44]
		cmp eax, dword ptr [ebp - 0x3c]
		_emit 0x7c /* jl jmp_1006381c */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp - 0x48], eax
		jmp short jmp_10063827
jmp_10063811:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1006381c:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10063827:
		mov eax, dword ptr [ebp + 0x1c]
		mov ah, al
		mov dword ptr [ebp + 0x1c], eax
		mov word ptr [ebp + 0x1e], ax
		mov eax, dword ptr [ebp - 0x50]
		add dword ptr [ebp + 0xc], eax
		mov eax, dword ptr [ebp - 0x54]
		add dword ptr [ebp + 0x10], eax
		mov eax, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 4], eax
		mov eax, dword ptr [ebp + 0x10]
		mov dword ptr [ebp - 8], eax
		mov dword ptr [ebp - 0xc], 0
		mov eax, dword ptr [ebp + 0x18]
		mov dword ptr [ebp - 0x10], eax
		mul eax
		mov dword ptr [ebp - 0x1c], eax
		shl eax, 1
		mov dword ptr [ebp - 0x20], eax
		mov eax, dword ptr [ebp + 0x14]
		mul eax
		mov dword ptr [ebp - 0x14], eax
		shl eax, 1
		mov dword ptr [ebp - 0x18], eax
		mov dword ptr [ebp - 0x24], 0
		mov eax, dword ptr [ebp - 0x18]
		mul dword ptr [ebp + 0x18]
		mov dword ptr [ebp - 0x28], eax
		mov eax, dword ptr [ebp - 0x14]
		shr eax, 2
		add eax, dword ptr [ebp - 0x1c]
		mov dword ptr [ebp - 0x2c], eax
		mov eax, dword ptr [ebp - 0x14]
		mul dword ptr [ebp + 0x18]
		sub dword ptr [ebp - 0x2c], eax
		mov ebx, dword ptr [ebp + 0x18]
jmp_10063897:
		mov eax, dword ptr [ebp - 0x24]
		sub eax, dword ptr [ebp - 0x28]
		jns jmp_1006398d
		push ebx
		mov ecx, dword ptr [ebp + 0x1c]
		mov edi, dword ptr [ebp - 4]
		add edi, dword ptr [ebp - 0xc]
		mov edx, dword ptr [ebp - 8]
		add edx, dword ptr [ebp - 0x10]
		cmp edi, dword ptr [ebp - 0x38]
		_emit 0x7c /* jl jmp_100638d5 */
		_emit 0x1d
		cmp edi, dword ptr [ebp - 0x40]
		_emit 0x7f /* jg jmp_100638d5 */
		_emit 0x18
		cmp edx, dword ptr [ebp - 0x3c]
		_emit 0x7c /* jl jmp_100638d5 */
		_emit 0x13
		cmp edx, dword ptr [ebp - 0x44]
		_emit 0x7f /* jg jmp_100638d5 */
		_emit 0x0e
		mov eax, edx
		imul dword ptr [ebp - 0x4c]
		add eax, dword ptr [ebp - 0x48]
		add eax, edi
		mov edi, eax
		mov byte ptr [edi], cl
jmp_100638d5:
		mov edi, dword ptr [ebp - 4]
		add edi, dword ptr [ebp - 0xc]
		mov edx, dword ptr [ebp - 8]
		sub edx, dword ptr [ebp - 0x10]
		cmp edi, dword ptr [ebp - 0x38]
		_emit 0x7c /* jl jmp_10063903 */
		_emit 0x1d
		cmp edi, dword ptr [ebp - 0x40]
		_emit 0x7f /* jg jmp_10063903 */
		_emit 0x18
		cmp edx, dword ptr [ebp - 0x3c]
		_emit 0x7c /* jl jmp_10063903 */
		_emit 0x13
		cmp edx, dword ptr [ebp - 0x44]
		_emit 0x7f /* jg jmp_10063903 */
		_emit 0x0e
		mov eax, edx
		imul dword ptr [ebp - 0x4c]
		add eax, dword ptr [ebp - 0x48]
		add eax, edi
		mov edi, eax
		mov byte ptr [edi], cl
jmp_10063903:
		mov edi, dword ptr [ebp - 4]
		sub edi, dword ptr [ebp - 0xc]
		mov edx, dword ptr [ebp - 8]
		add edx, dword ptr [ebp - 0x10]
		cmp edi, dword ptr [ebp - 0x38]
		_emit 0x7c /* jl jmp_10063931 */
		_emit 0x1d
		cmp edi, dword ptr [ebp - 0x40]
		_emit 0x7f /* jg jmp_10063931 */
		_emit 0x18
		cmp edx, dword ptr [ebp - 0x3c]
		_emit 0x7c /* jl jmp_10063931 */
		_emit 0x13
		cmp edx, dword ptr [ebp - 0x44]
		_emit 0x7f /* jg jmp_10063931 */
		_emit 0x0e
		mov eax, edx
		imul dword ptr [ebp - 0x4c]
		add eax, dword ptr [ebp - 0x48]
		add eax, edi
		mov edi, eax
		mov byte ptr [edi], cl
jmp_10063931:
		mov edi, dword ptr [ebp - 4]
		sub edi, dword ptr [ebp - 0xc]
		mov edx, dword ptr [ebp - 8]
		sub edx, dword ptr [ebp - 0x10]
		cmp edi, dword ptr [ebp - 0x38]
		_emit 0x7c /* jl jmp_1006395f */
		_emit 0x1d
		cmp edi, dword ptr [ebp - 0x40]
		_emit 0x7f /* jg jmp_1006395f */
		_emit 0x18
		cmp edx, dword ptr [ebp - 0x3c]
		_emit 0x7c /* jl jmp_1006395f */
		_emit 0x13
		cmp edx, dword ptr [ebp - 0x44]
		_emit 0x7f /* jg jmp_1006395f */
		_emit 0x0e
		mov eax, edx
		imul dword ptr [ebp - 0x4c]
		add eax, dword ptr [ebp - 0x48]
		add eax, edi
		mov edi, eax
		mov byte ptr [edi], cl
jmp_1006395f:
		pop ebx
		cmp dword ptr [ebp - 0x2c], 0
		_emit 0x78 /* js jmp_10063976 */
		_emit 0x10
		dec dword ptr [ebp - 0x10]
		dec ebx
		mov eax, dword ptr [ebp - 0x28]
		sub eax, dword ptr [ebp - 0x18]
		mov dword ptr [ebp - 0x28], eax
		sub dword ptr [ebp - 0x2c], eax
jmp_10063976:
		inc dword ptr [ebp - 0xc]
		mov eax, dword ptr [ebp - 0x24]
		add eax, dword ptr [ebp - 0x20]
		mov dword ptr [ebp - 0x24], eax
		add eax, dword ptr [ebp - 0x1c]
		add dword ptr [ebp - 0x2c], eax
		_emit 0xe9 /* jmp jmp_10063897 */
		_emit 0x0a
		_emit 0xff
		_emit 0xff
		_emit 0xff
jmp_1006398d:
		mov eax, dword ptr [ebp - 0x14]
		sub eax, dword ptr [ebp - 0x1c]
		mov edx, eax
		sar eax, 1
		add eax, edx
		sub eax, dword ptr [ebp - 0x24]
		sub eax, dword ptr [ebp - 0x28]
		sar eax, 1
		add dword ptr [ebp - 0x2c], eax
jmp_100639a4:
		push ebx
		mov ecx, dword ptr [ebp + 0x1c]
		mov edi, dword ptr [ebp - 4]
		add edi, dword ptr [ebp - 0xc]
		mov edx, dword ptr [ebp - 8]
		add edx, dword ptr [ebp - 0x10]
		cmp edi, dword ptr [ebp - 0x38]
		_emit 0x7c /* jl jmp_100639d6 */
		_emit 0x1d
		cmp edi, dword ptr [ebp - 0x40]
		_emit 0x7f /* jg jmp_100639d6 */
		_emit 0x18
		cmp edx, dword ptr [ebp - 0x3c]
		_emit 0x7c /* jl jmp_100639d6 */
		_emit 0x13
		cmp edx, dword ptr [ebp - 0x44]
		_emit 0x7f /* jg jmp_100639d6 */
		_emit 0x0e
		mov eax, edx
		imul dword ptr [ebp - 0x4c]
		add eax, dword ptr [ebp - 0x48]
		add eax, edi
		mov edi, eax
		mov byte ptr [edi], cl
jmp_100639d6:
		mov edi, dword ptr [ebp - 4]
		add edi, dword ptr [ebp - 0xc]
		mov edx, dword ptr [ebp - 8]
		sub edx, dword ptr [ebp - 0x10]
		cmp edi, dword ptr [ebp - 0x38]
		_emit 0x7c /* jl jmp_10063a04 */
		_emit 0x1d
		cmp edi, dword ptr [ebp - 0x40]
		_emit 0x7f /* jg jmp_10063a04 */
		_emit 0x18
		cmp edx, dword ptr [ebp - 0x3c]
		_emit 0x7c /* jl jmp_10063a04 */
		_emit 0x13
		cmp edx, dword ptr [ebp - 0x44]
		_emit 0x7f /* jg jmp_10063a04 */
		_emit 0x0e
		mov eax, edx
		imul dword ptr [ebp - 0x4c]
		add eax, dword ptr [ebp - 0x48]
		add eax, edi
		mov edi, eax
		mov byte ptr [edi], cl
jmp_10063a04:
		mov edi, dword ptr [ebp - 4]
		sub edi, dword ptr [ebp - 0xc]
		mov edx, dword ptr [ebp - 8]
		add edx, dword ptr [ebp - 0x10]
		cmp edi, dword ptr [ebp - 0x38]
		_emit 0x7c /* jl jmp_10063a32 */
		_emit 0x1d
		cmp edi, dword ptr [ebp - 0x40]
		_emit 0x7f /* jg jmp_10063a32 */
		_emit 0x18
		cmp edx, dword ptr [ebp - 0x3c]
		_emit 0x7c /* jl jmp_10063a32 */
		_emit 0x13
		cmp edx, dword ptr [ebp - 0x44]
		_emit 0x7f /* jg jmp_10063a32 */
		_emit 0x0e
		mov eax, edx
		imul dword ptr [ebp - 0x4c]
		add eax, dword ptr [ebp - 0x48]
		add eax, edi
		mov edi, eax
		mov byte ptr [edi], cl
jmp_10063a32:
		mov edi, dword ptr [ebp - 4]
		sub edi, dword ptr [ebp - 0xc]
		mov edx, dword ptr [ebp - 8]
		sub edx, dword ptr [ebp - 0x10]
		cmp edi, dword ptr [ebp - 0x38]
		_emit 0x7c /* jl jmp_10063a60 */
		_emit 0x1d
		cmp edi, dword ptr [ebp - 0x40]
		_emit 0x7f /* jg jmp_10063a60 */
		_emit 0x18
		cmp edx, dword ptr [ebp - 0x3c]
		_emit 0x7c /* jl jmp_10063a60 */
		_emit 0x13
		cmp edx, dword ptr [ebp - 0x44]
		_emit 0x7f /* jg jmp_10063a60 */
		_emit 0x0e
		mov eax, edx
		imul dword ptr [ebp - 0x4c]
		add eax, dword ptr [ebp - 0x48]
		add eax, edi
		mov edi, eax
		mov byte ptr [edi], cl
jmp_10063a60:
		pop ebx
		cmp dword ptr [ebp - 0x2c], 0
		_emit 0x79 /* jns jmp_10063a76 */
		_emit 0x0f
		inc dword ptr [ebp - 0xc]
		mov eax, dword ptr [ebp - 0x24]
		add eax, dword ptr [ebp - 0x20]
		mov dword ptr [ebp - 0x24], eax
		add dword ptr [ebp - 0x2c], eax
jmp_10063a76:
		dec dword ptr [ebp - 0x10]
		mov eax, dword ptr [ebp - 0x28]
		sub eax, dword ptr [ebp - 0x18]
		mov dword ptr [ebp - 0x28], eax
		sub eax, dword ptr [ebp - 0x14]
		sub dword ptr [ebp - 0x2c], eax
		dec ebx
		_emit 0x78 /* js jmp_10063a90 */
		_emit 0x05
		_emit 0xe9 /* jmp jmp_100639a4 */
		_emit 0x14
		_emit 0xff
		_emit 0xff
		_emit 0xff
jmp_10063a90:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

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

// The 16.16 cosine of 0 to 90 degrees in tenths of a degree; read backwards, the sine. The
// original keeps it in .text, between FUN_10063a96 and GetCosSin.
// GLOBAL: MW2 0x10063d94
MechS32 g_cosTable[0x385] = {
	0x10000, 0x10000, 0x10000, 0xffff, 0xfffe, 0xfffe, 0xfffc, 0xfffb, 0xfffa, 0xfff8, 0xfff6, 0xfff4, 0xfff2, 0xffef,
	0xffec,  0xffea,  0xffe6,  0xffe3, 0xffe0, 0xffdc, 0xffd8, 0xffd4, 0xffd0, 0xffcb, 0xffc7, 0xffc2, 0xffbd, 0xffb7,
	0xffb2,  0xffac,  0xffa6,  0xffa0, 0xff9a, 0xff93, 0xff8d, 0xff86, 0xff7f, 0xff77, 0xff70, 0xff68, 0xff60, 0xff58,
	0xff50,  0xff48,  0xff3f,  0xff36, 0xff2d, 0xff24, 0xff1a, 0xff10, 0xff07, 0xfefd, 0xfef2, 0xfee8, 0xfedd, 0xfed2,
	0xfec7,  0xfebc,  0xfeb1,  0xfea5, 0xfe99, 0xfe8d, 0xfe81, 0xfe74, 0xfe68, 0xfe5b, 0xfe4e, 0xfe40, 0xfe33, 0xfe25,
	0xfe18,  0xfe09,  0xfdfb,  0xfded, 0xfdde, 0xfdcf, 0xfdc0, 0xfdb1, 0xfda2, 0xfd92, 0xfd82, 0xfd72, 0xfd62, 0xfd52,
	0xfd41,  0xfd30,  0xfd1f,  0xfd0e, 0xfcfd, 0xfceb, 0xfcd9, 0xfcc7, 0xfcb5, 0xfca3, 0xfc90, 0xfc7d, 0xfc6a, 0xfc57,
	0xfc44,  0xfc30,  0xfc1c,  0xfc08, 0xfbf4, 0xfbe0, 0xfbcb, 0xfbb7, 0xfba2, 0xfb8d, 0xfb77, 0xfb62, 0xfb4c, 0xfb36,
	0xfb20,  0xfb0a,  0xfaf3,  0xfadc, 0xfac5, 0xfaae, 0xfa97, 0xfa80, 0xfa68, 0xfa50, 0xfa38, 0xfa20, 0xfa07, 0xf9ef,
	0xf9d6,  0xf9bd,  0xf9a3,  0xf98a, 0xf970, 0xf956, 0xf93c, 0xf922, 0xf908, 0xf8ed, 0xf8d2, 0xf8b7, 0xf89c, 0xf881,
	0xf865,  0xf84a,  0xf82e,  0xf811, 0xf7f5, 0xf7d9, 0xf7bc, 0xf79f, 0xf782, 0xf764, 0xf747, 0xf729, 0xf70b, 0xf6ed,
	0xf6cf,  0xf6b0,  0xf692,  0xf673, 0xf654, 0xf635, 0xf615, 0xf5f6, 0xf5d6, 0xf5b6, 0xf596, 0xf575, 0xf555, 0xf534,
	0xf513,  0xf4f2,  0xf4d0,  0xf4af, 0xf48d, 0xf46b, 0xf449, 0xf427, 0xf404, 0xf3e2, 0xf3bf, 0xf39c, 0xf378, 0xf355,
	0xf331,  0xf30e,  0xf2ea,  0xf2c5, 0xf2a1, 0xf27c, 0xf258, 0xf233, 0xf20e, 0xf1e8, 0xf1c3, 0xf19d, 0xf177, 0xf151,
	0xf12b,  0xf104,  0xf0de,  0xf0b7, 0xf090, 0xf068, 0xf041, 0xf019, 0xeff2, 0xefca, 0xefa2, 0xef79, 0xef51, 0xef28,
	0xeeff,  0xeed6,  0xeead,  0xee83, 0xee5a, 0xee30, 0xee06, 0xeddc, 0xedb1, 0xed87, 0xed5c, 0xed31, 0xed06, 0xecdb,
	0xecaf,  0xec83,  0xec58,  0xec2b, 0xebff, 0xebd3, 0xeba6, 0xeb79, 0xeb4c, 0xeb1f, 0xeaf2, 0xeac4, 0xea97, 0xea69,
	0xea3b,  0xea0d,  0xe9de,  0xe9b0, 0xe981, 0xe952, 0xe923, 0xe8f3, 0xe8c4, 0xe894, 0xe864, 0xe834, 0xe804, 0xe7d3,
	0xe7a3,  0xe772,  0xe741,  0xe710, 0xe6de, 0xe6ad, 0xe67b, 0xe649, 0xe617, 0xe5e5, 0xe5b3, 0xe580, 0xe54d, 0xe51a,
	0xe4e7,  0xe4b4,  0xe481,  0xe44d, 0xe419, 0xe3e5, 0xe3b1, 0xe37c, 0xe348, 0xe313, 0xe2de, 0xe2a9, 0xe274, 0xe23e,
	0xe209,  0xe1d3,  0xe19d,  0xe167, 0xe131, 0xe0fa, 0xe0c3, 0xe08d, 0xe056, 0xe01e, 0xdfe7, 0xdfb0, 0xdf78, 0xdf40,
	0xdf08,  0xded0,  0xde97,  0xde5f, 0xde26, 0xdded, 0xddb4, 0xdd7b, 0xdd41, 0xdd07, 0xdcce, 0xdc94, 0xdc5a, 0xdc1f,
	0xdbe5,  0xdbaa,  0xdb6f,  0xdb34, 0xdaf9, 0xdabe, 0xda82, 0xda47, 0xda0b, 0xd9cf, 0xd993, 0xd956, 0xd91a, 0xd8dd,
	0xd8a0,  0xd863,  0xd826,  0xd7e9, 0xd7ab, 0xd76d, 0xd72f, 0xd6f1, 0xd6b3, 0xd675, 0xd636, 0xd5f7, 0xd5b9, 0xd57a,
	0xd53a,  0xd4fb,  0xd4bb,  0xd47c, 0xd43c, 0xd3fc, 0xd3bc, 0xd37b, 0xd33b, 0xd2fa, 0xd2b9, 0xd278, 0xd237, 0xd1f5,
	0xd1b4,  0xd172,  0xd130,  0xd0ee, 0xd0ac, 0xd06a, 0xd027, 0xcfe5, 0xcfa2, 0xcf5f, 0xcf1c, 0xced8, 0xce95, 0xce51,
	0xce0e,  0xcdca,  0xcd85,  0xcd41, 0xccfd, 0xccb8, 0xcc73, 0xcc2e, 0xcbe9, 0xcba4, 0xcb5f, 0xcb19, 0xcad3, 0xca8e,
	0xca48,  0xca01,  0xc9bb,  0xc975, 0xc92e, 0xc8e7, 0xc8a0, 0xc859, 0xc812, 0xc7ca, 0xc783, 0xc73b, 0xc6f3, 0xc6ab,
	0xc663,  0xc61a,  0xc5d2,  0xc589, 0xc540, 0xc4f7, 0xc4ae, 0xc465, 0xc41b, 0xc3d2, 0xc388, 0xc33e, 0xc2f4, 0xc2aa,
	0xc260,  0xc215,  0xc1ca,  0xc180, 0xc135, 0xc0ea, 0xc09e, 0xc053, 0xc007, 0xbfbc, 0xbf70, 0xbf24, 0xbed8, 0xbe8b,
	0xbe3f,  0xbdf2,  0xbda5,  0xbd58, 0xbd0b, 0xbcbe, 0xbc71, 0xbc23, 0xbbd6, 0xbb88, 0xbb3a, 0xbaec, 0xba9e, 0xba4f,
	0xba01,  0xb9b2,  0xb963,  0xb914, 0xb8c5, 0xb876, 0xb827, 0xb7d7, 0xb787, 0xb738, 0xb6e8, 0xb698, 0xb647, 0xb5f7,
	0xb5a6,  0xb556,  0xb505,  0xb4b4, 0xb463, 0xb412, 0xb3c0, 0xb36f, 0xb31d, 0xb2cb, 0xb279, 0xb227, 0xb1d5, 0xb183,
	0xb130,  0xb0de,  0xb08b,  0xb038, 0xafe5, 0xaf92, 0xaf3e, 0xaeeb, 0xae97, 0xae44, 0xadf0, 0xad9c, 0xad48, 0xacf3,
	0xac9f,  0xac4b,  0xabf6,  0xaba1, 0xab4c, 0xaaf7, 0xaaa2, 0xaa4d, 0xa9f7, 0xa9a1, 0xa94c, 0xa8f6, 0xa8a0, 0xa84a,
	0xa7f3,  0xa79d,  0xa747,  0xa6f0, 0xa699, 0xa642, 0xa5eb, 0xa594, 0xa53d, 0xa4e5, 0xa48e, 0xa436, 0xa3de, 0xa386,
	0xa32e,  0xa2d6,  0xa27e,  0xa225, 0xa1cd, 0xa174, 0xa11b, 0xa0c2, 0xa069, 0xa010, 0x9fb7, 0x9f5d, 0x9f04, 0x9eaa,
	0x9e50,  0x9df6,  0x9d9c,  0x9d42, 0x9ce7, 0x9c8d, 0x9c32, 0x9bd8, 0x9b7d, 0x9b22, 0x9ac7, 0x9a6c, 0x9a11, 0x99b5,
	0x995a,  0x98fe,  0x98a2,  0x9846, 0x97ea, 0x978e, 0x9732, 0x96d6, 0x9679, 0x961c, 0x95c0, 0x9563, 0x9506, 0x94a9,
	0x944c,  0x93ee,  0x9391,  0x9334, 0x92d6, 0x9278, 0x921a, 0x91bc, 0x915e, 0x9100, 0x90a2, 0x9043, 0x8fe5, 0x8f86,
	0x8f27,  0x8ec8,  0x8e69,  0x8e0a, 0x8dab, 0x8d4c, 0x8cec, 0x8c8d, 0x8c2d, 0x8bcd, 0x8b6d, 0x8b0d, 0x8aad, 0x8a4d,
	0x89ed,  0x898c,  0x892c,  0x88cb, 0x886b, 0x880a, 0x87a9, 0x8748, 0x86e7, 0x8685, 0x8624, 0x85c2, 0x8561, 0x84ff,
	0x849d,  0x843c,  0x83da,  0x8377, 0x8315, 0x82b3, 0x8251, 0x81ee, 0x818b, 0x8129, 0x80c6, 0x8063, 0x8000, 0x7f9d,
	0x7f3a,  0x7ed6,  0x7e73,  0x7e0f, 0x7dac, 0x7d48, 0x7ce4, 0x7c80, 0x7c1c, 0x7bb8, 0x7b54, 0x7af0, 0x7a8c, 0x7a27,
	0x79c3,  0x795e,  0x78f9,  0x7894, 0x782f, 0x77ca, 0x7765, 0x7700, 0x769b, 0x7635, 0x75d0, 0x756a, 0x7504, 0x749f,
	0x7439,  0x73d3,  0x736d,  0x7307, 0x72a0, 0x723a, 0x71d4, 0x716d, 0x7107, 0x70a0, 0x7039, 0x6fd2, 0x6f6b, 0x6f04,
	0x6e9d,  0x6e36,  0x6dcf,  0x6d67, 0x6d00, 0x6c98, 0x6c31, 0x6bc9, 0x6b61, 0x6af9, 0x6a91, 0x6a29, 0x69c1, 0x6959,
	0x68f1,  0x6888,  0x6820,  0x67b7, 0x674f, 0x66e6, 0x667d, 0x6614, 0x65ab, 0x6542, 0x64d9, 0x6470, 0x6407, 0x639e,
	0x6334,  0x62cb,  0x6261,  0x61f8, 0x618e, 0x6124, 0x60ba, 0x6050, 0x5fe6, 0x5f7c, 0x5f12, 0x5ea8, 0x5e3d, 0x5dd3,
	0x5d69,  0x5cfe,  0x5c93,  0x5c29, 0x5bbe, 0x5b53, 0x5ae8, 0x5a7d, 0x5a12, 0x59a7, 0x593c, 0x58d1, 0x5865, 0x57fa,
	0x578f,  0x5723,  0x56b8,  0x564c, 0x55e0, 0x5574, 0x5509, 0x549d, 0x5431, 0x53c5, 0x5358, 0x52ec, 0x5280, 0x5214,
	0x51a7,  0x513b,  0x50ce,  0x5062, 0x4ff5, 0x4f88, 0x4f1c, 0x4eaf, 0x4e42, 0x4dd5, 0x4d68, 0x4cfb, 0x4c8e, 0x4c21,
	0x4bb4,  0x4b46,  0x4ad9,  0x4a6b, 0x49fe, 0x4990, 0x4923, 0x48b5, 0x4848, 0x47da, 0x476c, 0x46fe, 0x4690, 0x4622,
	0x45b4,  0x4546,  0x44d8,  0x446a, 0x43fb, 0x438d, 0x431f, 0x42b0, 0x4242, 0x41d3, 0x4165, 0x40f6, 0x4088, 0x4019,
	0x3faa,  0x3f3b,  0x3ecc,  0x3e5e, 0x3def, 0x3d80, 0x3d11, 0x3ca1, 0x3c32, 0x3bc3, 0x3b54, 0x3ae5, 0x3a75, 0x3a06,
	0x3996,  0x3927,  0x38b7,  0x3848, 0x37d8, 0x3769, 0x36f9, 0x3689, 0x3619, 0x35aa, 0x353a, 0x34ca, 0x345a, 0x33ea,
	0x337a,  0x330a,  0x329a,  0x322a, 0x31b9, 0x3149, 0x30d9, 0x3069, 0x2ff8, 0x2f88, 0x2f17, 0x2ea7, 0x2e37, 0x2dc6,
	0x2d55,  0x2ce5,  0x2c74,  0x2c04, 0x2b93, 0x2b22, 0x2ab1, 0x2a41, 0x29d0, 0x295f, 0x28ee, 0x287d, 0x280c, 0x279b,
	0x272a,  0x26b9,  0x2648,  0x25d7, 0x2566, 0x24f5, 0x2483, 0x2412, 0x23a1, 0x2330, 0x22be, 0x224d, 0x21dc, 0x216a,
	0x20f9,  0x2087,  0x2016,  0x1fa4, 0x1f33, 0x1ec1, 0x1e50, 0x1dde, 0x1d6d, 0x1cfb, 0x1c89, 0x1c18, 0x1ba6, 0x1b34,
	0x1ac2,  0x1a51,  0x19df,  0x196d, 0x18fb, 0x1889, 0x1817, 0x17a6, 0x1734, 0x16c2, 0x1650, 0x15de, 0x156c, 0x14fa,
	0x1488,  0x1416,  0x13a4,  0x1332, 0x12c0, 0x124e, 0x11dc, 0x1169, 0x10f7, 0x1085, 0x1013, 0xfa1,  0xf2f,  0xebd,
	0xe4a,   0xdd8,   0xd66,   0xcf4,  0xc81,  0xc0f,  0xb9d,  0xb2b,  0xab8,  0xa46,  0x9d4,  0x961,  0x8ef,  0x87d,
	0x80b,   0x798,   0x726,   0x6b4,  0x641,  0x5cf,  0x55c,  0x4ea,  0x478,  0x405,  0x393,  0x321,  0x2ae,  0x23c,
	0x1ca,   0x157,   0xe5,    0x72,   0x0
};

// Looks up the 16.16 cosine and sine of p_angle (in tenths of a degree) in g_cosTable.
#ifdef COMPAT_MODE
void GetCosSin(MechS32 p_angle, MechS32* p_cos, MechS32* p_sin)
{
	STUB(0x10064ba8);
}
#else
// FUNCTION: MW2 0x10064ba8
__declspec(naked) void GetCosSin(MechS32 p_angle, MechS32* p_cos, MechS32* p_sin)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov ebx, dword ptr [ebp+0x8]
		and ebx, ebx
		_emit 0x79 /* jns jmp_10064bc6 */
		_emit 0x10
jmp_10064bb6:
		add ebx, 0xe10
		_emit 0x78 /* js jmp_10064bb6 */
		_emit 0xf8
		_emit 0xeb /* jmp jmp_10064bc6 */
		_emit 0x06
jmp_10064bc0:
		sub ebx, 0xe10
jmp_10064bc6:
		cmp ebx, 0xe10
		_emit 0x7f /* jg jmp_10064bc0 */
		_emit 0xf2
		cmp ebx, 0x708
		_emit 0x77 /* ja jmp_10064c0e */
		_emit 0x38
		cmp ebx, 0x384
		_emit 0x77 /* ja jmp_10064bf1 */
		_emit 0x13
		shl ebx, 0x2
		mov eax, dword ptr [g_cosTable+ebx]
		neg ebx
		mov edx, dword ptr [g_cosTable+ebx+0xe10]
		_emit 0xeb /* jmp jmp_10064c50 */
		_emit 0x5f
jmp_10064bf1:
		neg ebx
		add ebx, 0x708
		shl ebx, 0x2
		mov eax, dword ptr [g_cosTable+ebx]
		neg eax
		neg ebx
		mov edx, dword ptr [g_cosTable+ebx+0xe10]
		_emit 0xeb /* jmp jmp_10064c50 */
		_emit 0x42
jmp_10064c0e:
		neg ebx
		add ebx, 0xe10
		cmp ebx, 0x384
		_emit 0x77 /* ja jmp_10064c33 */
		_emit 0x15
		shl ebx, 0x2
		mov eax, dword ptr [g_cosTable+ebx]
		neg ebx
		mov edx, dword ptr [g_cosTable+ebx+0xe10]
		neg edx
		_emit 0xeb /* jmp jmp_10064c50 */
		_emit 0x1d
jmp_10064c33:
		neg ebx
		add ebx, 0x708
		shl ebx, 0x2
		mov eax, dword ptr [g_cosTable+ebx]
		neg eax
		neg ebx
		mov edx, dword ptr [g_cosTable+ebx+0xe10]
		neg edx
jmp_10064c50:
		mov ebx, dword ptr [ebp+0xc]
		mov dword ptr [ebx], eax
		mov ebx, dword ptr [ebp+0x10]
		mov dword ptr [ebx], edx
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Stores the 16.16 product of p_a and p_b, rounded, in *p_result.
// FUNCTION: MW2 0x10064c60
void FUN_10064c60(MechS32 p_a, MechS32 p_b, MechS32* p_result)
{
	__asm {
		push es
		mov eax, p_a
		imul p_b
		add eax, 0x8000
		adc edx, 0
		mov ax, dx
		ror eax, 0x10
		mov edi, p_result
		mov dword ptr [edi], eax
		pop es
	}
}

// Rotates the point p_point about p_origin by p_angle (in tenths of a degree) and scales it
// by the 16.16 factors p_scaleX and p_scaleY, storing the result in p_result.
#ifdef COMPAT_MODE
void RotateScalePoint(
	MechS32* p_point,
	MechS32* p_result,
	MechS32* p_origin,
	MechS32 p_angle,
	MechS32 p_scaleX,
	MechS32 p_scaleY
)
{
	STUB(0x10064c86);
}
#else
// FUNCTION: MW2 0x10064c86
__declspec(naked) void RotateScalePoint(
	MechS32* p_point,
	MechS32* p_result,
	MechS32* p_origin,
	MechS32 p_angle,
	MechS32 p_scaleX,
	MechS32 p_scaleY
)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x20
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		lea eax, [ebp-0x8]
		push eax
		lea eax, [ebp-0x4]
		push eax
		push dword ptr [ebp+0x14]
		call GetCosSin
		add esp, 0xc
		mov esi, dword ptr [ebp+0x8]
		mov edi, dword ptr [ebp+0x10]
		mov eax, dword ptr [esi]
		sub eax, dword ptr [edi]
		shl eax, 0x10
		imul dword ptr [ebp+0x18]
		add eax, 0x8000
		adc edx, 0x0
		mov ebx, edx
		mov eax, ebx
		imul dword ptr [ebp-0x4]
		add eax, 0x8000
		adc edx, 0x0
		mov ax, dx
		ror eax, 0x10
		mov dword ptr [ebp-0xc], eax
		mov eax, ebx
		imul dword ptr [ebp-0x8]
		add eax, 0x8000
		adc edx, 0x0
		mov ax, dx
		ror eax, 0x10
		mov dword ptr [ebp-0x14], eax
		mov eax, dword ptr [esi+0x4]
		sub eax, dword ptr [edi+0x4]
		shl eax, 0x10
		imul dword ptr [ebp+0x1c]
		add eax, 0x8000
		adc edx, 0x0
		mov ecx, edx
		mov eax, ecx
		imul dword ptr [ebp-0x4]
		add eax, 0x8000
		adc edx, 0x0
		mov ax, dx
		ror eax, 0x10
		mov dword ptr [ebp-0x18], eax
		mov eax, ecx
		imul dword ptr [ebp-0x8]
		add eax, 0x8000
		adc edx, 0x0
		mov ax, dx
		ror eax, 0x10
		mov dword ptr [ebp-0x10], eax
		mov esi, dword ptr [ebp+0xc]
		mov edx, dword ptr [ebp-0xc]
		sub edx, dword ptr [ebp-0x10]
		add edx, dword ptr [edi]
		mov dword ptr [esi], edx
		mov edx, dword ptr [ebp-0x18]
		add edx, dword ptr [ebp-0x14]
		add edx, dword ptr [edi+0x4]
		mov dword ptr [esi+0x4], edx
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

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

#ifdef COMPAT_MODE
MechS32 BlitChar(RenderTarget* p_target, MechS32 p_x, MechS32 p_y, void* p_font, MechS32 p_char, void* p_unk0x14)
{
	STUB(0x10064d80);
	return 0;
}
#else
// FUNCTION: MW2 0x10064d80
__declspec(naked) MechS32
BlitChar(RenderTarget* p_target, MechS32 p_x, MechS32 p_y, void* p_font, MechS32 p_char, void* p_unk0x14)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x30
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp + 8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx + 4]
		inc eax
		mov dword ptr [ebp - 0x28], eax
		_emit 0x7e /* jle jmp_10064dff */
		_emit 0x64
		mov eax, dword ptr [ebx + 8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10064dff */
		_emit 0x5c
		mov eax, dword ptr [esi + 4]
		mov dword ptr [ebp - 0x2c], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_10064db3 */
		_emit 0x05
		mov eax, 0
jmp_10064db3:
		mov dword ptr [ebp - 0x14], eax
		mov eax, dword ptr [esi + 8]
		mov dword ptr [ebp - 0x30], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_10064dc6 */
		_emit 0x05
		mov eax, 0
jmp_10064dc6:
		mov dword ptr [ebp - 0x18], eax
		mov eax, dword ptr [esi + 0xc]
		mov edx, dword ptr [ebp - 0x28]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10064dd6 */
		_emit 0x02
		mov eax, edx
jmp_10064dd6:
		mov dword ptr [ebp - 0x1c], eax
		mov eax, dword ptr [esi + 0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10064de5 */
		_emit 0x02
		mov eax, edx
jmp_10064de5:
		mov dword ptr [ebp - 0x20], eax
		mov eax, dword ptr [ebp - 0x1c]
		cmp eax, dword ptr [ebp - 0x14]
		_emit 0x7c /* jl jmp_10064e0a */
		_emit 0x1a
		mov eax, dword ptr [ebp - 0x20]
		cmp eax, dword ptr [ebp - 0x18]
		_emit 0x7c /* jl jmp_10064e0a */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp - 0x24], eax
		jmp short jmp_10064e15
jmp_10064dff:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10064e0a:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10064e15:
		mov eax, dword ptr [ebp - 0x2c]
		add dword ptr [ebp + 0xc], eax
		mov eax, dword ptr [ebp - 0x30]
		add dword ptr [ebp + 0x10], eax
		mov esi, dword ptr [ebp + 0x14]
		mov edx, dword ptr [esi + 8]
		mov eax, dword ptr [ebp + 0x18]
		shl eax, 2
		add eax, dword ptr [ebp + 0x14]
		add eax, 0x10
		mov esi, dword ptr [eax]
		add esi, dword ptr [ebp + 0x14]
		mov dword ptr [ebp - 8], 0
		mov ecx, dword ptr [esi]
		mov dword ptr [ebp - 4], ecx
		cmp ecx, 0
		je jmp_10064eff
		add esi, 4
		mov edi, dword ptr [ebp + 8]
		mov eax, dword ptr [ebp - 0x1c]
		inc eax
		sub eax, ecx
		sub eax, dword ptr [ebp + 0xc]
		_emit 0x79 /* jns jmp_10064e66 */
		_emit 0x08
		add ecx, eax
		jle jmp_10064eff
jmp_10064e66:
		mov eax, dword ptr [ebp + 0xc]
		sub eax, dword ptr [ebp - 0x14]
		_emit 0x79 /* jns jmp_10064e7b */
		_emit 0x0d
		add ecx, eax
		jle jmp_10064eff
		sub esi, eax
		sub dword ptr [ebp + 0xc], eax
jmp_10064e7b:
		mov eax, dword ptr [ebp - 0x20]
		inc eax
		sub eax, edx
		sub eax, dword ptr [ebp + 0x10]
		_emit 0x79 /* jns jmp_10064e8a */
		_emit 0x04
		add edx, eax
		_emit 0x7e /* jle jmp_10064eff */
		_emit 0x75
jmp_10064e8a:
		mov eax, dword ptr [ebp + 0x10]
		sub eax, dword ptr [ebp - 0x18]
		_emit 0x79 /* jns jmp_10064e9f */
		_emit 0x0d
		add edx, eax
		_emit 0x7e /* jle jmp_10064eff */
		_emit 0x69
		sub dword ptr [ebp + 0x10], eax
		imul eax, dword ptr [ebp - 4]
		sub esi, eax
jmp_10064e9f:
		mov dword ptr [ebp - 0x10], edx
		mov eax, dword ptr [ebp + 0x10]
		imul dword ptr [ebp - 0x28]
		add eax, dword ptr [ebp - 0x24]
		add eax, dword ptr [ebp + 0xc]
		mov edi, eax
		mov dword ptr [ebp - 8], ecx
		sub dword ptr [ebp - 4], ecx
		mov eax, dword ptr [ebp - 0x28]
		sub eax, ecx
		mov dword ptr [ebp - 0xc], eax
		mov edx, dword ptr [ebp - 0x10]
		cmp dword ptr [ebp + 0x1c], 0
		_emit 0x75 /* jne jmp_10064ee1 */
		_emit 0x1a
jmp_10064ec7:
		rep movsb
		mov ecx, dword ptr [ebp - 8]
		add esi, dword ptr [ebp - 4]
		add edi, dword ptr [ebp - 0xc]
		dec edx
		_emit 0x75 /* jne jmp_10064ec7 */
		_emit 0xf2
		mov eax, dword ptr [ebp - 4]
		add eax, dword ptr [ebp - 8]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10064ee1:
		jecxz jmp_10064eff
		mov ebx, dword ptr [ebp + 0x1c]
jmp_10064ee6:
		mov al, byte ptr [esi]
		xlatb
		cmp al, 0xff
		_emit 0x74 /* je jmp_10064eef */
		_emit 0x02
		mov byte ptr [edi], al
jmp_10064eef:
		inc esi
		inc edi
		loop jmp_10064ee6
		mov ecx, dword ptr [ebp - 8]
		add esi, dword ptr [ebp - 4]
		add edi, dword ptr [ebp - 0xc]
		dec edx
		_emit 0x75 /* jne jmp_10064ee6 */
		_emit 0xe7
jmp_10064eff:
		mov eax, dword ptr [ebp - 4]
		add eax, dword ptr [ebp - 8]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Draws the non-empty string p_text, each glyph through BlitChar, which returns its width.
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
		call BlitChar
		add esp, 0x18
		add edi, eax
		inc esi
		cmp byte ptr [esi], 0
		_emit 0x75 /* jne jmp_10064f1b */
		_emit 0xdf
		pop es
	}
}

// Copies p_width pixels from p_src to row p_row of p_target, clipped to the target's rectangle.
// Returns -1 for an empty pixel buffer and -2 for an empty rectangle.
#ifdef COMPAT_MODE
MechS32 WriteViewRow(RenderTarget* p_target, MechS32 p_row, MechU8* p_src, MechS32 p_width)
{
	STUB(0x10064f42);
	return 0;
}
#else
// FUNCTION: MW2 0x10064f42
__declspec(naked) MechS32 WriteViewRow(RenderTarget* p_target, MechS32 p_row, MechU8* p_src, MechS32 p_width)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x24
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp + 8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx + 4]
		inc eax
		mov dword ptr [ebp - 0x1c], eax
		_emit 0x7e /* jle jmp_10064fc1 */
		_emit 0x64
		mov eax, dword ptr [ebx + 8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10064fc1 */
		_emit 0x5c
		mov eax, dword ptr [esi + 4]
		mov dword ptr [ebp - 0x20], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_10064f75 */
		_emit 0x05
		mov eax, 0
jmp_10064f75:
		mov dword ptr [ebp - 8], eax
		mov eax, dword ptr [esi + 8]
		mov dword ptr [ebp - 0x24], eax
		cmp eax, 0
		_emit 0x7f /* jg jmp_10064f88 */
		_emit 0x05
		mov eax, 0
jmp_10064f88:
		mov dword ptr [ebp - 0xc], eax
		mov eax, dword ptr [esi + 0xc]
		mov edx, dword ptr [ebp - 0x1c]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10064f98 */
		_emit 0x02
		mov eax, edx
jmp_10064f98:
		mov dword ptr [ebp - 0x10], eax
		mov eax, dword ptr [esi + 0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10064fa7 */
		_emit 0x02
		mov eax, edx
jmp_10064fa7:
		mov dword ptr [ebp - 0x14], eax
		mov eax, dword ptr [ebp - 0x10]
		cmp eax, dword ptr [ebp - 8]
		_emit 0x7c /* jl jmp_10064fcc */
		_emit 0x1a
		mov eax, dword ptr [ebp - 0x14]
		cmp eax, dword ptr [ebp - 0xc]
		_emit 0x7c /* jl jmp_10064fcc */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp - 0x18], eax
		jmp short jmp_10064fd7
jmp_10064fc1:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10064fcc:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10064fd7:
		mov dword ptr [ebp - 4], 0
		mov eax, dword ptr [ebp - 0x20]
		add dword ptr [ebp - 4], eax
		mov eax, dword ptr [ebp - 0x24]
		add dword ptr [ebp + 0xc], eax
		mov esi, dword ptr [ebp + 0x10]
		mov edi, dword ptr [ebp + 8]
		mov ecx, dword ptr [ebp + 0x14]
		mov eax, dword ptr [ebp - 0x10]
		sub eax, dword ptr [ebp - 4]
		inc eax
		sub eax, ecx
		_emit 0x79 /* jns jmp_10065002 */
		_emit 0x04
		add ecx, eax
		_emit 0x7e /* jle jmp_1006503f */
		_emit 0x3d
jmp_10065002:
		mov eax, dword ptr [ebp - 4]
		sub eax, dword ptr [ebp - 8]
		_emit 0x79 /* jns jmp_10065013 */
		_emit 0x09
		add ecx, eax
		_emit 0x7e /* jle jmp_1006503f */
		_emit 0x31
		sub esi, eax
		sub dword ptr [ebp - 4], eax
jmp_10065013:
		mov eax, dword ptr [ebp - 0x14]
		sub eax, dword ptr [ebp + 0xc]
		_emit 0x78 /* js jmp_1006503f */
		_emit 0x24
		mov eax, dword ptr [ebp + 0xc]
		sub eax, dword ptr [ebp - 0xc]
		_emit 0x78 /* js jmp_1006503f */
		_emit 0x1c
		mov eax, dword ptr [ebp + 0xc]
		imul dword ptr [ebp - 0x1c]
		add eax, dword ptr [ebp - 0x18]
		add eax, dword ptr [ebp - 4]
		mov edi, eax
		mov edx, ecx
		and ecx, 3
		rep movsb
		mov ecx, edx
		shr ecx, 2
		rep movsd
jmp_1006503f:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// The IFF chunk ids the ILBM decoder looks for, embedded in the code.
// GLOBAL: MW2 0x10065045
MechChar g_iffBmhdTag[4] = {'B', 'M', 'H', 'D'};

// GLOBAL: MW2 0x10065049
MechChar g_iffCmapTag[4] = {'C', 'M', 'A', 'P'};

// GLOBAL: MW2 0x1006504d
MechChar g_iffBodyTag[4] = {'B', 'O', 'D', 'Y'};

// Returns the data of the IFF chunk p_tag in the FORM p_iff (big-endian sizes). There must be one.
// FUNCTION: MW2 0x10065051
MechU8* FindIffChunk(MechChar* p_tag, MechU8* p_iff)
{
	__asm {
		push es
		cld
		push ds
		pop es
		mov esi, p_iff
		add esi, 0xc
jmp_10065061:
		cmp byte ptr [esi], 0
		_emit 0x75 /* jne jmp_10065069 */
		_emit 0x03
		inc esi
		jmp short jmp_10065061
jmp_10065069:
		mov ecx, 2
		mov edi, p_tag
		mov eax, esi
		repe cmpsw
		_emit 0x74 /* je jmp_1006508a */
		_emit 0x12
		mov esi, eax
		add esi, 6
		lodsw
		_emit 0x86 /* xchg ah, al */
		_emit 0xc4
		and eax, 0xffff
		add esi, eax
		jmp short jmp_10065061
jmp_1006508a:
		add eax, 8
		pop es
	}
}

// Decodes the ILBM image p_iff (planar, optionally run-length compressed) into p_target, a row at a
// time through WriteViewRow. Returns the BMHD's masking byte, or its compression byte... (unknown).
#ifdef COMPAT_MODE
MechS32 BlitIff(RenderTarget* p_target, MechU8* p_iff)
{
	STUB(0x10065093);
	return 0;
}
#else
// FUNCTION: MW2 0x10065093
__declspec(naked) MechS32 BlitIff(RenderTarget* p_target, MechU8* p_iff)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x3c
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov edi, dword ptr [ebp + 8]
		mov eax, dword ptr [edi + 0xc]
		sub eax, dword ptr [edi + 4]
		inc eax
		mov dword ptr [ebp - 0x24], eax
		mov eax, dword ptr [edi + 0x10]
		sub eax, dword ptr [edi + 8]
		inc eax
		mov dword ptr [ebp - 0x20], eax
		mov edi, dword ptr [edi]
		mov edi, dword ptr [edi]
		mov dword ptr [ebp - 0x3c], edi
		mov dword ptr [ebp - 0x28], 0
		mov edi, dword ptr [ebp + 0xc]
		mov eax, dword ptr [edi + 8]
		xor eax, 0x4d424c49
		mov dword ptr [ebp - 4], eax
		push dword ptr [ebp + 0xc]
		push offset g_iffBmhdTag
		call FindIffChunk
		add esp, 8
		mov esi, eax
		lodsw
		_emit 0x86 /* xchg ah, al */
		_emit 0xc4
		and eax, 0xffff
		mov dword ptr [ebp - 0xc], eax
		lodsw
		_emit 0x86 /* xchg ah, al */
		_emit 0xc4
		and eax, 0xffff
		cmp eax, dword ptr [ebp - 0x20]
		_emit 0x7c /* jl jmp_10065102 */
		_emit 0x03
		mov eax, dword ptr [ebp - 0x20]
jmp_10065102:
		mov dword ptr [ebp - 8], eax
		add esi, 5
		lodsb
		cmp al, 1
		je jmp_10065251
		mov eax, 0
		lodsb
		mov dword ptr [ebp - 0x2c], eax
		add esi, 2
		mov eax, 0
		lodsb
		mov dword ptr [ebp - 0x38], eax
		mov eax, dword ptr [ebp - 0xc]
		mov ebx, eax
		shr eax, 3
		and ebx, 7
		cmp ebx, 1
		sbb eax, -1
		mov ebx, eax
		and eax, 1
		add ebx, eax
		mov dword ptr [ebp - 0x30], ebx
		mov eax, dword ptr [ebp - 0xc]
		and eax, 1
		add eax, dword ptr [ebp - 0xc]
		mov dword ptr [ebp - 0x34], eax
		mov eax, dword ptr [ebp - 0xc]
		cmp eax, dword ptr [ebp - 0x24]
		_emit 0x7c /* jl jmp_10065158 */
		_emit 0x03
		mov eax, dword ptr [ebp - 0x24]
jmp_10065158:
		mov dword ptr [ebp - 0xc], eax
		push dword ptr [ebp + 0xc]
		push offset g_iffBodyTag
		call FindIffChunk
		add esp, 8
		mov dword ptr [ebp - 0x10], eax
jmp_1006516e:
		mov esi, dword ptr [ebp - 0x10]
		cmp dword ptr [ebp - 0x2c], 1
		_emit 0x75 /* jne jmp_100651c9 */
		_emit 0x52
		mov edi, offset g_unk0x100ac379
		mov edx, dword ptr [ebp - 0x34]
		add edx, edi
jmp_10065181:
		cmp edi, edx
		_emit 0x73 /* jae jmp_100651bf */
		_emit 0x3a
		lodsb
		movzx ecx, al
		cmp ecx, 0x80
		_emit 0x74 /* je jmp_10065181 */
		_emit 0xf0
		_emit 0x77 /* ja jmp_100651a2 */
		_emit 0x0f
		inc ecx
		push ecx
		and ecx, 3
		rep movsb
		pop ecx
		shr ecx, 2
		rep movsd
		jmp short jmp_10065181
jmp_100651a2:
		lodsb
		mov ah, al
		mov ebx, eax
		shl eax, 0x10
		mov ax, bx
		neg cl
		inc cl
		push ecx
		and ecx, 3
		rep stosb
		pop ecx
		shr ecx, 2
		rep stosd
		jmp short jmp_10065181
jmp_100651bf:
		mov dword ptr [ebp - 0x10], esi
		mov esi, offset g_unk0x100ac379
		jmp short jmp_100651d1
jmp_100651c9:
		mov eax, esi
		add eax, dword ptr [ebp - 0x34]
		mov dword ptr [ebp - 0x10], eax
jmp_100651d1:
		cmp dword ptr [ebp - 4], 0
		_emit 0x75 /* jne jmp_10065233 */
		_emit 0x5c
		mov edi, offset g_scanline
		mov eax, dword ptr [ebp - 0x30]
		mov dword ptr [ebp - 0x18], eax
		mov dword ptr [ebp - 0x1c], eax
		mov eax, dword ptr [ebp - 0xc]
		mov dword ptr [ebp - 0x14], eax
jmp_100651eb:
		mov edx, 0x80
jmp_100651f0:
		mov ebx, 0
		mov eax, 0x100
jmp_100651fa:
		movzx ecx, byte ptr [esi + ebx]
		and ecx, edx
		_emit 0x74 /* je jmp_10065204 */
		_emit 0x02
		or al, ah
jmp_10065204:
		add ebx, dword ptr [ebp - 0x1c]
		shl ah, 1
		_emit 0x75 /* jne jmp_100651fa */
		_emit 0xef
		stosb
		dec dword ptr [ebp - 0x14]
		_emit 0x74 /* je jmp_1006521b */
		_emit 0x0a
		shr dl, 1
		_emit 0x75 /* jne jmp_100651f0 */
		_emit 0xdb
		inc esi
		dec dword ptr [ebp - 0x18]
		_emit 0x75 /* jne jmp_100651eb */
		_emit 0xd0
jmp_1006521b:
		push dword ptr [ebp - 0xc]
		push offset g_scanline
		push dword ptr [ebp - 0x28]
		push dword ptr [ebp + 8]
		call WriteViewRow
		add esp, 0x10
		jmp short jmp_10065245
jmp_10065233:
		push dword ptr [ebp - 0xc]
		push esi
		push dword ptr [ebp - 0x28]
		push dword ptr [ebp + 8]
		call WriteViewRow
		add esp, 0x10
jmp_10065245:
		inc dword ptr [ebp - 0x28]
		dec dword ptr [ebp - 8]
		jne jmp_1006516e
jmp_10065251:
		mov eax, dword ptr [ebp - 0x38]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Copies the ILBM's CMAP to p_palette, scaled from 8 to 6 bits per component.
#ifdef COMPAT_MODE
void ReadIffPalette(MechU8* p_iff, MechU8* p_palette)
{
	STUB(0x1006525a);
}
#else
// FUNCTION: MW2 0x1006525a
__declspec(naked) void ReadIffPalette(MechU8* p_iff, MechU8* p_palette)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		push dword ptr [ebp + 8]
		push offset g_iffCmapTag
		call FindIffChunk
		add esp, 8
		mov esi, eax
		mov edi, dword ptr [ebp + 0xc]
		mov ecx, 0x300
jmp_1006527e:
		lodsb
		shr al, 2
		stosb
		loop jmp_1006527e
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Returns the ILBM's size from its BMHD: the width in the high word, the height in the low word.
#ifdef COMPAT_MODE
MechS32 GetIffSize(MechU8* p_iff)
{
	STUB(0x1006528b);
	return 0;
}
#else
// FUNCTION: MW2 0x1006528b
__declspec(naked) MechS32 GetIffSize(MechU8* p_iff)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		push dword ptr [ebp + 8]
		push offset g_iffBmhdTag
		call FindIffChunk
		add esp, 8
		mov esi, eax
		lodsw
		_emit 0x86 /* xchg ah, al */
		_emit 0xc4
		shl eax, 0x10
		lodsw
		_emit 0x86 /* xchg ah, al */
		_emit 0xc4
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Decodes the PCX image p_pcx (run-length compressed, one byte per pixel) into p_target, a row at a
// time through WriteViewRow. Returns 0.
#ifdef COMPAT_MODE
MechS32 BlitPicture(RenderTarget* p_target, MechU8* p_pcx)
{
	STUB(0x100652b8);
	return 0;
}
#else
// FUNCTION: MW2 0x100652b8
__declspec(naked) MechS32 BlitPicture(RenderTarget* p_target, MechU8* p_pcx)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0xc
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov dword ptr [ebp - 0xc], 0
		mov edi, dword ptr [ebp + 8]
		mov edi, dword ptr [edi]
		mov edi, dword ptr [edi]
		mov esi, dword ptr [ebp + 0xc]
		movzx ebx, word ptr [esi + 0xa]
		sub bx, word ptr [esi + 6]
		mov dword ptr [ebp - 8], ebx
		mov ebx, 0
		movzx eax, word ptr [esi + 0x42]
		mov dword ptr [ebp - 4], eax
		add esi, 0x80
jmp_100652f3:
		mov edi, offset g_scanline
		mov edx, edi
		add edx, dword ptr [ebp - 4]
jmp_100652fd:
		lodsb
		mov ah, al
		and ah, 0xc0
		xor ah, 0xc0
		_emit 0x75 /* jne jmp_10065312 */
		_emit 0x0a
		and eax, 0x3f
		mov ecx, eax
		lodsb
		rep stosb
		jmp short jmp_10065313
jmp_10065312:
		stosb
jmp_10065313:
		cmp edi, edx
		_emit 0x7c /* jl jmp_100652fd */
		_emit 0xe6
		push dword ptr [ebp - 4]
		push offset g_scanline
		push ebx
		push dword ptr [ebp + 8]
		call WriteViewRow
		add esp, 0x10
		inc ebx
		cmp ebx, dword ptr [ebp - 8]
		_emit 0x7e /* jle jmp_100652f3 */
		_emit 0xc2
		mov eax, dword ptr [ebp - 0xc]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Copies the palette at the end of the PCX file p_pcx (p_size bytes) to p_palette, scaled from 8 to
// 6 bits per component.
// FUNCTION: MW2 0x1006533a
void ReadPicturePalette(MechU8* p_pcx, MechS32 p_size, MechU8* p_palette)
{
	__asm {
		push es
		cld
		push ds
		pop es
		mov esi, p_pcx
		add esi, p_size
		sub esi, 0x300
		mov edi, p_palette
		mov ecx, 0x300
jmp_10065358:
		lodsb
		shr al, 2
		stosb
		loop jmp_10065358
		pop es
	}
}

// Returns the PCX image's size from its header: the width in the high word, the height in the low
// word.
// FUNCTION: MW2 0x10065365
MechS32 GetPictureSize(MechU8* p_pcx)
{
	__asm {
		push es
		mov esi, p_pcx
		mov ax, word ptr [esi + 8]
		sub ax, word ptr [esi + 4]
		inc ax
		shl eax, 0x10
		mov ax, word ptr [esi + 0xa]
		sub ax, word ptr [esi + 6]
		inc ax
		pop es
	}
}

// The GIF decoder's helpers keep its state in edi (BlitGif's p_state).
// Resets the LZW string table for ecx roots.
#ifdef COMPAT_MODE
void GifInitCodes(void)
{
	STUB(0x1006538c);
}
#else
// FUNCTION: MW2 0x1006538c
__declspec(naked) void GifInitCodes(void)
{
	__asm {
		mov ebx, 0
		mov eax, ecx
		add eax, 2
		mov dword ptr [edi], eax
		mov eax, ecx
		shl eax, 1
		mov dword ptr [edi + 4], eax
jmp_1006539f:
		cmp ebx, ecx
		_emit 0x7d /* jge jmp_100653be */
		_emit 0x1b
		mov byte ptr [ebx + edi + 0x102e], bl
		mov byte ptr [ebx + edi + 0x202e], bl
		mov word ptr [edi + ebx*2 + 0x302e], 0xffff
		inc ebx
		jmp short jmp_1006539f
jmp_100653be:
		cmp ebx, 0x1000
		_emit 0x7d /* jge jmp_100653d3 */
		_emit 0x0d
		mov word ptr [edi + ebx*2 + 0x302e], 0xfffe
		inc ebx
		jmp short jmp_100653be
jmp_100653d3:
		ret
	}
}
#endif

// Returns the next byte of the current GIF data sub-block in eax, starting the next sub-block when
// the current one is used up.
#ifdef COMPAT_MODE
void GifReadByte(void)
{
	STUB(0x100653d4);
}
#else
// FUNCTION: MW2 0x100653d4
__declspec(naked) void GifReadByte(void)
{
	__asm {
		cmp dword ptr [edi + 0x10], 0
		_emit 0x75 /* jne jmp_100653e3 */
		_emit 0x09
		lodsb
		and eax, 0xff
		mov dword ptr [edi + 0x10], eax
jmp_100653e3:
		lodsb
		and eax, 0xff
		dec dword ptr [edi + 0x10]
		ret
	}
}
#endif

// Returns the next edx-bit code of the GIF data in eax.
#ifdef COMPAT_MODE
void GifReadCode(void)
{
	STUB(0x100653ed);
}
#else
// FUNCTION: MW2 0x100653ed
__declspec(naked) void GifReadCode(void)
{
	__asm {
		cmp dword ptr [edi + 0x18], 0
		_emit 0x75 /* jne jmp_10065402 */
		_emit 0x0f
		call GifReadByte
		mov dword ptr [edi + 0x14], eax
		mov dword ptr [edi + 0x18], 8
jmp_10065402:
		mov eax, edx
		cmp dword ptr [edi + 0x18], eax
		_emit 0x7d /* jge jmp_1006541a */
		_emit 0x11
		call GifReadByte
		mov ecx, dword ptr [edi + 0x18]
		shl eax, cl
		or dword ptr [edi + 0x14], eax
		add dword ptr [edi + 0x18], 8
jmp_1006541a:
		mov ebx, edx
		movzx eax, byte ptr [g_gifCodeMasks + ebx]
		mov ebx, dword ptr [edi + 0x14]
		and ebx, eax
		push ebx
		sub dword ptr [edi + 0x18], edx
		mov ecx, edx
		shr dword ptr [edi + 0x14], cl
		pop eax
		ret
	}
}
#endif

// Adds the LZW string ebx (a code) followed by the first byte of ecx to the string table.
#ifdef COMPAT_MODE
void GifAddCode(void)
{
	STUB(0x10065433);
}
#else
// FUNCTION: MW2 0x10065433
__declspec(naked) void GifAddCode(void)
{
	__asm {
		push ebx
		mov ebx, dword ptr [edi]
		mov word ptr [edi + ebx*2 + 0x302e], cx
		pop ebx
		push ebx
		mov al, byte ptr [ebx + edi + 0x102e]
		mov ebx, dword ptr [edi]
		mov byte ptr [ebx + edi + 0x202e], al
		mov ebx, ecx
		mov al, byte ptr [ebx + edi + 0x102e]
		mov ebx, dword ptr [edi]
		mov byte ptr [ebx + edi + 0x102e], al
		pop ebx
		inc dword ptr [edi]
		mov eax, dword ptr [edi]
		cmp eax, dword ptr [edi + 4]
		_emit 0x75 /* jne jmp_10065478 */
		_emit 0x0c
		cmp dword ptr [edi + 0x1c], 0xc
		_emit 0x7d /* jge jmp_10065478 */
		_emit 0x06
		inc dword ptr [edi + 0x1c]
		shl dword ptr [edi + 4], 1
jmp_10065478:
		ret
	}
}
#endif

// Stores the pixel al: draws the row through WriteViewRow when it is complete, and moves to the next
// row, in the interlaced order if the image is.
#ifdef COMPAT_MODE
void GifPutPixel(void)
{
	STUB(0x10065479);
}
#else
// FUNCTION: MW2 0x10065479
__declspec(naked) void GifPutPixel(void)
{
	__asm {
		mov ebx, dword ptr [edi + 8]
		mov byte ptr [g_scanline + ebx], al
		inc dword ptr [edi + 8]
		dec dword ptr [edi + 0x20]
		cmp dword ptr [edi + 0x20], 0
		_emit 0x75 /* jne jmp_100654f5 */
		_emit 0x67
		push dword ptr [edi + 0x24]
		push offset g_scanline
		push dword ptr [edi + 0xc]
		push dword ptr [g_gifView]
		call WriteViewRow
		add esp, 0x10
		mov dword ptr [edi + 8], 0
		mov eax, dword ptr [edi + 0x24]
		mov dword ptr [edi + 0x20], eax
		cmp byte ptr [edi + 0x2c], 0
		_emit 0x74 /* je jmp_100654e3 */
		_emit 0x29
		movzx ebx, byte ptr [edi + 0x2d]
		movzx eax, byte ptr [g_gifPassSteps + ebx]
		add dword ptr [edi + 0xc], eax
		mov eax, dword ptr [edi + 0xc]
		cmp eax, dword ptr [edi + 0x28]
		_emit 0x7c /* jl jmp_100654e1 */
		_emit 0x11
		inc byte ptr [edi + 0x2d]
		movzx ebx, byte ptr [edi + 0x2d]
		movzx eax, byte ptr [g_gifPassStarts + ebx]
		mov dword ptr [edi + 0xc], eax
jmp_100654e1:
		jmp short jmp_100654f5
jmp_100654e3:
		inc dword ptr [edi + 0xc]
		mov eax, dword ptr [edi + 0xc]
		cmp eax, dword ptr [edi + 0x28]
		_emit 0x7c /* jl jmp_100654f5 */
		_emit 0x07
		mov dword ptr [edi + 0xc], 0
jmp_100654f5:
		ret
	}
}
#endif

// Decodes the GIF image p_gif (LZW compressed, optionally interlaced) into p_target, using p_state
// (0x502e bytes) for the decoder's state and string table.
#ifdef COMPAT_MODE
MechS32 BlitGif(RenderTarget* p_target, MechU8* p_gif, MechU8* p_state)
{
	STUB(0x100654f6);
	return 0;
}
#else
// FUNCTION: MW2 0x100654f6
__declspec(naked) MechS32 BlitGif(RenderTarget* p_target, MechU8* p_gif, MechU8* p_state)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x18
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov edi, dword ptr [ebp + 8]
		mov dword ptr [g_gifView], edi
		mov edi, dword ptr [ebp + 0x10]
		mov eax, 0
		mov ecx, 0x2e
		mov edx, ecx
		and ecx, 3
		rep stosb
		mov ecx, edx
		shr ecx, 2
		rep stosd
		mov edi, dword ptr [ebp + 0x10]
		mov esi, dword ptr [ebp + 0xc]
		movzx eax, byte ptr [esi + 0xb]
		mov dword ptr [ebp - 0x18], eax
		mov al, byte ptr [esi + 0xa]
		mov cl, al
		and cl, 7
		inc cl
		mov ebx, 1
		shl ebx, cl
		add esi, 0xd
		test al, 0x80
		_emit 0x74 /* je jmp_10065551 */
		_emit 0x05
		imul ebx, ebx, 3
		add esi, ebx
jmp_10065551:
		movzx eax, word ptr [esi + 5]
		mov dword ptr [edi + 0x24], eax
		movzx eax, word ptr [esi + 7]
		mov dword ptr [edi + 0x28], eax
		movzx eax, byte ptr [esi + 9]
		mov byte ptr [edi + 0x2c], al
		and byte ptr [edi + 0x2c], 0x40
		add esi, 0xa
		test al, 0x80
		_emit 0x74 /* je jmp_10065584 */
		_emit 0x13
		mov cl, al
		and cl, 7
		inc cl
		mov ebx, 1
		shl ebx, cl
		imul ebx, ebx, 3
		add esi, ebx
jmp_10065584:
		mov dword ptr [edi + 0x10], 0
		lodsb
		movzx ecx, al
		mov edx, 8
		push ecx
		push edx
		mov eax, 1
		shl eax, cl
		mov dword ptr [ebp - 4], eax
		inc eax
		mov dword ptr [ebp - 8], eax
		inc ecx
		mov dword ptr [edi + 0x1c], ecx
		mov ecx, dword ptr [ebp - 4]
		call GifInitCodes
		mov dword ptr [ebp - 0x10], 0xffff
		mov dword ptr [ebp - 0x14], 0
		mov byte ptr [edi + 0x2d], 0
		mov eax, dword ptr [edi + 0x24]
		mov dword ptr [edi + 0x20], eax
		mov dword ptr [edi + 8], 0
		mov dword ptr [edi + 0xc], 0
		pop edx
		pop ecx
jmp_100655d8:
		push ecx
		push edx
		mov edx, dword ptr [edi + 0x1c]
		cmp edx, 8
		_emit 0x7f /* jg jmp_100655eb */
		_emit 0x09
		push edx
		call GifReadCode
		pop edx
		jmp short jmp_10065608
jmp_100655eb:
		push edx
		mov edx, 8
		call GifReadCode
		pop edx
		push eax
		push edx
		sub edx, 8
		call GifReadCode
		pop edx
		shl eax, 8
		pop ebx
		or eax, ebx
jmp_10065608:
		mov dword ptr [ebp - 0xc], eax
		pop edx
		pop ecx
		cmp eax, dword ptr [ebp - 4]
		_emit 0x75 /* jne jmp_10065630 */
		_emit 0x1e
		push ecx
		push edx
		mov ecx, dword ptr [ebp - 4]
		call GifInitCodes
		pop edx
		pop ecx
		mov eax, ecx
		inc eax
		mov dword ptr [edi + 0x1c], eax
		mov dword ptr [ebp - 0x10], 0xffff
		jmp jmp_100656fb
jmp_10065630:
		cmp eax, dword ptr [ebp - 8]
		_emit 0x75 /* jne jmp_1006565c */
		_emit 0x27
jmp_10065635:
		cmp dword ptr [edi + 0x10], 0
		_emit 0x74 /* je jmp_10065641 */
		_emit 0x06
		lodsb
		dec dword ptr [edi + 0x10]
		jmp short jmp_10065635
jmp_10065641:
		lodsb
		and eax, 0xff
		mov dword ptr [edi + 0x10], eax
		cmp dword ptr [edi + 0x10], 0
		_emit 0x75 /* jne jmp_10065635 */
		_emit 0xe5
		mov dword ptr [ebp - 0x14], 0xffff
		jmp jmp_100656fb
jmp_1006565c:
		mov ebx, dword ptr [ebp - 0xc]
		cmp word ptr [edi + ebx*2 + 0x302e], -2
		_emit 0x74 /* je jmp_10065681 */
		_emit 0x17
		cmp dword ptr [ebp - 0x10], 0xffff
		_emit 0x74 /* je jmp_1006567f */
		_emit 0x0c
		push ecx
		push edx
		mov ecx, dword ptr [ebp - 0x10]
		call GifAddCode
		pop edx
		pop ecx
jmp_1006567f:
		jmp short jmp_10065690
jmp_10065681:
		push ecx
		push edx
		mov ebx, dword ptr [ebp - 0x10]
		mov ecx, dword ptr [ebp - 0x10]
		call GifAddCode
		pop edx
		pop ecx
jmp_10065690:
		push ecx
		push edx
		mov ebx, dword ptr [ebp - 0xc]
		push esi
		mov ecx, 0
		mov esi, edi
		add esi, 0x2e
jmp_100656a0:
		mov al, byte ptr [ebx + edi + 0x202e]
		mov byte ptr [esi], al
		inc esi
		inc ecx
		movzx ebx, word ptr [edi + ebx*2 + 0x302e]
		cmp ebx, 0xffff
		_emit 0x75 /* jne jmp_100656a0 */
		_emit 0xe5
		cmp edx, 1
		_emit 0x75 /* jne jmp_100656e1 */
		_emit 0x21
jmp_100656c0:
		dec esi
		mov al, byte ptr [esi]
		and eax, 1
		push ecx
		call GifPutPixel
		pop ecx
		mov al, byte ptr [esi]
		and eax, 0xff
		shr eax, 1
		push ecx
		call GifPutPixel
		pop ecx
		loop jmp_100656c0
		jmp short jmp_100656f2
jmp_100656e1:
		dec esi
		mov al, byte ptr [esi]
		and eax, 0xff
		push ecx
		call GifPutPixel
		pop ecx
		loop jmp_100656e1
jmp_100656f2:
		pop esi
		pop edx
		pop ecx
		mov eax, dword ptr [ebp - 0xc]
		mov dword ptr [ebp - 0x10], eax
jmp_100656fb:
		cmp dword ptr [ebp - 0x14], 0
		_emit 0x75 /* jne jmp_10065706 */
		_emit 0x05
		_emit 0xe9 /* jmp jmp_100655d8 */
		_emit 0xd2
		_emit 0xfe
		_emit 0xff
		_emit 0xff
jmp_10065706:
		mov eax, dword ptr [ebp - 0x18]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Copies the GIF's colour table to p_palette, scaled from 8 to 6 bits per component: the global
// one, then the image's local one over it.
// FUNCTION: MW2 0x1006570f
void ReadGifPalette(MechU8* p_gif, MechU8* p_palette)
{
	__asm {
		push es
		cld
		push ds
		pop es
		mov esi, p_gif
		mov al, byte ptr [esi + 0xa]
		add esi, 0xd
		test al, 0x80
		_emit 0x74 /* je jmp_10065743 */
		_emit 0x1d
		mov cl, al
		and cl, 7
		inc cl
		mov ebx, 1
		shl ebx, cl
		imul ebx, ebx, 3
		mov edi, p_palette
		mov ecx, ebx
jmp_1006573c:
		lodsb
		shr al, 2
		stosb
		loop jmp_1006573c
jmp_10065743:
		mov al, byte ptr [esi + 9]
		test al, 0x80
		_emit 0x74 /* je jmp_1006576a */
		_emit 0x20
		mov cl, al
		and cl, 7
		inc cl
		mov ebx, 1
		shl ebx, cl
		imul ebx, ebx, 3
		add esi, 0xa
		mov edi, p_palette
		mov ecx, ebx
jmp_10065763:
		lodsb
		shr al, 2
		stosb
		loop jmp_10065763
jmp_1006576a:
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

// Copies frame p_frame's palette entries into the 256-colour palette p_palette (RGB triples):
// each entry is a colour index and three bytes.
// FUNCTION: MW2 0x1006584b
void FUN_1006584b(void* p_shape, MechS32 p_frame, MechU8* p_palette)
{
	__asm {
		push es
		cld
		push ds
		pop es
		mov esi, p_shape
		add esi, 8
		mov eax, p_frame
		shl eax, 3
		add esi, eax
		add esi, 4
		mov esi, [esi]
		cmp esi, 0
		_emit 0x74 /* je jmp_10065890 */
		_emit 0x23
		add esi, p_shape
		lodsd
		mov ecx, eax
		mov edi, p_palette
jmp_10065876:
		lodsb
		and eax, 0xff
		mov ebx, eax
		shl ebx, 1
		add ebx, eax
		lodsb
		mov [ebx + edi], al
		inc ebx
		lodsw
		mov [ebx + edi], ax
		dec ecx
		_emit 0x75 /* jne jmp_10065876 */
		_emit 0xe6
jmp_10065890:
		pop es
	}
}

// Copies frame p_frame's palette entries (dwords) to p_out, if not NULL, and returns their count;
// 0 if the frame has none.
// FUNCTION: MW2 0x10065896
MechS32 FUN_10065896(void* p_shape, MechS32 p_frame, MechU32* p_out)
{
	__asm {
		push es
		cld
		push ds
		pop es
		mov esi, p_shape
		add esi, 8
		mov eax, p_frame
		shl eax, 3
		add esi, eax
		add esi, 4
		mov esi, [esi]
		cmp esi, 0
		_emit 0x75 /* jne jmp_100658bf */
		_emit 0x07
		mov eax, 0
		jmp short jmp_100658d8
jmp_100658bf:
		add esi, p_shape
		lodsd
		mov ebx, eax
		mov edi, p_out
		or edi, edi
		_emit 0x74 /* je jmp_100658d8 */
		_emit 0x0c
		mov ecx, ebx
		mov eax, 0
jmp_100658d3:
		movsd
		loop jmp_100658d3
		mov eax, ebx
jmp_100658d8:
		pop es
	}
}

// Copies p_in, if not NULL, over frame p_frame's palette entries and returns their count; 0 if
// the frame has none.
// FUNCTION: MW2 0x100658de
MechS32 FUN_100658de(void* p_shape, MechS32 p_frame, MechU32* p_in)
{
	__asm {
		push es
		cld
		push ds
		pop es
		mov esi, p_shape
		add esi, 8
		mov eax, p_frame
		shl eax, 3
		add esi, eax
		add esi, 4
		mov esi, [esi]
		cmp esi, 0
		_emit 0x75 /* jne jmp_10065907 */
		_emit 0x07
		mov eax, 0
		jmp short jmp_10065922
jmp_10065907:
		add esi, p_shape
		lodsd
		mov ebx, eax
		mov edi, esi
		mov esi, p_in
		or esi, esi
		_emit 0x74 /* je jmp_10065922 */
		_emit 0x0c
		mov ecx, ebx
		mov eax, 0
jmp_1006591d:
		movsd
		loop jmp_1006591d
		mov eax, ebx
jmp_10065922:
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

// Counts a shape's distinct frames (frames sharing data count once) and returns the count; lists
// the index of each distinct frame in p_out, if not NULL.
#ifdef COMPAT_MODE
MechS32 CountShpUniqueFrames(void* p_shape, MechS32* p_out)
{
	STUB(0x1006593b);
	return 0;
}
#else
// FUNCTION: MW2 0x1006593b
__declspec(naked) MechS32 CountShpUniqueFrames(void* p_shape, MechS32* p_out)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -4
		push ebx
		push esi
		push edi
		push es
		mov esi, [ebp + 8]
		mov ecx, [esi + 4]
		dec ecx
		add esi, 0x8
		mov [ebp - 4], esi
		mov ebx, 1
		mov edx, [ebp + 0xc]
		cmp edx, 0
		_emit 0x74 /* je jmp_10065968 */
		_emit 0x09
		mov dword ptr [edx], 0
		add edx, 4
jmp_10065968:
		cmp ecx, 0
		_emit 0x74 /* je jmp_10065995 */
		_emit 0x28
jmp_1006596d:
		add esi, 8
		mov eax, [esi]
		mov edi, [ebp - 4]
jmp_10065975:
		cmp eax, [edi]
		_emit 0x74 /* je jmp_10065993 */
		_emit 0x1a
		add edi, 8
		cmp edi, esi
		_emit 0x7c /* jl jmp_10065975 */
		_emit 0xf5
		cmp edx, 0
		_emit 0x74 /* je jmp_10065992 */
		_emit 0x0d
		mov eax, esi
		sub eax, [ebp - 4]
		shr eax, 3
		mov [edx], eax
		add edx, 4
jmp_10065992:
		inc ebx
jmp_10065993:
		loop jmp_1006596d
jmp_10065995:
		mov eax, ebx
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Counts a shape's distinct frames (frames sharing data count once) and returns the count; lists
// the index of each distinct frame in p_out, if not NULL. The frame table starts 4 bytes later than
// CountShpUniqueFrames's.
#ifdef COMPAT_MODE
MechS32 FUN_1006599d(void* p_shape, MechS32* p_out)
{
	STUB(0x1006599d);
	return 0;
}
#else
// FUNCTION: MW2 0x1006599d
__declspec(naked) MechS32 FUN_1006599d(void* p_shape, MechS32* p_out)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -4
		push ebx
		push esi
		push edi
		push es
		mov esi, [ebp + 8]
		mov ecx, [esi + 4]
		dec ecx
		add esi, 0xc
		mov [ebp - 4], esi
		mov ebx, 1
		mov edx, [ebp + 0xc]
		cmp edx, 0
		_emit 0x74 /* je jmp_100659ca */
		_emit 0x09
		mov dword ptr [edx], 0
		add edx, 4
jmp_100659ca:
		cmp ecx, 0
		_emit 0x74 /* je jmp_100659f7 */
		_emit 0x28
jmp_100659cf:
		add esi, 8
		mov eax, [esi]
		mov edi, [ebp - 4]
jmp_100659d7:
		cmp eax, [edi]
		_emit 0x74 /* je jmp_100659f5 */
		_emit 0x1a
		add edi, 8
		cmp edi, esi
		_emit 0x7c /* jl jmp_100659d7 */
		_emit 0xf5
		cmp edx, 0
		_emit 0x74 /* je jmp_100659f4 */
		_emit 0x0d
		mov eax, esi
		sub eax, [ebp - 4]
		shr eax, 3
		mov [edx], eax
		add edx, 4
jmp_100659f4:
		inc ebx
jmp_100659f5:
		loop jmp_100659cf
jmp_100659f7:
		mov eax, ebx
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

#pragma warning(default : 4035)

// STUB: MW2 0x10065a7b
MechS32 FUN_10065a7b(RenderTarget* p_dst, RenderTarget* p_src, MechS32 p_unk0x08, MechS32 p_unk0x0c)
{
	STUB(0x10065a7b);
	return 0;
}

// Counts the distinct colours in p_target's rectangle and lists them in p_out, if not NULL.
#ifdef COMPAT_MODE
MechS32 CountViewColors(RenderTarget* p_target, MechU32* p_out)
{
	STUB(0x10065e76);
	return 0;
}
#else
// FUNCTION: MW2 0x10065e76
__declspec(naked) MechS32 CountViewColors(RenderTarget* p_target, MechU32* p_out)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0xc
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov edi, offset g_unk0x100ac379
		mov eax, 0
		mov ecx, 0x40
		rep stosd
		mov esi, dword ptr [ebp + 8]
		mov ecx, dword ptr [esi + 0xc]
		sub ecx, dword ptr [esi + 4]
		mov dword ptr [ebp - 8], ecx
		mov ebx, dword ptr [esi + 0x10]
		sub ebx, dword ptr [esi + 8]
		mov esi, dword ptr [esi]
		mov eax, dword ptr [esi + 4]
		inc eax
		mov dword ptr [ebp - 0xc], eax
		mov esi, dword ptr [ebp + 8]
		mov eax, dword ptr [esi + 8]
		imul dword ptr [ebp - 0xc]
		add eax, dword ptr [esi + 4]
		mov esi, dword ptr [esi]
		mov esi, dword ptr [esi]
		add esi, eax
		mov edi, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 4], 0xffffffff
jmp_10065ecb:
		mov eax, 0
		movzx eax, byte ptr [ecx + esi]
		cmp byte ptr [g_unk0x100ac379 + eax], 0
		_emit 0x75 /* jne jmp_10065eed */
		_emit 0x10
		or byte ptr [g_unk0x100ac379 + eax], 1
		inc dword ptr [ebp - 4]
		cmp edi, 0
		_emit 0x74 /* je jmp_10065eed */
		_emit 0x01
		stosd
jmp_10065eed:
		dec ecx
		_emit 0x79 /* jns jmp_10065ecb */
		_emit 0xdb
		mov ecx, dword ptr [ebp - 8]
		add esi, dword ptr [ebp - 0xc]
		dec ebx
		_emit 0x79 /* jns jmp_10065ecb */
		_emit 0xd2
		mov eax, dword ptr [ebp - 4]
		inc eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif
