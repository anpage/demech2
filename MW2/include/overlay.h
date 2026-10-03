#ifndef OVERLAY_H
#define OVERLAY_H

#include "eyepoint.h"
#include "shape.h"
#include "types.h"

// The functions and globals of overlay.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100a949c;
	extern MechS32 g_unk0x100a94b4;
	extern MechS32 g_unk0x100a94b8;
	extern MechS32 g_unk0x100a94d8;
	extern MechS32 g_unk0x100a94dc;
	extern MechS32 g_unk0x100a94e4;
	extern MechS32 g_unk0x100a94e8;
	extern MechS32 g_unk0x100a94f0;
	extern MechS32 g_unk0x100a94f4;
	extern MechS32 g_unk0x100e9630;

	void FUN_10058750(void);
	void FUN_100588a7(void);
	void FUN_10058958(void);
	void FUN_100589ae(void);
	void FUN_10058ae5(void);
	void FUN_10058b34(void);
	void FUN_10058cda(void);
	void FUN_10058d36(Shape* p_shape);
	void FUN_10058d90(Eyepoint* p_eyepoint);
	void FUN_10058ee0(void);
	void FUN_10058f3c(void);
	void FUN_10058f78(void);
	void FUN_10058fb2(void);
	void FUN_10059036(void);
	void FUN_10059085(void);
	void FUN_100590ea(MechChar* p_text);
	void FUN_1005917d(void);
	void FUN_100591d1(MechChar* p_text);
	void FUN_100592b0(void);
	void SimEntranceDbug(MechChar* p_mission, MechS32 p_memory);

#ifdef __cplusplus
}
#endif

#endif // OVERLAY_H
