#ifndef ACCEL_FLOATPOINT_H
#define ACCEL_FLOATPOINT_H

#include "types.h"

// A point in floats: the accelerated editions' texture coordinates (g_bandTexCoords).
// SIZE 0x8
typedef struct FloatPoint {
	MechFloat m_x; // 0x00
	MechFloat m_y; // 0x04
} FloatPoint;

#endif // ACCEL_FLOATPOINT_H
