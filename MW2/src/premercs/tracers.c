// The early Mercenaries build's object after shots.c's, which GBL doesn't have: it creates, moves
// and draws small objects of its own (lines and points, through VFX), and SpawnShot starts one
// ahead of each machine-gun shot (shot type 5), so they look like tracers. Not decompiled yet.

#include "premercs/tracers.h"

#include "decomp.h"
#include "fixedfloat.h"
#include "types.h"

// Starts one at a point ahead of the shot, moving with it.
// STUB: MW2PREMERCS 0x10089624
void FUN_10089624(MechScalar p_x, MechScalar p_y, MechScalar p_z, MechScalar p_dx, MechScalar p_dy, MechScalar p_dz)
{
	STUB(0x10089624);
}
