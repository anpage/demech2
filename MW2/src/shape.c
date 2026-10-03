#include "shape.h"

#include "approxlen.h"
#include "boundbox.h"
#include "decomp.h"
#include "face.h"
#include "object.h"
#include "shapegeom.h"
#include "shapelists.h"
#include "simmain.h"
#include "types.h"
#include "vertex.h"

#include <math.h>
#include <windows.h>

// The flags a new shape starts with (CreateShape).
// GLOBAL: MW2 0x100a5898
MechU32 g_unk0x100a5898 = 0;

// FUNCTION: MW2 0x1003a530
MechU32 GetNewShapeFlags(void)
{
	return g_unk0x100a5898;
}

// Sets the flags new shapes start with; returns the previous ones.
// FUNCTION: MW2 0x1003a545
MechU32 SetNewShapeFlags(MechU32 p_flags)
{
	MechU32 previous;

	previous = g_unk0x100a5898;
	g_unk0x100a5898 = p_flags;
	return previous;
}

// Selects the shape's first model.
// FUNCTION: MW2 0x1003a56b
void SelectFirstModel(Shape* p_shape)
{
	SelectModel(p_shape, p_shape->m_models);
}

// FUNCTION: MW2 0x1003a589
void SelectModel(Shape* p_shape, Model* p_model)
{
	p_shape->m_model = p_model;
}

// Selects the next model, wrapping around to the first.
// FUNCTION: MW2 0x1003a59d
void SelectNextModel(Shape* p_shape)
{
	if (!p_shape->m_model || !p_shape->m_model->m_next) {
		SelectModel(p_shape, p_shape->m_models);
	}
	else {
		SelectModel(p_shape, p_shape->m_model->m_next);
	}
}

// Allocates a model with room for the vertices, the faces and p_extra more bytes (returned in
// p_extraData), inserts it into the shape's list by p_key and selects it.
// The size sum adds p_extra and vertexBytes in the other order, and the locals sit in
// permuted stack slots.
// FUNCTION: MW2 0x1003a5f3
Model* AddModel(
	Shape* p_shape,
	MechS32 p_key,
	MechS32 p_vertexCount,
	MechS32 p_faceCount,
	MechS32 p_extra,
	void** p_extraData
)
{
	Model* model;
	MechS32 vertexBytes;
	Model* cursor;
	void* memory;
	MechU32 size;
	MechS32 faceBytes;

	size = 0;
	faceBytes = 0;
	vertexBytes = 0;
	if (!p_shape) {
		return NULL;
	}

	vertexBytes = p_vertexCount * sizeof(Vertex);
	faceBytes = p_faceCount * sizeof(Face);
	size = p_extra + vertexBytes + faceBytes + 0x18;
	memory = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, size);
	if (!memory) {
		return NULL;
	}

	memset(memory, 0, size);
	model = memory;
	model->m_key = p_key;
	model->m_vertexCount = 0;
	model->m_faceCount = 0;
	model->m_unk0x10 = 0;
	model->m_unk0x14 = 0;
	model->m_faceOffset = (MechU16) (vertexBytes + sizeof(Model));
	*p_extraData = (Face*) ((MechU8*) model + model->m_faceOffset) + p_faceCount;
	SelectModel(p_shape, model);

	if (!p_shape->m_models) {
		p_shape->m_models = model;
		model->m_next = NULL;
		return model;
	}

	cursor = p_shape->m_models;
	if (cursor->m_key > p_key) {
		model->m_next = cursor;
		p_shape->m_models = model;
		return model;
	}

	while (cursor->m_next) {
		if (cursor->m_next->m_key > p_key) {
			break;
		}

		cursor = cursor->m_next;
	}

	model->m_next = cursor->m_next;
	cursor->m_next = model;
	return model;
}

// Selects the first model whose key is at most p_key.
// FUNCTION: MW2 0x1003a7a2
void SelectModelByKey(Shape* p_shape, MechS32 p_key)
{
	Model* model;

	for (model = p_shape->m_models; model; model = model->m_next) {
		if (model->m_key <= p_key) {
			SelectModel(p_shape, model);
			break;
		}
	}
}

// FUNCTION: MW2 0x1003a7f9
void FUN_1003a7f9(Shape* p_shape, MechS32 p_key)
{
	Model* model;

	model = p_shape->m_model;
	if (!model) {
		return;
	}

	model->m_key = p_key;
}

// FUNCTION: MW2 0x1003a827
MechS32 FUN_1003a827(Shape* p_shape)
{
	Model* model;

	model = p_shape->m_model;
	if (!model) {
		return 0;
	}

	return model->m_key;
}

// FUNCTION: MW2 0x1003a859
void FUN_1003a859(Shape* p_shape, MechS32 p_unk0x14)
{
	Model* model;

	model = p_shape->m_model;
	if (!model) {
		return;
	}

	model->m_unk0x14 = p_unk0x14;
}

// FUNCTION: MW2 0x1003a889
MechU32 FUN_1003a889(Shape* p_shape)
{
	Model* model;

	model = p_shape->m_model;
	if (!model) {
		return 0;
	}

	return model->m_unk0x14;
}

// Allocates a shape with one model (AddModel).
// FUNCTION: MW2 0x1003a8c1
Shape* CreateShape(MechS32 p_vertexCount, MechS32 p_faceCount, MechS32 p_extra, void** p_extraData)
{
	Shape* shape;

	shape = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(Shape));
	if (!shape) {
		return NULL;
	}

	shape->m_unk0x00 = g_unk0x100a5898 | 0x8000;
	shape->m_object = NULL;
	shape->m_model = shape->m_models = NULL;
	if (!AddModel(shape, 0, p_vertexCount, p_faceCount, p_extra, p_extraData)) {
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
void AddShapeVertex(Shape* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z, undefined4 p_unk0x18, undefined4 p_unk0x1c)
{
	Model* model;
	Vertex* vertex;

	model = p_shape->m_model;
	if (!model) {
		return;
	}

	vertex = (Vertex*) (model + 1) + model->m_vertexCount;
	vertex->m_unk0x0c = vertex->m_unk0x00 = p_x;
	vertex->m_unk0x10 = vertex->m_unk0x04 = p_y;
	vertex->m_unk0x14 = vertex->m_unk0x08 = p_z;
	vertex->m_unk0x18 = p_unk0x18;
	vertex->m_unk0x1c = p_unk0x1c;
	model->m_vertexCount++;
}

// Appends a face to the selected model, its vertex indices at p_indices.
// The only diff is a stack-slot permutation of model and face.
// FUNCTION: MW2 0x1003aab5
Face* AddShapeFace(Shape* p_shape, MechU16 p_unk0x00, MechU8* p_indices)
{
	Model* model;
	Face* face;

	model = p_shape->m_model;
	if (!model) {
		return NULL;
	}

	face = (Face*) ((MechU8*) model + model->m_faceOffset) + model->m_faceCount;
	face->m_unk0x04 = (MechU16) (p_indices - (MechU8*) face);
	face->m_unk0x20 = p_shape;
	face->m_unk0x00 = p_unk0x00;
	face->m_unk0x02 = 0;
	model->m_faceCount++;
	return face;
}

// Appends a vertex index to a face of the selected model.
// FUNCTION: MW2 0x1003ab34
void AddShapeFaceIndex(Shape* p_shape, Face* p_face, MechU32 p_index)
{
	Model* model;

	model = p_shape->m_model;
	if (!model) {
		return;
	}

	((MechU8*) p_face)[p_face->m_unk0x04 + p_face->m_unk0x02] = (MechU8) p_index;
	p_face->m_unk0x02++;
}

// FUNCTION: MW2 0x1003ab79
void FreeModel(Model* p_model)
{
	if (!p_model) {
		return;
	}

	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_model);
}

// Removes the selected model from the shape's list and frees it.
// The only diff is a stack-slot permutation of model and cursor.
// FUNCTION: MW2 0x1003aba5
void RemoveSelectedModel(Shape* p_shape)
{
	Model* model;
	Model* cursor;

	model = p_shape->m_model;
	if (!model || !p_shape->m_models) {
		return;
	}

	if (p_shape->m_models == model) {
		p_shape->m_models = model->m_next;
	}
	else {
		for (cursor = p_shape->m_models; cursor->m_next; cursor = cursor->m_next) {
			if (cursor->m_next == model) {
				break;
			}
		}

		if (!cursor->m_next) {
			return;
		}

		cursor->m_next = model->m_next;
	}

	FreeModel(model);
}

// Frees the shape and its models.
// FUNCTION: MW2 0x1003ac5f
void FreeShape(Shape* p_shape)
{
	Model* model;
	Model* current;

	model = p_shape->m_models;
	while (model) {
		current = model;
		model = current->m_next;
		FreeModel(current);
	}

	FUN_1006ed30(p_shape);
	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_shape);
}

// FUNCTION: MW2 0x1003acbe
void FUN_1003acbe(Shape* p_shape, MechS32 p_unk0x04)
{
	if (p_shape) {
		p_shape->m_unk0x00 = (p_shape->m_unk0x00 & ~0x7ef0) | (p_unk0x04 & 0x7ef0) | 0x8000;
	}
}

// FUNCTION: MW2 0x1003acf7
MechU32 FUN_1003acf7(Shape* p_shape)
{
	if (p_shape) {
		return p_shape->m_unk0x00 & 0x7ef0;
	}
	else {
		return 0;
	}
}

// FUNCTION: MW2 0x1003ad2d
void FUN_1003ad2d(Shape* p_shape, MechS32 p_unk0x02)
{
	if (p_shape) {
		p_shape->m_unk0x02 = p_unk0x02;
	}
}

// FUNCTION: MW2 0x1003ad4c
void FUN_1003ad4c(Shape* p_shape, MechU16 p_unk0x16)
{
	p_shape->m_unk0x16 = p_unk0x16;
}

// FUNCTION: MW2 0x1003ad62
void FUN_1003ad62(Shape* p_shape, MechU16 p_unk0x14)
{
	p_shape->m_unk0x14 = p_unk0x14;
}

// FUNCTION: MW2 0x1003ad78
MechU32 FUN_1003ad78(Shape* p_shape)
{
	return p_shape->m_unk0x02;
}

// FUNCTION: MW2 0x1003ad93
MechU32 FUN_1003ad93(Shape* p_shape)
{
	return p_shape->m_unk0x16;
}

// FUNCTION: MW2 0x1003adae
MechU32 FUN_1003adae(Shape* p_shape)
{
	return p_shape->m_unk0x14;
}

// FUNCTION: MW2 0x1003adc9
MechS32 FUN_1003adc9(Shape* p_shape, MechS32* p_unk0x34, MechS32* p_unk0x38, MechS32* p_unk0x3c)
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

// Computes the selected model's face normals, then the shape's center and radius (a shape
// LoadShapeRecord has just built).
// FUNCTION: MW2 0x1003ae1e
void ComputeNormalsAndBounds(Shape* p_shape)
{
	Model* model;
	MechS32 i;

	model = p_shape->m_model;
	if (!model) {
		return;
	}

	i = model->m_faceCount;
	while (i--) {
		ComputeFaceNormal((Face*) ((MechU8*) model + model->m_faceOffset) + i, (Vertex*) (model + 1));
	}

	ComputeShapeBounds(p_shape);
}

// Sets p_shape's center to the middle of its first model's bounding box, and its radius to the
// distance from there to the farthest vertex.
// Stack-slot permutation of the locals. The vertex address loads the index first where the
// original loads the model first (index order), and 4.1 sums the squares in another order, which
// also compares d before storing it; a probe of the same source picks yet another order.
// FUNCTION: MW2 0x1003ae96
void ComputeShapeBounds(Shape* p_shape)
{
	MechDouble dz;
	Model* model;
	MechDouble max;
	MechS32 minX;
	MechS32 i;
	MechS32 minY;
	MechS32 minZ;
	MechDouble d;
	MechS32 maxX;
	MechS32 maxY;
	MechDouble dx;
	MechS32 maxZ;
	Vertex* v;
	MechDouble dy;

	model = p_shape->m_models;
	if (!model) {
		return;
	}

	i = model->m_vertexCount - 1;
	v = (Vertex*) (model + 1) + i;
	minX = maxX = v->m_unk0x0c;
	minY = maxY = v->m_unk0x10;
	minZ = maxZ = v->m_unk0x14;
	while (i--) {
		v = (Vertex*) (model + 1) + i;
		if (v->m_unk0x0c < minX) {
			minX = v->m_unk0x0c;
		}
		if (v->m_unk0x10 < minY) {
			minY = v->m_unk0x10;
		}
		if (v->m_unk0x14 < minZ) {
			minZ = v->m_unk0x14;
		}
		if (v->m_unk0x0c > maxX) {
			maxX = v->m_unk0x0c;
		}
		if (v->m_unk0x10 > maxY) {
			maxY = v->m_unk0x10;
		}
		if (v->m_unk0x14 > maxZ) {
			maxZ = v->m_unk0x14;
		}
	}

	p_shape->m_unk0x28 = p_shape->m_unk0x34 = (maxX + minX) >> 1;
	p_shape->m_unk0x2c = p_shape->m_unk0x38 = (maxY + minY) >> 1;
	p_shape->m_unk0x30 = p_shape->m_unk0x3c = (maxZ + minZ) >> 1;

	max = 0.0;
	i = model->m_vertexCount;
	while (i--) {
		v = (Vertex*) (model + 1) + i;
		dx = v->m_unk0x0c - p_shape->m_unk0x34;
		dy = v->m_unk0x10 - p_shape->m_unk0x38;
		dz = v->m_unk0x14 - p_shape->m_unk0x3c;
		d = dx * dx + dy * dy + dz * dz;
		if (d > max) {
			max = d;
		}
	}

	p_shape->m_unk0x40 = (MechS32) sqrt(max);
}

// Sets the face's normal (both copies) from its vertices: a triangle's directly, otherwise from
// its longest edge and the vertex that gives the largest area with it, stopping at the first
// area over 16. A face of fewer than 3 vertices keeps the default (0x20000000, 0, 0).
// FUNCTION: MW2 0x1003b0e4
void ComputeFaceNormal(Face* p_face, Vertex* p_vertices)
{
	MechS32 length;
	MechS32 nx;
	Vertex* vertex;
	MechS32 ny;
	MechS32 nz;
	Vertex* prev;
	Vertex* a;
	MechS32 best;
	MechS32 i;
	MechS32 area;
	Vertex* b;
	MechS32 bestArea;
	Vertex* v0;
	Vertex* v1;
	Vertex* v2;

	i = 0;
	best = 0;
	p_face->m_unk0x08 = p_face->m_normal[0] = 0x20000000;
	p_face->m_unk0x0c = p_face->m_normal[1] = 0;
	p_face->m_unk0x10 = p_face->m_normal[2] = 0;
	if (p_face->m_unk0x02 < 3) {
		return;
	}

	if (p_face->m_unk0x02 == 3) {
		v0 = &p_vertices[((MechU8*) p_face)[p_face->m_unk0x04]];
		v1 = &p_vertices[((MechU8*) p_face)[p_face->m_unk0x04 + 1]];
		v2 = &p_vertices[((MechU8*) p_face)[p_face->m_unk0x04 + 2]];
		FUN_10039dda(
			v0->m_unk0x00,
			v0->m_unk0x04,
			v0->m_unk0x08,
			v1->m_unk0x00,
			v1->m_unk0x04,
			v1->m_unk0x08,
			v2->m_unk0x00,
			v2->m_unk0x04,
			v2->m_unk0x08,
			&nx,
			&ny,
			&nz
		);
		p_face->m_unk0x08 = p_face->m_normal[0] = nx;
		p_face->m_unk0x0c = p_face->m_normal[1] = ny;
		p_face->m_unk0x10 = p_face->m_normal[2] = nz;
		return;
	}

	prev = &p_vertices[((MechU8*) p_face)[p_face->m_unk0x04]];
	for (i = p_face->m_unk0x02; i--; prev = vertex) {
		vertex = &p_vertices[((MechU8*) p_face)[p_face->m_unk0x04 + i]];
		length = ApproximateVectorLength(
			prev->m_unk0x00 - vertex->m_unk0x00,
			prev->m_unk0x04 - vertex->m_unk0x04,
			prev->m_unk0x08 - vertex->m_unk0x08
		);
		if (length > best) {
			best = length;
			a = vertex;
			b = prev;
		}
	}

	if (best == 0) {
		return;
	}

	bestArea = -1;
	i = p_face->m_unk0x02;
	while (i--) {
		vertex = &p_vertices[((MechU8*) p_face)[p_face->m_unk0x04 + i]];
		if (vertex != a && vertex != b) {
			area = FUN_10039dda(
				a->m_unk0x00,
				a->m_unk0x04,
				a->m_unk0x08,
				b->m_unk0x00,
				b->m_unk0x04,
				b->m_unk0x08,
				vertex->m_unk0x00,
				vertex->m_unk0x04,
				vertex->m_unk0x08,
				&nx,
				&ny,
				&nz
			);
			if (area > bestArea) {
				p_face->m_unk0x08 = p_face->m_normal[0] = nx;
				p_face->m_unk0x0c = p_face->m_normal[1] = ny;
				p_face->m_unk0x10 = p_face->m_normal[2] = nz;
				bestArea = area;
				if (area > 16) {
					return;
				}
			}
		}
	}
}

// Returns the selected model's vertex and face counts.
// FUNCTION: MW2 0x1003b43d
void GetModelCounts(Shape* p_shape, MechS32* p_vertexCount, MechS32* p_faceCount)
{
	Model* model;

	model = p_shape->m_model;
	if (!model) {
		return;
	}

	if (p_vertexCount) {
		*p_vertexCount = model->m_vertexCount;
	}

	if (p_faceCount) {
		*p_faceCount = model->m_faceCount;
	}
}

// Calls p_fn for each shape of the list after p_shape.
// FUNCTION: MW2 0x1003b48f
void ForEachShape(Shape* p_shape, void (*p_fn)(Shape*))
{
	Shape* shape;
	Shape* next;

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
void GetTransformedVertex(Shape* p_shape, MechS32 p_index, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	Model* model;
	Vertex* vertex;

	model = p_shape->m_model;
	if (!model) {
		return;
	}

	vertex = &((Vertex*) (model + 1))[p_index];
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
// The vertex address adds the model and the index in the other order (GetTransformedVertex's
// identical expression doesn't).
// FUNCTION: MW2 0x1003b55a
void GetVertexPosition(Shape* p_shape, MechS32 p_index, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	Model* model;
	Vertex* vertex;

	model = p_shape->m_model;
	if (!model) {
		return;
	}

	vertex = &((Vertex*) (model + 1))[p_index];
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
	Shape* p_shape,
	MechS32 p_index,
	MechU32* p_unk0x00,
	MechU32* p_count,
	MechU32* p_indices,
	MechS32 p_max
)
{
	Model* model;
	Face* face;
	MechS32 count;

	model = p_shape->m_model;
	if (!model) {
		return;
	}

	face = (Face*) ((MechU8*) model + model->m_faceOffset) + p_index;
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
void FUN_1003b696(Shape* p_shape, MechS32 p_index, MechS32 p_unk0x00)
{
	Model* model;

	model = p_shape->m_model;
	if (!model) {
		return;
	}

	if (p_index < model->m_faceCount) {
		((Face*) ((MechU8*) model + model->m_faceOffset) + p_index)->m_unk0x00 = p_unk0x00;
	}
}

// FUNCTION: MW2 0x1003b6e5
struct SceneObject* GetShapeObject(Shape* p_shape)
{
	return p_shape->m_object;
}

// FUNCTION: MW2 0x1003b6fb
void SetShapeObject(Shape* p_shape, struct SceneObject* p_unk0x18)
{
	p_shape->m_object = p_unk0x18;
}

// FUNCTION: MW2 0x1003b70f
MechU32 FUN_1003b70f(Shape* p_shape)
{
	return p_shape->m_unk0x00 & 0x10f;
}

// FUNCTION: MW2 0x1003b72f
void FUN_1003b72f(Shape* p_shape, MechU32 p_flags)
{
	p_shape->m_unk0x00 = (p_shape->m_unk0x00 & ~0x10f) | (p_flags & 0x10f) | 0x8000;
}

// Unlinks the shape and frees it.
// FUNCTION: MW2 0x1003b75e
void DestroyShape(Shape* p_shape)
{
	if (p_shape) {
		FUN_1006d7fb(p_shape);
		FreeShape(p_shape);
	}
}

// Detaches the shape from its scene object and frees it. A ShapeCallback (FUN_10001f82).
// FUNCTION: MW2 0x1003b78b
void FUN_1003b78b(Shape* p_shape)
{
	struct SceneObject* obj;

	obj = GetShapeObject(p_shape);
	if (obj) {
		FUN_10001532(obj, NULL);
	}

	DestroyShape(p_shape);
}

// Returns the bytes the shape, its models and its bounding data take.
// The only diff is a stack-slot permutation of model, vertex and size.
// FUNCTION: MW2 0x1003b7cc
MechS32 GetShapeMemorySize(Shape* p_shape)
{
	Model* model;
	Vertex* vertex;
	MechS32 size;
	MechS32 i;
	Face* face;

	size = sizeof(Shape);
	if (p_shape->m_object) {
		size += sizeof(struct SceneObject);
	}

	for (model = p_shape->m_models; model; model = model->m_next) {
		size += sizeof(Model);
		vertex = (Vertex*) (model + 1);
		for (i = model->m_vertexCount; i--; vertex++) {
			size += sizeof(Vertex);
		}

		face = (Face*) ((MechU8*) model + model->m_faceOffset);
		for (i = model->m_faceCount; i--; face++) {
			size += face->m_unk0x02 + sizeof(Face);
		}
	}

	return size + FUN_1006edc3(p_shape);
}
