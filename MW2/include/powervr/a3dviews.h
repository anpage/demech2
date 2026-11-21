#ifndef POWERVR_A3DVIEWS_H
#define POWERVR_A3DVIEWS_H

#include "accel/a3d.h"
#include "types.h"

// What of the PowerVR renderer (powervr/a3d.c) only the satellite view (cockpit.c) and the mech
// view panel (mechviewpanel.c) use: declared here rather than in accel/a3d.h, which every drawing
// unit includes, and where the declarations would move their code.

#ifdef __cplusplus
extern "C"
{
#endif

	// The sky's list, which the satellite view hides (SwitchCockpitView).
	extern MechS32 g_a3dSkyList;

	// Cockpit panel 2's view, the mech view panel's inset.
	extern A3DView g_a3dPanel2View;

	// Sets all of the view p_view's qualities, in its list cleared.
	void FUN_10060770(A3DView* p_view, MechS32 p_fog, MechS32 p_texturing, MechS32 p_smoothShading, MechS32 p_shadows);

#ifdef __cplusplus
}
#endif

#endif // POWERVR_A3DVIEWS_H
