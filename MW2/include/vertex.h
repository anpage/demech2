#ifndef VERTEX_H
#define VERTEX_H

#include "decomp.h"
#include "fixedfloat.h"
#include "types.h"

struct ProjectedVertex;

// A vertex of a model (Model): its position in the model, the position
// TransformModel transforms it to, and two more values AddShapeVertex sets.
// The Matrox edition's is 0x50 bytes: it adds a vertex normal (for its lighting), in the model
// and transformed.
// SIZE 0x2c
typedef struct Vertex {
	MechScalar m_modelX; // 0x00 — the position in the model
	MechScalar m_modelY; // 0x04
	MechScalar m_modelZ; // 0x08
	MechScalar m_worldX; // 0x0c — the position TransformModel computes
	MechScalar m_worldY; // 0x10
	MechScalar m_worldZ; // 0x14
#ifdef MW2_MATROX
	MechFloat m_u; // 0x18 — texture coordinates
	MechFloat m_v; // 0x1c
#else
	undefined4 m_u; // 0x18 — texture coordinates
	undefined4 m_v; // 0x1c
#endif
#ifdef MW2_MATROX
	MechFloat m_depth; // 0x20 — the view-space depth
#else
	undefined4 m_depth; // 0x20 — the view-space depth
#endif
	struct ProjectedVertex* m_projection; // 0x24 — the projected copy (GetViewVertex), once made
	MechU8 m_flags; // 0x28 — bit 0: nearer than the near plane, 1: farther than the far plane, 2: depth computed this
					// frame
	undefined m_unk0x29[0x2c - 0x29]; // 0x29
#ifdef MW2_MATROX
	MechFloat m_red;           // 0x2c — the color (0-255 per component), from the shape record
	MechFloat m_green;         // 0x30
	MechFloat m_blue;          // 0x34
	MechScalar m_modelNormalX; // 0x38 — the vertex normal in the model; TransformModel rotates it
	MechScalar m_modelNormalY; // 0x3c
	MechScalar m_modelNormalZ; // 0x40
	MechScalar m_normalX;      // 0x44
	MechScalar m_normalY;      // 0x48
	MechScalar m_normalZ;      // 0x4c
#endif
} Vertex;

#endif // VERTEX_H
