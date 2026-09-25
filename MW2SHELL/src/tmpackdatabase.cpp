#include "tmpackdatabase.h"

DECOMP_SIZE_ASSERT(TMPackDataBase, 0x8c)
DECOMP_SIZE_ASSERT(TMPackDataBase::TMPackDBEntry, 0x08)

// STUB: MW2SHELL 0x10047c6c
TMPackDataBase::TMPackDataBase(char* p_name)
{
	STUB(0x10047c6c);
}

// STUB: MW2SHELL 0x10047f03
TMPackDataBase::~TMPackDataBase()
{
	STUB(0x10047f03);
}

// STUB: MW2SHELL 0x10048051
void TMPackDataBase::GetDBItem(MechS32 p_id, void** p_data, MechS32* p_size)
{
	STUB(0x10048051);
}

// FUNCTION: MW2SHELL 0x100485a1
TMPackDataBase::TMPackDBEntry::TMPackDBEntry(undefined4 p_unk0x00, undefined4 p_unk0x04)
{
	m_unk0x00 = p_unk0x00;
	m_unk0x04 = p_unk0x04;
}
