#ifndef GPANIM_H
#define GPANIM_H

#include "mech.h"
#include "types.h"

// The functions and globals of gpanim.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FirstGPAnim(void);
	void FUN_1000365a(Mech* p_mech);
	void FUN_1000369e(Mech* p_mech);
	MechS32* FUN_100036c3(Mech* p_mech, MechS32* p_offset);
	void FUN_100038c2(Mech* p_mech, MechS32 (*p_sounds)[4], MechS32* p_offset);
	void FUN_10003a10(Mech* p_mech);

#ifdef __cplusplus
}
#endif

#endif // GPANIM_H
