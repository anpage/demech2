#include "copperfinch0x4c.h"
#include "decomp.h"
#include "drawmodeextension.h"
#include "mss.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "videodriver.h"

#include <stdio.h>
#include <windows.h>

extern "C" HWND g_pWnd;
extern HMENU g_windowMenu;
extern MechU8 g_fDrawFmv;
extern "C" MechS32 g_fWindowActive;
extern MechS32 g_unk0x1006a9f0;
extern VideoDriver* g_pVideoDriver;
extern MechChar g_szDataDrivePath[];

// The original imports this one under its Miles name (wail32.def: _MEM_free_lock@4). It is
// declared here rather than in mss.h: one more symbol there flips a comparison in MW2's
// SimWindowProc.
extern "C" AILIMPORT void AILCALL MEM_free_lock(void* p_block);

// The draw mode table lives in the draw mode unit, a C translation unit.
extern "C" DrawModeExtension* g_currentDrawModeExtension;

void FUN_1001023c(HMENU p_menu);
void FUN_10010320(HMENU p_menu);
void FUN_100108e5(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, char**, MechS32));
void FUN_10016f45();
MechS32 FUN_10017460(
	MechS32 p_index,
	const char* p_name,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	MechU32 p_unk0x10,
	MechU32 p_unk0x14
);

// GLOBAL: MW2SHELL 0x100641a8
CopperFinch0x4c g_unk0x100641a8[32] = {0};

// GLOBAL: MW2SHELL 0x10064b28
undefined4 g_unk0x10064b28 = 0;

// GLOBAL: MW2SHELL 0x10064b2c
MechS32 g_unk0x10064b2c = 0x404;

// GLOBAL: MW2SHELL 0x10064b30
MechS32 g_unk0x10064b30 = 0x404;

// GLOBAL: MW2SHELL 0x1007cdd0
MechChar g_unk0x1007cdd0[0x100];

// FUNCTION: MW2SHELL 0x10015d30
MechChar* GetPathToVideo(const MechChar* p_name)
{
	if (g_unk0x10064b28 && g_szDataDrivePath[0]) {
		sprintf(g_unk0x1007cdd0, "%ssmk\\%s.smk", g_szDataDrivePath, p_name);
	}
	else {
		sprintf(g_unk0x1007cdd0, "smk\\%s.smk", p_name);
	}

	g_unk0x10064b28 = 0;
	return g_unk0x1007cdd0;
}

// STUB: MW2SHELL 0x10015e8e
void FUN_10015e8e(
	TMPackDataBase* p_database,
	MechS32* p_campaign,
	MechU8* p_pilotChosen,
	char** p_scenario,
	MechS32 p_msg
)
{
	STUB(0x10015e8e);
}

// FUNCTION: MW2SHELL 0x10015f58
MechS32 FUN_10015f58(const char* p_name, MechS32 p_msg, MechS32 p_wParam)
{
	MechS32 result;

	FUN_10016f45();
	result = FUN_10017460(0, p_name, 0, 0, 0x1000, 0);
	if (result == -1) {
		PostMessage(g_pWnd, p_msg, p_wParam, 0);
		return 0;
	}

	if (g_unk0x1006a9f0 != 0) {
		g_pVideoDriver->ActivateFramebuffer();
	}
	g_unk0x10064b2c = p_msg;
	g_unk0x10064b30 = p_wParam;
	FUN_100108e5(FUN_10015e8e);
	return 1;
}

// FUNCTION: MW2SHELL 0x10015fed
MechS32 PlayFullscreenVideo(const char* p_name, MechS32 p_msg, MechS32 p_wParam)
{
	if (FUN_10015f58(p_name, p_msg, p_wParam) == 0) {
		return 0;
	}

	FUN_10010320(g_windowMenu);
	g_fDrawFmv = TRUE;
	return 1;
}

// STUB: MW2SHELL 0x1001661b
void FUN_1001661b()
{
	STUB(0x1001661b);
}

// FUNCTION: MW2SHELL 0x10016be7
MechS32 FUN_10016be7()
{
	if ((g_unk0x100641a8[0].m_unk0x1c & 0x80000000) && (g_unk0x100641a8[0].m_unk0x1c & 0x1000)) {
		return 1;
	}

	return 0;
}

// FUNCTION: MW2SHELL 0x10016c1d
void FUN_10016c1d()
{
	g_unk0x100641a8[0].m_unk0x3c--;
	g_pVideoDriver->FUN_1000725d();
}

// FUNCTION: MW2SHELL 0x10016c3e
void FUN_10016c3e()
{
	MechS32 result;

	SmackGoto(g_unk0x100641a8[0].m_unk0x00, g_unk0x100641a8[0].m_unk0x3c);
	g_pVideoDriver->FUN_10007293();
	if (g_fWindowActive != 0) {
		result = g_currentDrawModeExtension->m_acquireFramebuffer();
	}
	else {
		result = -1;
	}

	if (result == 0) {
		SmackDoFrame(g_unk0x100641a8[0].m_unk0x00);
	}

	SmackNextFrame(g_unk0x100641a8[0].m_unk0x00);
	g_unk0x100641a8[0].m_unk0x3c++;
}

// FUNCTION: MW2SHELL 0x10016d27
void FUN_10016d27(MechS32 p_index)
{
	if (p_index >= 0 && p_index < 0x20 && (g_unk0x100641a8[p_index].m_unk0x1c & 0x20)) {
		g_unk0x100641a8[p_index].m_unk0x1c = g_unk0x100641a8[p_index].m_unk0x1c & ~0x20 | 0x100;
	}
}

// FUNCTION: MW2SHELL 0x10016d90
void FUN_10016d90(MechS32 p_index)
{
	if (p_index < 0 || p_index >= 0x20) {
		return;
	}

	if (g_unk0x100641a8[p_index].m_unk0x1c & 0x1000) {
		FUN_1001023c(g_windowMenu);
	}

	if (g_unk0x100641a8[p_index].m_unk0x00 != NULL) {
		SmackBufferNewPalette(g_unk0x100641a8[p_index].m_unk0x00);
	}

	if (g_unk0x100641a8[p_index].m_unk0x04 != NULL) {
		delete g_unk0x100641a8[p_index].m_unk0x04;
	}

	if (g_unk0x100641a8[p_index].m_unk0x14 != NULL) {
		MEM_free_lock(g_unk0x100641a8[p_index].m_unk0x14);
	}

	if (g_unk0x100641a8[p_index].m_unk0x18 != NULL) {
		MEM_free_lock(g_unk0x100641a8[p_index].m_unk0x18);
	}

	g_unk0x100641a8[p_index].m_unk0x00 = NULL;
	g_unk0x100641a8[p_index].m_unk0x04 = NULL;
	g_unk0x100641a8[p_index].m_unk0x14 = NULL;
	ZeroMemory(&g_unk0x100641a8[p_index].m_unk0x1c, 4);
	g_unk0x100641a8[p_index].m_unk0x18 = NULL;
}

// FUNCTION: MW2SHELL 0x10016f45
void FUN_10016f45()
{
	MechS32 i;

	for (i = 0; i < 0x20; i++) {
		FUN_10016d90(i);
	}
}

// FUNCTION: MW2SHELL 0x10016f82
void FUN_10016f82(MechS32 p_index, MechS32 p_left, MechS32 p_top)
{
	if (p_index >= 0 && p_index < 0x20 && (g_unk0x100641a8[p_index].m_unk0x1c & 0x80000000)) {
		if (g_unk0x100641a8[p_index].m_unk0x1c & 0x80) {
			p_left -= g_unk0x100641a8[p_index].m_width / 2;
			p_top -= g_unk0x100641a8[p_index].m_height;
		}

		g_unk0x100641a8[p_index].m_left = p_left;
		g_unk0x100641a8[p_index].m_top = p_top;
	}
}

// STUB: MW2SHELL 0x10017460
MechS32 FUN_10017460(
	MechS32 p_index,
	const char* p_name,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	MechU32 p_unk0x10,
	MechU32 p_unk0x14
)
{
	STUB(0x10017460);
	return -1;
}

// FUNCTION: MW2SHELL 0x10017698
void FUN_10017698(MechS32 p_index, MechS32 p_frame)
{
	if (p_index >= 0 && p_index < 0x20) {
		if (p_frame >= g_unk0x100641a8[p_index].m_unk0x40) {
			p_frame = 0;
		}

		g_unk0x100641a8[p_index].m_unk0x3c = p_frame;
		g_unk0x100641a8[p_index].m_unk0x38 = -1;
	}
}
