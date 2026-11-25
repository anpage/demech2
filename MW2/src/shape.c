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
#ifdef MW2_AIMED_SHAPE_RESET
#include "weapons.h"
#endif
#ifdef MW2_POWERVR
#include "accel/a3d.h"
#endif

#include <math.h>
#include <windows.h>

// The flags a new shape starts with (CreateShape).
// GLOBAL: MW2 0x100a5898
// GLOBAL: MW2GBL 0x100ae85c
// GLOBAL: MW2PREMERCS 0x100b3bf0
// GLOBAL: MW2MATROX 0x100a5964
// GLOBAL: MW23DFX 0x1024e400
// GLOBAL: MW2ATI 0x10182710
// GLOBAL: MW2S3 0x10065cd8
MechU32 g_newShapeFlags = 0;

// The Matrox edition starts this object with ComputeShapeBounds and AddModel, which 1.1 has
// further down.
#ifdef MW2_ACCEL
#include "addmodel.c"
#include "shapebounds.c"
#endif

#ifndef MW2_ACCEL
// FUNCTION: MW2 0x1003a530
// FUNCTION: MW2GBL 0x100407a0
// FUNCTION: MW2PREMERCS 0x1005b710
MechU32 GetNewShapeFlags(void)
{
	return g_newShapeFlags;
}

// Sets the flags new shapes start with; returns the previous ones.
// FUNCTION: MW2 0x1003a545
// FUNCTION: MW2GBL 0x100407b5
// FUNCTION: MW2PREMERCS 0x1005b725
MechU32 SetNewShapeFlags(MechU32 p_flags)
{
	MechU32 previous;

	previous = g_newShapeFlags;
	g_newShapeFlags = p_flags;
	return previous;
}

// Selects the shape's first model.
// FUNCTION: MW2 0x1003a56b
// FUNCTION: MW2GBL 0x100407db
// FUNCTION: MW2PREMERCS 0x1005b74b
void SelectFirstModel(Shape* p_shape)
{
	SelectModel(p_shape, p_shape->m_models);
}
#endif

// FUNCTION: MW2 0x1003a589
// FUNCTION: MW2GBL 0x100407f9
// FUNCTION: MW2PREMERCS 0x1005b769
// FUNCTION: MW2MATROX 0x10027e7e
// FUNCTION: MW23DFX 0x10029450
// FUNCTION: MW2ATI 0x10029110
// FUNCTION: MW2S3 0x10025b60
// FUNCTION: MW2POWERVR 0x10025f60
void SelectModel(Shape* p_shape, Model* p_model)
{
	p_shape->m_model = p_model;
}

#ifndef MW2_ACCEL
// Selects the next model, wrapping around to the first.
// FUNCTION: MW2 0x1003a59d
// FUNCTION: MW2GBL 0x1004080d
// FUNCTION: MW2PREMERCS 0x1005b77d
void SelectNextModel(Shape* p_shape)
{
	if (!p_shape->m_model || !p_shape->m_model->m_next) {
		SelectModel(p_shape, p_shape->m_models);
	}
	else {
		SelectModel(p_shape, p_shape->m_model->m_next);
	}
}
#endif

#ifndef MW2_ACCEL
#include "addmodel.c"
#endif

#ifndef MW2_ACCEL
// Selects the first model whose key is at most p_key.
// FUNCTION: MW2 0x1003a7a2
// FUNCTION: MW2GBL 0x10040a0e
// FUNCTION: MW2PREMERCS 0x1005b97e
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
#endif

// FUNCTION: MW2 0x1003a7f9
// FUNCTION: MW2GBL 0x10040a65
// FUNCTION: MW2PREMERCS 0x1005b9d5
// FUNCTION: MW2MATROX 0x10027e92
// FUNCTION: MW23DFX 0x10029580
// FUNCTION: MW2ATI 0x10029240
// FUNCTION: MW2S3 0x10025b70
// FUNCTION: MW2POWERVR 0x100260a0
void SetModelKey(Shape* p_shape, MechScalar p_key)
{
	Model* model;

	model = p_shape->m_model;
	if (!model) {
		return;
	}

	model->m_key = p_key;
}

#ifndef MW2_ACCEL
// FUNCTION: MW2 0x1003a827
// FUNCTION: MW2GBL 0x10040a93
// FUNCTION: MW2PREMERCS 0x1005ba03
MechS32 GetModelKey(Shape* p_shape)
{
	Model* model;

	model = p_shape->m_model;
	if (!model) {
		return 0;
	}

	return model->m_key;
}

// FUNCTION: MW2 0x1003a859
// FUNCTION: MW2GBL 0x10040ac5
// FUNCTION: MW2PREMERCS 0x1005ba35
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
// FUNCTION: MW2GBL 0x10040af5
// FUNCTION: MW2PREMERCS 0x1005ba65
MechU32 FUN_1003a889(Shape* p_shape)
{
	Model* model;

	model = p_shape->m_model;
	if (!model) {
		return 0;
	}

	return model->m_unk0x14;
}
#endif

// Allocates a shape with one model (AddModel).
// FUNCTION: MW2 0x1003a8c1
// FUNCTION: MW2GBL 0x10040b2d
// FUNCTION: MW2MATROX 0x10027ec0
// FUNCTION: MW23DFX 0x10029590
// FUNCTION: MW2ATI 0x10029250
// FUNCTION: MW2S3 0x10025b80
// FUNCTION: MW2POWERVR 0x100260b0
Shape* CreateShape(MechS32 p_vertexCount, MechS32 p_faceCount, MechS32 p_extra, void** p_extraData)
{
	Shape* shape;

	shape = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(Shape));
	if (!shape) {
		return NULL;
	}

	shape->m_flags = g_newShapeFlags | 0x8000;
	shape->m_object = NULL;
	shape->m_model = shape->m_models = NULL;
	if (!AddModel(shape, 0, p_vertexCount, p_faceCount, p_extra, p_extraData)) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, shape);
		return NULL;
	}

	shape->m_kind = 0;
	shape->m_partId = 0;
	shape->m_owner = 0;
	shape->m_centerX = shape->m_centerY = shape->m_centerZ = 0;
	shape->m_modelCenterX = shape->m_modelCenterY = shape->m_modelCenterZ = 0;
	shape->m_radius = 0;
	shape->m_collisionData = NULL;
	shape->m_prev = shape->m_next = NULL;
	shape->m_prevCollider = shape->m_nextCollider = NULL;
	shape->m_collisionType = 4;
	shape->m_transformCount = 0;
#ifdef MW2_POWERVR
	shape->m_list = -1;
#endif
	return shape;
}

// Appends a vertex to the selected model.
// FUNCTION: MW2 0x1003aa1d
// FUNCTION: MW2GBL 0x10040c89
// FUNCTION: MW2PREMERCS 0x1005bbf9
// FUNCTION: MW2MATROX 0x1002801c
// FUNCTION: MW23DFX 0x10029640
// FUNCTION: MW2ATI 0x10029300
// FUNCTION: MW2S3 0x10025c30
// FUNCTION: MW2POWERVR 0x10026170
void AddShapeVertex(
	Shape* p_shape,
	MechScalar p_x,
	MechScalar p_y,
	MechScalar p_z,
#ifdef MW2_FLOAT_SCALARS
	MechFloat p_u,
	MechFloat p_v,
#ifdef MW2_A3D
	MechFloat p_red,
	MechFloat p_green,
	MechFloat p_blue,
	MechFloat p_normalX,
	MechFloat p_normalY,
	MechFloat p_normalZ
#endif
#else
	undefined4 p_u,
	undefined4 p_v
#endif
)
{
	Model* model;
	Vertex* vertex;

	model = p_shape->m_model;
	if (!model) {
		return;
	}

	vertex = (Vertex*) (model + 1) + model->m_vertexCount;
	vertex->m_worldX = vertex->m_modelX = p_x;
	vertex->m_worldY = vertex->m_modelY = p_y;
	vertex->m_worldZ = vertex->m_modelZ = p_z;
	vertex->m_u = p_u;
	vertex->m_v = p_v;
#ifdef MW2_A3D
	vertex->m_red = p_red;
	vertex->m_green = p_green;
	vertex->m_blue = p_blue;
	vertex->m_modelNormalX = p_normalX;
	vertex->m_modelNormalY = p_normalY;
	vertex->m_modelNormalZ = p_normalZ;
#endif
	model->m_vertexCount++;
}

#ifdef MW2_POWERVR
// Appends a face to the selected model, its p_count vertex indices (p_values) stored at p_indices: the
// PowerVR edition's AddShapeFace takes the indices itself, and has no AddShapeFaceIndex.
// FUNCTION: MW2POWERVR 0x100261f0
Face* AddShapeFace(Shape* p_shape, MechU16 p_color, MechU8* p_indices, MechS32 p_count, MechU16* p_values)
{
	Model* model;
	Face* face;
	MechS32 i;
	MechU32 values[8];

	model = p_shape->m_model;
	if (!model) {
		return NULL;
	}

	face = (Face*) ((MechU8*) model + model->m_faceOffset) + model->m_faceCount;
	face->m_indexOffset = (MechU16) (p_indices - (MechU8*) face);
	for (i = 0; i < p_count; i++) {
		((MechU8*) face)[face->m_indexOffset + i] = (MechU8) p_values[i];
		values[i] = p_values[i];
	}

	face->m_color = p_color;
	model->m_faceCount++;
	face->m_shape = p_shape;
	face->m_indexCount = p_count;
	return face;
}
#else
// Appends a face to the selected model, its vertex indices at p_indices.
// FUNCTION: MW2 0x1003aab5
// FUNCTION: MW2GBL 0x10040d21
// FUNCTION: MW2PREMERCS 0x1005bc91
// FUNCTION: MW2MATROX 0x100280e6
// FUNCTION: MW23DFX 0x100296c0
// FUNCTION: MW2ATI 0x10029380
// FUNCTION: MW2S3 0x10025cb0
Face* AddShapeFace(Shape* p_shape, MechU16 p_color, MechU8* p_indices)
{
	Model* model;
	Face* face;

	model = p_shape->m_model;
	if (!model) {
		return NULL;
	}

	face = (Face*) ((MechU8*) model + model->m_faceOffset) + model->m_faceCount;
	face->m_indexOffset = (MechU16) (p_indices - (MechU8*) face);
	face->m_shape = p_shape;
	face->m_color = p_color;
	face->m_indexCount = 0;
	model->m_faceCount++;
	return face;
}

// Appends a vertex index to a face of the selected model.
// FUNCTION: MW2 0x1003ab34
// FUNCTION: MW2GBL 0x10040da0
// FUNCTION: MW2PREMERCS 0x1005bd10
// FUNCTION: MW2MATROX 0x10028165
// FUNCTION: MW23DFX 0x10029710
// FUNCTION: MW2ATI 0x100293d0
// FUNCTION: MW2S3 0x10025d00
void AddShapeFaceIndex(Shape* p_shape, Face* p_face, MechU32 p_index)
{
	Model* model;

	model = p_shape->m_model;
	if (!model) {
		return;
	}

	((MechU8*) p_face)[p_face->m_indexOffset + p_face->m_indexCount] = (MechU8) p_index;
	p_face->m_indexCount++;
}
#endif

// FUNCTION: MW2 0x1003ab79
// FUNCTION: MW2GBL 0x10040de5
// FUNCTION: MW2PREMERCS 0x1005bd55
// FUNCTION: MW2MATROX 0x100281aa
// FUNCTION: MW23DFX 0x10029740
// FUNCTION: MW2ATI 0x10029400
// FUNCTION: MW2S3 0x10025d30
// FUNCTION: MW2POWERVR 0x100262a0
void FreeModel(Model* p_model)
{
	if (!p_model) {
		return;
	}

	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_model);
}

// Removes the selected model from the shape's list and frees it.
// FUNCTION: MW2 0x1003aba5
// FUNCTION: MW2GBL 0x10040e11
// FUNCTION: MW2PREMERCS 0x1005bd81
// FUNCTION: MW2MATROX 0x100281d6
// FUNCTION: MW23DFX 0x10029760
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
// FUNCTION: MW2GBL 0x10040ecb
// FUNCTION: MW2PREMERCS 0x1005be3b
// FUNCTION: MW2MATROX 0x10028290
// FUNCTION: MW23DFX 0x100297c0
// FUNCTION: MW2ATI 0x10029420
// FUNCTION: MW2S3 0x10025d50
// FUNCTION: MW2POWERVR 0x100262c0
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

	FreeBoundBox(p_shape);
#ifdef MW2_POWERVR
	FUN_1005f280(p_shape->m_list);
#endif
	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_shape);
}

// FUNCTION: MW2 0x1003acbe
// FUNCTION: MW2GBL 0x10040f2a
// FUNCTION: MW2MATROX 0x100282ef
// FUNCTION: MW23DFX 0x10029800
// FUNCTION: MW2ATI 0x10029460
// FUNCTION: MW2S3 0x10025d90
// FUNCTION: MW2POWERVR 0x10026300
void SetShapeState(Shape* p_shape, MechS32 p_flags)
{
	if (p_shape) {
		p_shape->m_flags = (p_shape->m_flags & ~0x7ef0) | (p_flags & 0x7ef0) | 0x8000;
	}
}

#ifdef MW2_POWERVR
// Sets the shape's damage level (m_flags 0xf0) alone: the PowerVR edition's RaisePartDamageLevel
// calls it where the others set the whole state.
// FUNCTION: MW2POWERVR 0x10026320
void SetShapeDamageLevel(Shape* p_shape, MechU16 p_level)
{
	if (p_shape) {
		p_shape->m_flags = (p_shape->m_flags & ~0xf0) | (p_level & 0xf0);
	}
}
#endif

// FUNCTION: MW2 0x1003acf7
// FUNCTION: MW2GBL 0x10040f63
// FUNCTION: MW2MATROX 0x10028328
// FUNCTION: MW23DFX 0x10029820
// FUNCTION: MW2ATI 0x10029480
// FUNCTION: MW2S3 0x10025db0
// FUNCTION: MW2POWERVR 0x10026340
MechU32 GetShapeState(Shape* p_shape)
{
	if (p_shape) {
		return p_shape->m_flags & 0x7ef0;
	}
	else {
		return 0;
	}
}

// FUNCTION: MW2 0x1003ad2d
// FUNCTION: MW2GBL 0x10040f99
// FUNCTION: MW2MATROX 0x1002835e
// FUNCTION: MW23DFX 0x10029830
// FUNCTION: MW2ATI 0x10029490
// FUNCTION: MW2S3 0x10025dc0
// FUNCTION: MW2POWERVR 0x10026350
void SetShapeKind(Shape* p_shape, MechS32 p_kind)
{
	if (p_shape) {
		p_shape->m_kind = p_kind;
	}
}

// FUNCTION: MW2 0x1003ad4c
// FUNCTION: MW2GBL 0x10040fb8
// FUNCTION: MW2PREMERCS 0x1005befe
// FUNCTION: MW2MATROX 0x1002837d
// FUNCTION: MW23DFX 0x10029840
// FUNCTION: MW2ATI 0x100294a0
// FUNCTION: MW2S3 0x10025dd0
// FUNCTION: MW2POWERVR 0x10026360
void SetShapePartId(Shape* p_shape, MechU16 p_partId)
{
	p_shape->m_partId = p_partId;
}

// FUNCTION: MW2 0x1003ad62
// FUNCTION: MW2GBL 0x10040fce
// FUNCTION: MW2PREMERCS 0x1005bf14
// FUNCTION: MW2MATROX 0x10028393
// FUNCTION: MW23DFX 0x10029850
// FUNCTION: MW2ATI 0x100294b0
// FUNCTION: MW2S3 0x10025de0
// FUNCTION: MW2POWERVR 0x10026370
void SetShapeOwner(Shape* p_shape, MechU16 p_owner)
{
	p_shape->m_owner = p_owner;
}

// FUNCTION: MW2 0x1003ad78
// FUNCTION: MW2GBL 0x10040fe4
// FUNCTION: MW2PREMERCS 0x1005bf2a
// FUNCTION: MW2MATROX 0x100283a9
// FUNCTION: MW23DFX 0x10029860
MechU32 GetShapeKind(Shape* p_shape)
{
	return p_shape->m_kind;
}

// FUNCTION: MW2 0x1003ad93
// FUNCTION: MW2GBL 0x10040fff
// FUNCTION: MW2PREMERCS 0x1005bf45
// FUNCTION: MW2MATROX 0x100283c4
// FUNCTION: MW23DFX 0x10029870
MechU32 GetShapePartId(Shape* p_shape)
{
	return p_shape->m_partId;
}

// FUNCTION: MW2 0x1003adae
// FUNCTION: MW2GBL 0x1004101a
// FUNCTION: MW2PREMERCS 0x1005bf60
// FUNCTION: MW2MATROX 0x100283df
// FUNCTION: MW23DFX 0x10029880
MechU32 GetShapeOwner(Shape* p_shape)
{
	return p_shape->m_owner;
}

// FUNCTION: MW2 0x1003adc9
// FUNCTION: MW2GBL 0x10041035
// FUNCTION: MW2PREMERCS 0x1005bf7b
// FUNCTION: MW2MATROX 0x100283fa
// FUNCTION: MW23DFX 0x10029890
// FUNCTION: MW2ATI 0x100294c0
// FUNCTION: MW2S3 0x10025df0
// FUNCTION: MW2POWERVR 0x10026380
MechScalar GetShapeBounds(Shape* p_shape, MechScalar* p_x, MechScalar* p_y, MechScalar* p_z)
{
	if (p_x) {
		*p_x = p_shape->m_centerX;
	}

	if (p_y) {
		*p_y = p_shape->m_centerY;
	}

	if (p_z) {
		*p_z = p_shape->m_centerZ;
	}

	return p_shape->m_radius;
}

// Computes the selected model's face normals, then the shape's center and radius (a shape
// LoadShapeRecord has just built).
// FUNCTION: MW2 0x1003ae1e
// FUNCTION: MW2GBL 0x1004108a
// FUNCTION: MW2PREMERCS 0x1005bfd0
// FUNCTION: MW2MATROX 0x1002844f
// FUNCTION: MW23DFX 0x100298c0
// FUNCTION: MW2ATI 0x100294f0
// FUNCTION: MW2S3 0x10025e20
// FUNCTION: MW2POWERVR 0x100263b0
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

#ifndef MW2_ACCEL
#include "shapebounds.c"
#endif

// Sets the face's normal (both copies) from its vertices: a triangle's directly, otherwise from
// its longest edge and the vertex that gives the largest area with it, stopping at the first
// area over 16. A face of fewer than 3 vertices keeps the default (0x20000000, 0, 0).
// FUNCTION: MW2 0x1003b0e4
// FUNCTION: MW2GBL 0x10041358
// FUNCTION: MW2PREMERCS 0x1005c2a1
// FUNCTION: MW2MATROX 0x100284c7
// FUNCTION: MW2S3 0x10025e80
void ComputeFaceNormal(Face* p_face, Vertex* p_vertices)
{
	MechScalar length;
	MechScalar nx;
	Vertex* vertex;
	MechScalar ny;
	MechScalar nz;
	Vertex* prev;
	Vertex* a;
	MechScalar best;
	MechS32 i;
	MechS32 area;
	Vertex* b;
	MechS32 bestArea;
	Vertex* v0;
	Vertex* v1;
	Vertex* v2;

	i = 0;
	best = 0;
#ifdef MW2_FLOAT_SCALARS
	p_face->m_modelNormalX = p_face->m_normal[0] = 1.0f;
#else
	p_face->m_modelNormalX = p_face->m_normal[0] = 0x20000000;
#endif
	p_face->m_modelNormalY = p_face->m_normal[1] = 0;
	p_face->m_modelNormalZ = p_face->m_normal[2] = 0;
	if (p_face->m_indexCount < 3) {
		return;
	}

	if (p_face->m_indexCount == 3) {
		v0 = &p_vertices[((MechU8*) p_face)[p_face->m_indexOffset]];
		v1 = &p_vertices[((MechU8*) p_face)[p_face->m_indexOffset + 1]];
		v2 = &p_vertices[((MechU8*) p_face)[p_face->m_indexOffset + 2]];
		ComputeTriangleNormal(
			v0->m_modelX,
			v0->m_modelY,
			v0->m_modelZ,
			v1->m_modelX,
			v1->m_modelY,
			v1->m_modelZ,
			v2->m_modelX,
			v2->m_modelY,
			v2->m_modelZ,
			&nx,
			&ny,
			&nz
		);
		p_face->m_modelNormalX = p_face->m_normal[0] = nx;
		p_face->m_modelNormalY = p_face->m_normal[1] = ny;
		p_face->m_modelNormalZ = p_face->m_normal[2] = nz;
		return;
	}

	prev = &p_vertices[((MechU8*) p_face)[p_face->m_indexOffset]];
	for (i = p_face->m_indexCount; i--; prev = vertex) {
		vertex = &p_vertices[((MechU8*) p_face)[p_face->m_indexOffset + i]];
#ifdef MW2_FLOAT_SCALARS
		if ((length = ApproximateVectorLength(
				 prev->m_modelX - vertex->m_modelX,
				 prev->m_modelY - vertex->m_modelY,
				 prev->m_modelZ - vertex->m_modelZ
			 )) > best) {
#else
		length = ApproximateVectorLength(
			prev->m_modelX - vertex->m_modelX,
			prev->m_modelY - vertex->m_modelY,
			prev->m_modelZ - vertex->m_modelZ
		);
		if (length > best) {
#endif
			best = length;
			a = vertex;
			b = prev;
		}
	}

	if (!FIXED_IS_NONZERO(best)) {
		return;
	}

	bestArea = -1;
	i = p_face->m_indexCount;
	while (i--) {
		vertex = &p_vertices[((MechU8*) p_face)[p_face->m_indexOffset + i]];
		if (vertex != a && vertex != b) {
			area = ComputeTriangleNormal(
				a->m_modelX,
				a->m_modelY,
				a->m_modelZ,
				b->m_modelX,
				b->m_modelY,
				b->m_modelZ,
				vertex->m_modelX,
				vertex->m_modelY,
				vertex->m_modelZ,
				&nx,
				&ny,
				&nz
			);
			if (area > bestArea) {
				p_face->m_modelNormalX = p_face->m_normal[0] = nx;
				p_face->m_modelNormalY = p_face->m_normal[1] = ny;
				p_face->m_modelNormalZ = p_face->m_normal[2] = nz;
				bestArea = area;
			}

			if (area > 16) {
				return;
			}
		}
	}
}

// Returns the selected model's vertex and face counts.
// FUNCTION: MW2 0x1003b43d
// FUNCTION: MW2GBL 0x100416b1
// FUNCTION: MW2PREMERCS 0x1005c5fa
// FUNCTION: MW2MATROX 0x10028825
// FUNCTION: MW23DFX 0x10029920
// FUNCTION: MW2ATI 0x10029550
// FUNCTION: MW2S3 0x100260c0
// FUNCTION: MW2POWERVR 0x10026410
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
// FUNCTION: MW2GBL 0x10041703
// FUNCTION: MW2PREMERCS 0x1005c64c
// FUNCTION: MW2MATROX 0x10028877
// FUNCTION: MW23DFX 0x10029950
// FUNCTION: MW2ATI 0x10029580
// FUNCTION: MW2S3 0x100260f0
// FUNCTION: MW2POWERVR 0x10026440
void ForEachShape(Shape* p_shape, void (*p_fn)(Shape*))
{
	Shape* shape;
	Shape* next;

	if (!p_shape) {
		return;
	}

	shape = p_shape->m_next;
	while (shape) {
		next = shape->m_next;
		p_fn(shape);
		shape = next;
	}
}

#ifndef MW2_ACCEL
// Returns a vertex's transformed position.
// FUNCTION: MW2 0x1003b4dd
// FUNCTION: MW2GBL 0x10041751
// FUNCTION: MW2PREMERCS 0x1005c69a
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
		*p_x = vertex->m_worldX;
	}

	if (p_y) {
		*p_y = vertex->m_worldY;
	}

	if (p_z) {
		*p_z = vertex->m_worldZ;
	}
}

// Returns a vertex's position in the model.
// FUNCTION: MW2 0x1003b55a
// FUNCTION: MW2GBL 0x100417ce
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
		*p_x = vertex->m_modelX;
	}

	if (p_y) {
		*p_y = vertex->m_modelY;
	}

	if (p_z) {
		*p_z = vertex->m_modelZ;
	}
}

// Returns a face of the selected model and up to p_max of its vertex indices.
// FUNCTION: MW2 0x1003b5d6
// FUNCTION: MW2GBL 0x1004184a
// FUNCTION: MW2PREMERCS 0x1005c793
void GetShapeFace(
	Shape* p_shape,
	MechS32 p_index,
	MechU32* p_color,
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
	if (p_color) {
		*p_color = face->m_color;
	}

	count = face->m_indexCount;
	if (p_count) {
		*p_count = count;
	}

	if (p_indices) {
		if (count > p_max) {
			count = p_max;
		}

		while (count--) {
			p_indices[count] = ((MechU8*) face)[face->m_indexOffset + count];
		}
	}
}
#endif

// FUNCTION: MW2 0x1003b696
// FUNCTION: MW2GBL 0x1004190a
// FUNCTION: MW2PREMERCS 0x1005c853
// FUNCTION: MW2MATROX 0x100288c5
// FUNCTION: MW23DFX 0x10029970
// FUNCTION: MW2ATI 0x100295a0
// FUNCTION: MW2S3 0x10026110
// FUNCTION: MW2POWERVR 0x10026460
void SetFaceColor(Shape* p_shape, MechS32 p_index, MechS32 p_color)
{
	Model* model;

	model = p_shape->m_model;
	if (!model) {
		return;
	}

	if (p_index < model->m_faceCount) {
		((Face*) ((MechU8*) model + model->m_faceOffset) + p_index)->m_color = p_color;
	}
}

// FUNCTION: MW2 0x1003b6e5
// FUNCTION: MW2GBL 0x10041959
// FUNCTION: MW2PREMERCS 0x1005c8a2
// FUNCTION: MW2MATROX 0x10028914
// FUNCTION: MW23DFX 0x100299a0
// FUNCTION: MW2ATI 0x100295d0
// FUNCTION: MW2S3 0x10026140
// FUNCTION: MW2POWERVR 0x10026490
struct SceneObject* GetShapeObject(Shape* p_shape)
{
	return p_shape->m_object;
}

// FUNCTION: MW2 0x1003b6fb
// FUNCTION: MW2GBL 0x1004196f
// FUNCTION: MW2PREMERCS 0x1005c8b8
// FUNCTION: MW2MATROX 0x1002892a
// FUNCTION: MW23DFX 0x100299b0
// FUNCTION: MW2ATI 0x100295e0
// FUNCTION: MW2S3 0x10026150
// FUNCTION: MW2POWERVR 0x100264a0
void SetShapeObject(Shape* p_shape, struct SceneObject* p_object)
{
	p_shape->m_object = p_object;
}

#ifndef MW2_ACCEL
// FUNCTION: MW2 0x1003b70f
// FUNCTION: MW2GBL 0x10041983
// FUNCTION: MW2PREMERCS 0x1005c8cc
MechU32 GetShapeLoadFlags(Shape* p_shape)
{
	return p_shape->m_flags & 0x10f;
}
#endif

// FUNCTION: MW2 0x1003b72f
// FUNCTION: MW2GBL 0x100419a3
// FUNCTION: MW2PREMERCS 0x1005c8ec
// FUNCTION: MW2MATROX 0x1002893e
// FUNCTION: MW23DFX 0x100299c0
// FUNCTION: MW2ATI 0x100295f0
// FUNCTION: MW2S3 0x10026160
// FUNCTION: MW2POWERVR 0x100264b0
void SetShapeLoadFlags(Shape* p_shape, MechU32 p_flags)
{
	p_shape->m_flags = (p_shape->m_flags & ~0x10f) | (p_flags & 0x10f) | 0x8000;
}

// Unlinks the shape and frees it.
// FUNCTION: MW2 0x1003b75e
// FUNCTION: MW2GBL 0x100419d2
// FUNCTION: MW2PREMERCS 0x1005c91b
// FUNCTION: MW2MATROX 0x1002896d
// FUNCTION: MW23DFX 0x100299e0
// FUNCTION: MW2ATI 0x10029610
// FUNCTION: MW2S3 0x10026180
// FUNCTION: MW2POWERVR 0x100264d0
void DestroyShape(Shape* p_shape)
{
	if (p_shape) {
		RemoveSceneShape(p_shape);
		FreeShape(p_shape);
	}
}

// Detaches the shape from its scene object and frees it. A ShapeCallback (DestroyObjTree).
// FUNCTION: MW2 0x1003b78b
// FUNCTION: MW2GBL 0x100419ff
// FUNCTION: MW2PREMERCS 0x1005c948
// FUNCTION: MW2MATROX 0x1002899a
// FUNCTION: MW23DFX 0x10029a00
// FUNCTION: MW2ATI 0x10029630
// FUNCTION: MW2S3 0x100261a0
// FUNCTION: MW2POWERVR 0x100264f0
void DestroyObjShape(Shape* p_shape)
{
	struct SceneObject* obj;

	obj = GetShapeObject(p_shape);
	if (obj) {
		SetObjShape(obj, NULL);
	}

#ifdef MW2_AIMED_SHAPE_RESET
	if (p_shape == g_aimedShape) {
		g_aimedShape = NULL;
	}
#endif
	DestroyShape(p_shape);
}

// Returns the bytes the shape, its models and its bounding data take.
// FUNCTION: MW2 0x1003b7cc
// FUNCTION: MW2GBL 0x10041a40
// FUNCTION: MW2MATROX 0x100289f3
// FUNCTION: MW23DFX 0x10029a40
// FUNCTION: MW2ATI 0x10029670
// FUNCTION: MW2S3 0x100261e0
// FUNCTION: MW2POWERVR 0x10026530
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
			size += face->m_indexCount + sizeof(Face);
		}
	}

	size += GetBoundBoxSize(p_shape);
	return size;
}

// The Matrox edition ends this object with shapegeom.c's functions, a unit of its own in 1.1.
#ifdef MW2_ACCEL
#include "shapegeom.c"
#endif
