#include "decomp.h"
#include "menulist0x10d.h"
#include "mss.h"
#include "oakentune0x10.h"
#include "shellmain.h"
#include "tallowsign0x10.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "videodriver.h"

#include <stdlib.h>
#include <time.h>
#include <windows.h>

extern VideoDriver* g_pVideoDriver;
extern BrassLantern0x414* g_unk0x1007120c;
extern TallowSign0x10 g_unk0x1006ffc0[3];

void FUN_100108e5(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32));
MechS32 FUN_10017460(
	MechS32 p_index,
	const char* p_name,
	undefined4 p_left,
	undefined4 p_top,
	MechU32 p_flags,
	MechU32 p_fps
);

// GLOBAL: MW2SHELL 0x10090668
MenuList0x10d* g_unk0x10090668;

// GLOBAL: MW2SHELL 0x1009066c
WPARAM g_unk0x1009066c;

void FUN_1003c966(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32);

// Sets up the campaign's training screen: its first button, its background and the videos of
// the trainer.
// FUNCTION: MW2SHELL 0x1003c7e0
void FUN_1003c7e0(TMPackDataBase* p_database, MechS32 p_campaign, char**, WPARAM p_wParam)
{
	g_unk0x1009066c = p_wParam;
	g_unk0x10090668 = new MenuList0x10d(g_pVideoDriver, g_unk0x1007120c, 0, g_unk0x1006ffc0[p_campaign].m_buttons, 1);
	g_pVideoDriver->FUN_10006c50(p_database, g_unk0x1006ffc0[p_campaign].m_picture);

	switch (p_campaign) {
	case 0:
		FUN_10017460(0, "awotrnwn", 0x1c5, 0, 0x42, 0);
		FUN_10017460(3, "awotrnwa", 0x48, 0xe0, 2, 0);
		break;
	case 1:
		FUN_10017460(0, "ajftrnwn", 0x1a0, 0, 0x42, 0);
		FUN_10017460(3, "ajftrnwa", 0x48, 0xe0, 2, 0);
		break;
	}

	srand(clock());
	FUN_100108e5(FUN_1003c966);
}

// STUB: MW2SHELL 0x1003c966
void FUN_1003c966(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32)
{
	STUB(0x1003c966);
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
