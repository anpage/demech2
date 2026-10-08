#ifndef PROJECTEDVERTEX_H
#define PROJECTEDVERTEX_H

#include "decomp.h"
#include "fixedfloat.h"
#include "types.h"

#ifdef MW2_MATROX
// The Matrox edition's projected vertex: its view-space position and texture coordinates in floats,
// and the vertex's color and normal, with the light its renderer shades the vertex with.
// SIZE 0x3c
typedef struct ProjectedVertex {
	MechFloat m_x;                    // 0x00
	MechFloat m_y;                    // 0x04
	MechFloat m_z;                    // 0x08 — the depth
	MechS32 m_screenX;                // 0x0c
	MechS32 m_screenY;                // 0x10
	MechFloat m_u;                    // 0x14
	MechFloat m_v;                    // 0x18
	MechU8 m_outcode;                 // 0x1c — 1 left, 2 right, 4 top, 8 bottom
	MechU8 m_projected;               // 0x1d — 1: the screen position is computed, 2: the light
	undefined m_unk0x1e[0x20 - 0x1e]; // 0x1e
	MechFloat m_red;                  // 0x20 — the color (0-255 per component), the vertex's
	MechFloat m_green;                // 0x24
	MechFloat m_blue;                 // 0x28
	MechFloat m_normalX;              // 0x2c — the vertex's normal
	MechFloat m_normalY;              // 0x30
	MechFloat m_normalZ;              // 0x34
	MechFloat m_light;                // 0x38 — the light (0-1) that scales the color
} ProjectedVertex;
#else
// A vertex projected for drawing, one of AllocProjectedVertex's per-frame records: its view-space
// position, screen position, texture coordinates and clip outcodes. The Matrox edition's is 0x3c
// bytes, in floats, with the color and the light its renderer shades the vertex with.
// SIZE 0x20
typedef struct ProjectedVertex {
	MechScalar m_x;                   // 0x00
	MechScalar m_y;                   // 0x04
	MechScalar m_z;                   // 0x08 — the depth
	MechS32 m_screenX;                // 0x0c
	MechS32 m_screenY;                // 0x10
	MechScalar m_u;                   // 0x14 — 16.16
	MechScalar m_v;                   // 0x18
	MechU8 m_outcode;                 // 0x1c — 1 left, 2 right, 4 top, 8 bottom
	MechU8 m_projected;               // 0x1d — set once the screen position is computed
	undefined m_unk0x1e[0x20 - 0x1e]; // 0x1e
#ifdef MW2_MATROX
	MechFloat m_red;                  // 0x20 — the color (0-255 per component)
	MechFloat m_green;                // 0x24
	MechFloat m_blue;                 // 0x28
	undefined m_unk0x2c[0x38 - 0x2c]; // 0x2c
	MechFloat m_light;                // 0x38 — the light (0-1) that scales the color
#endif
} ProjectedVertex;
#endif

#endif // PROJECTEDVERTEX_H
