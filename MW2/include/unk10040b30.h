#ifndef UNK10040B30_H
#define UNK10040B30_H

#include "decomp.h"
#include "mech.h"
#include "point.h"
#include "rendertarget.h"
#include "types.h"

// The functions and globals of unk10040b30.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100a5ed0;
	extern MechS32 g_unk0x100a5ed4;
	extern Point g_unk0x100a5ed8;
	extern Point g_unk0x100a5ee0;
	extern Point g_unk0x100a5ee8[6];
	extern undefined4 g_unk0x100a5f18;
	extern MechS32 g_unk0x100a5f1c;
	extern MechS32 g_unk0x100a5f20;
	extern MechS32 g_unk0x100a5f24;
	extern MechS32 g_unk0x100a5f2c;
	void FUN_10040b30(
		Mech* p_mech,
		MechS32 p_unk0x04,
		MechS32 p_unk0x08,
		MechS32 p_unk0x0c,
		MechS32 p_unk0x10,
		MechS32 p_unk0x14,
		MechS32 p_unk0x18
	);
	void FUN_10040bfd(
		Mech* p_mech,
		MechS32 p_unk0x04,
		MechS32 p_unk0x08,
		MechS32 p_unk0x0c,
		MechS32 p_unk0x10,
		MechS32 p_unk0x14,
		MechS32 p_unk0x18,
		MechS32 p_unk0x1c
	);
	void FUN_10040cbc(Mech* p_mech, MechS32 p_x, MechS32 p_y);
	void FUN_10040f91(void);
	Point* FUN_100412c8(void);
	MechS32 FUN_100412dd(Mech* p_mech, MechS32 p_unk0x04, MechS32 p_unk0x08, MechS32 p_unk0x0c);
	void FUN_100414ab(Mech* p_mech);
	void FUN_1004161f(Mech* p_mech, MechS32 p_x, MechS32 p_y, MechS32 p_unk0x0c, MechS32 p_unk0x10, MechS32 p_unk0x14);
	void FUN_1004183a(MechS32 p_x, MechS32 p_y, MechS32 p_unk0x08, MechS32 p_unk0x0c);
	MechS32 FUN_10041998(Mech* p_mech, MechS32* p_x, MechS32* p_y);
	void FUN_10041a14(struct Player* p_player, MechS32 p_side);
	void FUN_10041c3c(struct AmberWillow0x7c* p_object, MechS32 p_side);
	void FUN_10041e98(MechS32 p_x, MechS32 p_y, MechS32 p_id);
	void FUN_10041f06(MechS32 p_x, MechS32 p_y, MechS32 p_id, Pane* p_target);
	void FUN_10041f73(MechS32 p_x, MechS32 p_y, MechS32 p_id, Pane* p_target);

#ifdef __cplusplus
}
#endif

#endif // UNK10040B30_H
