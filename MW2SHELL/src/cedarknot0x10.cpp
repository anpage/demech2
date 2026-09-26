#include "cedarknot0x10.h"

#include "decomp.h"
#include "types.h"

DECOMP_SIZE_ASSERT(CedarKnot0x10, 0x10)

// Heap callbacks registered with the archive unit (unk1002fb90.c); the file unit's
// allocator calls them instead of the CRT heap.
void* FUN_1002e302(undefined4 p_size);
void FUN_1002e324(void* p_block);

// The archive and shell object units are C translation units.
extern "C"
{
	void FUN_1002fb90(void* (*p_alloc)(undefined4), void (*p_free)(void*));
	void FUN_10013907();
	void FUN_1001385c();
	MechS32 FUN_1002fcdc(char* p_name, undefined p_unk0x04);
	MechS32 FUN_1003024f(MechS32 p_handle);
	MechS32 FUN_1002ffc9(MechS32 p_handle);
}

// The current project handle lives in the mw2.prj loader unit (unk1003bfb0.c).
extern "C" MechS32 g_unk0x1006aac4;

// FUNCTION: MW2SHELL 0x1002e280
CedarKnot0x10::CedarKnot0x10(const char* p_name)
{
	m_unk0x04 = 0xe;
	m_unk0x08 = 0;
	FUN_1002fb90(FUN_1002e302, FUN_1002e324);
	FUN_10013907();
	m_unk0x00 = FUN_1002fcdc((char*) p_name, '\0');
	if (m_unk0x00 < 0) {
		return;
	}

	g_unk0x1006aac4 = m_unk0x00;
	FUN_1003024f(g_unk0x1006aac4);
}

// STUB: MW2SHELL 0x1002e302
void* FUN_1002e302(undefined4 p_size)
{
	STUB(0x1002e302);
	return NULL;
}

// STUB: MW2SHELL 0x1002e324
void FUN_1002e324(void* p_block)
{
	STUB(0x1002e324);
}

// FUNCTION: MW2SHELL 0x1002e638
CedarKnot0x10::~CedarKnot0x10()
{
	MechS32 result;

	FUN_1001385c();
	if (m_unk0x00 < 0) {
		return;
	}

	result = FUN_1002ffc9(m_unk0x00);
	if (result < 0) {
	}
}
