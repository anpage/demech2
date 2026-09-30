#ifndef UNK100758A0_H
#define UNK100758A0_H

#include "types.h"

struct Mech;

// The functions and globals of unk100758a0.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_100765f8(struct Mech* p_mech, struct Mech* p_other);
	void FUN_1007669e(struct Mech* p_mech, struct Mech* p_other, MechS32 p_damage);

#ifdef __cplusplus
}
#endif

#endif // UNK100758A0_H
