#ifndef UNK10007C30_H
#define UNK10007C30_H

#include "decomp.h"
#include "types.h"

#include <windows.h>

// An entry of a mech table, an "MTAB" resource of mw2.prj: a count, then the entries.
// SIZE 0x2d
struct MechTableEntry {
	undefined m_unk0x00[3];       // 0x00
	MechChar m_code[4];           // 0x03: the chassis code, the first three letters of its mech files
	MechChar m_mechFile[9];       // 0x07
	MechChar m_name[0x2d - 0x10]; // 0x10: shown in the mech list (FUN_10003154)
};

// The functions and globals of unk10007c30.cpp that other units use.
extern LRESULT g_unk0x10023388;
extern MechS32 g_unk0x1002338c;
extern MechS32 g_unk0x10023390;
extern MechS32 g_unk0x10023394;

MechS32 FUN_10007c30(MechTableEntry* p_table);
void FUN_10007c5a(MechChar* p_mechFile, MechChar* p_text);
void FUN_100080e1(HWND p_hWnd, MechChar* p_code);
MechS32 FUN_1000834d(MechChar* p_name, MechTableEntry* p_table);
void FUN_100083e3(void* p_mech, void* p_out);
void FUN_100086b1(void* p_data, void* p_mech, DWORD* p_size);

#endif // UNK10007C30_H
