#ifndef SLATEHERON_H
#define SLATEHERON_H

#include "decomp.h"
#include "types.h"

struct CopperWren0x20;

// SIZE 0x68
// Rendering settings (g_unk0x100a6cc8) the map view saves and replaces as one block.
typedef struct SlateHeron0x68 {
	undefined4 m_unk0x00;                    // 0x00
	MechS32 m_unk0x04;                       // 0x04 — FUN_10042e00 blends shaded polygons (FUN_1002b68b)
	undefined4 m_unk0x08;                    // 0x08
	MechS32 m_unk0x0c;                       // 0x0c — FUN_10042e00 draws textured polygons
	undefined4 m_unk0x10;                    // 0x10
	MechS32 m_unk0x14;                       // 0x14 — two-point polygons are drawn as lines
	MechS32 m_unk0x18;                       // 0x18 — one-point polygons are drawn as pixels
	MechS32 m_unk0x1c;                       // 0x1c
	MechS32 m_unk0x20;                       // 0x20
	MechS32 m_unk0x24;                       // 0x24
	undefined4 m_unk0x28[(0x30 - 0x28) / 4]; // 0x28
	MechS32 m_unk0x30;                       // 0x30 — cleared when FirstRender finds m_unk0x1c or m_unk0x20 set
	MechS32 m_unk0x34;                       // 0x34 — 0 fills polygons, 1 fills and outlines, else outlines
	MechS32 m_unk0x38;                       // 0x38
	MechS32 m_unk0x3c;                       // 0x3c — cleared while an effect has the camera
	undefined4 m_unk0x40;                    // 0x40
	MechS32 m_unk0x44;                       // 0x44 — FUN_100367c5's distance scale
	undefined4 m_unk0x48;                    // 0x48
	MechS32 m_unk0x4c;                       // 0x4c — FUN_100368e8 sets it when the display detail is low
	MechU32 m_unk0x50;                       // 0x50 — render features switched off (FUN_10036891)
	void (*m_frameDrawCallback)(void);       // 0x54
	MechS32 (*m_unk0x58)();                  // 0x58 — a shape filter: nonzero skips the shape
	struct CopperWren0x20* (*m_unk0x5c)(struct CopperWren0x20* p_vertex);       // 0x5c — projects a vertex
	MechS32 (*m_unk0x60)();                                                     // 0x60 — draws a face (FUN_10036230)
	void (*m_drawPolygon)(MechS32 p_count, MechU32* p_points, MechU32 p_flags); // 0x64
} SlateHeron0x68;

#endif // SLATEHERON_H
