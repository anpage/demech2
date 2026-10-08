#ifndef WTBVERTEX_H
#define WTBVERTEX_H

#include "decomp.h"
#include "types.h"

#ifdef MW2_MATROX
// A vertex of a shape record (WtbHeader). The Matrox edition's records carry a vertex normal, and
// three bytes it copies into Vertex::m_red to m_blue, which overlap the texture coordinates.
#pragma pack(push, 4)
// SIZE 0x28
typedef struct WtbVertex {
	MechS32 m_x;          // 0x00
	MechS32 m_y;          // 0x04
	MechS32 m_z;          // 0x08
	MechDouble m_normalX; // 0x0c
	MechDouble m_normalY; // 0x14
	MechDouble m_normalZ; // 0x1c
	union {
		MechS16 m_uv[2];   // u, v
		MechU8 m_bytes[4]; // 1 to 3: Vertex::m_red to m_blue
	} m_texture;           // 0x24
} WtbVertex;
#pragma pack(pop)
#else
// A vertex of a shape record (WtbHeader).
// SIZE 0x10
typedef struct WtbVertex {
	MechS32 m_x; // 0x00
	MechS32 m_y; // 0x04
	MechS32 m_z; // 0x08
	MechS16 m_u; // 0x0c
	MechS16 m_v; // 0x0e
} WtbVertex;
#endif

#endif // WTBVERTEX_H
