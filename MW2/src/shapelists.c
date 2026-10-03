#include "shapelists.h"

#include "decomp.h"
#include "shape.h"
#include "shapelisthead.h"
#include "types.h"

#include <stddef.h>

DECOMP_SIZE_ASSERT(ShapeListHead, 0x18)

// GLOBAL: MW2 0x100ad5e8
ScarletOrchid0x4c* g_unk0x100ad5e8 = NULL;

// GLOBAL: MW2 0x100ad5ec
ScarletOrchid0x4c* g_unk0x100ad5ec = NULL;

// GLOBAL: MW2 0x100bef10
ShapeListHead g_unk0x100bef10;

// GLOBAL: MW2 0x100bef28
ShapeListHead g_unk0x100bef28;

// GLOBAL: MW2 0x100bef40
ShapeListHead g_unk0x100bef40;

// FUNCTION: MW2 0x1006d680
void FUN_1006d680(void)
{
	g_unk0x100bef10.m_unk0x04 = g_unk0x100bef10.m_unk0x0c = NULL;
	g_unk0x100bef10.m_unk0x08 = g_unk0x100bef10.m_unk0x10 = NULL;
	g_unk0x100bef10.m_unk0x00 = 0x4000;
	g_unk0x100bef28.m_unk0x04 = g_unk0x100bef28.m_unk0x0c = NULL;
	g_unk0x100bef28.m_unk0x08 = g_unk0x100bef28.m_unk0x10 = NULL;
	g_unk0x100bef28.m_unk0x00 = 0x4000;
	g_unk0x100bef40.m_unk0x04 = g_unk0x100bef40.m_unk0x0c = NULL;
	g_unk0x100bef40.m_unk0x08 = g_unk0x100bef40.m_unk0x10 = NULL;
	g_unk0x100bef40.m_unk0x00 = 0x4000;
	g_unk0x100ad5e8 = (ScarletOrchid0x4c*) &g_unk0x100bef10;
	g_unk0x100ad5ec = (ScarletOrchid0x4c*) &g_unk0x100bef28;
}

// FUNCTION: MW2 0x1006d732
void FUN_1006d732(ScarletOrchid0x4c* p_shape)
{
	ScarletOrchid0x4c* list;

	if (g_unk0x100ad5e8 != (ScarletOrchid0x4c*) &g_unk0x100bef10 || !p_shape) {
		return;
	}

	if (p_shape->m_unk0x00 & 0x1000) {
		list = (ScarletOrchid0x4c*) &g_unk0x100bef28;
	}
	else {
		list = (ScarletOrchid0x4c*) &g_unk0x100bef10;
	}

	FUN_1006dc3b(p_shape, list);
	if (p_shape->m_unk0x24 == 4) {
		p_shape->m_unk0x00 |= 0x800;
	}

	if (!(p_shape->m_unk0x00 & 0x800)) {
		if (g_unk0x100bef10.m_unk0x10) {
			g_unk0x100bef10.m_unk0x10->m_unk0x0c = p_shape;
		}

		p_shape->m_unk0x10 = g_unk0x100bef10.m_unk0x10;
		g_unk0x100bef10.m_unk0x10 = p_shape;
		p_shape->m_unk0x0c = (ScarletOrchid0x4c*) &g_unk0x100bef10;
	}
}

// Operand order: the original compares p_shape with g_unk0x100ad5e8 the other way round.
// FUNCTION: MW2 0x1006d7fb
void FUN_1006d7fb(ScarletOrchid0x4c* p_shape)
{
	if (!p_shape || p_shape == g_unk0x100ad5e8 || p_shape == (ScarletOrchid0x4c*) &g_unk0x100bef28) {
		return;
	}

	FUN_1006dbe2(p_shape);
	if (p_shape->m_unk0x10) {
		p_shape->m_unk0x10->m_unk0x0c = p_shape->m_unk0x0c;
	}

	if (p_shape->m_unk0x0c) {
		p_shape->m_unk0x0c->m_unk0x10 = p_shape->m_unk0x10;
	}

	p_shape->m_unk0x10 = p_shape->m_unk0x0c = NULL;
}

// FUNCTION: MW2 0x1006d88a
void FUN_1006d88a(ScarletOrchid0x4c* p_shape)
{
	if (g_unk0x100ad5e8 != (ScarletOrchid0x4c*) &g_unk0x100bef10 || !p_shape) {
		return;
	}

	FUN_1006dbe2(p_shape);
	FUN_1006dc3b(p_shape, (ScarletOrchid0x4c*) &g_unk0x100bef40);
}

// FUNCTION: MW2 0x1006d8d1
void FUN_1006d8d1(ScarletOrchid0x4c* p_shape)
{
	if (!p_shape) {
		return;
	}

	if (!(p_shape->m_unk0x00 & 0x800) || p_shape->m_unk0x24 == 4) {
		return;
	}

	p_shape->m_unk0x00 &= ~0x800;
	if (!p_shape->m_unk0x04) {
		return;
	}

	if (!g_unk0x100ad5e8) {
		return;
	}

	if (g_unk0x100ad5e8->m_unk0x10) {
		g_unk0x100ad5e8->m_unk0x10->m_unk0x0c = p_shape;
	}

	p_shape->m_unk0x10 = g_unk0x100ad5e8->m_unk0x10;
	g_unk0x100ad5e8->m_unk0x10 = p_shape;
	p_shape->m_unk0x0c = g_unk0x100ad5e8;
}

// FUNCTION: MW2 0x1006d989
void FUN_1006d989(ScarletOrchid0x4c* p_shape)
{
	if (!p_shape) {
		return;
	}

	if (p_shape->m_unk0x00 & 0x800) {
		return;
	}

	p_shape->m_unk0x00 |= 0x800;
	if (!g_unk0x100ad5e8) {
		return;
	}

	if (p_shape->m_unk0x10) {
		p_shape->m_unk0x10->m_unk0x0c = p_shape->m_unk0x0c;
	}

	if (p_shape->m_unk0x0c) {
		p_shape->m_unk0x0c->m_unk0x10 = p_shape->m_unk0x10;
	}

	p_shape->m_unk0x10 = p_shape->m_unk0x0c = NULL;
}

// FUNCTION: MW2 0x1006da2d
void FUN_1006da2d(ScarletOrchid0x4c* p_shape)
{
	if (!p_shape) {
		return;
	}

	if (p_shape->m_unk0x00 & 0x1000) {
		return;
	}

	p_shape->m_unk0x00 |= 0x1000;
	if (!p_shape->m_unk0x04) {
		return;
	}

	FUN_1006dbe2(p_shape);
	FUN_1006dc3b(p_shape, (ScarletOrchid0x4c*) &g_unk0x100bef28);
}

// FUNCTION: MW2 0x1006daa0
void FUN_1006daa0(ScarletOrchid0x4c* p_shape)
{
	if (!p_shape) {
		return;
	}

	if (!(p_shape->m_unk0x00 & 0x1000) || (p_shape->m_unk0x02 & 0xf0) == 0x70) {
		return;
	}

	p_shape->m_unk0x00 &= ~0x1000;
	if (!p_shape->m_unk0x04) {
		return;
	}

	FUN_1006dbe2(p_shape);
	FUN_1006dc3b(p_shape, (ScarletOrchid0x4c*) &g_unk0x100bef10);
}

// Frees the shapes of all three lists.
// FUNCTION: MW2 0x1006db28
void FUN_1006db28(void)
{
	ScarletOrchid0x4c* shape;

	if (g_unk0x100ad5e8 == (ScarletOrchid0x4c*) &g_unk0x100bef10) {
		while (g_unk0x100bef10.m_unk0x08) {
			shape = g_unk0x100bef10.m_unk0x08;
			FUN_1006d7fb(g_unk0x100bef10.m_unk0x08);
			FUN_1003ac5f(shape);
		}

		while (g_unk0x100bef28.m_unk0x08) {
			shape = g_unk0x100bef28.m_unk0x08;
			FUN_1006d7fb(g_unk0x100bef28.m_unk0x08);
			FUN_1003ac5f(shape);
		}

		while (g_unk0x100bef40.m_unk0x08) {
			shape = g_unk0x100bef40.m_unk0x08;
			FUN_1006d7fb(g_unk0x100bef40.m_unk0x08);
			FUN_1003ac5f(shape);
		}
	}
}

// Unlinks the shape from its list.
// FUNCTION: MW2 0x1006dbe2
void FUN_1006dbe2(ScarletOrchid0x4c* p_shape)
{
	if (p_shape->m_unk0x08) {
		p_shape->m_unk0x08->m_unk0x04 = p_shape->m_unk0x04;
	}

	if (p_shape->m_unk0x04) {
		p_shape->m_unk0x04->m_unk0x08 = p_shape->m_unk0x08;
	}

	p_shape->m_unk0x08 = p_shape->m_unk0x04 = NULL;
}

// Links the shape in at the front of p_list.
// FUNCTION: MW2 0x1006dc3b
void FUN_1006dc3b(ScarletOrchid0x4c* p_shape, ScarletOrchid0x4c* p_list)
{
	if (p_list->m_unk0x08) {
		p_list->m_unk0x08->m_unk0x04 = p_shape;
	}

	p_shape->m_unk0x08 = p_list->m_unk0x08;
	p_list->m_unk0x08 = p_shape;
	p_shape->m_unk0x04 = p_list;
}

// FUNCTION: MW2 0x1006dc7d
void FUN_1006dc7d(MechS32 p_enable)
{
	ScarletOrchid0x4c* shape;
	ScarletOrchid0x4c* next;

	if (p_enable) {
		for (shape = g_unk0x100bef28.m_unk0x08; shape != NULL; shape = next) {
			next = shape->m_unk0x08;
			if ((shape->m_unk0x02 & 0xf0) == 0xc0) {
				FUN_1006daa0(shape);
				FUN_1006d8d1(shape);
			}
		}
	}
	else {
		for (shape = g_unk0x100bef10.m_unk0x08; shape != NULL; shape = next) {
			next = shape->m_unk0x08;
			if ((shape->m_unk0x02 & 0xf0) == 0xc0) {
				FUN_1006da2d(shape);
				FUN_1006d989(shape);
			}
		}
	}
}
