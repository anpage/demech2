#ifndef POWERVR_A3DCLIENT_H
#define POWERVR_A3DCLIENT_H

#include "types.h"

// Functions only the PowerVR edition has, which its SimMain calls around the renderer, declared as
// the call sites show them until their units define them: environment.c's (0x10006f10 is the
// phase of a time of day, FUN_10006f30 the palette of the current one) and eyepoint.c's (turns the
// scene's shapes of kind 0x20, as TurnBillboards does those of kinds 0x10 and 0x60). The renderer's
// own functions are in accel/a3d.h.

#ifdef __cplusplus
extern "C"
{
#endif

	MechS32 FUN_10006f30(void);
	void FUN_10028ac0(void);

#ifdef __cplusplus
}
#endif

#endif // POWERVR_A3DCLIENT_H
