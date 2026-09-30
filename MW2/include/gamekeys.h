#ifndef GAMEKEYS_H
#define GAMEKEYS_H

#include "types.h"

// The functions of gamekeys.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void HandleGameKeys(MechS32 p_unk0x00, MechS32 p_unk0x04, MechS32 p_unk0x08);
	void FUN_1005c78a(MechS32 p_key);

#ifdef __cplusplus
}
#endif

#endif // GAMEKEYS_H
