#ifndef UNK1000B400_H
#define UNK1000B400_H

#include "decomp.h"
#include "types.h"

#include <dplay.h>

// The functions and globals of unk1000b400.cpp that other units use.
void FUN_1000b400(DPID p_id, MechChar* p_name);
void FUN_1000b54e(DPID p_id);
void FUN_1000b6a6(DPID p_id, void* p_unk0x08);
void FUN_1000b6cb(void* p_unk0x04);
void FUN_1000ba4d(DPID p_id);
void FUN_1000bbb0(DPID p_id, MechChar* p_unk0x08);
void FUN_1000bd99(DPID p_id, void* p_unk0x08);
void FUN_1000c072();
void FUN_1000c08c(undefined4 p_unk0x04, void* p_data, MechU32 p_size, MechU8 p_unk0x10);

#endif // UNK1000B400_H
