#include "audiosubsystem.h"
#include "copperfinch0x4c.h"
#include "decomp.h"
#include "drawmodeextension.h"
#include "hollowreed0x110.h"
#include "mousestate.h"
#include "mss.h"
#include "shellmain.h"
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
extern MouseState* g_pMouseState;
extern HollowReed0x110* g_unk0x100711f8;
extern MechU32 g_unk0x10071248;
extern AudioSubsystem* g_pAudioSubsystem;
extern MechS32 g_menuDialogOpen;

// The original imports this one under its Miles name (wail32.def: _MEM_free_lock@4). It is
// declared here rather than in mss.h: one more symbol there flips a comparison in MW2's
// SimWindowProc.
extern "C" AILIMPORT void AILCALL MEM_free_lock(void* p_block);
extern "C" AILIMPORT void AILCALL AIL_serve();
extern "C" AILIMPORT void* AILCALL FILE_read(const char* p_filename, void* p_dest);

// The draw mode table lives in the draw mode unit, a C translation unit.
extern "C" DrawModeExtension* g_currentDrawModeExtension;

void FUN_1001023c(HMENU p_menu);
void FUN_10010320(HMENU p_menu);
void FUN_100108e5(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, char**, MechS32));
void FUN_100108fd(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, char**, MechS32));
extern "C" MechS32 FUN_10037504(void* p_data, MechS32 p_index);
extern "C" MechS32 FUN_10037684(void* p_data);
MechS32 FUN_10016b11(MechS32 p_index);
void FUN_10016d90(MechS32 p_index);
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
MechChar g_unk0x1007cdd0[0x20];

// GLOBAL: MW2SHELL 0x1007cdf0
MechChar g_unk0x1007cdf0[0x20];

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

// FUNCTION: MW2SHELL 0x10015da1
MechChar* GetPathToShp(const MechChar* p_name)
{
	if (g_unk0x10064b28 && g_szDataDrivePath[0]) {
		sprintf(g_unk0x1007cdf0, "%ssmk\\%s.shp", g_szDataDrivePath, p_name);
	}
	else {
		sprintf(g_unk0x1007cdf0, "smk\\%s.shp", p_name);
	}

	g_unk0x10064b28 = 0;
	return g_unk0x1007cdf0;
}

// Looks for the video on the hard disk, then on the data drive; a video found there keeps
// g_unk0x10064b28 set for the next GetPathToVideo.
// FUNCTION: MW2SHELL 0x10015e12
BOOL CheckVideoExists(const MechChar* p_name)
{
	if (GetFileAttributes(GetPathToVideo(p_name)) == 0xffffffff) {
		g_unk0x10064b28 = 1;
		if (GetFileAttributes(GetPathToVideo(p_name)) == 0xffffffff) {
			return FALSE;
		}
		else {
			g_unk0x10064b28 = 1;
			return TRUE;
		}
	}
	else {
		return TRUE;
	}
}

// The screen callback while a full-screen video plays: a click, a key or any other message ends
// it.
// FUNCTION: MW2SHELL 0x10015e8e
void FUN_10015e8e(TMPackDataBase*, MechS32*, MechU8*, char**, MechS32 p_msg)
{
	CopperFinch0x4c* video;
	MechS32 msg;

	video = &g_unk0x100641a8[0];
	if (!FUN_10016b11(0) || g_pMouseState->GetLeftPressed() == 1 || g_unk0x100711f8->FUN_10044189() || p_msg != 0x404) {
		FUN_10016d90(0);
		FUN_100108fd(FUN_10015e8e);
		if (p_msg == 0x404) {
			msg = g_unk0x10064b2c;
		}
		else {
			msg = p_msg;
		}

		PostMessage(g_pWnd, msg, g_unk0x10064b30, 0);
		g_unk0x10064b2c = g_unk0x10064b30 = 0x404;
		g_fDrawFmv = FALSE;
	}
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

// Opens the movie p_name (retrying once, after setting g_unk0x10064b28) and draws its first
// frame at (p_left, p_top). Without a framebuffer it closes the movie again.
// FUNCTION: MW2SHELL 0x1001603a
SilverReel0x18::SilverReel0x18(MechChar* p_name, MechS32 p_left, MechS32 p_top)
{
	MechS32 result;

	m_smack = SmackOpen(GetPathToVideo(p_name), g_unk0x10071248, 0);
	if (m_smack == NULL) {
		g_unk0x10064b28 = 1;
		m_smack = SmackOpen(GetPathToVideo(p_name), g_unk0x10071248, 0);
		if (m_smack == NULL) {
			return;
		}
	}

	m_left = p_left;
	m_top = p_top;
	m_width = m_smack->Width;
	m_height = m_smack->Height;
	m_frame = 1;

	if (g_fWindowActive != 0) {
		result = g_currentDrawModeExtension->m_acquireFramebuffer();
	}
	else {
		result = -1;
	}

	if (result == 0) {
		SmackToBuffer(m_smack, m_left, m_top, 0x280, 0x1e0, g_pVideoDriver->m_screenBuffer.m_pixels, 0);
		SmackDoFrame(m_smack);
		g_pVideoDriver->ExpandRectBySize(m_left, m_top, m_width, m_height);
	}
	else {
		SmackClose(m_smack);
		m_smack = NULL;
	}
}

// FUNCTION: MW2SHELL 0x100161a8
SilverReel0x18::~SilverReel0x18()
{
	if (m_smack == NULL) {
		return;
	}

	SmackClose(m_smack);
}

// Moves the movie: restores the background under the old rectangle and draws the current
// frame at the new position.
// FUNCTION: MW2SHELL 0x100161dd
void SilverReel0x18::FUN_100161dd(MechS32 p_left, MechS32 p_top)
{
	MechS32 result;

	if (m_smack == NULL) {
		return;
	}

	g_pVideoDriver->FUN_100071ad(m_left, m_top, m_width, m_height);
	m_left = p_left;
	m_top = p_top;
	if (g_fWindowActive != 0) {
		result = g_currentDrawModeExtension->m_acquireFramebuffer();
	}
	else {
		result = -1;
	}

	if (result == 0) {
		SmackToBuffer(m_smack, m_left, m_top, 0x280, 0x1e0, g_pVideoDriver->m_screenBuffer.m_pixels, 0);
		SmackDoFrame(m_smack);
		g_pVideoDriver->ExpandRectBySize(m_left, m_top, m_width, m_height);
	}
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

// The largest sound chunk streamed so far.
// GLOBAL: MW2SHELL 0x10064b34
MechS32 g_unk0x10064b34 = 0;

// Streams the next chunk of a video's sound track to its sound object once it wants one.
// FUNCTION: MW2SHELL 0x1001643e
void FUN_1001643e(CopperFinch0x4c* p_video)
{
	AIL_serve();
	if (p_video->m_unk0x04 != NULL && p_video->m_unk0x10) {
		if (p_video->m_unk0x04->FUN_1003dad5()) {
			p_video->m_unk0x08 = p_video->m_unk0x04->FUN_1003db31();
			p_video->m_unk0x0c = SmackGetTrackData(p_video->m_unk0x00, p_video->m_unk0x08, 0x200);
			if (p_video->m_unk0x0c > g_unk0x10064b34) {
				g_unk0x10064b34 = p_video->m_unk0x0c;
			}

			p_video->m_unk0x04->FUN_1003db95(p_video->m_unk0x08, p_video->m_unk0x0c);
			p_video->m_unk0x10 = 0;
		}
	}
}

// Plays the next frame of the full-screen video in slot 0, closing it after the last.
// Stack-slot permutation: video, palette and result.
// FUNCTION: MW2SHELL 0x100164f2
void FUN_100164f2()
{
	MechS32 result;
	PaletteColor* palette;
	CopperFinch0x4c* video;

	video = &g_unk0x100641a8[0];
	if (!SmackWait(video->m_unk0x00)) {
		if (video->m_unk0x3c == 1) {
			g_pVideoDriver->m_unk0x3aa = 1;
		}

		if (video->m_unk0x00->NewPalette) {
			if (video->m_unk0x00->PalType == 1) {
				palette = (PaletteColor*) video->m_unk0x00->Palette;
			}
			else {
				palette = (PaletteColor*) video->m_unk0x00->AlternatePalette;
			}
			g_pVideoDriver->SetPalette(palette, 0);
		}

		if (g_fWindowActive) {
			result = g_currentDrawModeExtension->m_acquireFramebuffer();
		}
		else {
			result = -1;
		}

		if (result == 0) {
			SmackDoFrame(video->m_unk0x00);
			g_pVideoDriver->ExpandRectBySize(0, 0, video->m_width, video->m_height);
		}

		video->m_unk0x3c++;
		if (video->m_unk0x3c > video->m_unk0x40) {
			FUN_10016d90(0);
		}
		else {
			SmackNextFrame(video->m_unk0x00);
		}
	}
}

// Advances and draws the videos of all slots: Smacker videos frame by frame, sprite sheets
// (m_unk0x14) on their timer. A video past its last frame stops (flag 4), loops (flag 8) or
// closes.
// Not 100%: the stack slots of video and i are permuted, and m_unk0x34 != m_top loads m_unk0x34
// first (reversing the operands or retyping the member doesn't flip it).
// FUNCTION: MW2SHELL 0x1001661b
void FUN_1001661b()
{
	CopperFinch0x4c* video;
	MechS32 i;

	if (g_pAudioSubsystem) {
		AIL_serve();
	}

	if (g_menuDialogOpen) {
		return;
	}

	video = g_unk0x100641a8;
	if (video->m_unk0x1c & 0x80000000 && video->m_unk0x1c & 0x1000) {
		FUN_100164f2();
		return;
	}

	for (i = 0, video = g_unk0x100641a8; i < 32; i++, video++) {
		if (video->m_unk0x1c & 0x80000000) {
			if (video->m_unk0x00 && !video->m_unk0x3c && !(video->m_unk0x1c & 1)) {
				SmackDoFrame(video->m_unk0x00);
				if (video->m_unk0x1c & 2) {
					video->m_unk0x1c |= 0x10;
				}
				video->m_unk0x3c = 1;
			}

			if (video->m_unk0x00 && !(video->m_unk0x1c & 2)) {
				video->m_unk0x38 = video->m_unk0x3c;
			}

			if (video->m_unk0x30 != video->m_left || video->m_unk0x34 != video->m_top ||
				video->m_unk0x38 != video->m_unk0x3c || video->m_unk0x1c & 0x40000020) {
				if (video->m_unk0x1c & 0x10) {
					g_pVideoDriver->FUN_100071ad(video->m_unk0x30, video->m_unk0x34, video->m_width, video->m_height);
				}
				video->m_unk0x1c |= 0x100;
			}

			video->m_unk0x30 = video->m_left;
			video->m_unk0x34 = video->m_top;
			video->m_unk0x38 = video->m_unk0x3c;
			video->m_unk0x1c &= ~0x10;
			if (video->m_unk0x1c & 0x40000000) {
				FUN_10016d90(i);
			}
		}
	}

	g_pVideoDriver->FUN_10007763(0);
	for (i = 0, video = g_unk0x100641a8; i < 32; i++, video++) {
		if (video->m_unk0x1c & 0x80000000) {
			if (video->m_unk0x00) {
				if (video->m_unk0x18 && !(video->m_unk0x1c & 0x20)) {
					video->m_unk0x1c |= 0x10;
					if (video->m_unk0x1c & 0x100) {
						g_pVideoDriver->FUN_10006ed4(
							(undefined*) video->m_unk0x18,
							video->m_left,
							video->m_top,
							video->m_width,
							video->m_height
						);
					}
					else {
						g_pVideoDriver->FUN_10006f87(
							(undefined*) video->m_unk0x18,
							video->m_left,
							video->m_top,
							video->m_width,
							video->m_height
						);
					}
					video->m_unk0x1c &= ~0x100;
				}

				if (!(video->m_unk0x1c & 1) && !SmackWait(video->m_unk0x00)) {
					video->m_unk0x3c++;
					if (video->m_unk0x3c > video->m_unk0x40) {
						video->m_unk0x3c--;
						if (video->m_unk0x1c & 4) {
							video->m_unk0x1c |= 1;
							continue;
						}
						else if (video->m_unk0x1c & 8) {
							video->m_unk0x3c = 1;
							SmackGoto(video->m_unk0x00, video->m_unk0x3c);
						}
						else {
							video->m_unk0x1c |= 0x40000001;
							continue;
						}
					}
					else if (video->m_unk0x3c != 1) {
						SmackNextFrame(video->m_unk0x00);
					}

					SmackDoFrame(video->m_unk0x00);
					video->m_unk0x1c |= 0x100;
					if (video->m_unk0x1c & 2) {
						video->m_unk0x1c |= 0x10;
					}
				}
			}
			else if (video->m_unk0x14) {
				if (!(video->m_unk0x1c & 0x20)) {
					video->m_unk0x1c |= 0x10;
					if (video->m_unk0x1c & 0x100) {
						g_pVideoDriver->FUN_100073b3(
							(undefined4) video->m_unk0x14,
							video->m_unk0x3c,
							video->m_left,
							video->m_top,
							video->m_width,
							video->m_height
						);
					}
					else {
						g_pVideoDriver->FUN_10007430(
							(undefined4) video->m_unk0x14,
							video->m_unk0x3c,
							video->m_left,
							video->m_top,
							video->m_width,
							video->m_height
						);
					}
					video->m_unk0x1c &= ~0x100;
				}

				if (!(video->m_unk0x1c & 1) && !FUN_100163ff(video, timeGetTime())) {
					video->m_unk0x3c++;
					if (video->m_unk0x3c >= video->m_unk0x40) {
						video->m_unk0x3c--;
						if (video->m_unk0x1c & 4) {
							video->m_unk0x1c |= 1;
						}
						else if (video->m_unk0x1c & 8) {
							video->m_unk0x3c = 0;
						}
						else {
							video->m_unk0x1c |= 0x40000001;
						}
					}
					video->m_unk0x1c |= 0x100;
				}
			}
		}
	}
	g_pVideoDriver->FUN_10007763(1);
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

// Opens the video p_name in the slot. A video with sound that the audio subsystem can play gets
// Smacker's sound (flag 0x2000). With flag 0x1000 the first frame goes straight to the screen, with
// flag 2 to the back buffer, otherwise to a buffer of its own.
// Not 100%: the stack slots of dataDrive, result and the delete temporaries are permuted.
// FUNCTION: MW2SHELL 0x1001703b
BOOL LoadVideoFile(CopperFinch0x4c* p_slot, const MechChar* p_name)
{
	undefined4 dataDrive;
	MechS32 result;

	dataDrive = g_unk0x10064b28;
	p_slot->m_unk0x00 = SmackOpen(GetPathToVideo(p_name), ((p_slot->m_unk0x1c & 0x40) >> 1) | 0xfe00, 0);
	if (!p_slot->m_unk0x00) {
		return FALSE;
	}

	if (g_pAudioSubsystem && SmackSoundInTrack(p_slot->m_unk0x00, 0x200)) {
		SmackClose(p_slot->m_unk0x00);
		g_pAudioSubsystem->CloseDigitalDriver();
		g_unk0x10064b28 = dataDrive;
		p_slot->m_unk0x00 = SmackOpen(GetPathToVideo(p_name), ((p_slot->m_unk0x1c & 0x40) >> 1) | 0xfe00, 0);
		if (!p_slot->m_unk0x00) {
			return FALSE;
		}

		p_slot->m_unk0x1c |= 0x2000;
	}
	else {
		SmackClose(p_slot->m_unk0x00);
		g_unk0x10064b28 = dataDrive;
		p_slot->m_unk0x00 = SmackOpen(GetPathToVideo(p_name), (p_slot->m_unk0x1c & 0x40) >> 1, 0);
		if (!p_slot->m_unk0x00) {
			return FALSE;
		}
	}

	p_slot->m_width = p_slot->m_unk0x00->Width;
	p_slot->m_height = p_slot->m_unk0x00->Height;
	if (p_slot->m_unk0x1c & 0x80) {
		p_slot->m_left -= p_slot->m_width / 2;
		p_slot->m_top -= p_slot->m_height;
	}

	p_slot->m_unk0x3c = 0;
	p_slot->m_unk0x40 = p_slot->m_unk0x00->Frames;
	p_slot->m_unk0x38 = p_slot->m_unk0x3c;
	p_slot->m_unk0x18 = NULL;

	if (p_slot->m_unk0x1c & 0x1000) {
		p_slot->m_unk0x3c = 1;
		if (g_fWindowActive != 0) {
			result = g_currentDrawModeExtension->m_acquireFramebuffer();
		}
		else {
			result = -1;
		}

		if (result == 0) {
			SmackToBuffer(
				p_slot->m_unk0x00,
				p_slot->m_left,
				p_slot->m_top,
				0x280,
				0x1e0,
				g_pVideoDriver->m_screenBuffer.m_pixels,
				0
			);
		}
		else {
			if (p_slot->m_unk0x04) {
				delete p_slot->m_unk0x04;
			}
			p_slot->m_unk0x04 = NULL;
			SmackClose(p_slot->m_unk0x00);
			return FALSE;
		}
	}
	else if (p_slot->m_unk0x1c & 2) {
		SmackToBuffer(
			p_slot->m_unk0x00,
			p_slot->m_left,
			p_slot->m_top,
			0x280,
			0x1e0,
			g_pVideoDriver->m_backBuffer.m_pixels,
			0
		);
	}
	else {
		p_slot->m_unk0x18 = HeapAlloc(g_hPrimaryHeap, HEAP_NO_SERIALIZE, p_slot->m_width * p_slot->m_height);
		SmackToBuffer(p_slot->m_unk0x00, 0, 0, p_slot->m_width, p_slot->m_height, p_slot->m_unk0x18, 0);
	}

	return TRUE;
}

// Loads the SHP animation p_name into the slot.
// FUNCTION: MW2SHELL 0x10017376
BOOL LoadShpFile(CopperFinch0x4c* p_slot, const MechChar* p_name)
{
	MechS32 size;

	p_slot->m_unk0x14 = FILE_read(GetPathToShp(p_name), NULL);
	if (!p_slot->m_unk0x14) {
		return FALSE;
	}

	size = FUN_10037504(p_slot->m_unk0x14, 0);
	p_slot->m_width = (size >> 16) + 1;
	p_slot->m_height = (size & 0xffff) + 1;
	if (p_slot->m_unk0x1c & 0x80) {
		p_slot->m_left -= p_slot->m_width / 2;
		p_slot->m_top -= p_slot->m_height;
	}

	p_slot->m_unk0x3c = 0;
	p_slot->m_unk0x40 = FUN_10037684(p_slot->m_unk0x14);
	p_slot->m_unk0x38 = p_slot->m_unk0x3c;
	p_slot->m_unk0x18 = NULL;
	((VideoPlaybackTimer*) p_slot->m_unk0x44)->m_next = 0;

	return TRUE;
}

// Plays the video or SHP animation p_name in slot p_index at (p_left, p_top), p_fps frames a
// second (10 for 0), replacing what the slot played. Returns the slot, or -1.
// FUNCTION: MW2SHELL 0x10017460
MechS32 FUN_10017460(
	MechS32 p_index,
	const char* p_name,
	undefined4 p_left,
	undefined4 p_top,
	MechU32 p_flags,
	MechU32 p_fps
)
{
	CopperFinch0x4c* slot = &g_unk0x100641a8[p_index];

	if (p_index < 0 || p_index >= 0x20) {
		return -1;
	}

	if (slot->m_unk0x1c & 0x80000000) {
		if (slot->m_unk0x1c & 0x10) {
			g_pVideoDriver->FUN_100071ad(slot->m_unk0x30, slot->m_unk0x34, slot->m_width, slot->m_height);
		}
		FUN_10016d90(p_index);
	}

	slot->m_left = p_left;
	slot->m_top = p_top;
	slot->m_unk0x1c = p_flags | 0x80000100;
	if (!p_fps) {
		p_fps = 10;
	}
	((VideoPlaybackTimer*) slot->m_unk0x44)->m_interval = 1000 / p_fps;

	if (CheckVideoExists(p_name)) {
		if (!LoadVideoFile(slot, p_name)) {
			slot->m_unk0x1c = 0;
			return -1;
		}
	}
	else if (!LoadShpFile(slot, p_name)) {
		g_unk0x10064b28 = 1;
		if (!LoadShpFile(slot, p_name)) {
			slot->m_unk0x1c = 0;
			return -1;
		}
	}

	slot->m_unk0x30 = slot->m_left;
	slot->m_unk0x34 = slot->m_top;

	return p_index;
}

// Plays a video in the first free slot.
// FUNCTION: MW2SHELL 0x100175e2
MechS32 FUN_100175e2(MechChar* p_name, MechS32 p_left, MechS32 p_top, MechU32 p_flags, MechU32 p_unk0x14)
{
	MechS32 i;

	for (i = 0; i < 0x20; i++) {
		if (!(g_unk0x100641a8[i].m_unk0x1c & 0x80000000)) {
			return FUN_10017460(i, p_name, p_left, p_top, p_flags, p_unk0x14);
		}
	}

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
