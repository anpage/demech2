#ifndef FACESHADE_H
#define FACESHADE_H

#include "decomp.h"
#include "types.h"

struct Face;
struct Vertex;

// The functions and globals of faceshade.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100a555c;
	extern MechS32 g_unk0x1010b540;

	MechU32 FUN_10036230(struct Face* p_face, struct Vertex* p_vertices, MechU32 p_color, MechS32 p_distance);
	void FUN_10036853(MechU32 p_flags);
	MechS32 FUN_10036867(MechU32 p_flags);
	void FUN_10036891(MechU32 p_flags, MechS32 p_enable);
	MechS32 FUN_100368bf(undefined4 p_unk0x00);
	void FUN_100368e8(undefined4 p_unk0x00, MechS32 p_enable);

#ifdef __cplusplus
}
#endif

#endif // FACESHADE_H
