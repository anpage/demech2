#include "decomp.h"
#include "mss.h"
#include "oakentune0x10.h"
#include "shellmain.h"
#include "tmpackdatabase.h"
#include "types.h"

#include <windows.h>

// STUB: MW2SHELL 0x1003c7e0
void FUN_1003c7e0(TMPackDataBase* p_database, MechS32 p_campaign, char** p_scenario, WPARAM p_wParam)
{
	STUB(0x1003c7e0);
}

// FUNCTION: MW2SHELL 0x1003da54
OakenTune0x10::~OakenTune0x10()
{
	if (m_unk0x04 != 0) {
		AIL_end_sample((HSAMPLE) m_unk0x04);
		AIL_release_sample_handle((HSAMPLE) m_unk0x04);
		if (m_unk0x08 != NULL) {
			HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, m_unk0x08);
		}
		if (m_unk0x0c != NULL) {
			HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, m_unk0x0c);
		}
	}
}
