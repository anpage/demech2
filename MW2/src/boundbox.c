#include "boundbox.h"

#include "decomp.h"
#include "emberfern.h"
#include "object.h"
#include "quadtree.h"
#include "shape.h"
#include "simmain.h"
#include "types.h"

#include <windows.h>

DECOMP_SIZE_ASSERT(CinderBox0x18, 0x18)

// FUNCTION: MW2 0x1006e970
CinderBox0x18* FUN_1006e970(void)
{
	CinderBox0x18* box;

	box = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(CinderBox0x18));
	if (box) {
		box->m_minX = box->m_minY = box->m_minZ = 0;
		box->m_maxX = box->m_maxY = box->m_maxZ = 0;
	}

	return box;
}

// Gives the shape a bounding box, once.
// FUNCTION: MW2 0x1006e9e6
void FUN_1006e9e6(ScarletOrchid0x4c* p_shape)
{
	CinderBox0x18* box;

	if (!p_shape) {
		return;
	}

	if (p_shape->m_unk0x00 & 0x200) {
		return;
	}

	if (!p_shape->m_unk0x44) {
		p_shape->m_unk0x44 = FUN_1006e970();
	}

	box = p_shape->m_unk0x44;
	if (!box) {
		return;
	}

	FUN_1006eb80(p_shape, &box->m_minX, &box->m_maxX, &box->m_minY, &box->m_maxY, &box->m_minZ, &box->m_maxZ);
	p_shape->m_unk0x00 |= 0x200;
}

// FUNCTION: MW2 0x1006ea90
CinderBox0x18* FUN_1006ea90(ScarletOrchid0x4c* p_shape)
{
	CinderBox0x18* box;

	box = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, 0x78);
	if (box) {
		memset(box, 0, 0x78);
		FUN_1006eb80(p_shape, &box->m_minX, &box->m_maxX, &box->m_minY, &box->m_maxY, &box->m_minZ, &box->m_maxZ);
	}

	return box;
}

// FUNCTION: MW2 0x1006eb02
void FUN_1006eb02(ScarletOrchid0x4c* p_shape)
{
	CinderBox0x18* box;

	if (!p_shape) {
		return;
	}

	if (p_shape->m_unk0x00 & 0x200) {
		return;
	}

	if (!p_shape->m_unk0x44) {
		p_shape->m_unk0x44 = FUN_1006ea90(p_shape);
	}

	box = p_shape->m_unk0x44;
	if (!box) {
		return;
	}

	p_shape->m_unk0x00 |= 0x200;
}

// Computes the bounding box of the shape's first model, transforming it first if it is out of date.
// The model/shape stamp comparison loads its operands in the other order (the unit's symbol
// table), and model, vertex and selected sit in permuted stack slots.
// FUNCTION: MW2 0x1006eb80
void FUN_1006eb80(
	ScarletOrchid0x4c* p_shape,
	MechS32* p_minX,
	MechS32* p_maxX,
	MechS32* p_minY,
	MechS32* p_maxY,
	MechS32* p_minZ,
	MechS32* p_maxZ
)
{
	GraniteLattice0x18* model;
	EmberFern0x2c* vertex;
	MechS32 count;
	GraniteLattice0x18* selected;

	model = p_shape->m_unk0x1c;
	if (!model) {
		*p_minX = *p_maxX = *p_minY = *p_maxY = *p_minZ = *p_maxZ = 0;
		return;
	}

	if (model->m_unk0x10 != p_shape->m_unk0x48) {
		selected = p_shape->m_unk0x20;
		p_shape->m_unk0x20 = model;
		FUN_1000188b(p_shape);
		p_shape->m_unk0x20 = selected;
	}

	vertex = (EmberFern0x2c*) (model + 1);
	*p_minX = *p_minY = *p_minZ = 0x7fffffff;
	*p_maxX = *p_maxY = *p_maxZ = -0x7fffffff;
	for (count = model->m_unk0x04; count--; vertex++) {
		if (vertex->m_unk0x0c > *p_maxX) {
			*p_maxX = vertex->m_unk0x0c;
		}

		if (vertex->m_unk0x10 > *p_maxY) {
			*p_maxY = vertex->m_unk0x10;
		}

		if (vertex->m_unk0x14 > *p_maxZ) {
			*p_maxZ = vertex->m_unk0x14;
		}

		if (vertex->m_unk0x0c < *p_minX) {
			*p_minX = vertex->m_unk0x0c;
		}

		if (vertex->m_unk0x10 < *p_minY) {
			*p_minY = vertex->m_unk0x10;
		}

		if (vertex->m_unk0x14 < *p_minZ) {
			*p_minZ = vertex->m_unk0x14;
		}
	}
}

// Frees the shape's bounding data.
// FUNCTION: MW2 0x1006ed30
void FUN_1006ed30(ScarletOrchid0x4c* p_shape)
{
	void* data;

	if (!p_shape) {
		return;
	}

	data = p_shape->m_unk0x44;
	if (!data) {
		return;
	}

	switch (p_shape->m_unk0x24) {
	case 0:
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, data);
		break;
	case 5:
		FUN_1001e50d(data);
		break;
	}

	p_shape->m_unk0x44 = NULL;
}

// Returns the bytes the shape's bounding data take.
// FUNCTION: MW2 0x1006edc3
MechS32 FUN_1006edc3(ScarletOrchid0x4c* p_shape)
{
	MechS32 size;
	void* data;

	if (!p_shape) {
		return 0;
	}

	data = p_shape->m_unk0x44;
	if (!data) {
		return 0;
	}

	switch (p_shape->m_unk0x24) {
	case 0:
		size = 0x78;
		break;
	case 5:
		size = FUN_1001ee0f(data);
		break;
	default:
		size = 0;
	}

	return size;
}
