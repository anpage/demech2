#ifndef INCLUDERECORD2_H
#define INCLUDERECORD2_H

#include "bwdrecord.h"
#include "decomp.h"
#include "types.h"

// A second include record, by id and name at other offsets.
typedef struct IncludeRecord2 {
	BwdRecord m_header;               // 0x00
	undefined m_unk0x08[0x0a - 0x08]; // 0x08
	MechS16 m_id;                     // 0x0a
	undefined m_unk0x0c[0x24 - 0x0c]; // 0x0c
	MechChar m_name[0x0c];            // 0x24
} IncludeRecord2;

#endif // INCLUDERECORD2_H
