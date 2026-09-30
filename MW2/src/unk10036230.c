/* Hand-written assembly: FUN_10038ced is a C function with an __asm body. */
#include "unk10036230.h"

#include "decomp.h"
#include "ray.h"
#include "simmain.h"
#include "slateheron.h"
#include "transform.h"
#include "types.h"

// The luma table FUN_10038d0d draws through, copied in by FUN_10038ced.
// GLOBAL: MW2 0x100a5630
MechU16 g_lumaTable[0x80] = {0};

// FUNCTION: MW2 0x10036853
void FUN_10036853(MechU32 p_flags)
{
	g_unk0x100a6cc8.m_unk0x50 ^= p_flags;
}

// FUNCTION: MW2 0x10036867
MechS32 FUN_10036867(MechU32 p_flags)
{
	return !(p_flags & g_unk0x100a6cc8.m_unk0x50);
}

// FUNCTION: MW2 0x10036891
void FUN_10036891(MechU32 p_flags, MechS32 p_enable)
{
	if (p_enable) {
		g_unk0x100a6cc8.m_unk0x50 &= ~p_flags;
	}
	else {
		g_unk0x100a6cc8.m_unk0x50 |= p_flags;
	}
}

// FUNCTION: MW2 0x100368bf
MechS32 FUN_100368bf(undefined4 p_unk0x00)
{
	return !g_unk0x100a6cc8.m_unk0x4c;
}

// FUNCTION: MW2 0x100368e8
void FUN_100368e8(undefined4 p_unk0x00, MechS32 p_enable)
{
	if (!p_enable) {
		g_unk0x100a6cc8.m_unk0x4c = TRUE;
	}
	else {
		g_unk0x100a6cc8.m_unk0x4c = FALSE;
	}
}

// Fills a polygon of p_count points (6 dwords each) in the shade of each point.
// STUB: MW2 0x10036918
void FUN_10036918(RenderTarget* p_target, MechS32 p_count, MechU32* p_points)
{
	STUB(0x10036918);
}

// STUB: MW2 0x1003763b
void FUN_1003763b(RenderTarget* p_target, MechS32 p_unk0x04, MechS32 p_count, MechU32* p_points)
{
	STUB(0x1003763b);
}

// Copies the 0x80-entry luma table p_table into g_lumaTable. A C function with an __asm body.
// FUNCTION: MW2 0x10038ced
void FUN_10038ced(MechU16* p_table)
{
	__asm {
		push es
		cld
		push ds
		pop es
		mov esi, p_table
		lea edi, g_lumaTable
		mov ecx, 0x40
		rep movsd
		pop es
	}
}

// STUB: MW2 0x10038d0d
void FUN_10038d0d(RenderTarget* p_target, MechS32 p_count, MechU32* p_points, PixelBuffer* p_source, MechS32 p_mode)
{
	STUB(0x10038d0d);
}

// STUB: MW2 0x10039a30
void FUN_10039a30(GraniteLattice0x18* p_model, Matrix* p_matrix)
{
	STUB(0x10039a30);
}

// STUB: MW2 0x10039b94
void FUN_10039b94(struct ScarletOrchid0x4c* p_shape, Matrix* p_matrix)
{
	STUB(0x10039b94);
}

// STUB: MW2 0x10039c96
MechS32 FUN_10039c96(
	MechS32 p_normalX,
	MechS32 p_normalY,
	MechS32 p_normalZ,
	MechS32 p_unk0x0c,
	MechS32 p_dx,
	MechS32 p_dz
)
{
	STUB(0x10039c96);
	return 0;
}

// Returns a distance from (p_x, p_y, p_z) to the shape.
// STUB: MW2 0x10039ccc
MechS32 FUN_10039ccc(struct ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	STUB(0x10039ccc);
	return 0;
}

// Returns the distance along the ray to the shape.
// STUB: MW2 0x1003a096
MechS32 FUN_1003a096(struct ScarletOrchid0x4c* p_shape, Ray* p_ray)
{
	STUB(0x1003a096);
	return 0;
}
