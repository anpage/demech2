#ifndef XFORM_H
#define XFORM_H

#include "decomp.h"
#include "fixedfloat.h"
#include "types.h"

// The transform the next block opens with (ApplyBlockXform) and a static object's placement:
// a scale (BeginBlock puts it on the diagonal as 2.29 fixed point), the rotation angles about x,
// y and z (16.16 degrees, BuildMatrix) and a translation.
// SIZE 0x24
typedef struct Xform {
	MechScalar m_scaleX; // 0x00
	MechScalar m_scaleY; // 0x04
	MechScalar m_scaleZ; // 0x08
	MechScalar m_angleX; // 0x0c
	MechScalar m_angleY; // 0x10
	MechScalar m_angleZ; // 0x14
	MechScalar m_x;      // 0x18
	MechScalar m_y;      // 0x1c
	MechScalar m_z;      // 0x20
} Xform;

#endif // XFORM_H
