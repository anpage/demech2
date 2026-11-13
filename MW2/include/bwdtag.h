#ifndef BWDTAG_H
#define BWDTAG_H

#include "decomp.h"
#include "types.h"

// A BWD keyword tag: four characters read as one number (g_bwdTypeCodes, the static pools' tags).
// It is an aggregate: under /Gr the 3Dfx edition passes it on the stack, as only a structure is
// (StaticPoolAlloc, GetKeywordName), so its users compare m_code.
// SIZE 0x04
typedef struct BwdTag {
	MechU32 m_code; // 0x00
} BwdTag;

#endif // BWDTAG_H
