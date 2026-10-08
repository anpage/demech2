#include "view.h"

#include "approxlen.h"
#include "clock.h"
#include "decomp.h"
#include "depthsort.h"
#include "eyepoint.h"
#include "faceshade.h"
#include "fixeddot27.h"
#include "fixeddot29.h"
#include "fixedmul.h"
#include "fixedmul29.h"
#include "inradius.h"
#include "muladddiv.h"
#include "muldiv.h"
#include "mulnorm16.h"
#include "mulratio.h"
#include "objectanim.h"
#include "polydraw.h"
#include "shape.h"
#include "shiftdiv.h"
#include "transform.h"
#include "types.h"

#include <windows.h>
#ifdef MW2_MATROX
#include <stdio.h>
#endif

// The level of detail: 1 high, 2 low (TOGGLE_LOD_QUALITY); it divides Eyepoint::m_detailScale.
// GLOBAL: MW2 0x100a712c
// GLOBAL: MW2MATROX 0x100a5968
MechS32 g_lodQuality = 1;

#ifdef MW2_MATROX
// The Matrox edition's: a further scale of Eyepoint::m_detailScale, which UpdateProjection reads once
// from LOD.PAR's LOD_SCALE line.
// GLOBAL: MW2MATROX 0x100a596c
MechFloat g_lodScale = 1.0f;

// GLOBAL: MW2MATROX 0x100a5970
MechS32 g_lodScaleLoaded = 0;
#endif

// GLOBAL: MW2 0x100ea820
MechS32 g_viewNear;

// GLOBAL: MW2 0x100ea824
MechS32 g_viewShiftX;

// GLOBAL: MW2 0x100ea828
MechS32 g_viewShiftY;

// GLOBAL: MW2 0x100ea82c
MechS32 g_viewFar;

// GLOBAL: MW2 0x100ea830
// GLOBAL: MW2MATROX 0x101d68e4
MechS32 g_viewLeft;

// GLOBAL: MW2 0x100ea834
// GLOBAL: MW2MATROX 0x101d684c
MechS32 g_viewCenterX;

// GLOBAL: MW2 0x100ea838
// GLOBAL: MW2MATROX 0x101d6850
MechS32 g_viewHalfHeight;

// GLOBAL: MW2 0x100ea83c
// GLOBAL: MW2MATROX 0x101d6854
MechS32 g_viewHalfWidth;

// GLOBAL: MW2 0x100ea840
// GLOBAL: MW2MATROX 0x101d6864
MechS32 g_viewBottom;

// GLOBAL: MW2 0x100ea844
// GLOBAL: MW2MATROX 0x101d685c
MechScalar g_viewProjectScaleX;

// GLOBAL: MW2 0x100ea848
// GLOBAL: MW2MATROX 0x101d6860
MechScalar g_viewProjectScaleY;

// GLOBAL: MW2 0x100ea84c
// GLOBAL: MW2MATROX 0x101d68a4
MechS32 g_viewRight;

// GLOBAL: MW2 0x100ea850
// GLOBAL: MW2MATROX 0x101d6890
MechS32 g_viewTop;

// GLOBAL: MW2 0x100ea854
MechS32 g_viewLeftScaled;

// GLOBAL: MW2 0x100ea858
// GLOBAL: MW2MATROX 0x101d689c
MechS32 g_viewCenterY;

// GLOBAL: MW2 0x100ea85c
MechS32 g_viewBottomScaled;

// GLOBAL: MW2 0x100ea860
// GLOBAL: MW2MATROX 0x101d68a8
MechScalar g_viewFarPlane;

// GLOBAL: MW2 0x100ea864
// GLOBAL: MW2MATROX 0x101d68c0
MechScalar g_viewProjX0;

// GLOBAL: MW2 0x100ea868
// GLOBAL: MW2MATROX 0x101d68c4
MechScalar g_viewProjX1;

// GLOBAL: MW2 0x100ea86c
// GLOBAL: MW2MATROX 0x101d68c8
MechScalar g_viewProjX2;

// GLOBAL: MW2 0x100ea870
// GLOBAL: MW2MATROX 0x101d68cc
MechScalar g_viewProjY0;

// GLOBAL: MW2 0x100ea874
// GLOBAL: MW2MATROX 0x101d68d0
MechScalar g_viewProjY1;

// GLOBAL: MW2 0x100ea878
// GLOBAL: MW2MATROX 0x101d68d4
MechScalar g_viewProjY2;

// GLOBAL: MW2 0x100ea87c
// GLOBAL: MW2MATROX 0x101d68d8
MechScalar g_viewProjZ0;

// GLOBAL: MW2 0x100ea880
// GLOBAL: MW2MATROX 0x101d68dc
MechScalar g_viewProjZ1;

// GLOBAL: MW2 0x100ea884
// GLOBAL: MW2MATROX 0x101d68e0
MechScalar g_viewProjZ2;

// GLOBAL: MW2 0x100ea888
MechS32 g_viewProjectScaleX16;

// GLOBAL: MW2 0x100ea88c
MechS32 g_viewProjectScaleY16;

// GLOBAL: MW2 0x100ea890
// GLOBAL: MW2MATROX 0x101d6874
MechScalar g_viewRotX0;

// GLOBAL: MW2 0x100ea894
// GLOBAL: MW2MATROX 0x101d6878
MechScalar g_viewRotX1;

// GLOBAL: MW2 0x100ea898
// GLOBAL: MW2MATROX 0x101d687c
MechScalar g_viewRotX2;

// GLOBAL: MW2 0x100ea89c
// GLOBAL: MW2MATROX 0x101d6880
MechScalar g_viewRotY0;

// GLOBAL: MW2 0x100ea8a0
// GLOBAL: MW2MATROX 0x101d6884
MechScalar g_viewRotY1;

// GLOBAL: MW2 0x100ea8a4
// GLOBAL: MW2MATROX 0x101d6888
MechScalar g_viewRotY2;

// GLOBAL: MW2 0x100ea8a8
// GLOBAL: MW2MATROX 0x101d688c
MechScalar g_viewRotZ0;

// GLOBAL: MW2 0x100ea8ac
// GLOBAL: MW2MATROX 0x101d6894
MechScalar g_viewRotZ1;

// GLOBAL: MW2 0x100ea8b0
// GLOBAL: MW2MATROX 0x101d6898
MechScalar g_viewRotZ2;

// GLOBAL: MW2 0x100ea8b4
// GLOBAL: MW2MATROX 0x101d686c
MechScalar g_viewEyeY;

// GLOBAL: MW2 0x100ea8b8
// GLOBAL: MW2MATROX 0x101d6868
MechScalar g_viewEyeX;

// GLOBAL: MW2 0x100ea8bc
// GLOBAL: MW2MATROX 0x101d6870
MechScalar g_viewEyeZ;

// GLOBAL: MW2 0x100ea8c0
// GLOBAL: MW2MATROX 0x101d68b4
MechScalar g_viewLightZ;

// GLOBAL: MW2 0x100ea8c4
// GLOBAL: MW2MATROX 0x101d68ac
MechScalar g_viewLightX;

// GLOBAL: MW2 0x100ea8c8
// GLOBAL: MW2MATROX 0x101d68b0
MechScalar g_viewLightY;

// GLOBAL: MW2 0x100ea8cc
MechS32 g_viewTopScaled;

// GLOBAL: MW2 0x100ea8d0
// GLOBAL: MW2MATROX 0x101d68bc
MechScalar g_viewNearPlane;

// GLOBAL: MW2 0x100ea8d4
MechS32 g_viewRightScaled;

#ifdef MW2_MATROX
// The Matrox edition's shading: the ambient light as a fraction (out of 256), and the length of the
// light's position and that over 128 (SelectEyepoint).
// GLOBAL: MW2MATROX 0x101d68a0
MechFloat g_viewAmbientScale;

// GLOBAL: MW2MATROX 0x101d6858
MechFloat g_viewLightLength;

// GLOBAL: MW2MATROX 0x101d68b8
MechFloat g_viewLightScale;
#endif

// Makes p_eyepoint the current eyepoint and copies what the renderer uses each frame out of it:
// its rotation (also scaled by the projection factors), position, view rectangle and shading.
// MW2MATROX: the products of the rotation with the projection scales load their operands in the
// other order (commutative operand order).
// FUNCTION: MW2 0x1004b980
// FUNCTION: MW2MATROX 0x10029070
void SelectEyepoint(Eyepoint* p_eyepoint)
{
	Eyepoint* eyepoint;

	g_eyepoint = eyepoint = p_eyepoint;
	g_ambientLight = eyepoint->m_ambientLight;
#ifdef MW2_MATROX
	g_viewAmbientScale = g_ambientLight / 256.0f;
	g_directionalLight = eyepoint->m_directionalLight;
	g_viewProjectScaleX = eyepoint->m_projectScaleX;
	g_viewProjectScaleY = eyepoint->m_projectScaleY;
	g_viewRotX0 = eyepoint->m_viewMatrix.m_rows[0][0];
	g_viewRotX1 = eyepoint->m_viewMatrix.m_rows[0][1];
	g_viewRotX2 = eyepoint->m_viewMatrix.m_rows[0][2];
	g_viewRotY0 = eyepoint->m_viewMatrix.m_rows[1][0];
	g_viewRotY1 = eyepoint->m_viewMatrix.m_rows[1][1];
	g_viewRotY2 = eyepoint->m_viewMatrix.m_rows[1][2];
	g_viewProjZ0 = g_viewRotZ0 = eyepoint->m_viewMatrix.m_rows[2][0];
	g_viewProjZ1 = g_viewRotZ1 = eyepoint->m_viewMatrix.m_rows[2][1];
	g_viewProjZ2 = g_viewRotZ2 = eyepoint->m_viewMatrix.m_rows[2][2];
	g_viewProjX0 = g_viewRotX0 * g_viewProjectScaleX;
	g_viewProjX1 = g_viewRotX1 * g_viewProjectScaleX;
	g_viewProjX2 = g_viewRotX2 * g_viewProjectScaleX;
	g_viewProjY0 = g_viewRotY0 * g_viewProjectScaleY;
	g_viewProjY1 = g_viewRotY1 * g_viewProjectScaleY;
	g_viewProjY2 = g_viewRotY2 * g_viewProjectScaleY;
	g_viewEyeX = eyepoint->m_viewMatrix.m_rows[3][0];
	g_viewEyeY = eyepoint->m_viewMatrix.m_rows[3][1];
	g_viewEyeZ = eyepoint->m_viewMatrix.m_rows[3][2];
	g_viewLightX = eyepoint->m_lightX;
	g_viewLightY = eyepoint->m_lightY;
	g_viewLightZ = eyepoint->m_lightZ;
	if (g_directionalLight) {
		g_viewLightScale =
			(g_viewLightLength = ApproximateVectorLength(g_viewLightX, g_viewLightY, g_viewLightZ)) * 0.0078125f;
	}

	g_viewNearPlane = eyepoint->m_nearPlane;
	g_viewLeft = eyepoint->m_viewLeft;
	g_viewFarPlane = eyepoint->m_farPlane;
	g_viewTop = eyepoint->m_viewTop;
	g_viewRight = eyepoint->m_viewRight;
	g_viewBottom = eyepoint->m_viewBottom;
	g_viewHalfWidth = eyepoint->m_halfWidth;
	g_viewHalfHeight = eyepoint->m_halfHeight;
	g_viewCenterX = eyepoint->m_centerX;
	g_viewCenterY = eyepoint->m_centerY;
#else
	g_directionalLight = eyepoint->m_directionalLight;
	g_viewProjectScaleX16 = eyepoint->m_projectScaleX16;
	g_viewProjectScaleY16 = eyepoint->m_projectScaleY16;
	g_viewProjectScaleX = eyepoint->m_projectScaleX;
	g_viewProjectScaleY = eyepoint->m_projectScaleY;
	g_viewRotX0 = eyepoint->m_viewMatrix.m_rows[0][0];
	g_viewRotX1 = eyepoint->m_viewMatrix.m_rows[0][1];
	g_viewRotX2 = eyepoint->m_viewMatrix.m_rows[0][2];
	g_viewRotY0 = eyepoint->m_viewMatrix.m_rows[1][0];
	g_viewRotY1 = eyepoint->m_viewMatrix.m_rows[1][1];
	g_viewRotY2 = eyepoint->m_viewMatrix.m_rows[1][2];
	g_viewProjZ0 = g_viewRotZ0 = eyepoint->m_viewMatrix.m_rows[2][0];
	g_viewProjZ1 = g_viewRotZ1 = eyepoint->m_viewMatrix.m_rows[2][1];
	g_viewProjZ2 = g_viewRotZ2 = eyepoint->m_viewMatrix.m_rows[2][2];
	g_viewProjX0 = FixedMul16(g_viewProjectScaleX16, g_viewRotX0);
	g_viewProjX1 = FixedMul16(g_viewProjectScaleX16, g_viewRotX1);
	g_viewProjX2 = FixedMul16(g_viewProjectScaleX16, g_viewRotX2);
	g_viewProjY0 = FixedMul16(g_viewProjectScaleY16, g_viewRotY0);
	g_viewProjY1 = FixedMul16(g_viewProjectScaleY16, g_viewRotY1);
	g_viewProjY2 = FixedMul16(g_viewProjectScaleY16, g_viewRotY2);
	g_viewEyeX = eyepoint->m_viewMatrix.m_rows[3][0];
	g_viewEyeY = eyepoint->m_viewMatrix.m_rows[3][1];
	g_viewEyeZ = eyepoint->m_viewMatrix.m_rows[3][2];
	g_viewLightX = eyepoint->m_lightX;
	g_viewLightY = eyepoint->m_lightY;
	g_viewLightZ = eyepoint->m_lightZ;
	g_viewNearPlane = eyepoint->m_nearPlane;
	g_viewLeft = eyepoint->m_viewLeft;
	g_viewFarPlane = eyepoint->m_farPlane;
	g_viewTop = eyepoint->m_viewTop;
	g_viewRight = eyepoint->m_viewRight;
	g_viewBottom = eyepoint->m_viewBottom;
	g_viewFar = g_viewFarPlane << 2;
	g_viewNear = g_viewNearPlane << 2;
	g_viewTopScaled = g_viewTop << 2;
	g_viewBottomScaled = g_viewBottom << 2;
	g_viewLeftScaled = g_viewLeft << 2;
	g_viewRightScaled = g_viewRight << 2;
	g_viewHalfWidth = eyepoint->m_halfWidth;
	g_viewHalfHeight = eyepoint->m_halfHeight;
	g_viewCenterX = eyepoint->m_centerX;
	g_viewCenterY = eyepoint->m_centerY;
	g_viewShiftX = eyepoint->m_projectShiftX;
	g_viewShiftY = eyepoint->m_projectShiftY;
#endif
}

// Sets up the eyepoint's projection from its view rectangle, field of view and pixel aspect.
// Stack-slot permutation: every local.
// MW2MATROX: floats, without the projection offset and the 16-bit scales; the detail scale is
// also scaled by LOD.PAR's LOD_SCALE, read the first time.
// FUNCTION: MW2 0x1004bc2e
// FUNCTION: MW2MATROX 0x100292cc
void UpdateProjection(Eyepoint* p_eyepoint)
{
#ifdef MW2_MATROX
	MechS32 halfWidth;
	MechS32 top;
	MechS32 halfHeight;
	Eyepoint* eyepoint;
	MechScalar scaleX;
	MechS32 centerX;
	MechS32 a;
	MechS32 c;
	MechScalar scaleY;
	MechS32 centerY;
	MechS32 right;
	MechS32 bottom;
	MechScalar aspect;
	MechS32 left;
	MechScalar fov;
	MechScalar fovY;
	MechChar line[0x100];
	FILE* file;

	eyepoint = p_eyepoint;
	fov = eyepoint->m_fovX;
	aspect = eyepoint->m_pixelAspect;
	top = eyepoint->m_viewTop;
	bottom = eyepoint->m_viewBottom;
	left = eyepoint->m_viewLeft;
	right = eyepoint->m_viewRight;
	eyepoint->m_centerX = centerX = (right + left + 1) >> 1;
	eyepoint->m_centerY = centerY = (bottom + top + 1) >> 1;
	eyepoint->m_halfWidth = halfWidth = max((right - left + 1) >> 1, 1);
	eyepoint->m_halfHeight = halfHeight = max((bottom - top + 1) >> 1, 1);
	if (fov > 16.0f) {
		fov = 16.0f;
	}
	if (fov < 0.5f) {
		fov = 0.5f;
	}

	a = (MechS32) (fov * 16.0f);
	eyepoint->m_fovY = fovY = halfWidth * fov * aspect / halfHeight;
	c = (MechS32) (fovY * 16.0f);
	if (a > 799) {
		a = 799;
	}
	if (c > 799) {
		c = 799;
	}

	eyepoint->m_frustumScaleX = g_slopeSines[a] * fov + g_slopeCosines[a];
	eyepoint->m_frustumScaleY = g_slopeSines[c] * fovY + g_slopeCosines[c];
	scaleX = halfWidth * fov;
	scaleY = halfWidth * fov * aspect;
	eyepoint->m_nearPlane = scaleX * 0.5f + 1.0f;
	eyepoint->m_projectScaleX = scaleX;
	eyepoint->m_projectScaleY = scaleY;
	if (g_lodQuality <= 0) {
		g_lodQuality = 1;
	}

	eyepoint->m_detailScale = scaleX / (g_lodQuality * 160.0f);
	if (!g_lodScaleLoaded) {
		file = fopen("LOD.PAR", "rt");
		if (file) {
			while (fgets(line, sizeof(line), file)) {
				if (line[0] == '/') {
					continue;
				}

				if (sscanf(line, "LOD_SCALE = %f", &g_lodScale)) {
					continue;
				}
			}

			fclose(file);
		}

		g_lodScaleLoaded = 1;
	}

	eyepoint->m_detailScale *= g_lodScale;
#else
	MechS32 centerX;
	MechS32 bottom;
	MechS32 c;
	MechS32 centerY;
	MechS32 right;
	MechS32 aspect;
	MechS32 left;
	MechS32 scaled2;
	MechS32 low2;
	MechS32 offsetY;
	MechS16 shift1;
	MechS16 shift2;
	Eyepoint* eyepoint;
	MechS32 fovY;
	MechS32 d;
	MechS32 offsetX;
	MechS32 b;
	MechS32 halfWidth;
	MechS32 top;
	MechS32 halfHeight;
	MechS32 scaled1;
	MechS32 low1;
	MechS32 fov;
	MechS32 a;

	eyepoint = p_eyepoint;
	fov = eyepoint->m_fovX;
	aspect = eyepoint->m_pixelAspect;
	offsetX = (MechS16) eyepoint->m_offsetX;
	offsetY = (MechS16) eyepoint->m_offsetY;
	top = eyepoint->m_viewTop;
	bottom = eyepoint->m_viewBottom;
	left = eyepoint->m_viewLeft;
	right = eyepoint->m_viewRight;
	eyepoint->m_centerX = centerX = ((right + left + 1) >> 1) + offsetX;
	eyepoint->m_centerY = centerY = ((bottom + top + 1) >> 1) + offsetY;
	eyepoint->m_halfWidth = halfWidth = max((right - left + 1) >> 1, 1);
	eyepoint->m_halfHeight = halfHeight = max((bottom - top + 1) >> 1, 1);
	if (fov > 0x100000) {
		fov = 0x100000;
	}
	if (fov < 0x8000) {
		fov = 0x8000;
	}

	a = MulRatio(fov, halfWidth, offsetX);
	b = MulRatio(fov, halfWidth, -offsetX);
	eyepoint->m_fovY = fovY = MulDiv64(FixedMul16(fov, aspect), halfWidth, halfHeight);
	c = MulRatio(fovY, halfHeight, offsetY);
	d = MulRatio(fovY, halfHeight, -offsetY);
	if (a > 799) {
		a = 799;
	}
	if (b > 799) {
		b = 799;
	}
	if (c > 799) {
		c = 799;
	}
	if (d > 799) {
		d = 799;
	}

	eyepoint->m_frustumScaleX = FixedMul29(fov, g_slopeSines[a]) + (g_slopeCosines[a] >> 13);
	eyepoint->m_frustumScaleY = FixedMul29(fovY, g_slopeSines[c]) + (g_slopeCosines[c] >> 13);
	shift2 = shift1 = 2;
	MulNormalize16(&low1, &scaled1, &shift1, halfWidth, FixedMul16(fov, aspect));
	MulNormalize16(&low2, &scaled2, &shift2, halfWidth, fov);
	eyepoint->m_nearPlane = (low2 >> 17) + 1;
	eyepoint->m_cullDistance = eyepoint->m_farPlane;
	eyepoint->m_projectShiftX = shift2;
	eyepoint->m_projectShiftY = shift1;
	eyepoint->m_projectScaleX16 = scaled2;
	eyepoint->m_projectScaleY16 = scaled1;
	eyepoint->m_projectScaleX = low2;
	eyepoint->m_projectScaleY = low1;
	if (g_lodQuality <= 0) {
		g_lodQuality = 1;
	}

	eyepoint->m_detailScale = low2 / (g_lodQuality * 160);
#endif
}

// FUNCTION: MW2 0x1004bf61
// FUNCTION: MW2MATROX 0x10029608
void SetNearPlane(Eyepoint* p_eyepoint, MechScalar p_value)
{
	p_eyepoint->m_nearPlane = g_viewNearPlane = p_value;
#ifndef MW2_MATROX
	g_viewNear = p_value << 2;
#endif
}

// FUNCTION: MW2 0x1004bf8a
// FUNCTION: MW2MATROX 0x10029627
void SetFarPlane(Eyepoint* p_eyepoint, MechScalar p_value)
{
	p_eyepoint->m_farPlane = g_viewFarPlane = p_value;
#ifndef MW2_MATROX
	if (p_value < 0x1fffffff) {
		g_viewFar = p_value << 2;
		p_eyepoint->m_cullDistance = p_value;
	}
	else {
		g_viewFar = 0x7fffffff;
		p_eyepoint->m_cullDistance = 0x7fffffff;
	}
#endif
}

// Builds the eyepoint's view matrix (its rotation transposed, and its position) from its
// position and rotation.
// FUNCTION: MW2 0x1004bfe8
// FUNCTION: MW2MATROX 0x10029646
void UpdateViewMatrix(Eyepoint* p_eyepoint)
{
	Matrix matrix;

	BuildMatrix(
		&matrix,
		p_eyepoint->m_pitch,
		p_eyepoint->m_heading,
		p_eyepoint->m_roll,
		p_eyepoint->m_x,
		p_eyepoint->m_y,
		p_eyepoint->m_z
	);
	TransposeRotation(&matrix, &p_eyepoint->m_viewMatrix);
	p_eyepoint->m_viewMatrix.m_rows[3][0] = matrix.m_rows[3][0];
	p_eyepoint->m_viewMatrix.m_rows[3][1] = matrix.m_rows[3][1];
	p_eyepoint->m_viewMatrix.m_rows[3][2] = matrix.m_rows[3][2];
}

// FUNCTION: MW2 0x1004c05c
// FUNCTION: MW2MATROX 0x100296b7
void ResetEyepointView(Eyepoint* p_eyepoint)
{
#ifndef MW2_MATROX
	p_eyepoint->m_offsetX = 0;
	p_eyepoint->m_offsetY = 0;
#endif
	UpdateProjection(p_eyepoint);
	UpdateViewMatrix(p_eyepoint);
}

// Sets the eyepoint's view from a transform.
// FUNCTION: MW2 0x1004c093
// FUNCTION: MW2MATROX 0x100296da
void SetEyepointTransform(Eyepoint* p_eyepoint, Matrix* p_matrix)
{
	TransposeRotation(p_matrix, &p_eyepoint->m_viewMatrix);
	p_eyepoint->m_viewMatrix.m_rows[3][0] = p_matrix->m_rows[3][0];
	p_eyepoint->m_viewMatrix.m_rows[3][1] = p_matrix->m_rows[3][1];
	p_eyepoint->m_viewMatrix.m_rows[3][2] = p_matrix->m_rows[3][2];
}

// Gets the eyepoint's view as a transform.
// FUNCTION: MW2 0x1004c0d8
// FUNCTION: MW2MATROX 0x1002971c
void GetEyepointTransform(Eyepoint* p_eyepoint, Matrix* p_matrix)
{
	TransposeRotation(&p_eyepoint->m_viewMatrix, p_matrix);
	p_matrix->m_rows[3][0] = p_eyepoint->m_viewMatrix.m_rows[3][0];
	p_matrix->m_rows[3][1] = p_eyepoint->m_viewMatrix.m_rows[3][1];
	p_matrix->m_rows[3][2] = p_eyepoint->m_viewMatrix.m_rows[3][2];
}

// Projects the world point (*p_x, *p_y, *p_z) onto the screen in place (*p_z the depth). A point
// at or behind the near plane is projected mirrored. Returns whether it is in front and on the
// screen.
// Stack-slot permutation of the locals. The Matrox edition's rebuild sums the dot products' terms
// and compares the depth and the screen bounds in the other operand order (entropy).
// FUNCTION: MW2 0x1004c11d
// FUNCTION: MW2MATROX 0x1002975e
MechS32 ProjectWorldPoint(MechScalar* p_x, MechScalar* p_y, MechScalar* p_z)
{
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechScalar dx;
	MechScalar dy;
	MechS32 behind;
	MechScalar dz;
#ifdef MW2_MATROX
	MechS32 screenX;
	MechS32 screenY;
#endif

	x = *p_x;
	y = *p_y;
	z = *p_z;
	dx = x - g_viewEyeX;
	dy = y - g_viewEyeY;
	dz = z - g_viewEyeZ;
#ifdef MW2_MATROX
	// The Matrox edition projects in floats, dividing by the depth.
	x = dx * g_viewProjX0 + dy * g_viewProjX1 + dz * g_viewProjX2;
	y = dx * g_viewProjY0 + dy * g_viewProjY1 + dz * g_viewProjY2;
	z = dx * g_viewProjZ0 + dy * g_viewProjZ1 + dz * g_viewProjZ2;
	*p_z = z;
	if (z <= g_viewNearPlane) {
		behind = TRUE;
		if (FIXED_IS_NEGATIVE(z)) {
			z = -z;
		}
		else if (!FIXED_IS_NONZERO(z)) {
			z = 1.0f;
		}
	}
	else {
		behind = FALSE;
	}

	screenX = (MechS32) (x / z) + g_viewCenterX;
	// Two jumps to the next instruction in the original.
	if (0) {
	}
	if (0) {
	}
	screenY = g_viewBottom - g_viewTop - ((MechS32) (y / z) + g_viewCenterY);
	*p_x = screenX;
	*p_y = screenY;
	if (behind) {
		return 0;
	}
	else {
		return screenX >= g_viewLeft && screenX <= g_viewRight && screenY >= g_viewTop && screenY <= g_viewBottom;
	}
#else
	x = FixedDot27(dx, g_viewProjX0, dy, g_viewProjX1, dz, g_viewProjX2);
	y = FixedDot27(dx, g_viewProjY0, dy, g_viewProjY1, dz, g_viewProjY2);
	z = FixedDot27(dx, g_viewProjZ0, dy, g_viewProjZ1, dz, g_viewProjZ2);
	*p_z = z;
	if (z <= g_viewNear) {
		behind = TRUE;
		if (z < 0) {
			z = -z;
		}
		else if (z == 0) {
			z = 1;
		}
	}
	else {
		behind = FALSE;
	}

	*p_x = ProjectCoordinate(x, z, g_viewShiftX, g_viewCenterX);
	*p_y = g_viewBottom - g_viewTop - ProjectCoordinate(y, z, g_viewShiftY, g_viewCenterY);
	if (behind) {
		return 0;
	}

	return *p_x >= g_viewLeft && *p_x <= g_viewRight && *p_y >= g_viewTop && *p_y <= g_viewBottom;
#endif
}

// The scene's shape filter (RenderSettings::m_shapeFilter): culls a shape against the view frustum,
// like CullMapViewShape the map view's. 1: a shape of kind 0xa0 with g_inCockpitView, 5: out of
// range or past the far plane, 4: in front of the near plane, 6 and 7: outside the side planes.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x1004c2ef
// STUB: MW2MATROX 0x10029957
MechS32 CullSceneShape(Shape* p_shape)
{
	MechS32 y;
	MechS32 z;
	MechS32 radius;
	MechS32 limit;
	MechS32 dx;
	MechS32 side;
	MechS32 dy;
	MechS32 height;
	MechS32 fov;
	MechS32 dz;
	MechS32 depth;
	MechS32 x;

	if (g_inCockpitView && (p_shape->m_kind & 0xf0) == 0xa0) {
		return 1;
	}

	x = p_shape->m_centerX;
	y = p_shape->m_centerY;
	z = p_shape->m_centerZ;
	radius = p_shape->m_radius;
	dx = x - g_viewEyeX;
	dy = y - g_viewEyeY;
	dz = z - g_viewEyeZ;
	// The Matrox edition has no cull distance and tests against the far plane.
	if (!IsWithinRadius(
			dx,
			dy,
			dz,
#ifdef MW2_MATROX
			g_eyepoint->m_farPlane + radius
#else
			g_eyepoint->m_cullDistance + radius
#endif
		)) {
		return 5;
	}

	depth = g_queueDepth = FixedDot29(dx, g_viewRotZ0, dy, g_viewRotZ1, dz, g_viewRotZ2);
	if (radius + depth < g_viewNearPlane) {
		return 4;
	}

	if (depth - radius > g_viewFarPlane) {
		return 5;
	}

	side = FixedDot29(dx, g_viewRotX0, dy, g_viewRotX1, dz, g_viewRotX2);
	fov = g_eyepoint->m_fovX;
	if (side > 0) {
		limit = MulAddDiv(fov, side, -depth, g_eyepoint->m_frustumScaleX);
	}
	else {
		limit = MulAddDiv(fov, -side, -depth, g_eyepoint->m_frustumScaleX);
	}

	if (limit > radius) {
		return 6;
	}

	height = FixedDot29(dx, g_viewRotY0, dy, g_viewRotY1, dz, g_viewRotY2);
	fov = g_eyepoint->m_fovY;
	if (height > 0) {
		limit = MulAddDiv(fov, height, -depth, g_eyepoint->m_frustumScaleY);
	}
	else {
		limit = MulAddDiv(fov, -height, -depth, g_eyepoint->m_frustumScaleY);
	}

	if (limit > radius) {
		return 7;
	}

	return 0;
}

// Culls a shape against the view frustum like CullSceneShape, without its range test. 1 for a
// hidden shape (bit 0x1000).
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x1004c565
// STUB: MW2MATROX 0x10029c23
MechS32 CullShapeToFrustum(Shape* p_shape)
{
	MechS32 y;
	MechS32 z;
	MechS32 radius;
	MechS32 limit;
	MechS32 dx;
	MechS32 side;
	MechS32 dy;
	MechS32 height;
	MechS32 fov;
	MechS32 dz;
	MechS32 depth;
	MechS32 x;

	x = p_shape->m_centerX;
	y = p_shape->m_centerY;
	z = p_shape->m_centerZ;
	radius = p_shape->m_radius;
	dx = x - g_viewEyeX;
	dy = y - g_viewEyeY;
	dz = z - g_viewEyeZ;
	if (p_shape->m_flags & 0x1000) {
		return 1;
	}

	depth = g_queueDepth = FixedDot29(dx, g_viewRotZ0, dy, g_viewRotZ1, dz, g_viewRotZ2);
	if (radius + depth < g_viewNearPlane) {
		return 4;
	}

	side = FixedDot29(dx, g_viewRotX0, dy, g_viewRotX1, dz, g_viewRotX2);
	fov = g_eyepoint->m_fovX;
	if (side > 0) {
		limit = MulAddDiv(fov, side, -depth, g_eyepoint->m_frustumScaleX);
	}
	else {
		limit = MulAddDiv(fov, -side, -depth, g_eyepoint->m_frustumScaleX);
	}

	if (limit > radius) {
		return 6;
	}

	height = FixedDot29(dx, g_viewRotY0, dy, g_viewRotY1, dz, g_viewRotY2);
	fov = g_eyepoint->m_fovY;
	if (height > 0) {
		limit = MulAddDiv(fov, height, -depth, g_eyepoint->m_frustumScaleY);
	}
	else {
		limit = MulAddDiv(fov, -height, -depth, g_eyepoint->m_frustumScaleY);
	}

	if (limit > radius) {
		return 7;
	}

	return 0;
}

// FUNCTION: MW2 0x1004c779
// FUNCTION: MW2MATROX 0x10029e62
MechS32 CullHiddenShape(MechU16* p_flags)
{
	if (*p_flags & 0x1000) {
		return TRUE;
	}

	return FALSE;
}

// FUNCTION: MW2 0x1004c7a6
// FUNCTION: MW2MATROX 0x10029e8f
MechS32 IsLodQualityHigh(undefined4 p_unk0x00)
{
	return g_lodQuality == 1;
}

// FUNCTION: MW2 0x1004c7cf
// FUNCTION: MW2MATROX 0x10029eb8
void SetLodQualityHigh(undefined4 p_unk0x00, MechS32 p_enable)
{
	if (p_enable) {
		g_lodQuality = 1;
	}
	else {
		g_lodQuality = 2;
	}
}
