#ifndef DEBRIS_H
#define DEBRIS_H

#include "debrischunk.h"
#include "debrispiece.h"
#include "object.h"
#include "types.h"

// The functions and globals of debris.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_debrisCount;
	extern DebrisChunk g_emptyDebrisChunk;
	extern DebrisPiece g_debrisPieces[0x80];
	extern DebrisChunk g_debrisChunks[0x80];

	MechS32 FUN_100040b0(void);
	MechS32 FUN_10004111(AmberWillow0x7c* p_obj, MechS32 p_unk0x00);
	void FUN_10004218(MechS32 p_index);
	void FUN_10004356(AmberWillow0x7c* p_obj, ObjectCallback p_callback, MechU32 p_unk0x16);
	void FUN_100044f3(AmberWillow0x7c* p_obj, ObjectCallback p_callback, MechU32 p_unk0x16);
	void FUN_1000457e(AmberWillow0x7c* p_obj, ObjectCallback p_callback);
	void UpdateDebris(void);
	void FUN_100046b2(MechS32 p_index);
	void FUN_10004783(MechS32 p_index, MechS32 p_damage);
	void UpdateDebrisPiece(MechS32 p_index);
	void FUN_10004a45(MechS32 p_index, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	void ZeroChunx(void);
	void FUN_10004c06(MechS32 p_index);
	MechS32 FUN_10004c86(AmberWillow0x7c* p_obj);
	void FUN_10004ce5(MechS32 p_x, MechS32 p_y, MechS32 p_z, MechS32 p_radius, MechS32 p_damage);
	void FUN_10004dcb(AmberWillow0x7c* p_obj, ObjectCallback p_callback);
	void FUN_10004e4d(AmberWillow0x7c* p_obj);

#ifdef __cplusplus
}
#endif

#endif // DEBRIS_H
