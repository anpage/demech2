#ifndef PORTABLE_H
#define PORTABLE_H

#include "types.h"

// Types and helpers for the portable C that replaces hand-written assembly (PORTABLE_C in
// compat.h). The replacements are standard C with defined behaviour; the register-level
// operations whose C spelling would be implementation-defined are written here once, in defined
// terms.

// 64-bit integers (imul products, edx:eax dividends). VC++ 4.1 has no long long: its type is
// __int64, and constants are written as casts ((MechS64) 1 << 40). They live here rather than in
// types.h: every typedef there is a symbol in every unit, and 4.1's stack-slot assignment and
// operand order react to the symbol count.
#if defined(_MSC_VER) && _MSC_VER < 1200
typedef __int64 MechS64;
typedef unsigned __int64 MechU64;
#else
#include <stdint.h>
typedef int64_t MechS64;
typedef uint64_t MechU64;
#endif

#if defined(_MSC_VER) || !defined(__STDC_VERSION__)
#define PORTABLE_INLINE static __inline
#else
#define PORTABLE_INLINE static inline
#endif

// Reads a 32-bit register's bits as a signed value (two's complement). A plain cast of a value
// above 0x7fffffff is implementation-defined before C23; compilers fold this to nothing.
PORTABLE_INLINE MechS32 PortableS32(MechU32 p_value)
{
	if (p_value <= 0x7fffffff) {
		return (MechS32) p_value;
	}

	return -(MechS32) ~p_value - 1;
}

// `shrd lo, hi, n; adc lo, 0` on the 64-bit value hi:lo (1 <= n <= 31): the low 32 bits of the
// value shifted right by n, rounded up by the last bit shifted out (wrapping like the adc).
PORTABLE_INLINE MechU32 PortableShrdRound(MechU64 p_value, MechS32 p_shift)
{
	return (MechU32) (p_value >> p_shift) + ((MechU32) (p_value >> (p_shift - 1)) & 1);
}

#endif // PORTABLE_H
