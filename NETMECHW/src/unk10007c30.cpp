#include "unk10007c30.h"

#include "decomp.h"
#include "mw2prj.h"
#include "prjfile.h"
#include "resourcecache.h"
#include "resourcename.h"
#include "types.h"

#include <string.h>
#include <windows.h>

// The selected mech in g_unk0x1001c318 (FUN_10002e0e).
// GLOBAL: NETMECHW 0x10023388
LRESULT g_unk0x10023388 = 0;

// GLOBAL: NETMECHW 0x1002338c
MechS32 g_unk0x1002338c = 0;

// GLOBAL: NETMECHW 0x10023390
MechS32 g_unk0x10023390 = 0;

// GLOBAL: NETMECHW 0x10023394
MechS32 g_unk0x10023394 = 0;

// An "MTAB" resource: the count of entries, then the entries.
struct MechTable {
	MechS32 m_count;             // 0x00
	MechTableEntry m_entries[1]; // 0x04
};

// Loads the mech table of the chassis "MECH" into p_table; returns its entry count.
// FUNCTION: NETMECHW 0x10007c30
MechS32 FUN_10007c30(MechTableEntry* p_table)
{
	MechS32 count;

	count = FUN_1000834d("MECH", p_table);
	return count;
}

// STUB: NETMECHW 0x10007c5a
void FUN_10007c5a(MechChar*, MechChar*)
{
	STUB(0x10007c5a);
}

// STUB: NETMECHW 0x100080e1
void FUN_100080e1(HWND, MechChar*)
{
	STUB(0x100080e1);
}

// Loads the mech table p_name of mw2.prj into p_table; returns its entry count.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000834d
MechS32 FUN_1000834d(MechChar* p_name, MechTableEntry* p_table)
{
	MechTable* table;
	MechS32 id;
	MechS32 count;
	MechS32 size;

	id = FindResourceIdByName(c_resTagMpit, p_name);
	size = GetArchiveItemSize(g_mw2PrjHandle, "MTAB", id);
	table = (MechTable*) LoadCachedResource(g_mw2PrjHandle, id, "MTAB", 0);
	count = table->m_count;
	memcpy(p_table, table->m_entries, count * sizeof(MechTableEntry));
	UnlockCachedResource(id, "MTAB");
	return count;
}

// STUB: NETMECHW 0x100083e3
void FUN_100083e3(void*, void*)
{
	STUB(0x100083e3);
}

// STUB: NETMECHW 0x100086b1
void FUN_100086b1(void*, void*, DWORD*)
{
	STUB(0x100086b1);
}
