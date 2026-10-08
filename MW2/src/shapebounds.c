/* ComputeShapeBounds. Not a unit of its own: shape.c includes it, after ComputeNormalsAndBounds in
   1.1's build and at the start of the object in the Matrox edition's. */
#include "fixedfloat.h"
#include "model.h"
#include "shape.h"
#include "types.h"
#include "vertex.h"

#include <math.h>

// Sets p_shape's center to the middle of its first model's bounding box, and its radius to the
// distance from there to the farthest vertex.
// Stack-slot permutation of the locals. The vertex address loads the index first where the
// original loads the model first (index order), and 4.1 sums the squares in another order, which
// also compares d before storing it; a probe of the same source picks yet another order.
// MW2MATROX: d > max compares before it stores d (fcom, fstp) where the original stores first (fst,
// fcomp), an effect of the symbol order.
// FUNCTION: MW2 0x1003ae96
// FUNCTION: MW2MATROX 0x10027a60
void ComputeShapeBounds(Shape* p_shape)
{
	MechDouble dz;
	Model* model;
	MechDouble max;
	MechScalar minX;
	MechS32 i;
	MechScalar minY;
	MechScalar minZ;
	MechDouble d;
	MechScalar maxX;
	MechScalar maxY;
	MechDouble dx;
	MechScalar maxZ;
	Vertex* v;
	MechDouble dy;

	model = p_shape->m_models;
	if (!model) {
		return;
	}

	i = model->m_vertexCount - 1;
	v = (Vertex*) (model + 1) + i;
	minX = maxX = v->m_worldX;
	minY = maxY = v->m_worldY;
	minZ = maxZ = v->m_worldZ;
	while (i--) {
		v = (Vertex*) (model + 1) + i;
		if (v->m_worldX < minX) {
			minX = v->m_worldX;
		}
		if (v->m_worldY < minY) {
			minY = v->m_worldY;
		}
		if (v->m_worldZ < minZ) {
			minZ = v->m_worldZ;
		}
		if (v->m_worldX > maxX) {
			maxX = v->m_worldX;
		}
		if (v->m_worldY > maxY) {
			maxY = v->m_worldY;
		}
		if (v->m_worldZ > maxZ) {
			maxZ = v->m_worldZ;
		}
	}

#ifdef MW2_MATROX
	p_shape->m_modelCenterX = p_shape->m_centerX = (maxX + minX) * 0.5f;
	p_shape->m_modelCenterY = p_shape->m_centerY = (maxY + minY) * 0.5f;
	p_shape->m_modelCenterZ = p_shape->m_centerZ = (maxZ + minZ) * 0.5f;
#else
	p_shape->m_modelCenterX = p_shape->m_centerX = (maxX + minX) >> 1;
	p_shape->m_modelCenterY = p_shape->m_centerY = (maxY + minY) >> 1;
	p_shape->m_modelCenterZ = p_shape->m_centerZ = (maxZ + minZ) >> 1;
#endif

	max = 0.0;
	i = model->m_vertexCount;
	while (i--) {
		v = (Vertex*) (model + 1) + i;
		dx = v->m_worldX - p_shape->m_centerX;
		dy = v->m_worldY - p_shape->m_centerY;
		dz = v->m_worldZ - p_shape->m_centerZ;
		d = dx * dx + dy * dy + dz * dz;
		if (d > max) {
			max = d;
		}
	}

#ifdef MW2_MATROX
	p_shape->m_radius = sqrt(max);
#else
	p_shape->m_radius = (MechS32) sqrt(max);
#endif
}
