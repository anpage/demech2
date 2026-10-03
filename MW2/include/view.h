#ifndef VIEW_H
#define VIEW_H

#include "decomp.h"
#include "eyepoint.h"
#include "types.h"

struct Shape;

// The functions and globals of view.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100a712c;
	extern MechS32 g_viewNear;
	extern MechS32 g_viewShiftX;
	extern MechS32 g_viewShiftY;
	extern MechS32 g_viewFar;
	extern MechS32 g_viewLeft;
	extern MechS32 g_viewCenterX;
	extern MechS32 g_viewBottom;
	extern MechS32 g_viewRight;
	extern MechS32 g_viewTop;
	extern MechS32 g_viewCenterY;
	extern MechS32 g_viewFarPlane;
	extern MechS32 g_viewProjX0;
	extern MechS32 g_viewProjX1;
	extern MechS32 g_viewProjX2;
	extern MechS32 g_viewProjY0;
	extern MechS32 g_viewProjY1;
	extern MechS32 g_viewProjY2;
	extern MechS32 g_viewProjZ0;
	extern MechS32 g_viewProjZ1;
	extern MechS32 g_viewProjZ2;
	extern MechS32 g_viewRotX0;
	extern MechS32 g_viewRotX1;
	extern MechS32 g_viewRotX2;
	extern MechS32 g_viewRotY0;
	extern MechS32 g_viewRotY1;
	extern MechS32 g_viewRotY2;
	extern MechS32 g_viewRotZ0;
	extern MechS32 g_viewRotZ1;
	extern MechS32 g_viewRotZ2;
	extern MechS32 g_viewEyeY;
	extern MechS32 g_viewEyeX;
	extern MechS32 g_viewEyeZ;
	extern MechS32 g_viewLightZ;
	extern MechS32 g_viewLightX;
	extern MechS32 g_viewLightY;
	extern MechS32 g_viewNearPlane;

	void FUN_1004b980(Eyepoint* p_eyepoint);
	void FUN_1004bc2e(Eyepoint* p_eyepoint);
	void FUN_1004bf61(Eyepoint* p_eyepoint, MechS32 p_value);
	void FUN_1004bf8a(Eyepoint* p_eyepoint, MechS32 p_value);
	void FUN_1004bfe8(Eyepoint* p_eyepoint);
	void FUN_1004c05c(Eyepoint* p_eyepoint);
	void FUN_1004c093(Eyepoint* p_eyepoint, Matrix* p_matrix);
	void FUN_1004c0d8(Eyepoint* p_eyepoint, Matrix* p_matrix);
	MechS32 FUN_1004c11d(MechS32* p_x, MechS32* p_y, MechS32* p_z);
	MechS32 FUN_1004c2ef(struct Shape* p_shape);
	MechS32 FUN_1004c565(struct Shape* p_shape);
	MechS32 FUN_1004c779(MechU16* p_flags);
	MechS32 FUN_1004c7a6(undefined4 p_unk0x00);
	void FUN_1004c7cf(undefined4 p_unk0x00, MechS32 p_enable);

#ifdef __cplusplus
}
#endif

#endif // VIEW_H
