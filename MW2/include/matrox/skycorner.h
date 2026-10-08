#ifndef MATROX_SKYCORNER_H
#define MATROX_SKYCORNER_H

#include "types.h"

// A corner of the Matrox edition's textured sky (g_skyCorners): its position, and whether it
// takes the texture's far edge in u and in v.
// SIZE 0x14
typedef struct SkyCorner {
	MechFloat m_x;  // 0x00
	MechFloat m_y;  // 0x04
	MechFloat m_z;  // 0x08
	MechS32 m_farU; // 0x0c
	MechS32 m_farV; // 0x10
} SkyCorner;

#endif // MATROX_SKYCORNER_H
