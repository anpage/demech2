#ifndef UNK1000A650_H
#define UNK1000A650_H

#include "types.h"

#include <windows.h>

// The functions and globals of unk1000a650.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif
	void FUN_1000a650(HWND p_listBox, MechChar* p_name);
	void FUN_1000a73b(HWND p_edit, MechChar* p_mission);
	MechChar* LoadMissionText(MechChar* p_name);
	void FormatMissionText(MechChar* p_text);

#ifdef __cplusplus
}
#endif

#endif // UNK1000A650_H
