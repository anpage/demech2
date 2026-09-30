/* Hand-written assembly: FUN_10038ced, FUN_10039b94, FUN_10039c96, FUN_10039ccc, FUN_1003a05d and
   FUN_1003a096 are C functions with __asm bodies. */
#include "unk10036230.h"

#include "compat.h"
#include "decomp.h"
#include "duskmoth.h"
#include "emberfern.h"
#include "ray.h"
#include "simmain.h"
#include "slateheron.h"
#include "transform.h"
#include "types.h"
#include "unk1003a530.h"

// The luma table FUN_10038d0d draws through, copied in by FUN_10038ced.
// GLOBAL: MW2 0x100a5630
MechU16 g_lumaTable[0x80] = {0};

// Set by FUN_1004b980: FUN_100367c5's base shade, out of 0x80.
// GLOBAL: MW2 0x1010b540
MechS32 g_unk0x1010b540;

// Draws a face of a model (SlateHeron0x68::m_unk0x60).
// STUB: MW2 0x10036230
MechS32 FUN_10036230(undefined4 p_unk0x00, undefined4 p_unk0x04, undefined4 p_unk0x08, undefined4 p_unk0x0c)
{
	STUB(0x10036230);
	return 0;
}

// Returns a shade from 0 to 15 for a light level p_light (out of 0x80) and a brightness
// p_value, dimmed with the distance p_distance.
// FUNCTION: MW2 0x100367c5
MechS32 FUN_100367c5(MechS32 p_light, MechS32 p_value, MechS32 p_distance)
{
	MechS32 shade;

	shade = (((0x80 - g_unk0x1010b540) * p_light >> 7) + g_unk0x1010b540) * (p_value >> 1) / 0x440;
	if (g_unk0x100a6cc8.m_unk0x44) {
		shade -= (p_distance << 4) / g_unk0x100a6cc8.m_unk0x44 >> 4;
	}

	if (shade < 1) {
		shade = 0;
	}
	else if (shade > 15) {
		shade = 15;
	}

	return shade;
}

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

// Transforms p_model's vertices (their positions into m_unk0x0c-0x14) and face normals by
// p_matrix. The products are an __asm block.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x10039a30
void FUN_10039a30(GraniteLattice0x18* p_model, Matrix* p_matrix)
{
#ifdef COMPAT_MODE
	STUB(0x10039a30);
#else
	DuskMoth0x24* faces;
	MechS16 vertexCount;
	MechS32 vertexSize;
	EmberFern0x2c* vertices;
	MechS16 faceCount;
	MechS32 faceSize;

	vertices = (EmberFern0x2c*) (p_model + 1);
	faces = (DuskMoth0x24*) ((MechU8*) p_model + p_model->m_unk0x08);
	vertexSize = sizeof(EmberFern0x2c);
	faceSize = sizeof(DuskMoth0x24);
	vertexCount = p_model->m_unk0x04;
	faceCount = p_model->m_unk0x06;
	__asm {
		mov esi, p_matrix
		mov edi, vertices
	jmp_10039a78:
		mov eax, dword ptr [esi]
		imul dword ptr [edi]
		mov ecx, edx
		mov ebx, eax
		mov eax, dword ptr [esi + 4]
		imul dword ptr [edi + 4]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 8]
		imul dword ptr [edi + 8]
		add eax, ebx
		adc edx, ecx
		shrd eax, edx, 0x1d
		adc eax, dword ptr [esi + 0x24]
		mov dword ptr [edi + 0xc], eax
		mov eax, dword ptr [esi + 0xc]
		imul dword ptr [edi]
		mov ecx, edx
		mov ebx, eax
		mov eax, dword ptr [esi + 0x10]
		imul dword ptr [edi + 4]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 0x14]
		imul dword ptr [edi + 8]
		add eax, ebx
		adc edx, ecx
		shrd eax, edx, 0x1d
		adc eax, dword ptr [esi + 0x28]
		mov dword ptr [edi + 0x10], eax
		mov eax, dword ptr [esi + 0x18]
		imul dword ptr [edi]
		mov ecx, edx
		mov ebx, eax
		mov eax, dword ptr [esi + 0x1c]
		imul dword ptr [edi + 4]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 0x20]
		imul dword ptr [edi + 8]
		add eax, ebx
		adc edx, ecx
		shrd eax, edx, 0x1d
		adc eax, dword ptr [esi + 0x2c]
		mov dword ptr [edi + 0x14], eax
		add edi, vertexSize
		dec vertexCount
		je jmp_10039afe
		_emit 0xe9 /* jmp jmp_10039a78 */
		_emit 0x7a
		_emit 0xff
		_emit 0xff
		_emit 0xff
	jmp_10039afe:
		mov edi, faces
	jmp_10039b01:
		mov eax, dword ptr [esi]
		imul dword ptr [edi + 8]
		mov ecx, edx
		mov ebx, eax
		mov eax, dword ptr [esi + 4]
		imul dword ptr [edi + 0xc]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 8]
		imul dword ptr [edi + 0x10]
		add eax, ebx
		adc edx, ecx
		shrd eax, edx, 0x1d
		adc eax, 0
		mov dword ptr [edi + 0x14], eax
		mov eax, dword ptr [esi + 0xc]
		imul dword ptr [edi + 8]
		mov ecx, edx
		mov ebx, eax
		mov eax, dword ptr [esi + 0x10]
		imul dword ptr [edi + 0xc]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 0x14]
		imul dword ptr [edi + 0x10]
		add eax, ebx
		adc edx, ecx
		shrd eax, edx, 0x1d
		adc eax, 0
		mov dword ptr [edi + 0x18], eax
		mov eax, dword ptr [esi + 0x18]
		imul dword ptr [edi + 8]
		mov ecx, edx
		mov ebx, eax
		mov eax, dword ptr [esi + 0x1c]
		imul dword ptr [edi + 0xc]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 0x20]
		imul dword ptr [edi + 0x10]
		add eax, ebx
		adc edx, ecx
		shrd eax, edx, 0x1d
		adc eax, 0
		mov dword ptr [edi + 0x1c], eax
		add edi, faceSize
		dec faceCount
		je jmp_10039b8a
		_emit 0xe9 /* jmp jmp_10039b01 */
		_emit 0x77
		_emit 0xff
		_emit 0xff
		_emit 0xff
	}

	jmp_10039b8a : return;
#endif
}

// Transforms the shape's position (m_unk0x28-0x30) by p_matrix into m_unk0x34-0x3c, and bumps its
// transform count (m_unk0x48). The products are an __asm block.
// FUNCTION: MW2 0x10039b94
void FUN_10039b94(struct ScarletOrchid0x4c* p_shape, Matrix* p_matrix)
{
	p_shape->m_unk0x00 &= ~0x200;
	__asm {
		mov esi, p_matrix
		mov edi, p_shape
		mov eax, dword ptr [esi]
		imul dword ptr [edi + 0x28]
		mov ecx, edx
		mov ebx, eax
		mov eax, dword ptr [esi + 0x4]
		imul dword ptr [edi + 0x2c]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 0x8]
		imul dword ptr [edi + 0x30]
		add eax, ebx
		adc edx, ecx
		shrd eax, edx, 29
		adc eax, dword ptr [esi + 0x24]
		mov dword ptr [edi + 0x34], eax
		mov eax, dword ptr [esi + 0xc]
		imul dword ptr [edi + 0x28]
		mov ecx, edx
		mov ebx, eax
		mov eax, dword ptr [esi + 0x10]
		imul dword ptr [edi + 0x2c]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 0x14]
		imul dword ptr [edi + 0x30]
		add eax, ebx
		adc edx, ecx
		shrd eax, edx, 29
		adc eax, dword ptr [esi + 0x28]
		mov dword ptr [edi + 0x38], eax
		mov eax, dword ptr [esi + 0x18]
		imul dword ptr [edi + 0x28]
		mov ecx, edx
		mov ebx, eax
		mov eax, dword ptr [esi + 0x1c]
		imul dword ptr [edi + 0x2c]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 0x20]
		imul dword ptr [edi + 0x30]
		add eax, ebx
		adc edx, ecx
		shrd eax, edx, 29
		adc eax, dword ptr [esi + 0x2c]
		mov dword ptr [edi + 0x3c], eax
	}

	p_shape->m_unk0x48++;
}

// Transforms a shape and each of its models by p_matrix.
// FUNCTION: MW2 0x10039c36
void FUN_10039c36(struct ScarletOrchid0x4c* p_shape, Matrix* p_matrix)
{
	GraniteLattice0x18* model;

	FUN_10039b94(p_shape, p_matrix);
	for (model = p_shape->m_unk0x1c; model; model = model->m_unk0x0c) {
		FUN_10039a30(model, p_matrix);
		model->m_unk0x10 = p_shape->m_unk0x48;
	}
}

// Solves the plane p_normalX * x + p_normalY * y + p_normalZ * z + p_unk0x0c = 0 for y at
// (p_dx, p_dz): (p_normalX * p_dx + p_normalZ * p_dz + p_unk0x0c) / p_normalY, in 64 bits.
// FUNCTION: MW2 0x10039c96
MechS32 FUN_10039c96(
	MechS32 p_normalX,
	MechS32 p_normalY,
	MechS32 p_normalZ,
	MechS32 p_unk0x0c,
	MechS32 p_dx,
	MechS32 p_dz
)
{
	MechS32 result;

	__asm {
		mov eax, p_normalX
		imul p_dx
		mov esi, eax
		mov edi, edx
		mov eax, p_normalZ
		imul p_dz
		add eax, esi
		adc edx, edi
		add eax, p_unk0x0c
		adc edx, 0
		idiv p_normalY
		mov result, eax
	}

	return result;
}

// Returns an approximate distance from (p_x, p_y, p_z) to the shape's center, (4 * the largest +
// the others) / 4 of the offsets, or 0x7fffffff outside its bounding sphere.
// Stack-slot permutation: radius, deltaY and deltaZ.
// FUNCTION: MW2 0x10039ccc
MechS32 FUN_10039ccc(struct ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
#ifdef COMPAT_MODE
	STUB(0x10039ccc);
	return 0;
#else
	MechS32 result;
	MechS32 radius;
	MechS32 deltaX;
	MechS32 deltaY;
	MechS32 deltaZ;

	deltaX = p_shape->m_unk0x34 - p_x;
	deltaY = p_shape->m_unk0x38 - p_y;
	deltaZ = p_shape->m_unk0x3c - p_z;
	radius = p_shape->m_unk0x40;
	result = 0;
	__asm {
		mov ebx, radius
		mov ecx, ebx
		neg ecx
		mov eax, deltaX
		cmp eax, ebx
		jg jmp_10039d70
		cmp eax, ecx
		jl jmp_10039d70
		mov eax, deltaY
		cmp eax, ebx
		jg jmp_10039d70
		cmp eax, ecx
		jl jmp_10039d70
		mov eax, deltaZ
		cmp eax, ebx
		jg jmp_10039d70
		cmp eax, ecx
		jl jmp_10039d70
		imul eax
		mov ebx, eax
		mov ecx, edx
		mov eax, deltaX
		imul eax
		add ebx, eax
		adc ecx, edx
		mov eax, deltaY
		imul eax
		add ebx, eax
		adc ecx, edx
		mov eax, radius
		imul eax
		sub eax, ebx
		sbb edx, ecx
		jae jmp_10039d7a
jmp_10039d70:
		mov eax, 0x7fffffff
		jmp done
jmp_10039d7a:
		mov eax, deltaX
		cmp eax, 0
		jge jmp_10039d88
		neg eax
jmp_10039d88:
		mov ebx, deltaY
		cmp ebx, 0
		jge jmp_10039d96
		neg ebx
jmp_10039d96:
		cmp eax, ebx
		jge jmp_10039da4
		mov edx, eax
		mov eax, ebx
		mov ebx, edx
jmp_10039da4:
		mov ecx, deltaZ
		cmp ecx, 0
		jge jmp_10039db2
		neg ecx
jmp_10039db2:
		cmp eax, ecx
		jge jmp_10039dc0
		mov edx, eax
		mov eax, ecx
		mov ecx, edx
jmp_10039dc0:
		shl eax, 2
		add eax, ebx
		add eax, ecx
		shr eax, 2
		mov result, eax
	}

	return result;
done:;
#endif
}

// The normal of the triangle (p_x0, p_y0, p_z0), (p_x1, p_y1, p_z1), (p_x2, p_y2, p_z2): the cross
// product of its edges, scaled to 2.29 fixed point; returns the scale's exponent.
// STUB: MW2 0x10039dda
MechS32 FUN_10039dda(
	MechS32 p_x0,
	MechS32 p_y0,
	MechS32 p_z0,
	MechS32 p_x1,
	MechS32 p_y1,
	MechS32 p_z1,
	MechS32 p_x2,
	MechS32 p_y2,
	MechS32 p_z2,
	MechS32* p_nx,
	MechS32* p_ny,
	MechS32* p_nz
)
{
	STUB(0x10039dda);
	return 0;
}

// Returns p_value / (p_a - p_b) in 15.17 fixed point, or 0 if p_a and p_b are equal.
// FUNCTION: MW2 0x1003a05d
MechS32 FUN_1003a05d(MechS32 p_a, MechS32 p_b, MechS32 p_value)
{
	MechS32 result;

	result = 0;
	__asm {
		mov ebx, p_a
		sub ebx, p_b
		jz done
		mov eax, p_value
		cdq
		shld edx, eax, 17
		shl eax, 17
		idiv ebx
		mov result, eax
done:
	}

	return result;
}

// Returns how far along p_ray it passes closest to the shape's center, less the radius, or
// 0x7fffffff when it misses the bounding sphere or ends first; 10 when the ray starts inside it.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1003a096
MechS32 FUN_1003a096(struct ScarletOrchid0x4c* p_shape, Ray* p_ray)
{
#ifdef COMPAT_MODE
	STUB(0x1003a096);
	return 0;
#else
	MechS32 deltaX;
	MechS32 radius;
	MechS32 deltaY;
	MechS32 excess;
	MechS32 deltaZ;
	MechS32 t;

	radius = p_shape->m_unk0x40;
	if (radius <= 0) {
		return 0x7fffffff;
	}

	deltaX = p_shape->m_unk0x34 - p_ray->m_x0;
	deltaY = p_shape->m_unk0x38 - p_ray->m_y0;
	deltaZ = p_shape->m_unk0x3c - p_ray->m_z0;
	__asm {
		mov eax, deltaX
		imul eax
		mov ebx, eax
		mov ecx, edx
		mov eax, deltaY
		imul eax
		add ebx, eax
		adc ecx, edx
		mov eax, deltaZ
		imul eax
		add ebx, eax
		adc ecx, edx
		mov eax, radius
		imul eax
		sub ebx, eax
		sbb ecx, edx
		jae jmp_1003a11c
		mov eax, 10
		jmp done
jmp_1003a11c:
		mov excess, ebx
		mov ebx, p_ray
		mov eax, deltaX
		imul dword ptr [ebx + 0x18]
		mov edi, eax
		mov esi, edx
		mov eax, deltaY
		imul dword ptr [ebx + 0x1c]
		add edi, eax
		adc esi, edx
		mov eax, deltaZ
		imul dword ptr [ebx + 0x20]
		add edi, eax
		adc esi, edx
		jae jmp_1003a150
		mov eax, 0x7fffffff
		jmp done
jmp_1003a150:
		mov edx, esi
		mov eax, edi
		idiv dword ptr [ebx + 0x30]
		mov t, eax
		mov ebx, excess
		imul eax
		sub eax, ebx
		sbb edx, ecx
		jae jmp_1003a173
		mov eax, 0x7fffffff
		jmp done
jmp_1003a173:
	}

	if ((t -= radius) < 0)
	{
		t = 0;
	}

	return t >= GetRayLength(p_ray) ? 0x7fffffff : t;
done:;
#endif
}
