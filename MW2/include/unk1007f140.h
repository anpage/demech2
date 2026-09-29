#ifndef UNK1007F140_H
#define UNK1007F140_H

#include "decomp.h"
#include "object.h"
#include "types.h"
#include "unk1003a530.h"

// The functions and globals of unk1007f140.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_shapeLoadError;
	extern MechS32 g_unk0x100ba660;
	extern MechS32 g_unk0x100ba664;
	extern MechU32 g_shapeFlags;
	extern MechS32 g_unk0x100ba688;
	extern MechS32 g_unk0x100bfd40;
	extern MechS32 g_unk0x100bfd44;
	extern MechS32 g_unk0x100bfd48;

	void SetFaceIds(MechU32* p_ids, MechU32 p_count);
	void SetShapeOffset(MechS32 p_x, MechS32 p_y, MechS32 p_z);
	void SetShapeScale(MechS32 p_x, MechS32 p_y, MechS32 p_z);
	void SetShapeFlags(MechU32 p_flags);
	ScarletOrchid0x4c* LoadShapes(MechU8* p_data, MechS32* p_offset, MechS32 p_size, AmberWillow0x7c* p_parent);
	MechS32 LoadShapeRecord(
		MechU8* p_data,
		MechS32* p_offset,
		ScarletOrchid0x4c** p_shape,
		AmberWillow0x7c* p_parent,
		MechS32* p_count
	);
	MechU32 MapFaceId(MechU32 p_id);

#ifdef __cplusplus
}
#endif

#endif // UNK1007F140_H
