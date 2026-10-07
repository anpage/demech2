#ifndef PROBEDIRECTION_H
#define PROBEDIRECTION_H

#include "fixedfloat.h"
#include "types.h"

// One of the sixteen directions around a mech (g_probeDirections), as (x, z) divisors of a length.
// Floats in the Matrox edition.
// SIZE 0x8
typedef struct ProbeDirection {
	MechScalar m_x; // 0x00
	MechScalar m_y; // 0x04
} ProbeDirection;

#endif // PROBEDIRECTION_H
