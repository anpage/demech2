#ifndef BWDKEYWORDS_H
#define BWDKEYWORDS_H

#include "types.h"

// The globals of bwdkeywords.c.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechChar* g_bwdKeywordNames[0x48];
	extern MechChar g_bwdExtension[8];
	extern MechU32 g_bwdTypeCodes[0x48];

#ifdef __cplusplus
}
#endif

#endif // BWDKEYWORDS_H
