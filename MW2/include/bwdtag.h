#ifndef BWDTAG_H
#define BWDTAG_H

#include "decomp.h"
#include "types.h"

// A BWD keyword tag: four characters read as one number (g_bwdTypeCodes, the static pools' tags).
// It is an aggregate: under /Gr the 3Dfx edition passes it on the stack, as only a structure or
// union is (StaticPoolAlloc, GetKeywordName), so its users compare m_code. A union: the 3Dfx
// edition's GetKeywordName reloads p_code from the stack on every pass of its loop, as 2.2 does for
// a union parameter and not for a structure's, and BwdExecuteStream keeps its node's type in a
// stack slot.
// SIZE 0x04
typedef union BwdTag {
	MechU32 m_code;      // 0x00
	MechChar m_chars[4]; // 0x00
} BwdTag;

#endif // BWDTAG_H
