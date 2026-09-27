#include "copperfinch0x4c.h"
#include "decomp.h"
#include "drawmodeextension.h"
#include "mss.h"
#include "silverreel0x18.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "videodriver.h"

#include <stdio.h>
#include <stdlib.h>
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

DECOMP_SIZE_ASSERT(SilverReel0x18, 0x18)

// STUB: MW2SHELL 0x1001603a
SilverReel0x18::SilverReel0x18(MechChar*, MechS32, MechS32)
{
	STUB(0x1001603a);
}

// FUNCTION: MW2SHELL 0x100161a8
SilverReel0x18::~SilverReel0x18()
{
	if (m_smack == NULL) {
		return;
	}

	SmackClose(m_smack);
}

// FUNCTION: MW2SHELL 0x100162d3
void FUN_100162d3(size_t p_size)
{
	malloc(p_size);
}

// FUNCTION: MW2SHELL 0x100162ef
void FUN_100162ef(void* p_block)
{
	free(p_block);
}

// FUNCTION: MW2SHELL 0x1001630b
void SilverReel0x18::FUN_1001630b()
{
	if (m_smack == NULL) {
		return;
	}
	if (SmackWait(m_smack)) {
		return;
	}

	if ((g_fWindowActive ? g_currentDrawModeExtension->m_acquireFramebuffer() : -1) == 0) {
		m_frame++;
		if (m_smack->Frames < m_frame) {
			m_frame = 1;
			SmackGoto(m_smack, m_frame);
		}
		else {
			SmackNextFrame(m_smack);
		}
		SmackDoFrame(m_smack);
		g_pVideoDriver->ExpandRectBySize(m_left, m_top, m_width, m_height);
	}
}

struct VideoPlaybackTimer {
	MechU32 m_interval; // 0x44 in the video slot
	MechU32 m_next;     // 0x48 in the video slot
};

// FUNCTION: MW2SHELL 0x100163ff
MechS32 FUN_100163ff(CopperFinch0x4c* p_video, MechU32 p_time)
{
	if (p_time >= ((VideoPlaybackTimer*) p_video->m_unk0x44)->m_next) {
		((VideoPlaybackTimer*) p_video->m_unk0x44)->m_next =
			((VideoPlaybackTimer*) p_video->m_unk0x44)->m_interval + p_time;
		return 0;
	}
	else {
		return -1;
	}
}

// STUB: MW2SHELL 0x1001661b
void FUN_1001661b()
{
	STUB(0x1001661b);
}

// FUNCTION: MW2SHELL 0x10016b11
MechS32 FUN_10016b11(MechS32 p_index)
{
	if (p_index >= 0 && p_index < 0x20 && (g_unk0x100641a8[p_index].m_unk0x1c & 0x80000000) &&
		!(g_unk0x100641a8[p_index].m_unk0x1c & 1)) {
		return 1;
	}
	else {
		return 0;
	}
}

// FUNCTION: MW2SHELL 0x10016b78
MechS32 FUN_10016b78()
{
	MechS32 i;

	for (i = 0; i < 0x20; i++) {
		if ((g_unk0x100641a8[i].m_unk0x1c & 0x80000000) && (g_unk0x100641a8[i].m_unk0x1c & 0x2000)) {
			return 1;
		}
	}

	return 0;
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

// The original loads p_mask before p_value in p_mask & p_value; swapping the operands didn't flip it.
// The first statement reloads and stores the flags instead of and-ing them in place: an unsigned
// operation on a signed field does that, which is why the mask is cast.
// FUNCTION: MW2SHELL 0x10016cc0
void FUN_10016cc0(MechS32 p_index, MechS32 p_mask, MechS32 p_value)
{
	if (p_index >= 0 && p_index < 0x20) {
		g_unk0x100641a8[p_index].m_unk0x1c &= ~(MechU32) p_mask;
		g_unk0x100641a8[p_index].m_unk0x1c |= p_mask & p_value;
	}
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
		SmackClose(g_unk0x100641a8[p_index].m_unk0x00);
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

// FUNCTION: MW2SHELL 0x10017656
MechS32 FUN_10017656(MechS32 p_index)
{
	if (p_index >= 0 && p_index < 0x20) {
		return g_unk0x100641a8[p_index].m_unk0x3c;
	}
	else {
		return 0;
	}
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
