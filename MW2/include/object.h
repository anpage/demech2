#ifndef OBJECT_H
#define OBJECT_H

#include "decomp.h"
#include "transform.h"
#include "types.h"
#include "unk1003a530.h"

typedef struct AmberWillow0x7c AmberWillow0x7c;

typedef void (*ShapeCallback)(ScarletOrchid0x4c* p_shape);

/* A node of the scene tree: its transform relative to the parent, the world transform
   FUN_10001c3f derives from it, and an optional shape. FUN_100012d0 allocates it. */
// SIZE 0x7c
struct AmberWillow0x7c {
	AmberWillow0x7c* m_parent;      // 0x00
	AmberWillow0x7c* m_firstChild;  // 0x04
	AmberWillow0x7c* m_nextSibling; // 0x08
	Matrix m_local;                 // 0x0c
	Matrix m_world;                 // 0x3c
	ScarletOrchid0x4c* m_unk0x6c;   // 0x6c
	MechChar* m_name;               // 0x70
	MechU32 m_flags;                // 0x74
	MechU32 m_unk0x78;              // 0x78
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

	AmberWillow0x7c* FUN_100012d0(AmberWillow0x7c* p_parent, MechU32 p_flags);
	AmberWillow0x7c* FUN_1000145a(AmberWillow0x7c* p_parent, void* p_memory);
	void FUN_10001532(AmberWillow0x7c* p_obj, ScarletOrchid0x4c* p_unk0x6c);
	ScarletOrchid0x4c* FUN_1000154d(AmberWillow0x7c* p_obj);
	MechChar* FUN_10001563(AmberWillow0x7c* p_obj);
	void FUN_10001579(AmberWillow0x7c* p_obj, MechChar* p_name);
	void GetObjWorldPos(AmberWillow0x7c* p_obj, undefined4* p_unk0x04, undefined4* p_unk0x08, undefined4* p_unk0x0c);
	void FUN_100015bc(AmberWillow0x7c* p_obj, undefined4* p_unk0x04, undefined4* p_unk0x08, undefined4* p_unk0x0c);
	void GetObjPosition(AmberWillow0x7c* p_obj, MechS32* p_x, MechS32* p_y, MechS32* p_z);
	void FUN_1000160e(AmberWillow0x7c* p_obj, MechS32* p_x, MechS32* p_y, MechS32* p_z);
	void SetObjPosition(AmberWillow0x7c* p_obj, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	void FUN_10001667(AmberWillow0x7c* p_obj, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	void FUN_10001694(AmberWillow0x7c* p_obj, Matrix* p_matrix);
	void FUN_100016b6(AmberWillow0x7c* p_obj, Matrix* p_matrix);
	void FUN_10001722(AmberWillow0x7c* p_obj, Matrix* p_matrix);
	void FUN_1000179f(AmberWillow0x7c* p_obj, Matrix* p_matrix);
	void SetObjRotation(
		AmberWillow0x7c* p_obj,
		MechS32 p_unk0x04,
		MechS32 p_unk0x08,
		MechS32 p_unk0x0c,
		MechU32 p_unk0x10
	);
	void FUN_1000184b(
		AmberWillow0x7c* p_obj,
		MechS32 p_unk0x04,
		MechS32 p_unk0x08,
		MechS32 p_unk0x0c,
		MechU32 p_unk0x10
	);
	void FUN_1000188b(ScarletOrchid0x4c* p_shape);
	void FUN_100018ca(AmberWillow0x7c* p_obj);
	void FUN_10001926(AmberWillow0x7c* p_obj);
	void FUN_1000199a(AmberWillow0x7c* p_obj);
	void FUN_100019f6(AmberWillow0x7c* p_obj);
	void FUN_10001a52(AmberWillow0x7c* p_obj);
	void SetObjTreeFlag(AmberWillow0x7c* p_obj, MechS32 p_flag);
	void FUN_10001b0c(AmberWillow0x7c* p_obj, MechS32 p_unk0x14);
	void FUN_10001b6a(AmberWillow0x7c* p_obj, MechS32 p_unk0x24);
	void FUN_10001bce(AmberWillow0x7c* p_obj, MechS32 p_flags);
	void FUN_10001c3f(AmberWillow0x7c* p_obj);
	void FUN_10001cf8(AmberWillow0x7c* p_obj);
	AmberWillow0x7c* FUN_10001d63(AmberWillow0x7c* p_obj);
	AmberWillow0x7c* FUN_10001d8f(AmberWillow0x7c* p_obj);
	AmberWillow0x7c* FUN_10001da4(AmberWillow0x7c* p_obj);
	AmberWillow0x7c* FUN_10001dba(AmberWillow0x7c* p_obj);
	Matrix* FUN_10001dd0(AmberWillow0x7c* p_obj);
	void FUN_10001de6(AmberWillow0x7c* p_obj, Matrix* p_matrix);
	Matrix* FUN_10001e01(AmberWillow0x7c* p_obj);
	void FUN_10001e17(AmberWillow0x7c* p_obj, Matrix* p_matrix);
	void FUN_10001e32(AmberWillow0x7c* p_obj);
	void FUN_10001ef8(AmberWillow0x7c* p_obj, AmberWillow0x7c* p_parent);
	void FUN_10001f82(AmberWillow0x7c* p_obj, ShapeCallback p_callback);
	AmberWillow0x7c* FUN_10002016(AmberWillow0x7c* p_obj, const MechChar* p_name);
	AmberWillow0x7c* FUN_100020a6(AmberWillow0x7c* p_obj);
	AmberWillow0x7c* FUN_100020fa(AmberWillow0x7c* p_obj, MechS32 p_index);
	AmberWillow0x7c* FUN_100021b6(AmberWillow0x7c* p_obj, MechU32 p_unk0x16);
	void FUN_10002246(AmberWillow0x7c* p_obj, MechS32 p_unk0x04, MechU32 p_unk0x16);
	void FUN_10002314(AmberWillow0x7c* p_obj, MechU32 p_unk0x16);
	MechS32 FUN_100023a8(void);

#ifdef __cplusplus
}
#endif

#endif // OBJECT_H
