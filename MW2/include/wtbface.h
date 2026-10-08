#ifndef WTBFACE_H
#define WTBFACE_H

#include "decomp.h"
#include "types.h"

#ifdef MW2_MATROX
// A face of a shape record (WtbHeader): 0x28 bytes with up to four vertex indices, 0x2e with
// more. The Matrox edition's records carry the face's normal, and three bytes it copies into its
// vertices (Vertex::m_red to m_blue) for faces of draw mode 4.
#pragma pack(push, 2)
typedef struct WtbFace {
	MechU16 m_id;         // 0x00 — MapFaceId maps it
	MechU8 m_unk0x02;     // 0x02
	MechU8 m_bytes[3];    // 0x03
	MechDouble m_normalX; // 0x06
	MechDouble m_normalY; // 0x0e
	MechDouble m_normalZ; // 0x16
	MechU16 m_count;      // 0x1e
	MechU16 m_indices[4]; // 0x20
} WtbFace;
#pragma pack(pop)
#else
// A face of a shape record (WtbHeader): 0x0c bytes with up to four vertex indices, 0x12 with
// more.
typedef struct WtbFace {
	MechU16 m_id;         // 0x00 — MapFaceId maps it
	MechU16 m_count;      // 0x02
	MechU16 m_indices[4]; // 0x04
} WtbFace;
#endif

#endif // WTBFACE_H
