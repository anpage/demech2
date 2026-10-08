#ifndef MATROX_A3DVERTEX_H
#define MATROX_A3DVERTEX_H

#include "decomp.h"
#include "types.h"

// A vertex as the A3D renderer takes it: the screen position, the depth and its reciprocal, the
// color (0-255 per component) and the texture coordinates.
// SIZE 0x30
typedef struct A3DVertex {
	MechFloat m_x;       // 0x00
	MechFloat m_y;       // 0x04
	MechFloat m_z;       // 0x08
	MechFloat m_w;       // 0x0c — 1 / m_z (1 for flat polygons)
	MechFloat m_red;     // 0x10
	MechFloat m_green;   // 0x14
	MechFloat m_blue;    // 0x18
	MechFloat m_unk0x1c; // 0x1c
	MechFloat m_unk0x20; // 0x20
	MechFloat m_unk0x24; // 0x24
	MechFloat m_u;       // 0x28
	MechFloat m_v;       // 0x2c
} A3DVertex;

#endif // MATROX_A3DVERTEX_H
