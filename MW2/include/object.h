#ifndef OBJECT_H
#define OBJECT_H

#include "decomp.h"
#include "shape.h"
#include "transform.h"
#include "types.h"

typedef struct SceneObject SceneObject;

typedef void (*ShapeCallback)(Shape* p_shape);
typedef void (*ObjectCallback)(SceneObject* p_obj);

/* A node of the scene tree: its transform relative to the parent, the world transform
   FUN_10001c3f derives from it, and an optional shape. FUN_100012d0 allocates it. */
// SIZE 0x7c
struct SceneObject {
	SceneObject* m_parent;      // 0x00
	SceneObject* m_firstChild;  // 0x04
	SceneObject* m_nextSibling; // 0x08
	Matrix m_local;             // 0x0c
	Matrix m_world;             // 0x3c
	Shape* m_unk0x6c;           // 0x6c
	MechChar* m_name;           // 0x70
	MechU32 m_flags;            // 0x74
	MechU32 m_unk0x78;          // 0x78
};

enum {
	c_objectDirty = 0x01,  // m_world is out of date
	c_objectPooled = 0x02, // allocated from the static pool
	c_objectHeap = 0x04,   // allocated from the primary heap, freed by FUN_10001f82
};

// The functions and globals of object.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	SceneObject* FUN_100012d0(SceneObject* p_parent, MechU32 p_flags);
	SceneObject* FUN_1000145a(SceneObject* p_parent, void* p_memory);
	void FUN_10001532(SceneObject* p_obj, Shape* p_unk0x6c);
	Shape* FUN_1000154d(SceneObject* p_obj);
	MechChar* FUN_10001563(SceneObject* p_obj);
	void FUN_10001579(SceneObject* p_obj, MechChar* p_name);
	void GetObjWorldPos(SceneObject* p_obj, undefined4* p_unk0x04, undefined4* p_unk0x08, undefined4* p_unk0x0c);
	void FUN_100015bc(SceneObject* p_obj, undefined4* p_unk0x04, undefined4* p_unk0x08, undefined4* p_unk0x0c);
	void GetObjPosition(SceneObject* p_obj, MechS32* p_x, MechS32* p_y, MechS32* p_z);
	void FUN_1000160e(SceneObject* p_obj, MechS32* p_x, MechS32* p_y, MechS32* p_z);
	void SetObjPosition(SceneObject* p_obj, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	void FUN_10001667(SceneObject* p_obj, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	void FUN_10001694(SceneObject* p_obj, Matrix* p_matrix);
	void FUN_100016b6(SceneObject* p_obj, Matrix* p_matrix);
	void FUN_10001722(SceneObject* p_obj, Matrix* p_matrix);
	void FUN_1000179f(SceneObject* p_obj, Matrix* p_matrix);
	void SetObjRotation(SceneObject* p_obj, MechS32 p_unk0x04, MechS32 p_unk0x08, MechS32 p_unk0x0c, MechU32 p_unk0x10);
	void FUN_1000184b(SceneObject* p_obj, MechS32 p_unk0x04, MechS32 p_unk0x08, MechS32 p_unk0x0c, MechU32 p_unk0x10);
	void FUN_1000188b(Shape* p_shape);
	void FUN_100018ca(SceneObject* p_obj);
	void FUN_10001926(SceneObject* p_obj);
	void FUN_1000199a(SceneObject* p_obj);
	void FUN_100019f6(SceneObject* p_obj);
	void FUN_10001a52(SceneObject* p_obj);
	void SetObjTreeFlag(SceneObject* p_obj, MechS32 p_flag);
	void FUN_10001b0c(SceneObject* p_obj, MechS32 p_unk0x14);
	void FUN_10001b6a(SceneObject* p_obj, MechS32 p_unk0x24);
	void FUN_10001bce(SceneObject* p_obj, MechS32 p_flags);
	void FUN_10001c3f(SceneObject* p_obj);
	void FUN_10001cf8(SceneObject* p_obj);
	SceneObject* FUN_10001d63(SceneObject* p_obj);
	SceneObject* FUN_10001d8f(SceneObject* p_obj);
	SceneObject* FUN_10001da4(SceneObject* p_obj);
	SceneObject* FUN_10001dba(SceneObject* p_obj);
	Matrix* FUN_10001dd0(SceneObject* p_obj);
	void FUN_10001de6(SceneObject* p_obj, Matrix* p_matrix);
	Matrix* FUN_10001e01(SceneObject* p_obj);
	void FUN_10001e17(SceneObject* p_obj, Matrix* p_matrix);
	void FUN_10001e32(SceneObject* p_obj);
	void FUN_10001ef8(SceneObject* p_obj, SceneObject* p_parent);
	void FUN_10001f82(SceneObject* p_obj, ShapeCallback p_callback);
	SceneObject* FUN_10002016(SceneObject* p_obj, const MechChar* p_name);
	SceneObject* FUN_100020a6(SceneObject* p_obj);
	SceneObject* FUN_100020fa(SceneObject* p_obj, MechS32 p_index);
	SceneObject* FUN_100021b6(SceneObject* p_obj, MechU32 p_unk0x16);
	void FUN_10002246(SceneObject* p_obj, MechS32 p_unk0x04, MechU32 p_unk0x16);
	void FUN_10002314(SceneObject* p_obj, MechU32 p_unk0x16);
	MechS32 FUN_100023a8(void);

#ifdef __cplusplus
}
#endif

#endif // OBJECT_H
