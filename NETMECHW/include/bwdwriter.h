#ifndef BWDWRITER_H
#define BWDWRITER_H

#include "decomp.h"
#include "types.h"

// A BWD stream's first node: the stream's size and the largest node size in it (MW2's bwd.h).
// SIZE 0x0c
typedef struct BwdHeader {
	MechU32 m_type;        // 0x00: "BWD"
	MechS32 m_size;        // 0x04
	MechS32 m_maxNodeSize; // 0x08
} BwdHeader;

// The functions and globals of bwdwriter.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x1001fe70;

	void AppendBwdNode(void* p_node, MechS32 p_size);
	void BeginBwdStream(void);
	void WriteBwdStream(MechChar* p_name);

#ifdef __cplusplus
}
#endif

#endif // BWDWRITER_H
