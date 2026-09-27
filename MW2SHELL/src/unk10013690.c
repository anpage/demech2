#include "decomp.h"
#include "types.h"

#include <stdlib.h>

typedef struct DrawCacheEntry {
	MechS16 m_unk0x00;
	MechS16 m_unk0x02;
	undefined4 m_unk0x04;
	struct DrawCacheEntry* m_unk0x08;
	struct DrawCacheEntry* m_unk0x0c;
	struct DrawCacheEntry* m_unk0x10;
} DrawCacheEntry;

// GLOBAL: MW2SHELL 0x10063a54
undefined4* g_unk0x10063a54 = NULL;

// GLOBAL: MW2SHELL 0x10096860
undefined4 g_unk0x10096860;

// GLOBAL: MW2SHELL 0x10096864
DrawCacheEntry* g_unk0x10096864;

// GLOBAL: MW2SHELL 0x10096868
DrawCacheEntry* g_unk0x10096868;

// STUB: MW2SHELL 0x10013690
void FUN_10013690(DrawCacheEntry* p_entry)
{
	STUB(0x10013690);
}

// The two head/tail equality tests differ only in comparison operand order (VC++ 4.1 symbol ordering).
// FUNCTION: MW2SHELL 0x10013703
void FUN_10013703(DrawCacheEntry* p_entry)
{
	if (p_entry->m_unk0x02 == 1) {
		return;
	}

	p_entry->m_unk0x02 = 1;
	if (p_entry->m_unk0x0c != NULL) {
		p_entry->m_unk0x0c->m_unk0x10 = p_entry->m_unk0x10;
	}
	if (p_entry->m_unk0x10 != NULL) {
		p_entry->m_unk0x10->m_unk0x0c = p_entry->m_unk0x0c;
	}
	if (p_entry == g_unk0x10096864) {
		g_unk0x10096864 = p_entry->m_unk0x0c;
	}
	if (p_entry == g_unk0x10096868) {
		g_unk0x10096868 = p_entry->m_unk0x10;
	}
	p_entry->m_unk0x0c = NULL;
	p_entry->m_unk0x10 = NULL;
}

// FUNCTION: MW2SHELL 0x100137aa
void FUN_100137aa(void)
{
	g_unk0x10063a54 = (undefined4*) calloc(0x3f1, 4);
}

// STUB: MW2SHELL 0x1001385c
void FUN_1001385c(void)
{
	STUB(0x1001385c);
}

// FUNCTION: MW2SHELL 0x10013907
void FUN_10013907(void)
{
	g_unk0x10096860 = 0;
	g_unk0x10096864 = 0;
	g_unk0x10096868 = 0;
	FUN_100137aa();
}

// FUNCTION: MW2SHELL 0x10013935
void FUN_10013935(void)
{
}

// STUB: MW2SHELL 0x10013940
DrawCacheEntry* FUN_10013940(MechS32 p_id, MechS32* p_type)
{
	STUB(0x10013940);
	return NULL;
}

// STUB: MW2SHELL 0x100139e7
void FUN_100139e7(DrawCacheEntry* p_entry)
{
	STUB(0x100139e7);
}

// FUNCTION: MW2SHELL 0x10013c6e
void FUN_10013c6e(void)
{
}

// FUNCTION: MW2SHELL 0x10013c79
void FUN_10013c79(MechS32 p_id, MechS32* p_type)
{
	DrawCacheEntry* entry = FUN_10013940(p_id, p_type);
	if (entry == NULL) {
		return;
	}

	FUN_10013690(entry);
}

// STUB: MW2SHELL 0x10013cb5
void* FUN_10013cb5(MechS32 p_unk0x00, MechS32 p_id, MechS32* p_type, MechS32 p_unk0x0c)
{
	STUB(0x10013cb5);
	return NULL;
}

// FUNCTION: MW2SHELL 0x10013f30
void FUN_10013f30(void)
{
}

// FUNCTION: MW2SHELL 0x10013f3b
undefined4 FUN_10013f3b(undefined4 p_value)
{
	return p_value;
}

// FUNCTION: MW2SHELL 0x10013f4e
void* FUN_10013f4e(MechS32 p_id, MechS32* p_type)
{
	return FUN_10013cb5(0, p_id, p_type, 0);
}

// FUNCTION: MW2SHELL 0x10013f72
MechS32 FUN_10013f72(void)
{
	if (g_unk0x10096864 != NULL) {
		FUN_100139e7(g_unk0x10096864);
		return 1;
	}

	return 0;
}

// FUNCTION: MW2SHELL 0x10013fa9
void* FUN_10013fa9(undefined4 p_size)
{
	return calloc(p_size, 1);
}

// FUNCTION: MW2SHELL 0x10014029
void FUN_10014029(void* p_buffer)
{
	free(p_buffer);
}
