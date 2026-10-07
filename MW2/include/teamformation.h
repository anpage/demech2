#ifndef TEAMFORMATION_H
#define TEAMFORMATION_H

#include "fixedfloat.h"
#include "types.h"

// A named formation: each of the eight slots' offset from the leader and heading.
// SIZE 0x70
typedef struct TeamFormation {
	MechChar m_name[0x10];   // 0x00
	MechScalar m_x[8];       // 0x10
	MechScalar m_z[8];       // 0x30
	MechScalar m_heading[8]; // 0x50
} TeamFormation;

#endif // TEAMFORMATION_H
