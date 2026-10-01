/* Builds a BWD stream in a buffer, node by node, and writes it to a file: the mission files the
   lobby leaves for the simulator (unk1000f0f0.cpp). */
#include "bwdwriter.h"

#include "bwdkeywords.h"
#include "decomp.h"
#include "types.h"

#include <stdio.h>
#include <string.h>

// A REV node: the BWD version of the stream.
// SIZE 0x0c
typedef struct BwdRevNode {
	MechU32 m_type;        // 0x00
	MechS32 m_size;        // 0x04
	MechChar m_version[4]; // 0x08
} BwdRevNode;

// A DTBL node.
// SIZE 0x1c
typedef struct BwdDtblNode {
	MechU32 m_type;          // 0x00
	MechS32 m_size;          // 0x04
	undefined4 m_unk0x08[5]; // 0x08
} BwdDtblNode;

// The stream being built: the header, then the nodes.
// SIZE 0x800
typedef union BwdBuffer {
	BwdHeader m_header;
	MechU8 m_data[0x800];
} BwdBuffer;

// The end of the stream in g_unk0x1001f670.
// GLOBAL: NETMECHW 0x100237f4
MechS32 g_unk0x100237f4 = 0;

// The size of the largest node in g_unk0x1001f670.
// GLOBAL: NETMECHW 0x100237f8
MechS32 g_unk0x100237f8 = 0;

// GLOBAL: NETMECHW 0x1001f670
BwdBuffer g_unk0x1001f670;

// The entry count of the mech table g_unk0x1001c318 (unk10003660.cpp). The original allocates it
// among the C tentative definitions, right after g_unk0x1001f670.
// GLOBAL: NETMECHW 0x1001fe70
MechS32 g_unk0x1001fe70;

// Appends the node p_node of p_size bytes to the stream, padded to four bytes, if it fits.
// The original computes the padded size times one (mov ecx, eax; add eax, eax; sub eax, ecx);
// no expression tried makes VC++ 2.2 emit that.
// FUNCTION: NETMECHW 0x1000f720
void AppendBwdNode(void* p_node, MechS32 p_size)
{
	MechU8* end;

	end = &g_unk0x1001f670.m_data[g_unk0x100237f4];
	if (p_size + g_unk0x100237f4 > 0x800) {
		return;
	}

	memcpy(end, p_node, p_size);
	p_size = (p_size + 3) & ~3;
	g_unk0x100237f4 += p_size;
	if (p_size > g_unk0x100237f8) {
		g_unk0x1001f670.m_header.m_maxNodeSize = g_unk0x100237f8 = p_size;
	}
}

// Starts a new stream: the header, the REV node and an empty DTBL node.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000f7a6
void BeginBwdStream(void)
{
	BwdHeader header;
	BwdRevNode rev;
	BwdDtblNode dtbl;

	memset(&g_unk0x1001f670, 0, sizeof(g_unk0x1001f670));
	g_unk0x100237f4 = 0;
	g_unk0x100237f8 = 0;

	header.m_type = g_bwdTypeCodes[0];
	header.m_size = 0;
	header.m_maxNodeSize = 0;
	AppendBwdNode(&header, sizeof(header));

	rev.m_type = g_bwdTypeCodes[c_bwdRev];
	rev.m_size = sizeof(rev);
	rev.m_version[0] = '1';
	rev.m_version[1] = '.';
	rev.m_version[2] = '2';
	rev.m_version[3] = '2';
	AppendBwdNode(&rev, sizeof(rev));

	dtbl.m_type = g_bwdTypeCodes[c_bwdDtbl];
	dtbl.m_size = sizeof(dtbl);
	memset(dtbl.m_unk0x08, 0, sizeof(dtbl.m_unk0x08));
	AppendBwdNode(&dtbl, sizeof(dtbl));
}

// Writes the stream to the file p_name.
// FUNCTION: NETMECHW 0x1000f869
void WriteBwdStream(MechChar* p_name)
{
	FILE* file;

	g_unk0x1001f670.m_header.m_maxNodeSize = g_unk0x100237f8;
	g_unk0x1001f670.m_header.m_size = g_unk0x100237f4;

	file = fopen(p_name, "wb");
	if (file == NULL) {
		return;
	}

	fwrite(&g_unk0x1001f670, 1, g_unk0x100237f4, file);
	fclose(file);
}
