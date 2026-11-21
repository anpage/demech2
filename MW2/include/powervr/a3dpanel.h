#ifndef POWERVR_A3DPANEL_H
#define POWERVR_A3DPANEL_H

#include "accel/a3d.h"
#include "types.h"

// What the cockpit panels with a 3D view of their own (CockpitPanel::m_view) use of the PowerVR
// edition's renderer (powervr/a3d.c), apart from accel/a3d.h: a header of its own, so that the
// other units, which include accel/a3d.h, don't declare them.

#ifdef __cplusplus
extern "C"
{
#endif

	// White, and the light's color (FUN_100607d0): the colours FUN_10061160 is passed.
	extern MechFloat g_a3dWhite[3];
	extern MechFloat g_a3dLightColour[3];

	// Sets all of the view p_view's qualities, in its list cleared.
	void FUN_10060770(A3DView* p_view, MechS32 p_fog, MechS32 p_texturing, MechS32 p_smoothShading, MechS32 p_shadows);

#ifdef __cplusplus
}
#endif

#endif // POWERVR_A3DPANEL_H
