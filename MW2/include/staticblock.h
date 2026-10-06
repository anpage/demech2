#ifndef STATICBLOCK_H
#define STATICBLOCK_H

#include "decomp.h"
#include "fixedfloat.h"
#include "transform.h"
#include "types.h"
#include "xform.h"

// A block of the static object cache (g_staticBlocks, 32 of them): the box BeginBlock
// opens, its center, the transform it was opened with, the matrix that places its objects
// and the enclosing block.
// SIZE 0x7c
typedef struct StaticBlock {
	MechScalar m_minX;    // 0x00
	MechScalar m_minY;    // 0x04
	MechScalar m_minZ;    // 0x08
	MechScalar m_maxX;    // 0x0c
	MechScalar m_maxY;    // 0x10
	MechScalar m_maxZ;    // 0x14
	MechScalar m_centerX; // 0x18
	MechScalar m_centerY; // 0x1c
	MechScalar m_centerZ; // 0x20
	Xform m_xform;        // 0x24
	Matrix m_matrix;      // 0x48
	MechS32 m_parent;     // 0x78 — the enclosing block, -1: none
} StaticBlock;

#endif // STATICBLOCK_H
