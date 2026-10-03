#ifndef SHAPE_H
#define SHAPE_H

#include "decomp.h"
#include "shapegeom.h"
#include "types.h"

struct SceneObject;
struct Face;
struct Vertex;

/* A shape hung on a scene object (SceneObject::m_unk0x6c): a list of models by level of
   detail and the one in use. CreateShape allocates it. */
typedef struct Shape Shape;

// SIZE 0x4c
struct Shape {
	MechU16 m_flags; // 0x00 — 0x200: transformed bounds current, 0x800: not in the collision list, 0x1000: the second
					 // render list
	MechU16 m_kind;  // 0x02 — 0xf00: the owner class (0x100 player, 0x200 thing), 0xf0: the type
	struct Shape* m_prev;         // 0x04 — the previous shape in its list
	struct Shape* m_next;         // 0x08 — the next shape in its list
	struct Shape* m_prevCollider; // 0x0c — the collision list
	struct Shape* m_nextCollider; // 0x10
	MechU16 m_owner;              // 0x14 — a player index (kind 0x100) or thing index (0x200)
	MechU16 m_partId;             // 0x16 — the part a scene-tree search looks for (a mech section)
	struct SceneObject* m_object; // 0x18
	Model* m_models;              // 0x1c
	Model* m_model;               // 0x20
	MechS32 m_collisionType;      // 0x24 — indexes g_shapeCollisionFns; picks m_collisionData
	MechS32 m_modelCenterX;       // 0x28 — the center ComputeShapeBounds computes
	MechS32 m_modelCenterY;       // 0x2c
	MechS32 m_modelCenterZ;       // 0x30
	MechS32 m_centerX;            // 0x34 — the center transformed (TransformShapeCenter)
	MechS32 m_centerY;            // 0x38
	MechS32 m_centerZ;            // 0x3c
	MechS32 m_radius;             // 0x40 — the bounding sphere around the center
	void* m_collisionData;        // 0x44 — a BoundBox (type 0) or a QuadtreeNode (type 5)
	undefined4 m_transformCount;  // 0x48 — bumped per transform; Model::m_transformCount stamps it
};

// The functions and globals of shape.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechU32 GetNewShapeFlags(void);
	MechU32 SetNewShapeFlags(MechU32 p_flags);
	void SelectFirstModel(Shape* p_shape);
	void SelectModel(Shape* p_shape, Model* p_model);
	void SelectNextModel(Shape* p_shape);
	Model* AddModel(
		Shape* p_shape,
		MechS32 p_key,
		MechS32 p_vertexCount,
		MechS32 p_faceCount,
		MechS32 p_extra,
		void** p_extraData
	);
	void SelectModelByKey(Shape* p_shape, MechS32 p_key);
	void FUN_1003a7f9(Shape* p_shape, MechS32 p_key);
	MechS32 FUN_1003a827(Shape* p_shape);
	void FUN_1003a859(Shape* p_shape, MechS32 p_unk0x14);
	MechU32 FUN_1003a889(Shape* p_shape);
	Shape* CreateShape(MechS32 p_vertexCount, MechS32 p_faceCount, MechS32 p_extra, void** p_extraData);
	void AddShapeVertex(
		Shape* p_shape,
		MechS32 p_x,
		MechS32 p_y,
		MechS32 p_z,
		undefined4 p_unk0x18,
		undefined4 p_unk0x1c
	);
	struct Face* AddShapeFace(Shape* p_shape, MechU16 p_unk0x00, MechU8* p_indices);
	void AddShapeFaceIndex(Shape* p_shape, struct Face* p_face, MechU32 p_index);
	void FreeModel(Model* p_model);
	void RemoveSelectedModel(Shape* p_shape);
	void FreeShape(Shape* p_shape);
	void FUN_1003acbe(Shape* p_shape, MechS32 p_unk0x04);
	MechU32 FUN_1003acf7(Shape* p_shape);
	void FUN_1003ad2d(Shape* p_shape, MechS32 p_unk0x02);
	void FUN_1003ad4c(Shape* p_shape, MechU16 p_unk0x16);
	void FUN_1003ad62(Shape* p_shape, MechU16 p_unk0x14);
	MechU32 FUN_1003ad78(Shape* p_shape);
	MechU32 FUN_1003ad93(Shape* p_shape);
	MechU32 FUN_1003adae(Shape* p_shape);
	MechS32 FUN_1003adc9(Shape* p_shape, MechS32* p_unk0x34, MechS32* p_unk0x38, MechS32* p_unk0x3c);
	void ComputeNormalsAndBounds(Shape* p_shape);
	void ComputeShapeBounds(Shape* p_shape);
	void ComputeFaceNormal(struct Face* p_face, struct Vertex* p_vertices);
	void GetModelCounts(Shape* p_shape, MechS32* p_vertexCount, MechS32* p_faceCount);
	void ForEachShape(Shape* p_shape, void (*p_fn)(Shape*));
	void GetTransformedVertex(Shape* p_shape, MechS32 p_index, MechS32* p_x, MechS32* p_y, MechS32* p_z);
	void GetVertexPosition(Shape* p_shape, MechS32 p_index, MechS32* p_x, MechS32* p_y, MechS32* p_z);
	void FUN_1003b5d6(
		Shape* p_shape,
		MechS32 p_index,
		MechU32* p_unk0x00,
		MechU32* p_count,
		MechU32* p_indices,
		MechS32 p_max
	);
	void SetFaceColor(Shape* p_shape, MechS32 p_index, MechS32 p_unk0x00);
	struct SceneObject* GetShapeObject(Shape* p_shape);
	void SetShapeObject(Shape* p_shape, struct SceneObject* p_unk0x18);
	MechU32 FUN_1003b70f(Shape* p_shape);
	void FUN_1003b72f(Shape* p_shape, MechU32 p_flags);
	void DestroyShape(Shape* p_shape);
	void FUN_1003b78b(Shape* p_shape);
	MechS32 GetShapeMemorySize(Shape* p_shape);

#ifdef __cplusplus
}
#endif

#endif // SHAPE_H
