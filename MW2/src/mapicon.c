/* DrawMapIcon, which the Matrox edition places after DrawMapUnits in cockpit.c's object: cockpit.c
   includes this file at 1.1's position, and at the Matrox edition's. */
#include "cockpit.h"
#include "decomp.h"
#include "fixedfloat.h"
#include "loadres.h"
#include "mappoint.h"
#include "mapview.h"
#include "mw2prj.h"
#include "point.h"
#include "setres.h"
#include "types.h"
#include "vector3.h"
#include "vfxa.h"

#ifdef MW2_VFX16
#include "accel/vfx16.h"
#endif

// Draws icon p_icon at the world position p_pos, if the map view shows it. The Matrox edition
// takes the position by address.
// FUNCTION: MW2 0x1003e40a
// FUNCTION: MW2GBL 0x1004d75a
// FUNCTION: MW2MATROX 0x1007426c
// FUNCTION: MW23DFX 0x100038d0
// FUNCTION: MW2ATI 0x10003560
// FUNCTION: MW2S3 0x10009530
// FUNCTION: MW2POWERVR 0x10003420
#ifdef MW2_FLOAT_SCALARS
void DrawMapIcon(CockpitLayout* p_layout, Vector3* p_position, MechS32 p_icon)
#else
void DrawMapIcon(CockpitLayout* p_layout, MapPoint p_pos, MechS32 p_icon)
#endif
{
	PANE* viewport;
	void* shape;
	MechS32 visible;
#ifdef MW2_FLOAT_SCALARS
	Point screen;
#endif

	viewport = p_layout->m_viewport;
#ifdef MW2_FLOAT_SCALARS
	visible = ProjectMapPoint(p_position, &screen);
	if (visible && p_layout->m_gauges[1]) {
		visible = p_layout->m_gauges[1](viewport, &screen);
	}
#else
	visible = ProjectMapPoint(&p_pos);
	if (visible && p_layout->m_gauges[1]) {
		visible = p_layout->m_gauges[1](viewport, p_pos.m_xy);
	}
#endif

	if (visible) {
		shape = LoadCachedResource(g_mw2PrjHandle, p_icon + HUD_ART_RESOLUTION, g_resTagShp, 0);
		if (shape) {
#ifdef MW2_FLOAT_SCALARS
			VFX_shape_draw(viewport, shape, 0, screen.m_x, screen.m_y);
#else
			VFX_shape_draw(viewport, shape, 0, p_pos.m_xy.m_x, p_pos.m_xy.m_y);
#endif
			UnlockCachedResource(p_icon + HUD_ART_RESOLUTION, g_resTagShp);
		}
	}
}
