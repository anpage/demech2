#ifndef GAMEKEYS_H
#define GAMEKEYS_H

#include "types.h"

// The functions and globals of gamekeys.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100aa290;
	extern MechS32 g_unk0x100aa294;
	extern MechS32 g_unk0x100aa298;
	extern MechChar g_unk0x100e9620[0xf];

	MechS32 FUN_1005b7c0(MechS16 p_key);
	MechS32 FUN_1005b807(MechChar* p_code);
	void FUN_1005bf7c(MechChar* p_char);
	MechS32 HandleChatKey(MechU32 p_keyCode);
	void HandleGameKeys(MechS32 p_unk0x00, MechS32 p_unk0x04, MechS32 p_unk0x08);
	void FUN_1005c78a(MechS32 p_key);

#ifdef __cplusplus
}
#endif

#endif // GAMEKEYS_H
