#include "object.h"

#include "classtable.h"
#include "clock.h"
#include "collision.h"
#include "debris.h"
#include "decomp.h"
#include "poolsizes.h"
#include "shape.h"
#include "shapegeom.h"
#include "shapelists.h"
#include "simmain.h"
#include "staticmem.h"
#include "transform.h"
#include "types.h"

#include <mbstring.h>
#include <string.h>
#include <windows.h>

#ifdef MW2_POWERVR
#include "accel/a3d.h"
#endif

#ifdef MW2_POWERVR
DECOMP_SIZE_ASSERT(SceneObject, 0x8c)
#else
DECOMP_SIZE_ASSERT(SceneObject, 0x7c)
#endif

// The countdown the next object starts with, stepped through 64 to 191 so that objects
// renormalize on different frames.
// GLOBAL: MW2 0x100a00c8
// GLOBAL: MW2GBL 0x1009ee9c
// GLOBAL: MW2PREMERCS 0x100b0cf0
// GLOBAL: MW2MATROX 0x100a7cb4
// GLOBAL: MW23DFX 0x1024b3a0
// GLOBAL: MW2ATI 0x1017f720
// GLOBAL: MW2S3 0x10060fac
// GLOBAL: MW2POWERVR 0x101aa4c4
MechU32 g_nextRenormalizeCountdown = 100;

// FUNCTION: MW2 0x100012d0
// FUNCTION: MW2GBL 0x10019200
// FUNCTION: MW2PREMERCS 0x10032a40
// FUNCTION: MW2MATROX 0x10040dd0
// FUNCTION: MW23DFX 0x1000f5e0
// FUNCTION: MW2ATI 0x1000fcb0
// FUNCTION: MW2S3 0x10004a00
// FUNCTION: MW2POWERVR 0x1000e8e0
SceneObject* CreateObj(SceneObject* p_parent, MechU32 p_flags)
{
	SceneObject* obj;
	SceneObject* last;

	if (p_flags & 2) {
		obj = StaticPoolAlloc(sizeof(SceneObject), g_staticPoolTags[2]);
		if (obj == NULL) {
			return NULL;
		}

		obj->m_flags = c_objectPooled;
	}
	else if (p_flags & 4) {
		obj = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(SceneObject));
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
			last = GetObjLastChild(p_parent);
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

	SetIdentityMatrix(&obj->m_local);

	if (p_parent) {
#ifdef MW2_STRUCT_COPY_MEMCPY
		memcpy(&obj->m_world, &p_parent->m_world, sizeof(Matrix));
#else
		obj->m_world = p_parent->m_world;
#endif
	}
	else {
		SetIdentityMatrix(&obj->m_world);
	}

	obj->m_shape = NULL;
	obj->m_flags |= c_objectDirty;
	obj->m_name = NULL;
	obj->m_renormalizeCountdown = g_nextRenormalizeCountdown;
	g_nextRenormalizeCountdown = ((g_nextRenormalizeCountdown + 7) & 0x7f) + 0x40;
#ifdef MW2_POWERVR
	obj->m_list = -1;
	obj->m_translation = -1;
	obj->m_rotation = -1;
	obj->m_material = -1;
#endif
	return obj;
}

// FUNCTION: MW2 0x1000145a
// FUNCTION: MW2GBL 0x1001938f
// FUNCTION: MW2PREMERCS 0x10032bcf
// FUNCTION: MW2MATROX 0x10040f5a
// FUNCTION: MW23DFX 0x1000f6d0
// FUNCTION: MW2ATI 0x1000fda0
// FUNCTION: MW2S3 0x10004af0
// FUNCTION: MW2POWERVR 0x1000e9f0
SceneObject* InitObj(SceneObject* p_parent, void* p_memory)
{
	SceneObject* obj = p_memory;

	obj->m_parent = p_parent;
	obj->m_firstChild = NULL;

	if (p_parent) {
		obj->m_nextSibling = p_parent->m_firstChild;
		p_parent->m_firstChild = obj;
	}
	else {
		obj->m_nextSibling = NULL;
	}

	SetIdentityMatrix(&obj->m_local);

	if (p_parent) {
#ifdef MW2_STRUCT_COPY_MEMCPY
		memcpy(&obj->m_world, &p_parent->m_world, sizeof(Matrix));
#else
		obj->m_world = p_parent->m_world;
#endif
	}
	else {
		SetIdentityMatrix(&obj->m_world);
	}

	obj->m_shape = NULL;
	obj->m_flags = c_objectDirty;
	obj->m_name = NULL;
	obj->m_renormalizeCountdown = g_nextRenormalizeCountdown;
	g_nextRenormalizeCountdown = ((g_nextRenormalizeCountdown + 7) & 0x7f) + 0x40;
#ifdef MW2_POWERVR
	obj->m_list = -1;
	obj->m_translation = -1;
	obj->m_rotation = -1;
	obj->m_material = -1;
#endif
	return obj;
}

// The PowerVR edition also moves the shape's SGL list from the old shape's to the object's list.
// FUNCTION: MW2 0x10001532
// FUNCTION: MW2GBL 0x1001946c
// FUNCTION: MW2PREMERCS 0x10032cac
// FUNCTION: MW2MATROX 0x10041032
// FUNCTION: MW23DFX 0x1000f750
// FUNCTION: MW2ATI 0x1000fe20
// FUNCTION: MW2S3 0x10004b70
// FUNCTION: MW2POWERVR 0x1000ea80
void SetObjShape(SceneObject* p_obj, Shape* p_shape)
{
#ifdef MW2_POWERVR
	if (p_obj->m_list > 0) {
		if (p_obj->m_shape && p_obj->m_shape->m_list > 0) {
			FUN_1005f390(p_obj->m_shape->m_list);
		}

		if (p_shape && p_shape->m_list > 0) {
			FUN_1005f320(p_shape->m_list, p_obj->m_list);
		}
	}

#endif
	p_obj->m_shape = p_shape;
	p_obj->m_flags |= c_objectDirty;
}

// FUNCTION: MW2 0x1000154d
// FUNCTION: MW2GBL 0x10019487
// FUNCTION: MW2PREMERCS 0x10032cc7
// FUNCTION: MW2MATROX 0x1004104d
// FUNCTION: MW23DFX 0x1000f760
// FUNCTION: MW2ATI 0x1000fe30
// FUNCTION: MW2S3 0x10004b80
// FUNCTION: MW2POWERVR 0x1000ead0
Shape* GetObjShape(SceneObject* p_obj)
{
	return p_obj->m_shape;
}

// FUNCTION: MW2 0x10001563
// FUNCTION: MW2GBL 0x1001949d
// FUNCTION: MW2PREMERCS 0x10032cdd
// FUNCTION: MW2MATROX 0x10041063
// FUNCTION: MW23DFX 0x1000f770
MechChar* GetObjName(SceneObject* p_obj)
{
	return p_obj->m_name;
}

// FUNCTION: MW2 0x10001579
// FUNCTION: MW2GBL 0x100194b3
// FUNCTION: MW2PREMERCS 0x10032cf3
// FUNCTION: MW2MATROX 0x10041079
// FUNCTION: MW23DFX 0x1000f780
void SetObjName(SceneObject* p_obj, MechChar* p_name)
{
	p_obj->m_name = (MechChar*) _mbsdup((unsigned char*) p_name);
}

// FUNCTION: MW2 0x10001596
// FUNCTION: MW2GBL 0x100194d0
// FUNCTION: MW2PREMERCS 0x10032d10
// FUNCTION: MW2MATROX 0x10041096
// FUNCTION: MW23DFX 0x1000f7a0
// FUNCTION: MW2ATI 0x1000fe40
// FUNCTION: MW2S3 0x10004b90
// FUNCTION: MW2POWERVR 0x1000eae0
void GetObjWorldAngles(SceneObject* p_obj, MechScalar* p_angleX, MechScalar* p_angleY, MechScalar* p_angleZ)
{
	GetMatrixAngles(&p_obj->m_world, p_angleX, p_angleY, p_angleZ);
}

// FUNCTION: MW2 0x100015bc
// FUNCTION: MW2GBL 0x100194f6
// FUNCTION: MW2PREMERCS 0x10032d36
// FUNCTION: MW2MATROX 0x100410bc
// FUNCTION: MW23DFX 0x1000f7c0
// FUNCTION: MW2ATI 0x1000fe60
// FUNCTION: MW2S3 0x10004bb0
// FUNCTION: MW2POWERVR 0x1000eb00
void GetObjAngles(SceneObject* p_obj, MechScalar* p_angleX, MechScalar* p_angleY, MechScalar* p_angleZ)
{
	GetMatrixAngles(&p_obj->m_local, p_angleX, p_angleY, p_angleZ);
}

// FUNCTION: MW2 0x100015e2
// FUNCTION: MW2GBL 0x1001951c
// FUNCTION: MW2PREMERCS 0x10032d5c
// FUNCTION: MW2MATROX 0x100410e2
// FUNCTION: MW23DFX 0x1000f7e0
// FUNCTION: MW2ATI 0x1000fe80
// FUNCTION: MW2S3 0x10004bd0
// FUNCTION: MW2POWERVR 0x1000eb20
void GetObjPosition(SceneObject* p_obj, MechScalar* p_x, MechScalar* p_y, MechScalar* p_z)
{
	*p_x = p_obj->m_world.m_rows[3][0];
	*p_y = p_obj->m_world.m_rows[3][1];
	*p_z = p_obj->m_world.m_rows[3][2];
}

// FUNCTION: MW2 0x1000160e
// FUNCTION: MW2GBL 0x10019548
// FUNCTION: MW2PREMERCS 0x10032d88
// FUNCTION: MW2MATROX 0x1004110e
// FUNCTION: MW23DFX 0x1000f800
// FUNCTION: MW2ATI 0x1000fea0
// FUNCTION: MW2S3 0x10004bf0
// FUNCTION: MW2POWERVR 0x1000eb40
void GetObjLocalPosition(SceneObject* p_obj, MechScalar* p_x, MechScalar* p_y, MechScalar* p_z)
{
	*p_x = p_obj->m_local.m_rows[3][0];
	*p_y = p_obj->m_local.m_rows[3][1];
	*p_z = p_obj->m_local.m_rows[3][2];
}

// The PowerVR edition also sets the object's translation transform.
// FUNCTION: MW2 0x1000163a
// FUNCTION: MW2GBL 0x10019574
// FUNCTION: MW2PREMERCS 0x10032db4
// FUNCTION: MW2MATROX 0x1004113a
// FUNCTION: MW23DFX 0x1000f820
// FUNCTION: MW2ATI 0x1000fec0
// FUNCTION: MW2S3 0x10004c10
// FUNCTION: MW2POWERVR 0x1000eb60
void SetObjPosition(SceneObject* p_obj, MechScalar p_x, MechScalar p_y, MechScalar p_z)
{
	p_obj->m_local.m_rows[3][0] = p_x;
	p_obj->m_local.m_rows[3][1] = p_y;
	p_obj->m_local.m_rows[3][2] = p_z;
	p_obj->m_flags |= c_objectDirty;
#ifdef MW2_POWERVR
	FUN_1005fb60(p_obj->m_translation, TRUE, p_x, p_y, p_z);
#endif
}

// The PowerVR edition also moves the object's translation transform.
// FUNCTION: MW2 0x10001667
// FUNCTION: MW2GBL 0x100195a1
// FUNCTION: MW2PREMERCS 0x10032de1
// FUNCTION: MW2MATROX 0x10041167
// FUNCTION: MW23DFX 0x1000f850
// FUNCTION: MW2ATI 0x1000fef0
// FUNCTION: MW2S3 0x10004c40
// FUNCTION: MW2POWERVR 0x1000eba0
void MoveObj(SceneObject* p_obj, MechScalar p_x, MechScalar p_y, MechScalar p_z)
{
	p_obj->m_local.m_rows[3][0] += p_x;
	p_obj->m_local.m_rows[3][1] += p_y;
	p_obj->m_local.m_rows[3][2] += p_z;
	p_obj->m_flags |= c_objectDirty;
#ifdef MW2_POWERVR
	FUN_1005fb60(p_obj->m_translation, FALSE, p_x, p_y, p_z);
#endif
}

// The PowerVR edition also sets the object's translation and rotation transforms.
// FUNCTION: MW2 0x10001694
// FUNCTION: MW2GBL 0x100195ce
// FUNCTION: MW2PREMERCS 0x10032e0e
// FUNCTION: MW2MATROX 0x100411a6
// FUNCTION: MW23DFX 0x1000f880
// FUNCTION: MW2ATI 0x1000ff20
// FUNCTION: MW2S3 0x10004c70
// FUNCTION: MW2POWERVR 0x1000ebf0
void SetObjTransform(SceneObject* p_obj, Matrix* p_matrix)
{
#ifdef MW2_POWERVR
	MechScalar angleX;
	MechScalar angleY;
	MechScalar angleZ;

#endif
#ifdef MW2_STRUCT_COPY_MEMCPY
	memcpy(&p_obj->m_local, p_matrix, sizeof(Matrix));
#else
	p_obj->m_local = *p_matrix;
#endif
	p_obj->m_flags |= c_objectDirty;
#ifdef MW2_POWERVR
	FUN_1005fb60(
		p_obj->m_translation,
		TRUE,
		p_obj->m_local.m_rows[3][0],
		p_obj->m_local.m_rows[3][1],
		p_obj->m_local.m_rows[3][2]
	);
	GetObjAngles(p_obj, &angleX, &angleY, &angleZ);
	FUN_1005fbc0(p_obj->m_rotation, 0, angleX, angleY, angleZ, TRUE);
#endif
}

// Multiplies the object's local matrix by p_matrix, renormalizing the rotation now and then.
// FUNCTION: MW2 0x100016b6
// FUNCTION: MW2GBL 0x100195f5
// FUNCTION: MW2PREMERCS 0x10032e35
// FUNCTION: MW2MATROX 0x100411c8
// FUNCTION: MW23DFX 0x1000f8a0
void TransformObj(SceneObject* p_obj, Matrix* p_matrix)
{
	MultiplyMatrix(&p_obj->m_local, p_matrix, &p_obj->m_local);
	p_obj->m_flags |= c_objectDirty;

	p_obj->m_renormalizeCountdown--;
	if (p_obj->m_renormalizeCountdown <= 0) {
		p_obj->m_renormalizeCountdown = g_nextRenormalizeCountdown;
		g_nextRenormalizeCountdown = ((g_nextRenormalizeCountdown + 7) & 0x7f) + 0x40;
		NormalizeRotation(&p_obj->m_local);
	}
}

// The PowerVR edition also sets the object's rotation transform.
// FUNCTION: MW2 0x10001722
// FUNCTION: MW2GBL 0x10019661
// FUNCTION: MW2PREMERCS 0x10032ea1
// FUNCTION: MW2MATROX 0x10041234
// FUNCTION: MW23DFX 0x1000f8f0
// FUNCTION: MW2ATI 0x1000ff40
// FUNCTION: MW2S3 0x10004c90
// FUNCTION: MW2POWERVR 0x1000ec70
void SetObjRotationMatrix(SceneObject* p_obj, Matrix* p_matrix)
{
#ifdef MW2_POWERVR
	MechScalar angleX;
	MechScalar angleY;
	MechScalar angleZ;

#endif
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
#ifdef MW2_POWERVR
	GetObjAngles(p_obj, &angleX, &angleY, &angleZ);
	FUN_1005fbc0(p_obj->m_rotation, 0, angleX, angleY, angleZ, TRUE);
#endif
}

// In the PowerVR edition the caller turns the rotation transform (RotateObj), and a renormalized
// rotation sets it.
// FUNCTION: MW2 0x1000179f
// FUNCTION: MW2GBL 0x100196de
// FUNCTION: MW2PREMERCS 0x10032f1e
// FUNCTION: MW2MATROX 0x100412b1
// FUNCTION: MW23DFX 0x1000f940
// FUNCTION: MW2ATI 0x1000ff90
// FUNCTION: MW2S3 0x10004ce0
// FUNCTION: MW2POWERVR 0x1000ecf0
void RotateObjMatrix(SceneObject* p_obj, Matrix* p_matrix)
{
#ifdef MW2_POWERVR
	MechScalar angleX;
	MechScalar angleY;
	MechScalar angleZ;

#endif
	MultiplyRotations(&p_obj->m_local, p_matrix, &p_obj->m_local);
	p_obj->m_flags |= c_objectDirty;

	p_obj->m_renormalizeCountdown--;
	if (p_obj->m_renormalizeCountdown <= 0) {
		p_obj->m_renormalizeCountdown = g_nextRenormalizeCountdown;
		g_nextRenormalizeCountdown = ((g_nextRenormalizeCountdown + 7) & 0x7f) + 0x40;
		NormalizeRotation(&p_obj->m_local);
#ifdef MW2_POWERVR
		GetObjAngles(p_obj, &angleX, &angleY, &angleZ);
		FUN_1005fbc0(p_obj->m_rotation, 0, angleX, angleY, angleZ, TRUE);
#endif
	}
}

// The PowerVR edition copies the rotation itself, and sets the object's rotation transform from the
// angles.
// FUNCTION: MW2 0x1000180b
// FUNCTION: MW2GBL 0x1001974a
// FUNCTION: MW2PREMERCS 0x100330a7
// FUNCTION: MW2MATROX 0x1004131d
// FUNCTION: MW23DFX 0x1000f990
// FUNCTION: MW2ATI 0x1000ffe0
// FUNCTION: MW2S3 0x10004d30
// FUNCTION: MW2POWERVR 0x1000ed70
void SetObjRotation(SceneObject* p_obj, MechScalar p_angleX, MechScalar p_angleY, MechScalar p_angleZ, MechU32 p_flags)
{
	Matrix matrix;

	BuildMatrixEx(&matrix, p_angleX, p_angleY, p_angleZ, 0, 0, 0, p_flags);
#ifdef MW2_POWERVR
	p_obj->m_local.m_rows[0][0] = matrix.m_rows[0][0];
	p_obj->m_local.m_rows[0][1] = matrix.m_rows[0][1];
	p_obj->m_local.m_rows[0][2] = matrix.m_rows[0][2];
	p_obj->m_local.m_rows[1][0] = matrix.m_rows[1][0];
	p_obj->m_local.m_rows[1][1] = matrix.m_rows[1][1];
	p_obj->m_local.m_rows[1][2] = matrix.m_rows[1][2];
	p_obj->m_local.m_rows[2][0] = matrix.m_rows[2][0];
	p_obj->m_local.m_rows[2][1] = matrix.m_rows[2][1];
	p_obj->m_local.m_rows[2][2] = matrix.m_rows[2][2];
	p_obj->m_flags |= c_objectDirty;
	FUN_1005fbc0(p_obj->m_rotation, p_flags, p_angleX, p_angleY, p_angleZ, TRUE);
#else
	SetObjRotationMatrix(p_obj, &matrix);
#endif
}

// The PowerVR edition turns the object's rotation transform first.
// FUNCTION: MW2 0x1000184b
// FUNCTION: MW2GBL 0x1001978a
// FUNCTION: MW2PREMERCS 0x100330e7
// FUNCTION: MW2MATROX 0x1004135d
// FUNCTION: MW23DFX 0x1000f9d0
// FUNCTION: MW2ATI 0x10010020
// FUNCTION: MW2S3 0x10004d70
// FUNCTION: MW2POWERVR 0x1000ee10
void RotateObj(SceneObject* p_obj, MechScalar p_angleX, MechScalar p_angleY, MechScalar p_angleZ, MechU32 p_flags)
{
	Matrix matrix;

#ifdef MW2_POWERVR
	FUN_1005fbc0(p_obj->m_rotation, p_flags, p_angleX, p_angleY, p_angleZ, FALSE);
#endif
	BuildMatrixEx(&matrix, p_angleX, p_angleY, p_angleZ, 0, 0, 0, p_flags);
	RotateObjMatrix(p_obj, &matrix);
}

// FUNCTION: MW2 0x1000188b
// FUNCTION: MW2GBL 0x100197ca
// FUNCTION: MW2PREMERCS 0x10033127
// FUNCTION: MW2MATROX 0x1004139d
// FUNCTION: MW23DFX 0x1000fa10
// FUNCTION: MW2ATI 0x10010060
// FUNCTION: MW2S3 0x10004db0
// FUNCTION: MW2POWERVR 0x1000ee70
void TransformShapeModel(Shape* p_shape)
{
	Model* model = p_shape->m_model;
	SceneObject* obj = p_shape->m_object;

	TransformModel(model, &obj->m_world);
	model->m_transformCount = p_shape->m_transformCount;
}

// FUNCTION: MW2 0x100018ca
// FUNCTION: MW2GBL 0x10019809
// FUNCTION: MW2PREMERCS 0x10033166
// FUNCTION: MW2MATROX 0x100413dc
// FUNCTION: MW23DFX 0x1000fa30
// FUNCTION: MW2ATI 0x10010080
// FUNCTION: MW2S3 0x10004dd0
// FUNCTION: MW2POWERVR 0x1000ee90
void HideObjTree(SceneObject* p_obj)
{
	SceneObject* child;

	if (p_obj->m_shape) {
		HideShape(p_obj->m_shape);
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		HideObjTree(child);
	}
}

// FUNCTION: MW2 0x10001926
// FUNCTION: MW2GBL 0x10019865
// FUNCTION: MW2PREMERCS 0x100331c2
// FUNCTION: MW2MATROX 0x10041438
// FUNCTION: MW23DFX 0x1000fa60
// FUNCTION: MW2ATI 0x100100b0
// FUNCTION: MW2S3 0x10004e00
// FUNCTION: MW2POWERVR 0x1000eec0
void ShowObjTree(SceneObject* p_obj)
{
	SceneObject* child;

	if (p_obj->m_shape && (p_obj->m_shape->m_kind & 0xf0) != 0x70) {
		ShowShape(p_obj->m_shape);
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		ShowObjTree(child);
	}
}

// FUNCTION: MW2 0x1000199a
// FUNCTION: MW2GBL 0x100198d9
// FUNCTION: MW2PREMERCS 0x10033236
// FUNCTION: MW2MATROX 0x100414ac
// FUNCTION: MW23DFX 0x1000fa90
// FUNCTION: MW2ATI 0x100100e0
// FUNCTION: MW2S3 0x10004e30
// FUNCTION: MW2POWERVR 0x1000eef0
void DisableObjTreeCollision(SceneObject* p_obj)
{
	SceneObject* child;

	if (p_obj->m_shape) {
		DisableShapeCollision(p_obj->m_shape);
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		DisableObjTreeCollision(child);
	}
}

// FUNCTION: MW2 0x100019f6
// FUNCTION: MW2GBL 0x10019935
// FUNCTION: MW2PREMERCS 0x10033292
// FUNCTION: MW2MATROX 0x10041508
// FUNCTION: MW23DFX 0x1000fac0
// FUNCTION: MW2ATI 0x10010110
// FUNCTION: MW2S3 0x10004e60
// FUNCTION: MW2POWERVR 0x1000ef20
void EnableObjTreeCollision(SceneObject* p_obj)
{
	SceneObject* child;

	if (p_obj->m_shape) {
		EnableShapeCollision(p_obj->m_shape);
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		EnableObjTreeCollision(child);
	}
}

// FUNCTION: MW2 0x10001a52
// FUNCTION: MW2GBL 0x10019991
// FUNCTION: MW2PREMERCS 0x100332ee
// FUNCTION: MW2MATROX 0x10041564
// FUNCTION: MW23DFX 0x1000faf0
// FUNCTION: MW2ATI 0x10010140
// FUNCTION: MW2S3 0x10004e90
// FUNCTION: MW2POWERVR 0x1000ef50
void DetachObjTreeShapes(SceneObject* p_obj)
{
	SceneObject* child;

	if (p_obj->m_shape) {
		DetachShape(p_obj->m_shape);
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		DetachObjTreeShapes(child);
	}
}

// FUNCTION: MW2 0x10001aae
// FUNCTION: MW2GBL 0x100199ed
// FUNCTION: MW2PREMERCS 0x1003334a
// FUNCTION: MW2MATROX 0x100415c0
// FUNCTION: MW23DFX 0x1000fb20
// FUNCTION: MW2ATI 0x10010170
// FUNCTION: MW2S3 0x10004ec0
// FUNCTION: MW2POWERVR 0x1000ef80
void SetObjTreeKind(SceneObject* p_obj, MechS32 p_kind)
{
	SceneObject* child;

	if (p_obj->m_shape) {
		p_obj->m_shape->m_kind = p_kind;
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		SetObjTreeKind(child, p_kind);
	}
}

// FUNCTION: MW2 0x10001b0c
// FUNCTION: MW2GBL 0x10019a4b
// FUNCTION: MW2PREMERCS 0x100333a8
// FUNCTION: MW2MATROX 0x1004161e
// FUNCTION: MW23DFX 0x1000fb50
// FUNCTION: MW2ATI 0x100101a0
// FUNCTION: MW2S3 0x10004ef0
// FUNCTION: MW2POWERVR 0x1000efb0
void SetObjTreeOwner(SceneObject* p_obj, MechS32 p_owner)
{
	SceneObject* child;

	if (p_obj->m_shape) {
		p_obj->m_shape->m_owner = p_owner;
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		SetObjTreeOwner(child, p_owner);
	}
}

// FUNCTION: MW2 0x10001b6a
// FUNCTION: MW2GBL 0x10019aa9
// FUNCTION: MW2PREMERCS 0x10033406
// FUNCTION: MW2MATROX 0x1004167c
// FUNCTION: MW23DFX 0x1000fb80
// FUNCTION: MW2ATI 0x100101d0
// FUNCTION: MW2S3 0x10004f20
// FUNCTION: MW2POWERVR 0x1000efe0
void SetObjTreeCollisionType(SceneObject* p_obj, MechS32 p_collisionType)
{
	SceneObject* child;

	if (p_obj->m_shape) {
		SetShapeCollisionType(p_obj->m_shape, p_collisionType);
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		SetObjTreeCollisionType(child, p_collisionType);
	}
}

// FUNCTION: MW2 0x10001bce
// FUNCTION: MW2GBL 0x10019b0d
// FUNCTION: MW2PREMERCS 0x1003346a
// FUNCTION: MW2MATROX 0x100416e0
// FUNCTION: MW23DFX 0x1000fbb0
// FUNCTION: MW2ATI 0x10010200
// FUNCTION: MW2S3 0x10004f50
// FUNCTION: MW2POWERVR 0x1000f010
void ClearObjTreeKind(SceneObject* p_obj, MechS32 p_flags)
{
	SceneObject* child;

	if (p_obj->m_shape) {
		p_obj->m_shape->m_kind &= (MechS16) ~p_flags;
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		ClearObjTreeKind(child, p_flags);
	}
}

// Recomputes the world matrices of the object and its subtree, and their shapes' centers.
// FUNCTION: MW2 0x10001c3f
// FUNCTION: MW2GBL 0x10019b7e
// FUNCTION: MW2PREMERCS 0x100334db
// FUNCTION: MW2MATROX 0x10041751
// FUNCTION: MW23DFX 0x1000fbe0
// FUNCTION: MW2ATI 0x10010230
// FUNCTION: MW2S3 0x10004f80
// FUNCTION: MW2POWERVR 0x1000f040
void UpdateObjWorld(SceneObject* p_obj)
{
	Shape* shape;
	SceneObject* child;

	p_obj->m_flags &= ~c_objectDirty;

	if (p_obj->m_parent) {
		MultiplyMatrix(&p_obj->m_parent->m_world, &p_obj->m_local, &p_obj->m_world);
	}
	else {
#ifdef MW2_STRUCT_COPY_MEMCPY
		memcpy(&p_obj->m_world, &p_obj->m_local, sizeof(Matrix));
#else
		p_obj->m_world = p_obj->m_local;
#endif
	}

	shape = p_obj->m_shape;
	if (shape) {
		TransformShapeCenter(shape, &p_obj->m_world);
		shape->m_object = p_obj;
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		UpdateObjWorld(child);
	}
}

// FUNCTION: MW2 0x10001cf8
// FUNCTION: MW2GBL 0x10019c3c
// FUNCTION: MW2PREMERCS 0x10033599
// FUNCTION: MW2MATROX 0x1004180a
// FUNCTION: MW23DFX 0x1000fc40
// FUNCTION: MW2ATI 0x10010290
// FUNCTION: MW2S3 0x10004fe0
// FUNCTION: MW2POWERVR 0x1000f0a0
#ifdef MW2_SHAPE_GRID
void UpdateObjTree(SceneObject* p_obj)
#else
void UpdateObj(SceneObject* p_obj)
#endif
{
	SceneObject* child;

	if (p_obj->m_flags & c_objectDirty) {
		UpdateObjWorld(p_obj);
	}
	else if (p_obj->m_firstChild) {
		for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
#ifdef MW2_SHAPE_GRID
			UpdateObjTree(child);
#else
			UpdateObj(child);
#endif
		}
	}
}

#ifdef MW2_SHAPE_GRID
// The early Mercenaries build's UpdateObj: the tree updated, a root's shape is filed in the grid's
// cell it now stands in (UpdateObjTree is the others' UpdateObj).
// FUNCTION: MW2PREMERCS 0x10033604
void UpdateObj(SceneObject* p_obj)
{
	UpdateObjTree(p_obj);
	if (g_shapeGridEnabled && !p_obj->m_parent) {
		UpdateShapeGridCell(p_obj->m_shape);
	}
}
#endif

// FUNCTION: MW2 0x10001d63
// FUNCTION: MW2GBL 0x10019ca7
// FUNCTION: MW2PREMERCS 0x10033643
// FUNCTION: MW2MATROX 0x10041875
// FUNCTION: MW23DFX 0x1000fc70
SceneObject* GetObjRoot(SceneObject* p_obj)
{
	while (p_obj->m_parent) {
		p_obj = p_obj->m_parent;
	}

	return p_obj;
}

// FUNCTION: MW2 0x10001d8f
// FUNCTION: MW2GBL 0x10019cd3
// FUNCTION: MW2PREMERCS 0x1003366f
// FUNCTION: MW2MATROX 0x100418a1
// FUNCTION: MW23DFX 0x1000fc80
SceneObject* GetObjParent(SceneObject* p_obj)
{
	return p_obj->m_parent;
}

// FUNCTION: MW2 0x10001da4
// FUNCTION: MW2GBL 0x10019ce8
// FUNCTION: MW2PREMERCS 0x10033684
// FUNCTION: MW2MATROX 0x100418b6
// FUNCTION: MW23DFX 0x1000fc90
// FUNCTION: MW2ATI 0x100102c0
// FUNCTION: MW2S3 0x10005010
// FUNCTION: MW2POWERVR 0x1000f0d0
SceneObject* GetObjFirstChild(SceneObject* p_obj)
{
	return p_obj->m_firstChild;
}

// FUNCTION: MW2 0x10001dba
// FUNCTION: MW2GBL 0x10019cfe
// FUNCTION: MW2PREMERCS 0x1003369a
// FUNCTION: MW2MATROX 0x100418cc
// FUNCTION: MW23DFX 0x1000fca0
// FUNCTION: MW2ATI 0x100102d0
// FUNCTION: MW2S3 0x10005020
// FUNCTION: MW2POWERVR 0x1000f0e0
SceneObject* GetObjNextSibling(SceneObject* p_obj)
{
	return p_obj->m_nextSibling;
}

// FUNCTION: MW2 0x10001dd0
// FUNCTION: MW2GBL 0x10019d14
// FUNCTION: MW2PREMERCS 0x100336b0
// FUNCTION: MW2MATROX 0x100418e2
// FUNCTION: MW23DFX 0x1000fcb0
Matrix* GetObjLocalMatrix(SceneObject* p_obj)
{
	return &p_obj->m_local;
}

// The PowerVR edition also sets the object's translation and rotation transforms.
// FUNCTION: MW2 0x10001de6
// FUNCTION: MW2GBL 0x10019d2a
// FUNCTION: MW2PREMERCS 0x100336c6
// FUNCTION: MW2MATROX 0x100418f8
// FUNCTION: MW23DFX 0x1000fcc0
// FUNCTION: MW2ATI 0x100102e0
// FUNCTION: MW2S3 0x10005030
// FUNCTION: MW2POWERVR 0x1000f0f0
void SetObjLocalMatrix(SceneObject* p_obj, Matrix* p_matrix)
{
#ifdef MW2_POWERVR
	MechScalar angleX;
	MechScalar angleY;
	MechScalar angleZ;

#endif
#ifdef MW2_STRUCT_COPY_MEMCPY
	memcpy(&p_obj->m_local, p_matrix, sizeof(Matrix));
#else
	p_obj->m_local = *p_matrix;
#endif
#ifdef MW2_POWERVR
	FUN_1005fb60(
		p_obj->m_translation,
		TRUE,
		p_obj->m_local.m_rows[3][0],
		p_obj->m_local.m_rows[3][1],
		p_obj->m_local.m_rows[3][2]
	);
	GetObjAngles(p_obj, &angleX, &angleY, &angleZ);
	FUN_1005fbc0(p_obj->m_rotation, 0, angleX, angleY, angleZ, TRUE);
#endif
}

// FUNCTION: MW2 0x10001e01
// FUNCTION: MW2GBL 0x10019d4a
// FUNCTION: MW2PREMERCS 0x100336e6
// FUNCTION: MW2MATROX 0x10041913
// FUNCTION: MW23DFX 0x1000fce0
// FUNCTION: MW2ATI 0x10010300
// FUNCTION: MW2S3 0x10005050
// FUNCTION: MW2POWERVR 0x1000f160
Matrix* GetObjWorldMatrix(SceneObject* p_obj)
{
	return &p_obj->m_world;
}

// FUNCTION: MW2 0x10001e17
// FUNCTION: MW2GBL 0x10019d60
// FUNCTION: MW2PREMERCS 0x100336fc
// FUNCTION: MW2MATROX 0x10041929
// FUNCTION: MW23DFX 0x1000fcf0
void SetObjWorldMatrix(SceneObject* p_obj, Matrix* p_matrix)
{
#ifdef MW2_STRUCT_COPY_MEMCPY
	memcpy(&p_obj->m_world, p_matrix, sizeof(Matrix));
#else
	p_obj->m_world = *p_matrix;
#endif
}

// Unlinks the object from its parent, keeping its world placement.
// In the PowerVR edition the object's transforms take its world placement, and its list moves to the
// scene's.
// FUNCTION: MW2 0x10001e32
// FUNCTION: MW2GBL 0x10019d80
// FUNCTION: MW2MATROX 0x10041944
// FUNCTION: MW23DFX 0x1000fd10
// FUNCTION: MW2ATI 0x10010310
// FUNCTION: MW2S3 0x10005060
// FUNCTION: MW2POWERVR 0x1000f170
void DetachObj(SceneObject* p_obj)
{
	SceneObject* parent;
	SceneObject* sibling;
	SceneObject* first;

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

#ifdef MW2_POWERVR
	SetObjLocalMatrix(p_obj, &p_obj->m_world);
	FUN_1005f320(p_obj->m_list, g_a3dMainView.m_root);
#endif
	p_obj->m_parent = NULL;
	p_obj->m_nextSibling = NULL;
	memcpy(&p_obj->m_local, &p_obj->m_world, sizeof(p_obj->m_local));
	p_obj->m_flags |= c_objectDirty;
	UpdateObj(p_obj);
}

// Links the object under p_parent, keeping its world placement.
// FUNCTION: MW2 0x10001ef8
// FUNCTION: MW2GBL 0x10019e4b
// FUNCTION: MW2MATROX 0x10041a0a
// FUNCTION: MW23DFX 0x1000fd70
void AttachObj(SceneObject* p_obj, SceneObject* p_parent)
{
	Matrix inverse;

	if (p_obj->m_parent) {
		DetachObj(p_obj);
	}

	p_obj->m_parent = p_parent;
	p_obj->m_nextSibling = p_parent->m_firstChild;
	p_parent->m_firstChild = p_obj;

	InvertMatrix(&p_parent->m_world, &inverse);
	MultiplyMatrix(&inverse, &p_obj->m_world, &p_obj->m_local);
	p_obj->m_flags &= ~c_objectDirty;
	p_parent->m_flags |= c_objectDirty;
	UpdateObj(p_parent);
}

// The PowerVR edition also deletes the object's list.
// FUNCTION: MW2 0x10001f82
// FUNCTION: MW2GBL 0x10019ed5
// FUNCTION: MW2PREMERCS 0x10033871
// FUNCTION: MW2MATROX 0x10041a94
// FUNCTION: MW23DFX 0x1000fdd0
// FUNCTION: MW2ATI 0x10010370
// FUNCTION: MW2S3 0x100050c0
// FUNCTION: MW2POWERVR 0x1000f1f0
void DestroyObjTree(SceneObject* p_obj, ShapeCallback p_callback)
{
	SceneObject* child;
	SceneObject* next;

	DetachObj(p_obj);

	child = p_obj->m_firstChild;
	while (child) {
		next = child->m_nextSibling;
		DestroyObjTree(child, p_callback);
		child = next;
	}

	if (p_obj->m_shape && p_callback) {
		p_callback(p_obj->m_shape);
	}

#ifdef MW2_POWERVR
	FUN_1005f280(p_obj->m_list);
	p_obj->m_list = -1;
#endif
	if (p_obj->m_flags & c_objectHeap) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_obj);
	}
}

// FUNCTION: MW2 0x10002016
// FUNCTION: MW2GBL 0x10019f69
// FUNCTION: MW2PREMERCS 0x10033905
// FUNCTION: MW2MATROX 0x10041b28
// FUNCTION: MW23DFX 0x1000fe20
SceneObject* FindObjByName(SceneObject* p_obj, const MechChar* p_name)
{
	SceneObject* child;

	if (p_obj->m_name && _strcmpi(p_obj->m_name, p_name) == 0) {
		return p_obj;
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		p_obj = FindObjByName(child, p_name);
		if (p_obj) {
			return p_obj;
		}
	}

	return NULL;
}

// FUNCTION: MW2 0x100020a6
// FUNCTION: MW2GBL 0x10019ff9
// FUNCTION: MW2PREMERCS 0x10033995
// FUNCTION: MW2MATROX 0x10041bb8
// FUNCTION: MW23DFX 0x1000fe60
// FUNCTION: MW2ATI 0x100103c0
// FUNCTION: MW2S3 0x10005110
// FUNCTION: MW2POWERVR 0x1000f250
SceneObject* GetObjLastChild(SceneObject* p_obj)
{
	SceneObject* child;
	SceneObject* last;

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

// Returns child p_index in the order the children were added (they are prepended), or NULL.
// FUNCTION: MW2 0x100020fa
// FUNCTION: MW2GBL 0x1001a04d
// FUNCTION: MW2PREMERCS 0x100339e9
// FUNCTION: MW2MATROX 0x10041c0c
// FUNCTION: MW23DFX 0x1000fe80
// FUNCTION: MW2ATI 0x100103e0
// FUNCTION: MW2S3 0x10005130
// FUNCTION: MW2POWERVR 0x1000f270
SceneObject* GetObjChild(SceneObject* p_obj, MechS32 p_index)
{
	SceneObject* result;
	SceneObject* child;
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
// FUNCTION: MW2GBL 0x1001a109
// FUNCTION: MW2PREMERCS 0x10033aa5
// FUNCTION: MW2MATROX 0x10041cc8
// FUNCTION: MW23DFX 0x1000fec0
// FUNCTION: MW2ATI 0x10010420
// FUNCTION: MW2S3 0x10005170
// FUNCTION: MW2POWERVR 0x1000f2b0
SceneObject* FindObjByPart(SceneObject* p_obj, MechU32 p_partId)
{
	SceneObject* child;
	MechU32 id = p_partId;

	if (p_obj->m_shape && p_obj->m_shape->m_partId == id) {
		return p_obj;
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		p_obj = FindObjByPart(child, p_partId);
		if (p_obj) {
			return p_obj;
		}
	}

	return NULL;
}

// Raises the damage level (shape flags 0xf0) of mech part p_partId's shapes to p_level (1 to 15).
// FUNCTION: MW2 0x10002246
// FUNCTION: MW2GBL 0x1001a199
// FUNCTION: MW2PREMERCS 0x10033b35
// FUNCTION: MW2MATROX 0x10041d58
// FUNCTION: MW23DFX 0x1000ff00
// FUNCTION: MW2ATI 0x10010460
// FUNCTION: MW2S3 0x100051b0
// FUNCTION: MW2POWERVR 0x1000f2f0
void RaisePartDamageLevel(SceneObject* p_obj, MechS32 p_level, MechU32 p_partId)
{
	MechS32 current;
	SceneObject* child;
	Shape* shape;

	shape = p_obj->m_shape;
	current = (GetShapeState(shape) & 0xf0) >> 4;

	if (p_level > 0 && p_level < 0x10 && p_level > current) {
		p_level <<= 4;
	}
	else {
		return;
	}

	if (p_obj->m_shape && p_obj->m_shape->m_partId == p_partId) {
#ifdef MW2_POWERVR
		SetShapeDamageLevel(p_obj->m_shape, p_level);
#else
		SetShapeState(p_obj->m_shape, p_level);
#endif
	}

	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		RaisePartDamageLevel(child, p_level >> 4, p_partId);
	}
}

// Blows mech part p_partId's objects off the tree as debris.
// FUNCTION: MW2 0x10002314
// FUNCTION: MW2GBL 0x1001a267
// FUNCTION: MW2PREMERCS 0x10033d57
// FUNCTION: MW2MATROX 0x10041e26
// FUNCTION: MW23DFX 0x1000ff70
// FUNCTION: MW2ATI 0x100104d0
// FUNCTION: MW2S3 0x10005220
// FUNCTION: MW2POWERVR 0x1000f360
void BlowOffPart(SceneObject* p_obj, MechU32 p_partId)
{
	if (p_obj == NULL) {
		return;
	}

	BlowOffPart(p_obj->m_nextSibling, p_partId);

	if (p_obj->m_shape && p_obj->m_shape->m_partId == p_partId) {
		BlowOffObjTree(p_obj->m_firstChild, ReleaseObjShape, p_partId);
		BlowOffChunk(p_obj, ReleaseObjShape, p_partId);
	}
	else {
		BlowOffPart(p_obj->m_firstChild, p_partId);
	}
}

// FUNCTION: MW2 0x100023a8
// FUNCTION: MW2GBL 0x1001a2fb
// FUNCTION: MW2PREMERCS 0x10033deb
// FUNCTION: MW2MATROX 0x10041eba
// FUNCTION: MW23DFX 0x1000ffc0
// FUNCTION: MW2ATI 0x10010520
// FUNCTION: MW2S3 0x10005270
// FUNCTION: MW2POWERVR 0x1000f3b0
MechS32 GetObjSize(void)
{
	return sizeof(SceneObject);
}
