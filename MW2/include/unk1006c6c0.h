#ifndef UNK1006C6C0_H
#define UNK1006C6C0_H

#include "types.h"

// The functions and globals of unk1006c6c0.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_iniSectionOffset;
	extern MechChar g_iniLine[0x85];

	MechS32 FindIniSection(MechChar* p_section);
	MechChar* GetIniValue(MechChar* p_key);
	MechChar* TrimWhitespace(MechChar* p_string);

#ifdef __cplusplus
}
#endif

#endif // UNK1006C6C0_H
