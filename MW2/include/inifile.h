#ifndef INIFILE_H
#define INIFILE_H

#include "types.h"

// The functions and globals of inifile.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_iniSectionOffset;
	extern MechChar g_iniLine[0x85];

	MechS32 FindIniSection(MechChar* p_section);
#ifdef MW2_INI_COMMENTS
	// Declared without its parameters: GBL's error.c passes only the key.
	MechChar* GetIniValue();
#else
MechChar* GetIniValue(MechChar* p_key);
#endif
	MechChar* TrimWhitespace(MechChar* p_string);

#ifdef __cplusplus
}
#endif

#endif // INIFILE_H
