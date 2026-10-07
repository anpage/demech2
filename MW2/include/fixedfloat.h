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
// the edition; FIXED_IS_NEGATIVE(x): a sign test, which the edition reads from the float's sign
// bit (test byte ptr [x+3], 0x80); FIXED_FROM_INT(x): 1.1's x << 16 of a value in whole units,
// which the edition keeps as it is; FIXED_RAW(x): a constant written as 1.1's raw 16.16 value
// (0x8000 for one half), a float the edition's plain units; FIXED_IS_NONZERO(x): a test for
// non-zero, which the edition makes against a small constant ((float) fabs(x) >= 1e-07f);
// FIXED_MAX, FIXED_MIN: the extremes searches start from (0x7fffffff and -0x7fffffff in 1.1,
// +-3.4e+38 in the edition); FIXED_TO_SCALAR(x): a 16.16 value the data holds (a record's
// angle) as a MechScalar; FIXED_LITERAL(x, f): 1.1's raw 16.16 constant x where the edition wrote
// its own float literal f (0.0296f for gravity's 0x794).
#ifdef MW2_MATROX
#include <math.h>

#define MechScalar MechFloat
#define FIXED_TO_INT(x) ((MechS32) (x))
#define FIXED_CONST(n) ((MechFloat) (n))
#define FIXED_MOD360(x) fmod((x), 360.0)
#define FIXED_SHR(x, n) ((x) / (1 << (n)))
#define FIXED_SHL(x, n) ((x) * (1 << (n)))
#define FIXED_IS_NEGATIVE(x) (*(MechU32*) &(x) & 0x80000000)
#define FIXED_FROM_INT(x) (x)
#define FIXED_RAW(x) ((x) / 65536.0f)
#define FIXED_IS_NONZERO(x) ((MechFloat) fabs(x) >= 1e-07f)
#define FIXED_MAX 3.4e+38f
#define FIXED_MIN (-3.4e+38f)
#define FIXED_TO_SCALAR(x) ((x) * (1.0f / 65536.0f))
#define FIXED_LITERAL(x, f) (f)
#else
#define MechScalar MechS32
#define FIXED_TO_INT(x) ((x) >> 16)
#define FIXED_CONST(n) ((n) * 0x10000)
#define FIXED_MOD360(x) ((x) % 0x1680000)
#define FIXED_SHR(x, n) ((x) >> (n))
#define FIXED_SHL(x, n) ((x) << (n))
#define FIXED_IS_NEGATIVE(x) ((x) < 0)
#define FIXED_FROM_INT(x) ((x) << 16)
#define FIXED_RAW(x) (x)
#define FIXED_IS_NONZERO(x) (x)
#define FIXED_MAX 0x7fffffff
#define FIXED_MIN (-0x7fffffff)
#define FIXED_TO_SCALAR(x) (x)
#define FIXED_LITERAL(x, f) (x)
#endif

#endif // FIXEDFLOAT_H
