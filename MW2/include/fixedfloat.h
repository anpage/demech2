#ifndef FIXEDFLOAT_H
#define FIXEDFLOAT_H

// The Matrox edition (MW2_MATROX) keeps the simulation's 16.16 fixed-point values as floats in
// plain units: a member that 1.1 shifts down to its integer part, the edition converts (fld,
// then __ftol). Macros only, so that 1.1's units see no new symbols.
#ifdef MW2_MATROX
#define FIXED_TO_INT(x) ((MechS32) (x))
#else
#define FIXED_TO_INT(x) ((x) >> 16)
#endif

#endif // FIXEDFLOAT_H
