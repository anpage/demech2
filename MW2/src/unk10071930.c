/* Conversions between the screen and the eyepoint's view, with the view matrix's second column.
   Hand-written assembly: each function's products and quotients are an __asm block (imul/idiv on
   64-bit intermediates). */
#include "unk10071930.h"

#include "eyepoint.h"
#include "types.h"

#pragma warning(disable : 4102) /* a label only the __asm block jumps to */

// Returns whether the screen point (p_x, p_y) lies below the horizon of p_eyepoint's view.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10071930
MechS32 FUN_10071930(MechS32 p_x, MechS32 p_y, Eyepoint* p_eyepoint)
{
	MechS32 offsetX;
	MechS32 offsetY;
	MechS32 b;
	MechS32 d;
	MechS32 a;
	MechS32 c;
	MechS32 e;

	offsetX = p_x - p_eyepoint->m_centerX;
	offsetY = -p_y + p_eyepoint->m_centerY;
	b = p_eyepoint->m_unk0x94;
	d = p_eyepoint->m_unk0x98;
	a = p_eyepoint->m_unk0x54.m_rows[0][1];
	c = p_eyepoint->m_unk0x54.m_rows[1][1];
	e = p_eyepoint->m_unk0x54.m_rows[2][1];
	__asm {
		mov eax, offsetX
		imul a
		idiv b
		mov ecx, eax
		mov eax, offsetY
		imul c
		idiv d
		add ecx, eax
		mov eax, e
		neg eax
		sar eax, 16
		cmp ecx, eax
		jge below
	}
	return FALSE;

below:
	return TRUE;
}

// Returns the screen y of the horizon at screen x p_x in p_eyepoint's view.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x100719ca
MechS32 FUN_100719ca(MechS32 p_x, Eyepoint* p_eyepoint)
{
	MechS32 y;
	MechS32 a;
	MechS32 b;
	MechS32 d;
	MechS32 c;
	MechS32 e;

	b = p_eyepoint->m_unk0x94;
	d = p_eyepoint->m_unk0x98;
	a = p_eyepoint->m_unk0x54.m_rows[0][1];
	c = p_eyepoint->m_unk0x54.m_rows[1][1];
	e = p_eyepoint->m_unk0x54.m_rows[2][1];
	p_x -= p_eyepoint->m_centerX;
	__asm {
		mov eax, p_x
		imul a
		idiv b
		mov edx, e
		sar edx, 16
		add eax, edx
		neg eax
		imul d
		idiv c
		mov y, eax
	}
	return -y + p_eyepoint->m_centerY;
}

// Returns the screen x of the horizon at screen y p_y in p_eyepoint's view.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10071a4c
MechS32 FUN_10071a4c(MechS32 p_y, Eyepoint* p_eyepoint)
{
	MechS32 x;
	MechS32 a;
	MechS32 b;
	MechS32 d;
	MechS32 c;
	MechS32 e;

	b = p_eyepoint->m_unk0x94;
	d = p_eyepoint->m_unk0x98;
	a = p_eyepoint->m_unk0x54.m_rows[0][1];
	c = p_eyepoint->m_unk0x54.m_rows[1][1];
	e = p_eyepoint->m_unk0x54.m_rows[2][1];
	p_y = -p_y + p_eyepoint->m_centerY;
	__asm {
		mov eax, p_y
		imul c
		idiv d
		mov edx, e
		sar edx, 16
		add eax, edx
		neg eax
		imul b
		idiv a
		mov x, eax
	}
	return p_eyepoint->m_centerX + x;
}
