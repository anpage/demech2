#include "unk1003a530.h"

#include "decomp.h"
#include "duskmoth.h"
#include "emberfern.h"
#include "object.h"
#include "simmain.h"
#include "types.h"
#include "unk10036230.h"
#include "unk1006d680.h"
#include "unk1006e970.h"

#include <windows.h>

// The flags a new shape starts with (FUN_1003a8c1).
// GLOBAL: MW2 0x100a5898
MechU32 g_unk0x100a5898 = 0;

// FUNCTION: MW2 0x1003a530
MechU32 FUN_1003a530(void)
{
	return g_unk0x100a5898;
}

// Sets the flags new shapes start with; returns the previous ones.
// FUNCTION: MW2 0x1003a545
MechU32 FUN_1003a545(MechU32 p_flags)
{
	MechU32 previous;

	previous = g_unk0x100a5898;
	g_unk0x100a5898 = p_flags;
	return previous;
}

// Selects the shape's first model.
// FUNCTION: MW2 0x1003a56b
void FUN_1003a56b(ScarletOrchid0x4c* p_shape)
{
	FUN_1003a589(p_shape, p_shape->m_unk0x1c);
}

// FUNCTION: MW2 0x1003a589
void FUN_1003a589(ScarletOrchid0x4c* p_shape, GraniteLattice0x18* p_model)
{
	p_shape->m_unk0x20 = p_model;
}

// Selects the next model, wrapping around to the first.
// FUNCTION: MW2 0x1003a59d
void FUN_1003a59d(ScarletOrchid0x4c* p_shape)
{
	if (!p_shape->m_unk0x20 || !p_shape->m_unk0x20->m_unk0x0c) {
		FUN_1003a589(p_shape, p_shape->m_unk0x1c);
	}
	else {
		FUN_1003a589(p_shape, p_shape->m_unk0x20->m_unk0x0c);
	}
}

// Allocates a model with room for the vertices, the faces and p_extra more bytes (returned in
// p_extraData), inserts it into the shape's list by p_key and selects it.
// The size sum adds p_extra and vertexBytes in the other order, and the locals sit in
// permuted stack slots.
// FUNCTION: MW2 0x1003a5f3
GraniteLattice0x18* FUN_1003a5f3(
	ScarletOrchid0x4c* p_shape,
	MechS32 p_key,
	MechS32 p_vertexCount,
	MechS32 p_faceCount,
	MechS32 p_extra,
	void** p_extraData
)
{
	GraniteLattice0x18* model;
	MechS32 vertexBytes;
	GraniteLattice0x18* cursor;
	void* memory;
	MechU32 size;
	MechS32 faceBytes;

	size = 0;
	faceBytes = 0;
	vertexBytes = 0;
	if (!p_shape) {
		return NULL;
	}

	vertexBytes = p_vertexCount * sizeof(EmberFern0x2c);
	faceBytes = p_faceCount * sizeof(DuskMoth0x24);
	size = p_extra + vertexBytes + faceBytes + 0x18;
	memory = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, size);
	if (!memory) {
		return NULL;
	}

	memset(memory, 0, size);
	model = memory;
	model->m_unk0x00 = p_key;
	model->m_unk0x04 = 0;
	model->m_unk0x06 = 0;
	model->m_unk0x10 = 0;
	model->m_unk0x14 = 0;
	model->m_unk0x08 = (MechU16) (vertexBytes + sizeof(GraniteLattice0x18));
	*p_extraData = (DuskMoth0x24*) ((MechU8*) model + model->m_unk0x08) + p_faceCount;
	FUN_1003a589(p_shape, model);

	if (!p_shape->m_unk0x1c) {
		p_shape->m_unk0x1c = model;
		model->m_unk0x0c = NULL;
		return model;
	}

	cursor = p_shape->m_unk0x1c;
	if (cursor->m_unk0x00 > p_key) {
		model->m_unk0x0c = cursor;
		p_shape->m_unk0x1c = model;
		return model;
	}

	while (cursor->m_unk0x0c) {
		if (cursor->m_unk0x0c->m_unk0x00 > p_key) {
			break;
		}

		cursor = cursor->m_unk0x0c;
	}

	model->m_unk0x0c = cursor->m_unk0x0c;
	cursor->m_unk0x0c = model;
	return model;
}

// Selects the first model whose key is at most p_key.
// FUNCTION: MW2 0x1003a7a2
void FUN_1003a7a2(ScarletOrchid0x4c* p_shape, MechS32 p_key)
{
	GraniteLattice0x18* model;

	for (model = p_shape->m_unk0x1c; model; model = model->m_unk0x0c) {
		if (model->m_unk0x00 <= p_key) {
			FUN_1003a589(p_shape, model);
			break;
		}
	}
}

// FUNCTION: MW2 0x1003a7f9
void FUN_1003a7f9(ScarletOrchid0x4c* p_shape, MechS32 p_key)
{
	GraniteLattice0x18* model;

	model = p_shape->m_unk0x20;
	if (!model) {
		return;
	}

	model->m_unk0x00 = p_key;
}

// FUNCTION: MW2 0x1003a827
MechS32 FUN_1003a827(ScarletOrchid0x4c* p_shape)
{
	GraniteLattice0x18* model;

	model = p_shape->m_unk0x20;
	if (!model) {
		return 0;
	}

	return model->m_unk0x00;
}

// FUNCTION: MW2 0x1003a859
void FUN_1003a859(ScarletOrchid0x4c* p_shape, MechS32 p_unk0x14)
{
	GraniteLattice0x18* model;

	model = p_shape->m_unk0x20;
	if (!model) {
		return;
	}

	model->m_unk0x14 = p_unk0x14;
}

// FUNCTION: MW2 0x1003a889
MechU32 FUN_1003a889(ScarletOrchid0x4c* p_shape)
{
	GraniteLattice0x18* model;

	model = p_shape->m_unk0x20;
	if (!model) {
		return 0;
	}

	return model->m_unk0x14;
}

// Allocates a shape with one model (FUN_1003a5f3).
// FUNCTION: MW2 0x1003a8c1
ScarletOrchid0x4c* FUN_1003a8c1(MechS32 p_vertexCount, MechS32 p_faceCount, MechS32 p_extra, void** p_extraData)
{
	ScarletOrchid0x4c* shape;

	shape = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(ScarletOrchid0x4c));
	if (!shape) {
		return NULL;
	}

	shape->m_unk0x00 = g_unk0x100a5898 | 0x8000;
	shape->m_unk0x18 = NULL;
	shape->m_unk0x20 = shape->m_unk0x1c = NULL;
	if (!FUN_1003a5f3(shape, 0, p_vertexCount, p_faceCount, p_extra, p_extraData)) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, shape);
		return NULL;
	}

	shape->m_unk0x02 = 0;
	shape->m_unk0x16 = 0;
	shape->m_unk0x14 = 0;
	shape->m_unk0x34 = shape->m_unk0x38 = shape->m_unk0x3c = 0;
	shape->m_unk0x28 = shape->m_unk0x2c = shape->m_unk0x30 = 0;
	shape->m_unk0x40 = 0;
	shape->m_unk0x44 = NULL;
	shape->m_unk0x04 = shape->m_unk0x08 = NULL;
	shape->m_unk0x0c = shape->m_unk0x10 = NULL;
	shape->m_unk0x24 = 4;
	shape->m_unk0x48 = 0;
	return shape;
}

// Appends a vertex to the selected model.
// FUNCTION: MW2 0x1003aa1d
void FUN_1003aa1d(
	ScarletOrchid0x4c* p_shape,
	MechS32 p_x,
	MechS32 p_y,
	MechS32 p_z,
	undefined4 p_unk0x18,
	undefined4 p_unk0x1c
)
{
	GraniteLattice0x18* model;
	EmberFern0x2c* vertex;

	model = p_shape->m_unk0x20;
	if (!model) {
		return;
	}

	vertex = (EmberFern0x2c*) (model + 1) + model->m_unk0x04;
	vertex->m_unk0x0c = vertex->m_unk0x00 = p_x;
	vertex->m_unk0x10 = vertex->m_unk0x04 = p_y;
	vertex->m_unk0x14 = vertex->m_unk0x08 = p_z;
	vertex->m_unk0x18 = p_unk0x18;
	vertex->m_unk0x1c = p_unk0x1c;
	model->m_unk0x04++;
}

// Appends a face to the selected model, its vertex indices at p_indices.
// The only diff is a stack-slot permutation of model and face.
// FUNCTION: MW2 0x1003aab5
DuskMoth0x24* FUN_1003aab5(ScarletOrchid0x4c* p_shape, MechU16 p_unk0x00, MechU8* p_indices)
{
	GraniteLattice0x18* model;
	DuskMoth0x24* face;

	model = p_shape->m_unk0x20;
	if (!model) {
		return NULL;
	}

	face = (DuskMoth0x24*) ((MechU8*) model + model->m_unk0x08) + model->m_unk0x06;
	face->m_unk0x04 = (MechU16) (p_indices - (MechU8*) face);
	face->m_unk0x20 = p_shape;
	face->m_unk0x00 = p_unk0x00;
	face->m_unk0x02 = 0;
	model->m_unk0x06++;
	return face;
}

// Appends a vertex index to a face of the selected model.
// FUNCTION: MW2 0x1003ab34
void FUN_1003ab34(ScarletOrchid0x4c* p_shape, DuskMoth0x24* p_face, MechU8 p_index)
{
	GraniteLattice0x18* model;

	model = p_shape->m_unk0x20;
	if (!model) {
		return;
	}

	((MechU8*) p_face)[p_face->m_unk0x04 + p_face->m_unk0x02] = p_index;
	p_face->m_unk0x02++;
}

// FUNCTION: MW2 0x1003ab79
void FUN_1003ab79(GraniteLattice0x18* p_model)
{
	if (!p_model) {
		return;
	}

	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_model);
}

// Removes the selected model from the shape's list and frees it.
// The only diff is a stack-slot permutation of model and cursor.
// FUNCTION: MW2 0x1003aba5
void FUN_1003aba5(ScarletOrchid0x4c* p_shape)
{
	GraniteLattice0x18* model;
	GraniteLattice0x18* cursor;

	model = p_shape->m_unk0x20;
	if (!model || !p_shape->m_unk0x1c) {
		return;
	}

	if (p_shape->m_unk0x1c == model) {
		p_shape->m_unk0x1c = model->m_unk0x0c;
	}
	else {
		for (cursor = p_shape->m_unk0x1c; cursor->m_unk0x0c; cursor = cursor->m_unk0x0c) {
			if (cursor->m_unk0x0c == model) {
				break;
			}
		}

		if (!cursor->m_unk0x0c) {
			return;
		}

		cursor->m_unk0x0c = model->m_unk0x0c;
	}

	FUN_1003ab79(model);
}

// Frees the shape and its models.
// FUNCTION: MW2 0x1003ac5f
void FUN_1003ac5f(ScarletOrchid0x4c* p_shape)
{
	GraniteLattice0x18* model;
	GraniteLattice0x18* current;

	model = p_shape->m_unk0x1c;
	while (model) {
		current = model;
		model = current->m_unk0x0c;
		FUN_1003ab79(current);
	}

	FUN_1006ed30(p_shape);
	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_shape);
}

// FUNCTION: MW2 0x1003acbe
void FUN_1003acbe(ScarletOrchid0x4c* p_shape, MechS32 p_unk0x04)
{
	if (p_shape) {
		p_shape->m_unk0x00 = (p_shape->m_unk0x00 & ~0x7ef0) | (p_unk0x04 & 0x7ef0) | 0x8000;
	}
}

// FUNCTION: MW2 0x1003acf7
MechU32 FUN_1003acf7(ScarletOrchid0x4c* p_shape)
{
	if (p_shape) {
		return p_shape->m_unk0x00 & 0x7ef0;
	}
	else {
		return 0;
	}
}

// FUNCTION: MW2 0x1003ad2d
void FUN_1003ad2d(ScarletOrchid0x4c* p_shape, MechS32 p_unk0x02)
{
	if (p_shape) {
		p_shape->m_unk0x02 = p_unk0x02;
	}
}

// FUNCTION: MW2 0x1003ad4c
void FUN_1003ad4c(ScarletOrchid0x4c* p_shape, MechU16 p_unk0x16)
{
	p_shape->m_unk0x16 = p_unk0x16;
}

// FUNCTION: MW2 0x1003ad62
void FUN_1003ad62(ScarletOrchid0x4c* p_shape, MechU16 p_unk0x14)
{
	p_shape->m_unk0x14 = p_unk0x14;
}

// FUNCTION: MW2 0x1003ad78
MechU32 FUN_1003ad78(ScarletOrchid0x4c* p_shape)
{
	return p_shape->m_unk0x02;
}

// FUNCTION: MW2 0x1003ad93
MechU32 FUN_1003ad93(ScarletOrchid0x4c* p_shape)
{
	return p_shape->m_unk0x16;
}

// FUNCTION: MW2 0x1003adae
MechU32 FUN_1003adae(ScarletOrchid0x4c* p_shape)
{
	return p_shape->m_unk0x14;
}

// FUNCTION: MW2 0x1003adc9
MechS32 FUN_1003adc9(ScarletOrchid0x4c* p_shape, MechS32* p_unk0x34, MechS32* p_unk0x38, MechS32* p_unk0x3c)
{
	if (p_unk0x34) {
		*p_unk0x34 = p_shape->m_unk0x34;
	}

	if (p_unk0x38) {
		*p_unk0x38 = p_shape->m_unk0x38;
	}

	if (p_unk0x3c) {
		*p_unk0x3c = p_shape->m_unk0x3c;
	}

	return p_shape->m_unk0x40;
}

// FUNCTION: MW2 0x1003ae1e
void FUN_1003ae1e(ScarletOrchid0x4c* p_shape)
{
	GraniteLattice0x18* model;
	MechS32 i;

	model = p_shape->m_unk0x20;
	if (!model) {
		return;
	}

	i = model->m_unk0x06;
	while (i--) {
		FUN_1003b0e4((DuskMoth0x24*) ((MechU8*) model + model->m_unk0x08) + i, (EmberFern0x2c*) (model + 1));
	}

	FUN_1003ae96(p_shape);
}

// STUB: MW2 0x1003ae96
void FUN_1003ae96(ScarletOrchid0x4c* p_shape)
{
	STUB(0x1003ae96);
}

// STUB: MW2 0x1003b0e4
void FUN_1003b0e4(DuskMoth0x24* p_face, EmberFern0x2c* p_vertices)
{
	STUB(0x1003b0e4);
}

// Returns the selected model's vertex and face counts.
// FUNCTION: MW2 0x1003b43d
void FUN_1003b43d(ScarletOrchid0x4c* p_shape, MechS32* p_vertexCount, MechS32* p_faceCount)
{
	GraniteLattice0x18* model;

	model = p_shape->m_unk0x20;
	if (!model) {
		return;
	}

	if (p_vertexCount) {
		*p_vertexCount = model->m_unk0x04;
	}

	if (p_faceCount) {
		*p_faceCount = model->m_unk0x06;
	}
}

// Calls p_fn for each shape of the list after p_shape.
// FUNCTION: MW2 0x1003b48f
void FUN_1003b48f(ScarletOrchid0x4c* p_shape, void (*p_fn)(ScarletOrchid0x4c*))
{
	ScarletOrchid0x4c* shape;
	ScarletOrchid0x4c* next;

	if (!p_shape) {
		return;
	}

	shape = p_shape->m_unk0x08;
	while (shape) {
		next = shape->m_unk0x08;
		p_fn(shape);
		shape = next;
	}
}

// Returns a vertex's transformed position.
// FUNCTION: MW2 0x1003b4dd
void FUN_1003b4dd(ScarletOrchid0x4c* p_shape, MechS32 p_index, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	GraniteLattice0x18* model;
	EmberFern0x2c* vertex;

	model = p_shape->m_unk0x20;
	if (!model) {
		return;
	}

	vertex = &((EmberFern0x2c*) (model + 1))[p_index];
	if (p_x) {
		*p_x = vertex->m_unk0x0c;
	}

	if (p_y) {
		*p_y = vertex->m_unk0x10;
	}

	if (p_z) {
		*p_z = vertex->m_unk0x14;
	}
}

// Returns a vertex's position in the model.
// The vertex address adds the model and the index in the other order (FUN_1003b4dd's
// identical expression doesn't).
// FUNCTION: MW2 0x1003b55a
void FUN_1003b55a(ScarletOrchid0x4c* p_shape, MechS32 p_index, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	GraniteLattice0x18* model;
	EmberFern0x2c* vertex;

	model = p_shape->m_unk0x20;
	if (!model) {
		return;
	}

	vertex = &((EmberFern0x2c*) (model + 1))[p_index];
	if (p_x) {
		*p_x = vertex->m_unk0x00;
	}

	if (p_y) {
		*p_y = vertex->m_unk0x04;
	}

	if (p_z) {
		*p_z = vertex->m_unk0x08;
	}
}

// Returns a face of the selected model and up to p_max of its vertex indices.
// FUNCTION: MW2 0x1003b5d6
void FUN_1003b5d6(
	ScarletOrchid0x4c* p_shape,
	MechS32 p_index,
	MechU32* p_unk0x00,
	MechU32* p_count,
	MechU32* p_indices,
	MechS32 p_max
)
{
	GraniteLattice0x18* model;
	DuskMoth0x24* face;
	MechS32 count;

	model = p_shape->m_unk0x20;
	if (!model) {
		return;
	}

	face = (DuskMoth0x24*) ((MechU8*) model + model->m_unk0x08) + p_index;
	if (p_unk0x00) {
		*p_unk0x00 = face->m_unk0x00;
	}

	count = face->m_unk0x02;
	if (p_count) {
		*p_count = count;
	}

	if (p_indices) {
		if (count > p_max) {
			count = p_max;
		}

		while (count--) {
			p_indices[count] = ((MechU8*) face)[face->m_unk0x04 + count];
		}
	}
}

// FUNCTION: MW2 0x1003b696
void FUN_1003b696(ScarletOrchid0x4c* p_shape, MechS32 p_index, MechS32 p_unk0x00)
{
	GraniteLattice0x18* model;

	model = p_shape->m_unk0x20;
	if (!model) {
		return;
	}

	if (p_index < model->m_unk0x06) {
		((DuskMoth0x24*) ((MechU8*) model + model->m_unk0x08) + p_index)->m_unk0x00 = p_unk0x00;
	}
}

// FUNCTION: MW2 0x1003b6e5
struct AmberWillow0x7c* FUN_1003b6e5(ScarletOrchid0x4c* p_shape)
{
	return p_shape->m_unk0x18;
}

// FUNCTION: MW2 0x1003b6fb
void FUN_1003b6fb(ScarletOrchid0x4c* p_shape, struct AmberWillow0x7c* p_unk0x18)
{
	p_shape->m_unk0x18 = p_unk0x18;
}

// FUNCTION: MW2 0x1003b70f
MechU32 FUN_1003b70f(ScarletOrchid0x4c* p_shape)
{
	return p_shape->m_unk0x00 & 0x10f;
}

// FUNCTION: MW2 0x1003b72f
void FUN_1003b72f(ScarletOrchid0x4c* p_shape, MechU32 p_flags)
{
	p_shape->m_unk0x00 = (p_shape->m_unk0x00 & ~0x10f) | (p_flags & 0x10f) | 0x8000;
}

// Unlinks the shape and frees it.
// FUNCTION: MW2 0x1003b75e
void FUN_1003b75e(ScarletOrchid0x4c* p_shape)
{
	if (p_shape) {
		FUN_1006d7fb(p_shape);
		FUN_1003ac5f(p_shape);
	}
}

// Detaches the shape from its scene object and frees it. A ShapeCallback (FUN_10001f82).
// FUNCTION: MW2 0x1003b78b
void FUN_1003b78b(ScarletOrchid0x4c* p_shape)
{
	struct AmberWillow0x7c* obj;

	obj = FUN_1003b6e5(p_shape);
	if (obj) {
		FUN_10001532(obj, NULL);
	}

	FUN_1003b75e(p_shape);
}

// Returns the bytes the shape, its models and its bounding data take.
// The only diff is a stack-slot permutation of model, vertex and size.
// FUNCTION: MW2 0x1003b7cc
MechS32 FUN_1003b7cc(ScarletOrchid0x4c* p_shape)
{
	GraniteLattice0x18* model;
	EmberFern0x2c* vertex;
	MechS32 size;
	MechS32 i;
	DuskMoth0x24* face;

	size = sizeof(ScarletOrchid0x4c);
	if (p_shape->m_unk0x18) {
		size += sizeof(struct AmberWillow0x7c);
	}

	for (model = p_shape->m_unk0x1c; model; model = model->m_unk0x0c) {
		size += sizeof(GraniteLattice0x18);
		vertex = (EmberFern0x2c*) (model + 1);
		for (i = model->m_unk0x04; i--; vertex++) {
			size += sizeof(EmberFern0x2c);
		}

		face = (DuskMoth0x24*) ((MechU8*) model + model->m_unk0x08);
		for (i = model->m_unk0x06; i--; face++) {
			size += face->m_unk0x02 + sizeof(DuskMoth0x24);
		}
	}

	return size + FUN_1006edc3(p_shape);
}
