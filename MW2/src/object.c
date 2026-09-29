#include "object.h"

#include "clock.h"
#include "debris.h"
#include "decomp.h"
#include "simmain.h"
#include "staticmem.h"
#include "transform.h"
#include "types.h"
#include "unk1001ce90.h"
#include "unk10034a40.h"
#include "unk10036230.h"
#include "unk1003a530.h"
#include "unk100563d0.h"
#include "unk1006d680.h"

#include <mbstring.h>
#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(AmberWillow0x7c, 0x7c)

// GLOBAL: MW2 0x100a00c8
MechU32 g_unk0x100a00c8 = 100;

// The only diff is a stack-slot permutation of obj and last.
// FUNCTION: MW2 0x100012d0
AmberWillow0x7c* FUN_100012d0(AmberWillow0x7c* p_parent, MechU32 p_flags)
{
	AmberWillow0x7c* obj;
	AmberWillow0x7c* last;

	if (p_flags & 2) {
		obj = StaticPoolAlloc(sizeof(AmberWillow0x7c), g_unk0x100a9430);
		if (obj == NULL) {
			return NULL;
		}

		obj->m_flags = c_objectPooled;
	}
	else if (p_flags & 4) {
		obj = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(AmberWillow0x7c));
		if (obj == NULL) {
			return NULL;
		}

		obj->m_flags = c_objectHeap;
	}
	else {
		return NULL;
	}

	obj->m_parent = p_parent;
	obj->m_firstChild = NULL;
	obj->m_nextSibling = NULL;

	if (p_parent) {
		if (p_flags & 0x10) {
			last = FUN_100020a6(p_parent);
			if (last) {
				last->m_nextSibling = obj;
			}
			else {
				p_parent->m_firstChild = obj;
			}
		}
		else {
			obj->m_nextSibling = p_parent->m_firstChild;
			p_parent->m_firstChild = obj;
		}
	}

	FUN_1000dd4d(&obj->m_local);

	if (p_parent) {
		obj->m_world = p_parent->m_world;
	}
	else {
		FUN_1000dd4d(&obj->m_world);
	}

	obj->m_unk0x6c = NULL;
	obj->m_flags |= c_objectDirty;
	obj->m_name = NULL;
	obj->m_unk0x78 = g_unk0x100a00c8;
	g_unk0x100a00c8 = ((g_unk0x100a00c8 + 7) & 0x7f) + 0x40;
	return obj;
}

// FUNCTION: MW2 0x1000145a
AmberWillow0x7c* FUN_1000145a(AmberWillow0x7c* p_parent, void* p_memory)
{
	AmberWillow0x7c* obj = p_memory;

	obj->m_parent = p_parent;
	obj->m_firstChild = NULL;

	if (p_parent) {
		obj->m_nextSibling = p_parent->m_firstChild;
		p_parent->m_firstChild = obj;
	}
	else {
		obj->m_nextSibling = NULL;
	}

	FUN_1000dd4d(&obj->m_local);

	if (p_parent) {
		obj->m_world = p_parent->m_world;
	}
	else {
		FUN_1000dd4d(&obj->m_world);
	}

	obj->m_unk0x6c = NULL;
	obj->m_flags = c_objectDirty;
	obj->m_name = NULL;
	obj->m_unk0x78 = g_unk0x100a00c8;
	g_unk0x100a00c8 = ((g_unk0x100a00c8 + 7) & 0x7f) + 0x40;
	return obj;
}

// FUNCTION: MW2 0x10001532
void FUN_10001532(AmberWillow0x7c* p_obj, ScarletOrchid0x4c* p_unk0x6c)
{
	p_obj->m_unk0x6c = p_unk0x6c;
	p_obj->m_flags |= c_objectDirty;
}

// FUNCTION: MW2 0x1000154d
ScarletOrchid0x4c* FUN_1000154d(AmberWillow0x7c* p_obj)
{
	return p_obj->m_unk0x6c;
}

// FUNCTION: MW2 0x10001563
MechChar* FUN_10001563(AmberWillow0x7c* p_obj)
{
	return p_obj->m_name;
}

// FUNCTION: MW2 0x10001579
void FUN_10001579(AmberWillow0x7c* p_obj, MechChar* p_name)
{
	p_obj->m_name = (MechChar*) _mbsdup((unsigned char*) p_name);
}

// FUNCTION: MW2 0x10001596
void GetObjWorldPos(AmberWillow0x7c* p_obj, undefined4* p_unk0x04, undefined4* p_unk0x08, undefined4* p_unk0x0c)
{
	FUN_1000e2ea(&p_obj->m_world, p_unk0x04, p_unk0x08, p_unk0x0c);
}

// FUNCTION: MW2 0x100015bc
void FUN_100015bc(AmberWillow0x7c* p_obj, undefined4* p_unk0x04, undefined4* p_unk0x08, undefined4* p_unk0x0c)
{
	FUN_1000e2ea(&p_obj->m_local, p_unk0x04, p_unk0x08, p_unk0x0c);
}

// FUNCTION: MW2 0x100015e2
void GetObjPosition(AmberWillow0x7c* p_obj, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	*p_x = p_obj->m_world.m_rows[3][0];
	*p_y = p_obj->m_world.m_rows[3][1];
	*p_z = p_obj->m_world.m_rows[3][2];
}

// FUNCTION: MW2 0x1000160e
void FUN_1000160e(AmberWillow0x7c* p_obj, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	*p_x = p_obj->m_local.m_rows[3][0];
	*p_y = p_obj->m_local.m_rows[3][1];
	*p_z = p_obj->m_local.m_rows[3][2];
}

// FUNCTION: MW2 0x1000163a
void SetObjPosition(AmberWillow0x7c* p_obj, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	p_obj->m_local.m_rows[3][0] = p_x;
	p_obj->m_local.m_rows[3][1] = p_y;
	p_obj->m_local.m_rows[3][2] = p_z;
	p_obj->m_flags |= c_objectDirty;
}

// FUNCTION: MW2 0x10001667
void FUN_10001667(AmberWillow0x7c* p_obj, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	p_obj->m_local.m_rows[3][0] += p_x;
	p_obj->m_local.m_rows[3][1] += p_y;
	p_obj->m_local.m_rows[3][2] += p_z;
	p_obj->m_flags |= c_objectDirty;
}

// FUNCTION: MW2 0x10001694
void FUN_10001694(AmberWillow0x7c* p_obj, Matrix* p_matrix)
{
	p_obj->m_local = *p_matrix;
	p_obj->m_flags |= c_objectDirty;
}

// FUNCTION: MW2 0x100016b6
void FUN_100016b6(AmberWillow0x7c* p_obj, Matrix* p_matrix)
{
	FUN_1000dbba(&p_obj->m_local, p_matrix, &p_obj->m_local);
	p_obj->m_flags |= c_objectDirty;

	p_obj->m_unk0x78--;
	if (p_obj->m_unk0x78 <= 0) {
		p_obj->m_unk0x78 = g_unk0x100a00c8;
		g_unk0x100a00c8 = ((g_unk0x100a00c8 + 7) & 0x7f) + 0x40;
		FUN_1007cbf1(&p_obj->m_local);
	}
}

// FUNCTION: MW2 0x10001722
void FUN_10001722(AmberWillow0x7c* p_obj, Matrix* p_matrix)
{
	p_obj->m_local.m_rows[0][0] = p_matrix->m_rows[0][0];
	p_obj->m_local.m_rows[0][1] = p_matrix->m_rows[0][1];
	p_obj->m_local.m_rows[0][2] = p_matrix->m_rows[0][2];
	p_obj->m_local.m_rows[1][0] = p_matrix->m_rows[1][0];
	p_obj->m_local.m_rows[1][1] = p_matrix->m_rows[1][1];
	p_obj->m_local.m_rows[1][2] = p_matrix->m_rows[1][2];
	p_obj->m_local.m_rows[2][0] = p_matrix->m_rows[2][0];
	p_obj->m_local.m_rows[2][1] = p_matrix->m_rows[2][1];
	p_obj->m_local.m_rows[2][2] = p_matrix->m_rows[2][2];
	p_obj->m_flags |= c_objectDirty;
}

// FUNCTION: MW2 0x1000179f
void FUN_1000179f(AmberWillow0x7c* p_obj, Matrix* p_matrix)
{
	FUN_1000da0c(&p_obj->m_local, p_matrix, &p_obj->m_local);
	p_obj->m_flags |= c_objectDirty;

	p_obj->m_unk0x78--;
	if (p_obj->m_unk0x78 <= 0) {
		p_obj->m_unk0x78 = g_unk0x100a00c8;
		g_unk0x100a00c8 = ((g_unk0x100a00c8 + 7) & 0x7f) + 0x40;
		FUN_1007cbf1(&p_obj->m_local);
	}
}

// FUNCTION: MW2 0x1000180b
void SetObjRotation(AmberWillow0x7c* p_obj, MechS32 p_unk0x04, MechS32 p_unk0x08, MechS32 p_unk0x0c, MechU32 p_unk0x10)
{
	Matrix matrix;

	FUN_1000de3b(&matrix, p_unk0x04, p_unk0x08, p_unk0x0c, 0, 0, 0, p_unk0x10);
	FUN_10001722(p_obj, &matrix);
}

// FUNCTION: MW2 0x1000184b
void FUN_1000184b(AmberWillow0x7c* p_obj, MechS32 p_unk0x04, MechS32 p_unk0x08, MechS32 p_unk0x0c, MechU32 p_unk0x10)
{
	Matrix matrix;

	FUN_1000de3b(&matrix, p_unk0x04, p_unk0x08, p_unk0x0c, 0, 0, 0, p_unk0x10);
	FUN_1000179f(p_obj, &matrix);
}

// The only diff is a stack-slot permutation of model and obj.
// FUNCTION: MW2 0x1000188b
void FUN_1000188b(ScarletOrchid0x4c* p_shape)
{
	GraniteLattice0x18* model = p_shape->m_unk0x20;
	AmberWillow0x7c* obj = p_shape->m_unk0x18;

	FUN_10039a30(model, &obj->m_world);
	model->m_unk0x10 = p_shape->m_unk0x48;
}

// FUNCTION: MW2 0x100018ca
void FUN_100018ca(AmberWillow0x7c* p_obj)
{
	AmberWillow0x7c* child;

	if (p_obj->m_unk0x6c) {
		FUN_1006da2d(p_obj->m_unk0x6c);
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		FUN_100018ca(child);
	}
}

// FUNCTION: MW2 0x10001926
void FUN_10001926(AmberWillow0x7c* p_obj)
{
	AmberWillow0x7c* child;

	if (p_obj->m_unk0x6c && (p_obj->m_unk0x6c->m_unk0x02 & 0xf0) != 0x70) {
		FUN_1006daa0(p_obj->m_unk0x6c);
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		FUN_10001926(child);
	}
}

// FUNCTION: MW2 0x1000199a
void FUN_1000199a(AmberWillow0x7c* p_obj)
{
	AmberWillow0x7c* child;

	if (p_obj->m_unk0x6c) {
		FUN_1006d989(p_obj->m_unk0x6c);
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		FUN_1000199a(child);
	}
}

// FUNCTION: MW2 0x100019f6
void FUN_100019f6(AmberWillow0x7c* p_obj)
{
	AmberWillow0x7c* child;

	if (p_obj->m_unk0x6c) {
		FUN_1006d8d1(p_obj->m_unk0x6c);
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		FUN_100019f6(child);
	}
}

// FUNCTION: MW2 0x10001a52
void FUN_10001a52(AmberWillow0x7c* p_obj)
{
	AmberWillow0x7c* child;

	if (p_obj->m_unk0x6c) {
		FUN_1006d88a(p_obj->m_unk0x6c);
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		FUN_10001a52(child);
	}
}

// FUNCTION: MW2 0x10001aae
void SetObjTreeFlag(AmberWillow0x7c* p_obj, MechS32 p_flag)
{
	AmberWillow0x7c* child;

	if (p_obj->m_unk0x6c) {
		p_obj->m_unk0x6c->m_unk0x02 = p_flag;
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		SetObjTreeFlag(child, p_flag);
	}
}

// FUNCTION: MW2 0x10001b0c
void FUN_10001b0c(AmberWillow0x7c* p_obj, MechS32 p_unk0x14)
{
	AmberWillow0x7c* child;

	if (p_obj->m_unk0x6c) {
		p_obj->m_unk0x6c->m_unk0x14 = p_unk0x14;
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		FUN_10001b0c(child, p_unk0x14);
	}
}

// FUNCTION: MW2 0x10001b6a
void FUN_10001b6a(AmberWillow0x7c* p_obj, MechS32 p_unk0x24)
{
	AmberWillow0x7c* child;

	if (p_obj->m_unk0x6c) {
		FUN_10034a40(p_obj->m_unk0x6c, p_unk0x24);
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		FUN_10001b6a(child, p_unk0x24);
	}
}

// FUNCTION: MW2 0x10001bce
void FUN_10001bce(AmberWillow0x7c* p_obj, MechS32 p_flags)
{
	AmberWillow0x7c* child;

	if (p_obj->m_unk0x6c) {
		p_obj->m_unk0x6c->m_unk0x02 &= (MechS16) ~p_flags;
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		FUN_10001bce(child, p_flags);
	}
}

// FUNCTION: MW2 0x10001c3f
void FUN_10001c3f(AmberWillow0x7c* p_obj)
{
	ScarletOrchid0x4c* shape;
	AmberWillow0x7c* child;

	p_obj->m_flags &= ~c_objectDirty;

	if (p_obj->m_parent) {
		FUN_1000dbba(&p_obj->m_parent->m_world, &p_obj->m_local, &p_obj->m_world);
	}
	else {
		p_obj->m_world = p_obj->m_local;
	}

	shape = p_obj->m_unk0x6c;
	if (shape) {
		FUN_10039b94(shape, &p_obj->m_world);
		shape->m_unk0x18 = p_obj;
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		FUN_10001c3f(child);
	}
}

// FUNCTION: MW2 0x10001cf8
void FUN_10001cf8(AmberWillow0x7c* p_obj)
{
	AmberWillow0x7c* child;

	if (p_obj->m_flags & c_objectDirty) {
		FUN_10001c3f(p_obj);
	}
	else if (p_obj->m_firstChild) {
		for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
			FUN_10001cf8(child);
		}
	}
}

// FUNCTION: MW2 0x10001d63
AmberWillow0x7c* FUN_10001d63(AmberWillow0x7c* p_obj)
{
	while (p_obj->m_parent) {
		p_obj = p_obj->m_parent;
	}

	return p_obj;
}

// FUNCTION: MW2 0x10001d8f
AmberWillow0x7c* FUN_10001d8f(AmberWillow0x7c* p_obj)
{
	return p_obj->m_parent;
}

// FUNCTION: MW2 0x10001da4
AmberWillow0x7c* FUN_10001da4(AmberWillow0x7c* p_obj)
{
	return p_obj->m_firstChild;
}

// FUNCTION: MW2 0x10001dba
AmberWillow0x7c* FUN_10001dba(AmberWillow0x7c* p_obj)
{
	return p_obj->m_nextSibling;
}

// FUNCTION: MW2 0x10001dd0
Matrix* FUN_10001dd0(AmberWillow0x7c* p_obj)
{
	return &p_obj->m_local;
}

// FUNCTION: MW2 0x10001de6
void FUN_10001de6(AmberWillow0x7c* p_obj, Matrix* p_matrix)
{
	p_obj->m_local = *p_matrix;
}

// FUNCTION: MW2 0x10001e01
Matrix* FUN_10001e01(AmberWillow0x7c* p_obj)
{
	return &p_obj->m_world;
}

// FUNCTION: MW2 0x10001e17
void FUN_10001e17(AmberWillow0x7c* p_obj, Matrix* p_matrix)
{
	p_obj->m_world = *p_matrix;
}

// The only diff is a stack-slot permutation of parent and first.
// FUNCTION: MW2 0x10001e32
void FUN_10001e32(AmberWillow0x7c* p_obj)
{
	AmberWillow0x7c* parent;
	AmberWillow0x7c* sibling;
	AmberWillow0x7c* first;

	parent = p_obj->m_parent;
	if (parent) {
		if (parent->m_firstChild == p_obj) {
			first = p_obj->m_nextSibling;
			parent->m_firstChild = first;
		}
		else {
			first = parent->m_firstChild;
			sibling = first;
			while (sibling) {
				if (sibling->m_nextSibling == p_obj) {
					sibling->m_nextSibling = p_obj->m_nextSibling;
					break;
				}

				sibling = sibling->m_nextSibling;
			}
		}
	}

	p_obj->m_parent = NULL;
	p_obj->m_nextSibling = NULL;
	p_obj->m_local = p_obj->m_world;
	p_obj->m_flags |= c_objectDirty;
	FUN_10001cf8(p_obj);
}

// FUNCTION: MW2 0x10001ef8
void FUN_10001ef8(AmberWillow0x7c* p_obj, AmberWillow0x7c* p_parent)
{
	Matrix inverse;

	if (p_obj->m_parent) {
		FUN_10001e32(p_obj);
	}

	p_obj->m_parent = p_parent;
	p_obj->m_nextSibling = p_parent->m_firstChild;
	p_parent->m_firstChild = p_obj;

	FUN_1000dcbd(&p_parent->m_world, &inverse);
	FUN_1000dbba(&inverse, &p_obj->m_world, &p_obj->m_local);
	p_obj->m_flags &= ~c_objectDirty;
	p_parent->m_flags |= c_objectDirty;
	FUN_10001cf8(p_parent);
}

// FUNCTION: MW2 0x10001f82
void FUN_10001f82(AmberWillow0x7c* p_obj, ShapeCallback p_callback)
{
	AmberWillow0x7c* child;
	AmberWillow0x7c* next;

	FUN_10001e32(p_obj);

	child = p_obj->m_firstChild;
	while (child) {
		next = child->m_nextSibling;
		FUN_10001f82(child, p_callback);
		child = next;
	}

	if (p_obj->m_unk0x6c && p_callback) {
		p_callback(p_obj->m_unk0x6c);
	}

	if (p_obj->m_flags & c_objectHeap) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_obj);
	}
}

// FUNCTION: MW2 0x10002016
AmberWillow0x7c* FUN_10002016(AmberWillow0x7c* p_obj, const MechChar* p_name)
{
	AmberWillow0x7c* child;

	if (p_obj->m_name && _strcmpi(p_obj->m_name, p_name) == 0) {
		return p_obj;
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		p_obj = FUN_10002016(child, p_name);
		if (p_obj) {
			return p_obj;
		}
	}

	return NULL;
}

// The only diff is a stack-slot permutation of child and last.
// FUNCTION: MW2 0x100020a6
AmberWillow0x7c* FUN_100020a6(AmberWillow0x7c* p_obj)
{
	AmberWillow0x7c* child;
	AmberWillow0x7c* last;

	last = NULL;
	child = p_obj->m_firstChild;
	if (child) {
		last = child;
		while ((child = child->m_nextSibling)) {
			last = child;
		}
	}

	return last;
}

// The only diff is a stack-slot permutation of child and count.
// FUNCTION: MW2 0x100020fa
AmberWillow0x7c* FUN_100020fa(AmberWillow0x7c* p_obj, MechS32 p_index)
{
	AmberWillow0x7c* result;
	AmberWillow0x7c* child;
	MechS32 count;

	result = NULL;
	count = 0;

	if (p_obj == NULL) {
		return NULL;
	}

	child = p_obj->m_firstChild;
	if (child == NULL) {
		return NULL;
	}

	while (child) {
		count++;
		result = child;
		child = child->m_nextSibling;
	}

	count = count - p_index - 1;
	if (count < 0) {
		return NULL;
	}

	result = p_obj->m_firstChild;
	while (count-- && result) {
		result = result->m_nextSibling;
	}

	return result;
}

// FUNCTION: MW2 0x100021b6
AmberWillow0x7c* FUN_100021b6(AmberWillow0x7c* p_obj, MechU32 p_unk0x16)
{
	AmberWillow0x7c* child;
	MechU32 id = p_unk0x16;

	if (p_obj->m_unk0x6c && p_obj->m_unk0x6c->m_unk0x16 == id) {
		return p_obj;
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		p_obj = FUN_100021b6(child, p_unk0x16);
		if (p_obj) {
			return p_obj;
		}
	}

	return NULL;
}

// The remaining diffs are a stack-slot permutation of current and child, and the operand
// order of p_unk0x04 > current (declaration order didn't flip it).
// FUNCTION: MW2 0x10002246
void FUN_10002246(AmberWillow0x7c* p_obj, MechS32 p_unk0x04, MechU32 p_unk0x16)
{
	MechS32 current;
	AmberWillow0x7c* child;
	ScarletOrchid0x4c* shape;

	shape = p_obj->m_unk0x6c;
	current = (FUN_1003acf7(shape) & 0xf0) >> 4;

	if (p_unk0x04 > 0 && p_unk0x04 < 0x10 && p_unk0x04 > current) {
		p_unk0x04 <<= 4;
	}
	else {
		return;
	}

	if (p_obj->m_unk0x6c && p_obj->m_unk0x6c->m_unk0x16 == p_unk0x16) {
		FUN_1003acbe(p_obj->m_unk0x6c, p_unk0x04);
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		FUN_10002246(child, p_unk0x04 >> 4, p_unk0x16);
	}
}

// FUNCTION: MW2 0x10002314
void FUN_10002314(AmberWillow0x7c* p_obj, MechU32 p_unk0x16)
{
	if (p_obj == NULL) {
		return;
	}

	FUN_10002314(p_obj->m_nextSibling, p_unk0x16);

	if (p_obj->m_unk0x6c && p_obj->m_unk0x6c->m_unk0x16 == p_unk0x16) {
		FUN_100044f3(p_obj->m_firstChild, FUN_1001ddf2, p_unk0x16);
		FUN_10004356(p_obj, FUN_1001ddf2, p_unk0x16);
	}
	else {
		FUN_10002314(p_obj->m_firstChild, p_unk0x16);
	}
}

// FUNCTION: MW2 0x100023a8
MechS32 FUN_100023a8(void)
{
	return sizeof(AmberWillow0x7c);
}
