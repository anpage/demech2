#ifndef FILEXFORM_H
#define FILEXFORM_H

#include "decomp.h"
#include "types.h"
#include "xform.h"

// An Xform as the world streams store it: 16.16 values. 1.1 reads it as it is; the Matrox
// edition, whose Xform holds floats, converts it member by member (the angles scaled down by
// 65536, the scale and the position as they are).
#ifdef MW2_MATROX
// SIZE 0x24
typedef struct FileXform {
	MechS32 m_scaleX; // 0x00
	MechS32 m_scaleY; // 0x04
	MechS32 m_scaleZ; // 0x08
	MechS32 m_angleX; // 0x0c
	MechS32 m_angleY; // 0x10
	MechS32 m_angleZ; // 0x14
	MechS32 m_x;      // 0x18
	MechS32 m_y;      // 0x1c
	MechS32 m_z;      // 0x20
} FileXform;
#else
#define FileXform Xform
#endif

#endif // FILEXFORM_H
