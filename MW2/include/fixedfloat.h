#ifndef FIXEDFLOAT_H
#define FIXEDFLOAT_H

#include "types.h"

// The Matrox edition (MW2_MATROX) keeps the simulation's 16.16 fixed-point values as floats in
// plain units: a member that 1.1 shifts down to its integer part, the edition converts (fld,
// then __ftol). Macros only, so that 1.1's units see no new symbols.
// MechScalar: the type of such a value (a member, a local, a parameter).
// FIXED_CONST(n): the constant n (180 for 180 degrees); FIXED_MOD360(x): an angle wrapped to
// [0, 360) degrees (16.16 in 1.1: % 0x1680000), for which the edition calls fmod (math.h);
// FIXED_SHR(x, n), FIXED_SHL(x, n): 1.1's shifts of a scaled value, divisions and products for
// the edition.
#ifdef MW2_MATROX
#include <math.h>

#define MechScalar MechFloat
#define FIXED_TO_INT(x) ((MechS32) (x))
#define FIXED_CONST(n) ((MechFloat) (n))
#define FIXED_MOD360(x) fmod((x), 360.0)
#define FIXED_SHR(x, n) ((x) / (1 << (n)))
#define FIXED_SHL(x, n) ((x) * (1 << (n)))
#else
#define MechScalar MechS32
#define FIXED_TO_INT(x) ((x) >> 16)
#define FIXED_CONST(n) ((n) * 0x10000)
#define FIXED_MOD360(x) ((x) % 0x1680000)
#define FIXED_SHR(x, n) ((x) >> (n))
#define FIXED_SHL(x, n) ((x) << (n))
#endif

#endif // FIXEDFLOAT_H
