#ifndef VECTOR3_H
#define VECTOR3_H

#include "fixedfloat.h"
#include "types.h"

// A point in the world: 16.16, or plain floats in the Matrox edition (fixedfloat.h).
// SIZE 0xc
typedef struct Vector3 {
	MechScalar m_x; // 0x00
	MechScalar m_y; // 0x04
	MechScalar m_z; // 0x08
} Vector3;

#endif // VECTOR3_H
