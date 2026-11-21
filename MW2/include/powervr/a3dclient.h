#ifndef POWERVR_A3DCLIENT_H
#define POWERVR_A3DCLIENT_H

#include "types.h"

// Functions only the PowerVR edition has, which its SimMain calls around the renderer:
// environment.c's (FUN_10006f30, the palette of the current time of day, MW2_TIME_OF_DAY_LOOKUP)
// and eyepoint.c's (FUN_10028ac0 turns the scene's shapes of kind 0x20, as TurnBillboards does
// those of kinds 0x10 and 0x60), declared here rather than in eyepoint.h, which would declare it
// for every unit that includes it. The renderer's own functions are in accel/a3d.h.

#ifdef __cplusplus
extern "C"
{
#endif

	MechS32 FUN_10006f30(void);
	void FUN_10028ac0(void);

	// The renderer's fog colour (an sgl_colour), which a cheat code passes back to SGL.
	extern MechFloat g_a3dFogColour[3];

#ifdef __cplusplus
}
#endif

#endif // POWERVR_A3DCLIENT_H
