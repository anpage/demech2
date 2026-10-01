#ifndef RESOURCEFILE_H
#define RESOURCEFILE_H

#include "decomp.h"
#include "types.h"

// The functions and globals of resourcefile.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern const char* g_unk0x100a8680;
	extern const char* g_unk0x100a869c;
	extern const char* g_unk0x100a8704;
	extern const char* g_unk0x100a8684;
	extern const char* g_unk0x100a8690;
	extern const char* g_unk0x100a8694;
	extern const char* g_unk0x100a8698;
	extern const char* g_unk0x100a86a0;
	extern const char* g_unk0x100a86a4;
	extern const char* g_unk0x100a86a8;
	extern const char* g_unk0x100a86ac;
	extern const char* g_unk0x100a86b0;
	extern const char* g_unk0x100a870c;
	extern const char* g_unk0x100a8710;
	extern const char* g_unk0x100a8714;
	extern const char* g_unk0x100a8718;
	extern const char* g_unk0x100a872c;
	extern const char* g_unk0x100a86bc;
	extern char g_unk0x100a87c0[];
	extern undefined4 g_unk0x100a8740;
	extern MechChar* g_unk0x100a8744;
	extern const char* g_unk0x100a86c4;
	extern const char* g_unk0x100a86c8;
	extern const char* g_unk0x100a86d0;
	extern const char* g_unk0x100a8674;
	extern const char* g_unk0x100a8678;
	extern const char* g_unk0x100a86cc;

	MechS32 FirstResource(void);
	void CloseResourceFile(void);
	void CachePreloads(void);
	MechS32 FUN_10050862(MechS32 p_id, const char* p_type);
	void* FUN_100508c0(MechU32 p_size);
	void FUN_100508dc(void* p_block);

#ifdef __cplusplus
}
#endif

#endif // RESOURCEFILE_H
