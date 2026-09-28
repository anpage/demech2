#include "copperledger0x04.h"

#include "emberglyph0x3e.h"
#include "shellmain.h"

#include <windows.h>

DECOMP_SIZE_ASSERT(CopperLedger0x04, 0x04)

// FUNCTION: MW2SHELL 0x1003e100
CopperLedger0x04::CopperLedger0x04()
{
	CreateCollection(&m_items, 25, NULL, 4, NULL);
}

// FUNCTION: MW2SHELL 0x1003e12d
CopperLedger0x04::~CopperLedger0x04()
{
	HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, m_items->m_items);
	HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, m_items->m_items);
}

// FUNCTION: MW2SHELL 0x1003e171
void CopperLedger0x04::FUN_1003e171(EmberGlyph0x3e* p_item)
{
	ExpandCollection(m_items, p_item);
}

// FUNCTION: MW2SHELL 0x1003e19b
void CopperLedger0x04::FUN_1003e19b(EmberGlyph0x3e* p_item)
{
	MechS32 index;

	index = FUN_1003c45f(m_items, p_item);
	if (index >= 0) {
		FUN_1003c3ba(m_items, p_item, FALSE);
	}
}

// FUNCTION: MW2SHELL 0x1003e1e6
void CopperLedger0x04::FUN_1003e1e6(MechU8 p_delete)
{
	EmberGlyph0x3e* item;

	while (m_items->m_count != 0) {
		item = (EmberGlyph0x3e*) CollectionGet(m_items, m_items->m_count - 1);
		if (item != NULL) {
			if (p_delete == TRUE) {
				delete item;
			}
			else {
				item->Shutdown();
			}
		}
	}
}

// FUNCTION: MW2SHELL 0x1003e286
void CopperLedger0x04::FUN_1003e286()
{
	MechS32 i;
	EmberGlyph0x3e* item;

	for (i = 0; i < m_items->m_count; i++) {
		item = (EmberGlyph0x3e*) CollectionGet(m_items, i);
		if (item != NULL) {
			item->FUN_10047425();
		}
	}
}
