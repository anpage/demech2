#ifndef FADEPAL_H
#define FADEPAL_H

#include "decomp.h"
#include "types.h"

struct SceneObject;

struct Mech;

// The functions of fadepal.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_1004c890(undefined4 p_unk0x00, MechS32 p_unk0x04, undefined4 p_unk0x08);
	void FUN_1004c8bd(MechU32 p_target, MechS32 p_fovX, MechS32* p_view, struct SceneObject* p_object);
	void FUN_1004ca0d(void);
	void FUN_1004ca29(MechU32 p_level);
	void FadeToEndPalette(MechS32 p_alternate);
	void FUN_1004cb11(struct Mech* p_mech);
	void FUN_1004cc27(struct Mech* p_mech);
	void FUN_1004ccba(struct Mech* p_mech);
	void FUN_1004ce3e(struct Mech* p_mech, MechS32 p_speed);
	void PlayPlayerHitFeedback(MechS32 p_x, MechS32 p_y, MechS32 p_z);

#ifdef __cplusplus
}
#endif

#endif // FADEPAL_H
