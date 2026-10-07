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

	extern MechS32 g_lodQuality;
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
	extern MechScalar g_viewFarPlane;
	extern MechScalar g_viewProjX0;
	extern MechScalar g_viewProjX1;
	extern MechScalar g_viewProjX2;
	extern MechScalar g_viewProjY0;
	extern MechScalar g_viewProjY1;
	extern MechScalar g_viewProjY2;
	extern MechScalar g_viewProjZ0;
	extern MechScalar g_viewProjZ1;
	extern MechScalar g_viewProjZ2;
	extern MechScalar g_viewRotX0;
	extern MechScalar g_viewRotX1;
	extern MechScalar g_viewRotX2;
	extern MechScalar g_viewRotY0;
	extern MechScalar g_viewRotY1;
	extern MechScalar g_viewRotY2;
	extern MechScalar g_viewRotZ0;
	extern MechScalar g_viewRotZ1;
	extern MechScalar g_viewRotZ2;
	extern MechScalar g_viewEyeY;
	extern MechScalar g_viewEyeX;
	extern MechScalar g_viewEyeZ;
	extern MechScalar g_viewLightZ;
	extern MechScalar g_viewLightX;
	extern MechScalar g_viewLightY;
	extern MechScalar g_viewNearPlane;

	void SelectEyepoint(Eyepoint* p_eyepoint);
	void UpdateProjection(Eyepoint* p_eyepoint);
	void SetNearPlane(Eyepoint* p_eyepoint, MechScalar p_value);
	void SetFarPlane(Eyepoint* p_eyepoint, MechScalar p_value);
	void UpdateViewMatrix(Eyepoint* p_eyepoint);
	void ResetEyepointView(Eyepoint* p_eyepoint);
	void SetEyepointTransform(Eyepoint* p_eyepoint, Matrix* p_matrix);
	void GetEyepointTransform(Eyepoint* p_eyepoint, Matrix* p_matrix);
	MechS32 ProjectWorldPoint(MechScalar* p_x, MechScalar* p_y, MechScalar* p_z);
	MechS32 CullSceneShape(struct Shape* p_shape);
	MechS32 CullShapeToFrustum(struct Shape* p_shape);
	MechS32 CullHiddenShape(MechU16* p_flags);
	MechS32 IsLodQualityHigh(undefined4 p_unk0x00);
	void SetLodQualityHigh(undefined4 p_unk0x00, MechS32 p_enable);

#ifdef __cplusplus
}
#endif

#endif // VIEW_H
