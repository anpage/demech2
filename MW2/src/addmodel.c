/* AddModel. Not a unit of its own: shape.c includes it, after SelectNextModel in 1.1's build and
   at the start of the object (after ComputeShapeBounds) in the Matrox edition's. */
#include "decomp.h"
#include "face.h"
#include "fixedfloat.h"
#include "model.h"
#include "shape.h"
#include "simmain.h"
#include "types.h"
#include "vertex.h"

#include <string.h>
#include <windows.h>

// Allocates a model with room for the vertices, the faces and p_extra more bytes (returned in
// p_extraData), inserts it into the shape's list by p_key and selects it.
// The size sum adds p_extra and vertexBytes in the other order, and the locals sit in
// permuted stack slots.
// FUNCTION: MW2 0x1003a5f3
// FUNCTION: MW2MATROX 0x10027cb8
Model* AddModel(
	Shape* p_shape,
	MechScalar p_key,
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

#ifdef MW2_MATROX
	if (!p_faceCount) {
		p_faceCount = 1;
	}

#endif
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
	model->m_transformCount = 0;
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
