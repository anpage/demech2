/* Hand-written assembly: FUN_10039b94, FUN_10039c96, FUN_10039ccc, FUN_10039dda, FUN_1003a05d and
   FUN_1003a096 are C functions with __asm bodies. */
#include "unk10039a30.h"

#include "clock.h"
#include "compat.h"
#include "decomp.h"
#include "duskmoth.h"
#include "emberfern.h"
#include "ray.h"
#include "slateheron.h"
#include "transform.h"
#include "types.h"
#include "unk1003a530.h"

#pragma warning(disable : 4102) /* a label only an __asm block jumps to */

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
// product of its edges, scaled to 2.29 fixed point; returns the scale's exponent. A degenerate
// triangle gets the normal (-1, -1, -1) and 0. The products and the scaling are __asm blocks.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x10039dda
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
#ifdef COMPAT_MODE
	STUB(0x10039dda);
	return 0;
#else
	MechS32 nyAbs;
	MechS32 nxAbs;
	MechS32 nzAbs;
	MechS32 nyHigh;
	MechS32 nx;
	MechS32 nzHigh;
	MechS32 ny;
	MechS16 shift;
	MechS32 nz;
	MechS32 nyAbsHigh;
	MechS32 nxAbsHigh;
	MechS32 nzAbsHigh;
	MechS32 nxHigh;

	__asm {
		mov eax, p_y1
		sub eax, p_y0
		mov ecx, p_z2
		sub ecx, p_z1
		imul ecx
		mov edi, edx
		mov esi, eax
		mov eax, p_y2
		sub eax, p_y1
		mov ecx, p_z1
		sub ecx, p_z0
		imul ecx
		sub esi, eax
		sbb edi, edx
		mov nxHigh, edi
		mov nx, esi
		jge jmp_10039e1d
		not edi
		not esi
		add esi, 1
		adc edi, 0
jmp_10039e1d:
		mov nxAbsHigh, edi
		mov nxAbs, esi
		mov eax, p_z1
		sub eax, p_z0
		mov ecx, p_x2
		sub ecx, p_x1
		imul ecx
		mov edi, edx
		mov esi, eax
		mov eax, p_z2
		sub eax, p_z1
		mov ecx, p_x1
		sub ecx, p_x0
		imul ecx
		sub esi, eax
		sbb edi, edx
		mov nyHigh, edi
		mov ny, esi
		jge jmp_10039e5d
		not edi
		not esi
		add esi, 1
		adc edi, 0
jmp_10039e5d:
		mov nyAbsHigh, edi
		mov nyAbs, esi
		mov eax, p_x1
		sub eax, p_x0
		mov ecx, p_y2
		sub ecx, p_y1
		imul ecx
		mov edi, edx
		mov esi, eax
		mov eax, p_x2
		sub eax, p_x1
		mov ecx, p_y1
		sub ecx, p_y0
		imul ecx
		sub esi, eax
		sbb edi, edx
		mov nzHigh, edi
		mov nz, esi
		jge jmp_10039e9d
		not edi
		not esi
		add esi, 1
		adc edi, 0
jmp_10039e9d:
		mov nzAbsHigh, edi
		mov nzAbs, esi
		or esi, nyAbs
		or esi, nxAbs
		or edi, nyAbsHigh
		or edi, nxAbsHigh
		je jmp_10039f0e
		xor ax, ax
		test edi, 0xffff0000
		je jmp_10039ecb
		add ax, 0x10
		shr edi, 0x10
jmp_10039ecb:
		test edi, 0xff00
		je jmp_10039ede
		add ax, 8
		shr edi, 8
jmp_10039ede:
		shl edi, 8
		bsr cx, di
		sub cx, 5
		add cx, ax
		mov eax, nxHigh
		shrd nx, eax, cl
		mov eax, nyHigh
		shrd ny, eax, cl
		mov eax, nzHigh
		shrd nz, eax, cl
		add cx, 0x1d
		mov shift, cx
		jmp done
jmp_10039f0e:
		or esi, esi
		je zero
		mov ax, 0x18
		test esi, 0xffff0000
		je jmp_10039f2d
		sub ax, 0x10
		shr esi, 0x10
jmp_10039f2d:
		test esi, 0xff00
		je jmp_10039f40
		sub ax, 8
		shr esi, 8
jmp_10039f40:
		shl esi, 8
		bsr cx, si
		neg cx
		add cx, 0xd
		add cx, ax
		je jmp_10039f75
		jg jmp_10039f82
		neg cx
		mov eax, nxHigh
		shrd nx, eax, cl
		mov eax, nyHigh
		shrd ny, eax, cl
		mov eax, nzHigh
		shrd nz, eax, cl
jmp_10039f75:
		add cx, 0x1d
		mov shift, cx
		jmp done
jmp_10039f82:
		shl nx, cl
		shl ny, cl
		shl nz, cl
		mov ax, 0x1d
		sub ax, cx
		mov shift, ax
done:
	}
	goto normalize;

zero:
	*p_nx = *p_ny = *p_nz = -1;
	return 0;

normalize:
	__asm {
		mov eax, nx
		sar eax, 0x10
		imul ax
		xor ebx, ebx
		mov bx, dx
		mov cx, ax
		mov eax, ny
		sar eax, 0x10
		imul ax
		add cx, ax
		adc bx, dx
		mov eax, nz
		sar eax, 0x10
		imul ax
		add cx, ax
		adc bx, dx
		shr bx, 4
		shl bx, 1
		add ebx, dword ptr [g_sqrtTable]
		mov cx, word ptr [ebx]
		movzx ecx, cx
		mov eax, nx
		cdq
		shld edx, eax, 0xd
		shl eax, 0xd
		idiv ecx
		mov nx, eax
		mov eax, ny
		cdq
		shld edx, eax, 0xd
		shl eax, 0xd
		idiv ecx
		mov ny, eax
		mov eax, nz
		cdq
		shld edx, eax, 0xd
		shl eax, 0xd
		idiv ecx
		mov nz, eax
	}

	*p_nx = -nx;
	*p_ny = -ny;
	*p_nz = -nz;
	return shift;
#endif
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
