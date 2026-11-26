#include "mapview.h"

#include "animation.h"
#include "decomp.h"
#include "depthsort.h"
#include "eyepoint.h"
#include "fixeddot27.h"
#include "fixeddot29.h"
#include "fixedfloat.h"
#include "objectanim.h"
#include "palette.h"
#include "polydraw.h"
#include "recordstacks.h"
#include "render.h"
#include "rendersettings.h"
#include "shapelists.h"
#include "shiftdiv.h"
#include "targeting.h"
#include "types.h"
#include "view.h"

#ifdef MW2_POWERVR
#include "accel/a3d.h"
#endif

// The map (satellite) view's projection: a top-down view of p_worldSpan units across.

#ifdef MW2_FLOAT_SCALARS
// The Matrox edition's projection of a view coordinate onto the screen, an __inline function this
// unit's /Ob1 expands (its return leaves two jumps to the next instruction where it is assigned).
__inline MechS32 ProjectMapCoordinate(MechFloat p_value, MechFloat p_scale, MechS32 p_center)
{
	return (MechS32) (p_value / p_scale) + p_center;
}
#endif

// GLOBAL: MW2 0x10109ab0
// GLOBAL: MW2GBL 0x100bb9e0
// GLOBAL: MW2PREMERCS 0x102144f0
// GLOBAL: MW2MATROX 0x10212cc0
// GLOBAL: MW23DFX 0x10245a20
// GLOBAL: MW2ATI 0x10179840
// GLOBAL: MW2S3 0x101820cc
// GLOBAL: MW2POWERVR 0x101a43c0
MechS32 g_savedPalettePending;

// GLOBAL: MW2 0x10109ac0
// GLOBAL: MW2GBL 0x100bb9f0
// GLOBAL: MW2PREMERCS 0x10214500
// GLOBAL: MW2MATROX 0x10212ba0
// GLOBAL: MW23DFX 0x10245900
// GLOBAL: MW2ATI 0x10179720
// GLOBAL: MW2S3 0x101820d0
// GLOBAL: MW2POWERVR 0x101a42a0
Eyepoint g_savedEyepoint;

// GLOBAL: MW2 0x10109ba0
// GLOBAL: MW2GBL 0x100bbad0
// GLOBAL: MW2PREMERCS 0x102145e0
// GLOBAL: MW2MATROX 0x10212cd0
// GLOBAL: MW23DFX 0x10245a30
// GLOBAL: MW2ATI 0x10179850
// GLOBAL: MW2S3 0x10182180
// GLOBAL: MW2POWERVR 0x101a43d0
MechScalar g_mapViewMinX;

// GLOBAL: MW2 0x10109ba4
// GLOBAL: MW2GBL 0x100bbad4
// GLOBAL: MW2PREMERCS 0x102145e4
// GLOBAL: MW2MATROX 0x10212cd4
// GLOBAL: MW23DFX 0x10245a34
// GLOBAL: MW2ATI 0x10179854
// GLOBAL: MW2S3 0x10182184
// GLOBAL: MW2POWERVR 0x101a43d4
MechScalar g_mapViewMaxX;

// GLOBAL: MW2 0x10109ba8
// GLOBAL: MW2GBL 0x100bbad8
// GLOBAL: MW2PREMERCS 0x102145e8
// GLOBAL: MW2MATROX 0x10212cd8
// GLOBAL: MW23DFX 0x10245a38
// GLOBAL: MW2ATI 0x10179858
// GLOBAL: MW2S3 0x10182188
// GLOBAL: MW2POWERVR 0x101a43d8
MechScalar g_mapViewMaxY;

// GLOBAL: MW2 0x10109bac
// GLOBAL: MW2GBL 0x100bbadc
// GLOBAL: MW2PREMERCS 0x102145ec
// GLOBAL: MW2MATROX 0x10212cdc
// GLOBAL: MW23DFX 0x10245a3c
// GLOBAL: MW2ATI 0x1017985c
// GLOBAL: MW2S3 0x1018218c
// GLOBAL: MW2POWERVR 0x101a43dc
MechScalar g_mapViewMinY;

// GLOBAL: MW2 0x10109bb0
// GLOBAL: MW2GBL 0x100bbae0
// GLOBAL: MW2PREMERCS 0x102145f0
// GLOBAL: MW2MATROX 0x10212ce0
// GLOBAL: MW23DFX 0x10245a40
// GLOBAL: MW2ATI 0x10179860
// GLOBAL: MW2S3 0x10182190
// GLOBAL: MW2POWERVR 0x101a43e0
MechScalar g_mapViewNear;

// GLOBAL: MW2 0x10109bb4
// GLOBAL: MW2GBL 0x100bbae4
// GLOBAL: MW2PREMERCS 0x102145f4
// GLOBAL: MW2MATROX 0x10212ce4
// GLOBAL: MW23DFX 0x10245a44
// GLOBAL: MW2ATI 0x10179864
// GLOBAL: MW2S3 0x10182194
// GLOBAL: MW2POWERVR 0x101a43e4
MechScalar g_mapViewFar;

// GLOBAL: MW2 0x10109bb8
// GLOBAL: MW2GBL 0x100bbae8
// GLOBAL: MW2PREMERCS 0x102145f8
// GLOBAL: MW2MATROX 0x10212cc4
// GLOBAL: MW23DFX 0x10245a24
// GLOBAL: MW2ATI 0x10179844
// GLOBAL: MW2S3 0x10182198
// GLOBAL: MW2POWERVR 0x101a43c4
MechScalar g_mapViewScale;

// GLOBAL: MW2 0x10109bc0
// GLOBAL: MW2GBL 0x100bbaf0
// GLOBAL: MW2PREMERCS 0x10214600
// GLOBAL: MW2MATROX 0x10212c50
// GLOBAL: MW23DFX 0x102459b0
// GLOBAL: MW2ATI 0x101797d0
// GLOBAL: MW2S3 0x101821a0
// GLOBAL: MW2POWERVR 0x101a4350
RenderSettings g_savedRenderSettings;

// Saves the eyepoint and the rendering settings, and sets up a view from p_pose (position, then
// rotation) of p_worldSpan units across pane p_slot, as far as p_far.
// FUNCTION: MW2 0x10041fa0
// FUNCTION: MW2GBL 0x100802b0
// FUNCTION: MW2PREMERCS 0x1001b550
// FUNCTION: MW2MATROX 0x1001e170
// FUNCTION: MW23DFX 0x10001000
// FUNCTION: MW2ATI 0x10001000
// FUNCTION: MW2S3 0x10001f30
// FUNCTION: MW2POWERVR 0x10001000
void BeginMapView(MechScalar* p_pose, MechS32 p_slot, MechS32 p_worldSpan, MechS32 p_far)
{
#ifdef MW2_FLOAT_SCALARS
	g_mapViewScale = (MechFloat) p_worldSpan / (g_panes[p_slot].m_x1 - g_panes[p_slot].m_x0 + 1);
#if defined(MW2_ATI) || defined(MW2_S3) || defined(MW2_POWERVR) || defined(MW2_3DFX)
	g_mapViewMaxX = -(g_mapViewMinX = p_worldSpan / -2.0f);
#else
	g_mapViewMaxX = -(g_mapViewMinX = -(p_worldSpan / 2.0f));
#endif
#else
	g_mapViewScale = p_worldSpan / (g_panes[p_slot].m_x1 - g_panes[p_slot].m_x0 + 1);
	g_mapViewMaxX = -(g_mapViewMinX = -(p_worldSpan / 2));
#endif
	g_mapViewMinY = -(g_mapViewMaxY = (g_panes[p_slot].m_y1 - g_panes[p_slot].m_y0 + 1) * g_mapViewScale / 2);
	g_mapViewNear = 0;
	g_mapViewFar = p_far;
	g_savedPalettePending = g_palettePending;
	g_savedEyepoint = *g_eyepoint;
	g_eyepoint->m_heading = p_pose[3];
	g_eyepoint->m_pitch = p_pose[4];
	g_eyepoint->m_roll = p_pose[5];
	g_eyepoint->m_x = p_pose[0];
	g_eyepoint->m_y = p_pose[1];
	g_eyepoint->m_z = p_pose[2];
	SelectPane(p_slot);
	g_eyepoint->m_farPlane = g_mapViewFar;
	g_savedRenderSettings = g_renderSettings;
	g_renderSettings.m_shapeFilter = CullMapViewShape;
	g_renderSettings.m_projectVertex = ProjectMapViewVertex;
	UpdateProjection(g_eyepoint);
#ifdef MW2_FLOAT_SCALARS
	g_eyepoint->m_projectScaleX = 1.0;
	g_eyepoint->m_projectScaleY = g_eyepoint->m_pixelAspect;
#else
	g_eyepoint->m_projectScaleX16 = 0x2000;
	g_eyepoint->m_projectShiftX = 3;
	g_eyepoint->m_projectScaleY16 = FIXED_SHR(g_eyepoint->m_pixelAspect, 3);
	g_eyepoint->m_projectShiftY = 3;
#endif
	UpdateViewMatrix(g_eyepoint);
	SelectEyepoint(g_eyepoint);
	g_projectionDirty = 0;
}

// Draws the map view's scene: the terrain when bit 0 of p_flags is set, then the shapes. The
// PowerVR edition has its renderer draw the frame (FUN_10062b10) whatever the flags, and the early
// Mercenaries build draws the shape grid's cells ahead of the eyepoint.
// FUNCTION: MW2 0x1004215f
// FUNCTION: MW2GBL 0x1008046f
// FUNCTION: MW2PREMERCS 0x1001b70f
// FUNCTION: MW2MATROX 0x1001e36d
// FUNCTION: MW23DFX 0x10001180
// FUNCTION: MW2ATI 0x10001180
// FUNCTION: MW2S3 0x10002090
// FUNCTION: MW2POWERVR 0x10001180
void DrawMapViewScene(MapViewFlags p_flags)
{
#ifdef MW2_SHAPE_GRID
	MechS32 i;
	Shape* list;

#endif
#ifdef MW2_POWERVR
	FUN_10062b10();
#else
	if (p_flags.m_bits & 1) {
		DrawSkyAndGround(g_eyepoint);
	}

#ifdef MW2_SHAPE_GRID
	ListShapeGridCellsAhead(g_eyepoint);
	i = 0;
	for (list = g_shapeGridLists[i]; list; list = g_shapeGridLists[i]) {
		DrawShapeList(list);
		i++;
	}
#else
	DrawShapeList(g_sceneShapes);
#endif
	FUN_10069591();
#endif
}

// Restores the eyepoint and the rendering settings BeginMapView saved.
// FUNCTION: MW2 0x10042195
// FUNCTION: MW2GBL 0x100804a5
// FUNCTION: MW2PREMERCS 0x1001b78c
// FUNCTION: MW2MATROX 0x1001e3a3
// FUNCTION: MW23DFX 0x100011b0
// FUNCTION: MW2ATI 0x100011b0
// FUNCTION: MW2S3 0x100020c0
// FUNCTION: MW2POWERVR 0x10001190
void EndMapView(void)
{
	g_renderSettings = g_savedRenderSettings;
	*g_eyepoint = g_savedEyepoint;
	g_palettePending = g_savedPalettePending;
	ResetPane();
	UpdateProjection(g_eyepoint);
	UpdateViewMatrix(g_eyepoint);
	SelectEyepoint(g_eyepoint);
	g_projectionDirty = 0;
}

// Culls a shape against the map view's frustum: 1 hidden, 4 in front of the near plane, 5 past
// the far plane, 6 and 7 outside the side planes, 0 visible. Keeps its depth in g_queueDepth.
// FUNCTION: MW2 0x10042206
// FUNCTION: MW2GBL 0x10080516
// FUNCTION: MW2PREMERCS 0x1001b7fd
// FUNCTION: MW2MATROX 0x1001e414
// FUNCTION: MW23DFX 0x10001220
// FUNCTION: MW2ATI 0x10001220
// FUNCTION: MW2S3 0x10002130
// FUNCTION: MW2POWERVR 0x10001200
MechS32 CullMapViewShape(Shape* p_shape)
{
	MechScalar y;
	MechScalar z;
	MechScalar radius;
	MechScalar dist;
	MechScalar dx;
	MechScalar side;
	MechScalar dy;
	MechScalar height;
	MechScalar dz;
	MechScalar depth;
	MechScalar x;

	if (p_shape->m_flags & 0x1000) {
		return 1;
	}

	x = p_shape->m_centerX;
	y = p_shape->m_centerY;
	z = p_shape->m_centerZ;
	radius = p_shape->m_radius;
	dx = x - g_viewEyeX;
	dy = y - g_viewEyeY;
	dz = z - g_viewEyeZ;
#ifdef MW2_FLOAT_SCALARS
	depth = g_queueDepth = dx * g_viewRotZ0 + dy * g_viewRotZ1 + dz * g_viewRotZ2;
#else
	depth = g_queueDepth = FixedDot29(dx, g_viewRotZ0, dy, g_viewRotZ1, dz, g_viewRotZ2);
#endif
	if (depth + radius < g_viewNearPlane) {
		return 4;
	}

	if (depth - radius > g_viewFarPlane) {
		return 5;
	}

#ifdef MW2_FLOAT_SCALARS
	side = dx * g_viewRotX0 + dy * g_viewRotX1 + dz * g_viewRotX2;
	if (!FIXED_IS_NEGATIVE(side)) {
#else
	side = FixedDot29(dx, g_viewRotX0, dy, g_viewRotX1, dz, g_viewRotX2);
	if (side > 0) {
#endif
		dist = side - g_mapViewMaxX;
	}
	else {
		dist = g_mapViewMinX - side;
	}

	if (dist > radius) {
		return 6;
	}

#ifdef MW2_FLOAT_SCALARS
	height = dx * g_viewRotY0 + dy * g_viewRotY1 + dz * g_viewRotY2;
	if (!FIXED_IS_NEGATIVE(height)) {
#else
	height = FixedDot29(dx, g_viewRotY0, dy, g_viewRotY1, dz, g_viewRotY2);
	if (height > 0) {
#endif
		dist = height - g_mapViewMaxY;
	}
	else {
		dist = g_mapViewMinY - height;
	}

	if (dist > radius) {
		return 7;
	}

	return 0;
}

// Projects a vertex onto the map view (ProjectCoordinate's scaling) once per frame, with its clip
// outcodes, and adds it to the polygon being built: ProjectVertex's map-view counterpart.
// FUNCTION: MW2 0x100423b3
// FUNCTION: MW2GBL 0x100806c3
// FUNCTION: MW2PREMERCS 0x1001b9aa
// FUNCTION: MW2MATROX 0x1001e5c0
// FUNCTION: MW23DFX 0x100013a0
// FUNCTION: MW2ATI 0x100013a0
// FUNCTION: MW2S3 0x100022b0
// FUNCTION: MW2POWERVR 0x10001380
ProjectedVertex* ProjectMapViewVertex(ProjectedVertex* p_vertex)
{
	MechS32 x;
	MechU8 outcode;
	MechS32 y;

	outcode = 0;
	if (!p_vertex->m_projected) {
#ifdef MW2_FLOAT_SCALARS
		x = ProjectMapCoordinate(p_vertex->m_x, g_mapViewScale, g_viewCenterX);
		y = g_viewBottom - g_viewTop - ProjectMapCoordinate(p_vertex->m_y, g_mapViewScale, g_viewCenterY);
#else
		x = p_vertex->m_x;
		y = p_vertex->m_y;
		x = ProjectCoordinate(x, g_mapViewScale, g_viewShiftX, g_viewCenterX);
		y = g_viewBottom - g_viewTop - ProjectCoordinate(y, g_mapViewScale, g_viewShiftY, g_viewCenterY);
#endif
		if (x - g_viewLeft < 0) {
			outcode |= 1;
		}

		if (x - g_viewRight > 0) {
			outcode |= 2;
		}

		if (y - g_viewTop < 0) {
			outcode |= 4;
		}

		if (y - g_viewBottom > 0) {
			outcode |= 8;
		}

		p_vertex->m_screenX = x;
		p_vertex->m_screenY = y;
		p_vertex->m_outcode = outcode;
		p_vertex->m_projected = 1;
	}

	g_polygonOrCodes |= outcode;
	g_polygonAndCodes &= outcode;
	if (g_polygonPointCount >= 20) {
		g_queueHasRoom = 0;
	}
	else {
		g_polygonPoints[g_polygonPointCount] = p_vertex;
		g_polygonPointCount++;
	}

	return p_vertex;
}

// Projects p_point onto the map view in place. Returns whether it is in front of the eye and
// on the screen. The Matrox edition projects the position p_position to p_screen instead.
// FUNCTION: MW2 0x1004251e
// FUNCTION: MW2GBL 0x1008082c
// FUNCTION: MW2PREMERCS 0x1001bb15
// FUNCTION: MW2MATROX 0x1001e743
// FUNCTION: MW23DFX 0x100014b0
// FUNCTION: MW2ATI 0x100014b0
// FUNCTION: MW2S3 0x10002390
// FUNCTION: MW2POWERVR 0x10001490
#ifdef MW2_FLOAT_SCALARS
MechS32 ProjectMapPoint(Vector3* p_position, Point* p_screen)
#else
MechS32 ProjectMapPoint(MapPoint* p_point)
#endif
{
#ifdef MW2_FLOAT_SCALARS
	MechS32 visible;
	MechS32 screenX;
	MechScalar dx;
	MechScalar dy;
	MechScalar dz;
	MechS32 screenY;
	MechScalar x;
	MechScalar y;
	MechScalar z;

	visible = FALSE;
	x = p_position->m_x;
	y = p_position->m_y;
	z = p_position->m_z;
	dx = x - g_viewEyeX;
	dy = y - g_viewEyeY;
	dz = z - g_viewEyeZ;
	x = dx * g_viewProjX0 + dy * g_viewProjX1 + dz * g_viewProjX2;
	y = dx * g_viewProjY0 + dy * g_viewProjY1 + dz * g_viewProjY2;
	z = dx * g_viewProjZ0 + dy * g_viewProjZ1 + dz * g_viewProjZ2;
	screenX = ProjectMapCoordinate(x, g_mapViewScale, g_viewCenterX);
	screenY = g_viewBottom - g_viewTop - ProjectMapCoordinate(y, g_mapViewScale, g_viewCenterY);
	if (!FIXED_IS_NEGATIVE(z)) {
		if (screenX >= g_viewLeft && screenX <= g_viewRight && screenY >= g_viewTop && screenY <= g_viewBottom) {
			visible = TRUE;
		}
		else {
			visible = FALSE;
		}
	}

	p_screen->m_x = screenX;
	p_screen->m_y = screenY;
	return visible;
#else
	MechS32 visible;
	MechS32 x;
	MechS32 y;
	MechS32 z;
	MechS32 dx;
	MechS32 dy;
	MechS32 dz;

	visible = FALSE;
	x = p_point->m_xy.m_x;
	y = p_point->m_xy.m_y;
	z = p_point->m_z;
	dx = x - g_viewEyeX;
	dy = y - g_viewEyeY;
	dz = z - g_viewEyeZ;
	x = FixedDot27(dx, g_viewProjX0, dy, g_viewProjX1, dz, g_viewProjX2);
	y = FixedDot27(dx, g_viewProjY0, dy, g_viewProjY1, dz, g_viewProjY2);
	z = FixedDot27(dx, g_viewProjZ0, dy, g_viewProjZ1, dz, g_viewProjZ2);
	p_point->m_xy.m_x = ProjectCoordinate(x, g_mapViewScale, g_viewShiftX, g_viewCenterX);
	p_point->m_xy.m_y = g_viewBottom - g_viewTop - ProjectCoordinate(y, g_mapViewScale, g_viewShiftY, g_viewCenterY);
	p_point->m_z = z;
	if (z > 0) {
		if (p_point->m_xy.m_x >= g_viewLeft && p_point->m_xy.m_x <= g_viewRight && p_point->m_xy.m_y >= g_viewTop &&
			p_point->m_xy.m_y <= g_viewBottom) {
			visible = TRUE;
		}
		else {
			visible = FALSE;
		}
	}

	return visible;
#endif
}
