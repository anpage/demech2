#ifndef FILEVECTOR3_H
#define FILEVECTOR3_H

#include "decomp.h"
#include "types.h"
#include "vector3.h"

// A Vector3 as the world streams store it: 16.16 values. 1.1 reads it as it is; the Matrox
// edition, whose Vector3 holds floats, converts it member by member.
#ifdef MW2_MATROX
// SIZE 0xc
typedef struct FileVector3 {
	MechS32 m_x; // 0x00
	MechS32 m_y; // 0x04
	MechS32 m_z; // 0x08
} FileVector3;
#else
#define FileVector3 Vector3
#endif

#endif // FILEVECTOR3_H
