#include "shapelists.h"

#include "decomp.h"
#include "shape.h"
#include "shapelisthead.h"
#include "types.h"

#include <stddef.h>

DECOMP_SIZE_ASSERT(ShapeListHead, 0x18)

// GLOBAL: MW2 0x100ad5e8
Shape* g_unk0x100ad5e8 = NULL;

// GLOBAL: MW2 0x100ad5ec
Shape* g_unk0x100ad5ec = NULL;

// GLOBAL: MW2 0x100bef10
ShapeListHead g_unk0x100bef10;

// GLOBAL: MW2 0x100bef28
ShapeListHead g_unk0x100bef28;

// GLOBAL: MW2 0x100bef40
ShapeListHead g_unk0x100bef40;

// FUNCTION: MW2 0x1006d680
void FUN_1006d680(void)
{
	g_unk0x100bef10.m_prev = g_unk0x100bef10.m_prevCollider = NULL;
	g_unk0x100bef10.m_next = g_unk0x100bef10.m_nextCollider = NULL;
	g_unk0x100bef10.m_flags = 0x4000;
	g_unk0x100bef28.m_prev = g_unk0x100bef28.m_prevCollider = NULL;
	g_unk0x100bef28.m_next = g_unk0x100bef28.m_nextCollider = NULL;
	g_unk0x100bef28.m_flags = 0x4000;
	g_unk0x100bef40.m_prev = g_unk0x100bef40.m_prevCollider = NULL;
	g_unk0x100bef40.m_next = g_unk0x100bef40.m_nextCollider = NULL;
	g_unk0x100bef40.m_flags = 0x4000;
	g_unk0x100ad5e8 = (Shape*) &g_unk0x100bef10;
	g_unk0x100ad5ec = (Shape*) &g_unk0x100bef28;
}

// FUNCTION: MW2 0x1006d732
void FUN_1006d732(Shape* p_shape)
{
	Shape* list;

	if (g_unk0x100ad5e8 != (Shape*) &g_unk0x100bef10 || !p_shape) {
		return;
	}

	if (p_shape->m_flags & 0x1000) {
		list = (Shape*) &g_unk0x100bef28;
	}
	else {
		list = (Shape*) &g_unk0x100bef10;
	}

	FUN_1006dc3b(p_shape, list);
	if (p_shape->m_collisionType == 4) {
		p_shape->m_flags |= 0x800;
	}

	if (!(p_shape->m_flags & 0x800)) {
		if (g_unk0x100bef10.m_nextCollider) {
			g_unk0x100bef10.m_nextCollider->m_prevCollider = p_shape;
		}

		p_shape->m_nextCollider = g_unk0x100bef10.m_nextCollider;
		g_unk0x100bef10.m_nextCollider = p_shape;
		p_shape->m_prevCollider = (Shape*) &g_unk0x100bef10;
	}
}

// Operand order: the original compares p_shape with g_unk0x100ad5e8 the other way round.
// FUNCTION: MW2 0x1006d7fb
void FUN_1006d7fb(Shape* p_shape)
{
	if (!p_shape || p_shape == g_unk0x100ad5e8 || p_shape == (Shape*) &g_unk0x100bef28) {
		return;
	}

	FUN_1006dbe2(p_shape);
	if (p_shape->m_nextCollider) {
		p_shape->m_nextCollider->m_prevCollider = p_shape->m_prevCollider;
	}

	if (p_shape->m_prevCollider) {
		p_shape->m_prevCollider->m_nextCollider = p_shape->m_nextCollider;
	}

	p_shape->m_nextCollider = p_shape->m_prevCollider = NULL;
}

// FUNCTION: MW2 0x1006d88a
void FUN_1006d88a(Shape* p_shape)
{
	if (g_unk0x100ad5e8 != (Shape*) &g_unk0x100bef10 || !p_shape) {
		return;
	}

	FUN_1006dbe2(p_shape);
	FUN_1006dc3b(p_shape, (Shape*) &g_unk0x100bef40);
}

// FUNCTION: MW2 0x1006d8d1
void FUN_1006d8d1(Shape* p_shape)
{
	if (!p_shape) {
		return;
	}

	if (!(p_shape->m_flags & 0x800) || p_shape->m_collisionType == 4) {
		return;
	}

	p_shape->m_flags &= ~0x800;
	if (!p_shape->m_prev) {
		return;
	}

	if (!g_unk0x100ad5e8) {
		return;
	}

	if (g_unk0x100ad5e8->m_nextCollider) {
		g_unk0x100ad5e8->m_nextCollider->m_prevCollider = p_shape;
	}

	p_shape->m_nextCollider = g_unk0x100ad5e8->m_nextCollider;
	g_unk0x100ad5e8->m_nextCollider = p_shape;
	p_shape->m_prevCollider = g_unk0x100ad5e8;
}

// FUNCTION: MW2 0x1006d989
void FUN_1006d989(Shape* p_shape)
{
	if (!p_shape) {
		return;
	}

	if (p_shape->m_flags & 0x800) {
		return;
	}

	p_shape->m_flags |= 0x800;
	if (!g_unk0x100ad5e8) {
		return;
	}

	if (p_shape->m_nextCollider) {
		p_shape->m_nextCollider->m_prevCollider = p_shape->m_prevCollider;
	}

	if (p_shape->m_prevCollider) {
		p_shape->m_prevCollider->m_nextCollider = p_shape->m_nextCollider;
	}

	p_shape->m_nextCollider = p_shape->m_prevCollider = NULL;
}

// FUNCTION: MW2 0x1006da2d
void FUN_1006da2d(Shape* p_shape)
{
	if (!p_shape) {
		return;
	}

	if (p_shape->m_flags & 0x1000) {
		return;
	}

	p_shape->m_flags |= 0x1000;
	if (!p_shape->m_prev) {
		return;
	}

	FUN_1006dbe2(p_shape);
	FUN_1006dc3b(p_shape, (Shape*) &g_unk0x100bef28);
}

// FUNCTION: MW2 0x1006daa0
void FUN_1006daa0(Shape* p_shape)
{
	if (!p_shape) {
		return;
	}

	if (!(p_shape->m_flags & 0x1000) || (p_shape->m_kind & 0xf0) == 0x70) {
		return;
	}

	p_shape->m_flags &= ~0x1000;
	if (!p_shape->m_prev) {
		return;
	}

	FUN_1006dbe2(p_shape);
	FUN_1006dc3b(p_shape, (Shape*) &g_unk0x100bef10);
}

// Frees the shapes of all three lists.
// FUNCTION: MW2 0x1006db28
void FUN_1006db28(void)
{
	Shape* shape;

	if (g_unk0x100ad5e8 == (Shape*) &g_unk0x100bef10) {
		while (g_unk0x100bef10.m_next) {
			shape = g_unk0x100bef10.m_next;
			FUN_1006d7fb(g_unk0x100bef10.m_next);
			FreeShape(shape);
		}

		while (g_unk0x100bef28.m_next) {
			shape = g_unk0x100bef28.m_next;
			FUN_1006d7fb(g_unk0x100bef28.m_next);
			FreeShape(shape);
		}

		while (g_unk0x100bef40.m_next) {
			shape = g_unk0x100bef40.m_next;
			FUN_1006d7fb(g_unk0x100bef40.m_next);
			FreeShape(shape);
		}
	}
}

// Unlinks the shape from its list.
// FUNCTION: MW2 0x1006dbe2
void FUN_1006dbe2(Shape* p_shape)
{
	if (p_shape->m_next) {
		p_shape->m_next->m_prev = p_shape->m_prev;
	}

	if (p_shape->m_prev) {
		p_shape->m_prev->m_next = p_shape->m_next;
	}

	p_shape->m_next = p_shape->m_prev = NULL;
}

// Links the shape in at the front of p_list.
// FUNCTION: MW2 0x1006dc3b
void FUN_1006dc3b(Shape* p_shape, Shape* p_list)
{
	if (p_list->m_next) {
		p_list->m_next->m_prev = p_shape;
	}

	p_shape->m_next = p_list->m_next;
	p_list->m_next = p_shape;
	p_shape->m_prev = p_list;
}

// FUNCTION: MW2 0x1006dc7d
void FUN_1006dc7d(MechS32 p_enable)
{
	Shape* shape;
	Shape* next;

	if (p_enable) {
		for (shape = g_unk0x100bef28.m_next; shape != NULL; shape = next) {
			next = shape->m_next;
			if ((shape->m_kind & 0xf0) == 0xc0) {
				FUN_1006daa0(shape);
				FUN_1006d8d1(shape);
			}
		}
	}
	else {
		for (shape = g_unk0x100bef10.m_next; shape != NULL; shape = next) {
			next = shape->m_next;
			if ((shape->m_kind & 0xf0) == 0xc0) {
				FUN_1006da2d(shape);
				FUN_1006d989(shape);
			}
		}
	}
}
