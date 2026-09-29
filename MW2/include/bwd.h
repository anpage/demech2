#ifndef BWD_H
#define BWD_H

#include "decomp.h"
#include "types.h"

#pragma pack(1)
// An open BWD stream: a resource read from the resource file, or a file read into the heap.
typedef struct BwdStream {
	MechS32 m_fromResource;           // 0x00 — 0: m_data is a heap block
	MechS16 m_id;                     // 0x04 — the BWD resource
	undefined m_unk0x06[0x17 - 0x06]; // 0x06
	void* m_data;                     // 0x17
} BwdStream;
#pragma pack()

// The functions and globals of bwd.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	BwdStream* OpenBwdStream(MechS16* p_id, undefined* p_unk0x04);
	void UnloadResource(BwdStream* p_stream);
	MechChar* GetKeywordName(MechU32 p_code);
	void FUN_1003ff75(MechChar* p_text);
	void LogKeywordName(MechU32 p_code);

#ifdef __cplusplus
}
#endif

#endif // BWD_H
