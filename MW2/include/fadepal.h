#ifndef FADEPAL_H
#define FADEPAL_H

#include "decomp.h"
#include "fixedfloat.h"
#include "types.h"

struct SceneObject;

struct Mech;

// The functions of fadepal.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void PlayWeaponLaunchSound(undefined4 p_shotType, MechS32 p_sound, undefined4 p_pan);
	void RenderViewToPane(MechU32 p_target, MechScalar p_fovX, MechS32* p_view, struct SceneObject* p_object);
	void FlashZappedPalette(void);
	void FlashZappedPaletteLevel(MechU32 p_level);
	void FadeToEndPalette(MechS32 p_alternate);
	void EmitWreckSmoke(struct Mech* p_mech);
	void BreakUpMech(struct Mech* p_mech);
	void FireJumpJetEffects(struct Mech* p_mech);
	void PlayMechLanding(struct Mech* p_mech, MechScalar p_speed);
	void PlayPlayerHitFeedback(MechScalar p_x, MechScalar p_y, MechScalar p_z);

#ifdef __cplusplus
}
#endif

#endif // FADEPAL_H
