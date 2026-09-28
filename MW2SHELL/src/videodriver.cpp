#include "videodriver.h"

#include "audiosubsystem.h"
#include "blit.h"
#include "debugprint.h"
#include "displaybackend.h"
#include "gdi.h"
#include "refreshmode.h"
#include "shellglobals.h"
#include "textglyphlist.h"
#include "tmpackdatabase.h"
#include "unk1003bf90.h"
#include "video.h"

#include <stdlib.h>
#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(VideoDriver, 0x3ae)
DECOMP_SIZE_ASSERT(PixelBuffer, 0x14)
DECOMP_SIZE_ASSERT(PixelView, 0x14)

// Only the height field of the font data header is used by the text drawing wrappers.
struct VideoFontHeader {
	undefined m_unk0x00[8]; // 0x00
	MechS32 m_height;       // 0x08
};

// GLOBAL: MW2SHELL 0x1005c2a0
MechS32 g_unk0x1005c2a0 = 0;

// GLOBAL: MW2SHELL 0x1005c2a4
MechS32 g_unk0x1005c2a4 = 0;

// GLOBAL: MW2SHELL 0x10079698
PaletteColor g_unk0x10079698[0x100];

// GLOBAL: MW2SHELL 0x10079998
MechU8 g_unk0x10079998[0x100];

// 0 once the framebuffer can be drawn to; -1 while the window is inactive.
#define ACQUIRE_FRAMEBUFFER() (g_fWindowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1)

// FUNCTION: MW2SHELL 0x10005e70
void QuitWithVDriverError(MechS32 p_code)
{
	ShutdownRefreshMode();

	if (g_pAudioSubsystem) {
		delete g_pAudioSubsystem;
	}

	FUN_10016f45();
	DebugPrint("vdriver error\n");
	exit(p_code + 1);
}

// FUNCTION: MW2SHELL 0x10005eea
void ClearPalette()
{
	memset(g_unk0x10079698, 0, sizeof(g_unk0x10079698));
	g_currentDisplayBackend->m_setPalette(0, 0x100, g_unk0x10079698, 1);
}

// Matches except for the operand order of m_width * m_height in the back buffer allocation
// (the original loads m_height first). It tracks the number of symbols declared ahead of this
// unit: declaring AudioSubsystem's seventh new method flipped it (while fixing FUN_100071ad's
// additions); neither the declaration order nor the source operand order flips it back.
// FUNCTION: MW2SHELL 0x10005f21
VideoDriver::VideoDriver()
{
	MechS32 i;

	if (!InitRefreshMode(5, 0, &m_screenBuffer, g_windowWidth, g_windowHeight, 1)) {
		QuitWithVDriverError(1);
	}

	m_width = g_windowWidth;
	m_height = g_windowHeight;
	m_unk0x39a = m_width - 1;
	m_unk0x39e = m_height - 1;
	m_screenBuffer.m_maxX = m_backBuffer.m_maxX = m_width - 1;
	m_screenBuffer.m_maxY = m_backBuffer.m_maxY = m_height - 1;

	m_backBuffer.m_pixels =
		(undefined*) HeapAlloc(g_hPrimaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, m_width * m_height);
	if (!m_backBuffer.m_pixels) {
		QuitWithVDriverError(1);
	}

	m_screenView.m_left = m_backView.m_left = 0;
	m_screenView.m_top = m_backView.m_top = 0;
	m_screenView.m_right = m_backView.m_right = m_screenBuffer.m_maxX;
	m_screenView.m_bottom = m_backView.m_bottom = m_screenBuffer.m_maxY;
	m_screenView.m_buffer = &m_screenBuffer;
	m_backView.m_buffer = &m_backBuffer;

	// Start with an empty (inverted) dirty rectangle.
	m_dirtyView.m_left = m_screenView.m_right;
	m_dirtyView.m_top = m_screenView.m_bottom;
	m_dirtyView.m_right = m_screenView.m_left;
	m_dirtyView.m_bottom = m_screenView.m_top;
	m_dirtyView.m_buffer = m_screenView.m_buffer;

	m_unk0x16 = new TextGlyphList();
	m_unk0x1a = new TextGlyphList();

	g_unk0x10079998[0] = 0xff;
	for (i = 1; i < 0x100; i++) {
		g_unk0x10079998[i] = i;
	}

	m_unk0x3a2 = 0;
	m_unk0x3a6 = -1;
}

// FUNCTION: MW2SHELL 0x10006202
VideoDriver::~VideoDriver()
{
	delete m_unk0x16;
	delete m_unk0x1a;
	HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, m_backBuffer.m_pixels);
	ShutdownRefreshMode();
}

// FUNCTION: MW2SHELL 0x100062a0
void VideoDriver::ExpandRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	if (p_left < m_dirtyView.m_left) {
		m_dirtyView.m_left = p_left;
	}
	if (p_top < m_dirtyView.m_top) {
		m_dirtyView.m_top = p_top;
	}
	if (p_right > m_dirtyView.m_right) {
		m_dirtyView.m_right = p_right;
	}
	if (p_bottom > m_dirtyView.m_bottom) {
		m_dirtyView.m_bottom = p_bottom;
	}
}

// FUNCTION: MW2SHELL 0x10006330
void VideoDriver::ExpandRectBySize(MechS32 p_left, MechS32 p_top, MechS32 p_width, MechS32 p_height)
{
	MechS32 right = p_left + p_width - 1;
	MechS32 bottom = p_top + p_height - 1;

	if (p_left < m_dirtyView.m_left) {
		m_dirtyView.m_left = p_left;
	}
	if (p_top < m_dirtyView.m_top) {
		m_dirtyView.m_top = p_top;
	}
	if (right > m_dirtyView.m_right) {
		m_dirtyView.m_right = right;
	}
	if (bottom > m_dirtyView.m_bottom) {
		m_dirtyView.m_bottom = bottom;
	}
}

// FUNCTION: MW2SHELL 0x100063d4
MechS32 VideoDriver::IntersectsRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	if (p_left > m_dirtyView.m_right || p_right < m_dirtyView.m_left || p_top > m_dirtyView.m_bottom ||
		p_bottom < m_dirtyView.m_top) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

// FUNCTION: MW2SHELL 0x10006445
MechS32 VideoDriver::IntersectsRectBySize(MechS32 p_left, MechS32 p_top, MechS32 p_width, MechS32 p_height)
{
	MechS32 right = p_left + p_width - 1;
	MechS32 bottom = p_top + p_height - 1;

	if (p_left > m_dirtyView.m_right || right < m_dirtyView.m_left || p_top > m_dirtyView.m_bottom ||
		bottom < m_dirtyView.m_top) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

// FUNCTION: MW2SHELL 0x100064ca
void VideoDriver::UpdatePalette()
{
	g_currentDisplayBackend->m_setPalette(0, 0x100, m_palette, m_unk0x22);
}

// Presents the frame: reloads the palette if it changed (redrawing the whole screen), otherwise
// blits the dirty rectangle, then empties it.
// FUNCTION: MW2SHELL 0x10006502
void VideoDriver::DrawShell()
{
	if (m_screenView.m_left > m_dirtyView.m_left) {
		m_dirtyView.m_left = m_screenView.m_left;
	}
	if (m_dirtyView.m_top < m_screenView.m_top) {
		m_dirtyView.m_top = m_screenView.m_top;
	}
	if (m_dirtyView.m_right > m_screenView.m_right) {
		m_dirtyView.m_right = m_screenView.m_right;
	}
	if (m_dirtyView.m_bottom > m_screenView.m_bottom) {
		m_dirtyView.m_bottom = m_screenView.m_bottom;
	}

	if (m_unk0x1e) {
		if (m_unk0x22) {
			if (g_unk0x1005c2a4) {
				ClearPalette();
				g_currentRefreshMode->m_flip();
				g_currentDisplayBackend->m_setPalette(0, 0x100, m_palette, m_unk0x22);
				g_currentRefreshMode->m_flip();
				g_unk0x1005c2a4 = 0;
			}
			else {
				ActivateFramebuffer();
				g_currentDisplayBackend->m_setPalette(0, 0x100, m_palette, m_unk0x22);
				memcpy(m_screenBuffer.m_pixels, m_backBuffer.m_pixels, m_width * m_height);
				g_currentRefreshMode->m_flip();
			}
		}
		else if (m_unk0x3aa) {
			memcpy(m_backBuffer.m_pixels, m_screenBuffer.m_pixels, m_width * m_height);
			ActivateFramebuffer();
			g_currentDisplayBackend->m_setPalette(0, 0x100, m_palette, m_unk0x22);
			memcpy(m_screenBuffer.m_pixels, m_backBuffer.m_pixels, m_width * m_height);
			g_currentRefreshMode->m_flip();
			m_unk0x3aa = 0;
		}
		else {
			g_currentDisplayBackend->m_setPalette(0, 0x100, m_palette, m_unk0x22);
			g_currentRefreshMode
				->m_blitRect(m_screenView.m_left, m_screenView.m_top, m_screenView.m_right, m_screenView.m_bottom);
		}

		m_unk0x1e = 0;
	}
	else if (m_dirtyView.m_right >= m_dirtyView.m_left && m_dirtyView.m_top <= m_dirtyView.m_bottom) {
		if (g_menuVisible) {
			GdiBitBltRectWithMenu(m_dirtyView.m_left, m_dirtyView.m_top, m_dirtyView.m_right, m_dirtyView.m_bottom);
		}
		else {
			g_currentRefreshMode
				->m_blitRect(m_dirtyView.m_left, m_dirtyView.m_top, m_dirtyView.m_right, m_dirtyView.m_bottom);
		}
	}

	m_dirtyView.m_left = m_screenView.m_right;
	m_dirtyView.m_top = m_screenView.m_bottom;
	m_dirtyView.m_right = m_screenView.m_left;
	m_dirtyView.m_bottom = m_screenView.m_top;
}

// DrawShell for movie frames: the dirty rectangle is stretched to the screen, or drawn unscaled
// at (160, 140) with g_unk0x1006a9f0.
// Operand order: the original loads m_height first in both m_width * m_height (as in
// FUN_10005f21; the source operand order doesn't flip it).
// FUNCTION: MW2SHELL 0x10006842
void VideoDriver::DrawFmv()
{
	if (m_unk0x1e) {
		if (m_unk0x3aa) {
			memcpy(m_backBuffer.m_pixels, m_screenBuffer.m_pixels, m_width * m_height);
			ActivateFramebuffer();
			g_currentDisplayBackend->m_setPalette(0, 0x100, m_palette, m_unk0x22);
			memcpy(m_screenBuffer.m_pixels, m_backBuffer.m_pixels, m_width * m_height);
			m_unk0x3aa = 0;
		}
		else {
			g_currentDisplayBackend->m_setPalette(0, 0x100, m_palette, m_unk0x22);
		}

		m_unk0x1e = 0;
	}

	if (m_dirtyView.m_left <= m_dirtyView.m_right && m_dirtyView.m_top <= m_dirtyView.m_bottom) {
		if (g_unk0x1006a9f0) {
			FUN_10031106(m_dirtyView.m_left, m_dirtyView.m_top, m_dirtyView.m_right, m_dirtyView.m_bottom);
		}
		else {
			g_currentRefreshMode
				->m_stretchBlit(m_dirtyView.m_left, m_dirtyView.m_top, m_dirtyView.m_right, m_dirtyView.m_bottom);
		}
	}

	m_dirtyView.m_left = m_screenView.m_right;
	m_dirtyView.m_top = m_screenView.m_bottom;
	m_dirtyView.m_right = m_screenView.m_left;
	m_dirtyView.m_bottom = m_screenView.m_top;
}

// FUNCTION: MW2SHELL 0x10006a04
void VideoDriver::GetPalette(PaletteColor* p_palette)
{
	memcpy(p_palette, m_palette, sizeof(m_palette));
}

// FUNCTION: MW2SHELL 0x10006a2f
void VideoDriver::SetPalette(PaletteColor* p_palette, undefined4 p_unk0x22)
{
	memcpy(m_palette, p_palette, sizeof(m_palette));
	m_unk0x1e = 1;
	m_unk0x22 = p_unk0x22;
}

// FUNCTION: MW2SHELL 0x10006a6d
void VideoDriver::FUN_10006a6d(undefined* p_data, MechS32 p_size, PaletteColor* p_palette)
{
	FUN_10037096(p_data, p_size, p_palette);
}

// FUNCTION: MW2SHELL 0x10006a99
void VideoDriver::FUN_10006a99(MechS32* p_maxX, MechS32* p_maxY, undefined* p_data, MechS32 p_size, MechS32 p_type)
{
	if (p_size) {
	}

	switch (p_type) {
	case 2:
		m_unk0x00 = FUN_100370c1(p_data);
		break;
	default:
		m_unk0x00 = 0;
		QuitWithVDriverError(0);
	}

	*p_maxX = (m_unk0x00 >> 16) - 1;
	*p_maxY = (m_unk0x00 & 0xffff) - 1;
}

// FUNCTION: MW2SHELL 0x10006b21
void VideoDriver::FUN_10006b21(
	undefined* p_unk0x0a,
	MechS32 p_unk0x0e,
	MechS32 p_unk0x12,
	MechU8 p_unk0x08,
	MechU8 p_unk0x09
)
{
	m_unk0x0a = p_unk0x0a;
	m_unk0x0e = p_unk0x0e;
	m_unk0x12 = p_unk0x12;
	m_unk0x08 = p_unk0x08;
	m_unk0x09 = p_unk0x09;

	FUN_10006a99(&m_unk0x39a, &m_unk0x39e, m_unk0x0a, m_unk0x0e, m_unk0x12);
	FUN_10006a6d(m_unk0x0a, m_unk0x0e, m_palette);
	m_unk0x1e = 1;
	m_unk0x22 = 1;

	if (ACQUIRE_FRAMEBUFFER() == 0) {
		FUN_10037014(&m_screenView, m_unk0x0a);
		FUN_10034f18(&m_screenView, 0, 0, &m_backView, 0, 0, -1);
	}

	ExpandRect(m_screenView.m_left, m_screenView.m_top, m_screenView.m_right, m_screenView.m_bottom);
}

// FUNCTION: MW2SHELL 0x10006c50
void VideoDriver::FUN_10006c50(TMPackDataBase* p_database, MechS32 p_id)
{
	MechS32 size;
	undefined* data;

	if (p_database->GetDBItemLZ(p_id, (void**) &data, &size)) {
		return;
	}

	FUN_10037096(data, size, g_unk0x10079698);
	if (memcmp(g_unk0x10079698, m_palette, sizeof(m_palette))) {
		memcpy(m_palette, g_unk0x10079698, sizeof(m_palette));
		m_unk0x1e = 1;
		m_unk0x22 = 1;
	}

	if (m_unk0x1e) {
		FUN_10037014(&m_backView, data);
		DrawShell();
	}
	else if (ACQUIRE_FRAMEBUFFER() == 0) {
		FUN_10037014(&m_screenView, data);
		memcpy(m_backBuffer.m_pixels, m_screenBuffer.m_pixels, m_width * m_height);
		ExpandRect(m_screenView.m_left, m_screenView.m_top, m_screenView.m_right, m_screenView.m_bottom);
	}

	HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, data);
}

// FUNCTION: MW2SHELL 0x10006da9
void VideoDriver::FUN_10006da9(undefined* p_data, MechS32 p_type, PixelView* p_view)
{
	switch (p_type) {
	case 2:
		if (ACQUIRE_FRAMEBUFFER() == 0) {
			FUN_10037014(p_view, p_data);
		}
		break;
	default:
		QuitWithVDriverError(0);
	}

	ExpandRect(m_screenView.m_left, m_screenView.m_top, m_screenView.m_right, m_screenView.m_bottom);
}

// FUNCTION: MW2SHELL 0x10006e51
void VideoDriver::FUN_10006e51(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom, MechS32 p_color)
{
	if (ACQUIRE_FRAMEBUFFER() == 0) {
		FUN_10032449(&m_screenView, p_left, p_top, p_right, p_bottom, 0, p_color);
	}

	ExpandRect(p_left, p_top, p_right, p_bottom);
}

// FUNCTION: MW2SHELL 0x10006ed4
void VideoDriver::FUN_10006ed4(undefined* p_pixels, MechS32 p_left, MechS32 p_top, MechS32 p_width, MechS32 p_height)
{
	PixelView view;
	PixelBuffer buffer;

	view.m_buffer = &buffer;
	view.m_left = 0;
	view.m_top = 0;
	view.m_right = buffer.m_maxX = p_width - 1;
	view.m_bottom = buffer.m_maxY = p_height - 1;
	buffer.m_pixels = p_pixels;

	if (ACQUIRE_FRAMEBUFFER() == 0) {
		FUN_10034f18(&view, 0, 0, &m_screenView, p_left, p_top, -1);
	}

	ExpandRectBySize(p_left, p_top, p_width, p_height);
}

// FUNCTION: MW2SHELL 0x10006f87
void VideoDriver::FUN_10006f87(undefined* p_pixels, MechS32 p_left, MechS32 p_top, MechS32 p_width, MechS32 p_height)
{
	PixelView view;
	PixelBuffer buffer;

	if (!IntersectsRectBySize(p_left, p_top, p_width, p_height)) {
		return;
	}

	view.m_buffer = &buffer;
	view.m_left = 0;
	view.m_top = 0;
	view.m_right = buffer.m_maxX = p_width - 1;
	view.m_bottom = buffer.m_maxY = p_height - 1;
	buffer.m_pixels = p_pixels;

	if (ACQUIRE_FRAMEBUFFER() == 0) {
		FUN_10034f18(&view, 0, 0, &m_screenView, p_left, p_top, -1);
	}

	ExpandRectBySize(p_left, p_top, p_width, p_height);
}

// FUNCTION: MW2SHELL 0x1000705f
void VideoDriver::FUN_1000705f(undefined* p_pixels, MechS32 p_left, MechS32 p_top, MechS32 p_width, MechS32 p_height)
{
	PixelView view;
	PixelBuffer buffer;

	view.m_buffer = &buffer;
	view.m_left = 0;
	view.m_top = 0;
	view.m_right = buffer.m_maxX = p_width - 1;
	view.m_bottom = buffer.m_maxY = p_height - 1;
	buffer.m_pixels = p_pixels;

	if (ACQUIRE_FRAMEBUFFER() == 0) {
		FUN_10034f18(&view, 0, 0, &m_screenView, p_left, p_top, -1);
	}

	ExpandRectBySize(p_left, p_top, p_width, p_height);
}

// FUNCTION: MW2SHELL 0x10007112
void VideoDriver::FUN_10007112(undefined* p_pixels, MechS32 p_left, MechS32 p_top, MechS32 p_width, MechS32 p_height)
{
	PixelView view;
	PixelBuffer buffer;

	view.m_buffer = &buffer;
	view.m_left = 0;
	view.m_top = 0;
	view.m_right = buffer.m_maxX = p_width - 1;
	view.m_bottom = buffer.m_maxY = p_height - 1;
	buffer.m_pixels = p_pixels;

	if (ACQUIRE_FRAMEBUFFER() == 0) {
		FUN_10034f18(&m_screenView, p_left, p_top, &view, 0, 0, -1);
	}
}

// FUNCTION: MW2SHELL 0x100071ad
void VideoDriver::FUN_100071ad(MechS32 p_left, MechS32 p_top, MechS32 p_width, MechS32 p_height)
{
	PixelView view;

	view.m_left = p_left;
	view.m_top = p_top;
	view.m_right = p_left + p_width - 1;
	view.m_bottom = p_top + p_height - 1;
	view.m_buffer = &m_backBuffer;

	if (ACQUIRE_FRAMEBUFFER() == 0) {
		FUN_10034f18(&view, 0, 0, &m_screenView, p_left, p_top, m_unk0x3a6);
	}

	ExpandRectBySize(p_left, p_top, p_width, p_height);
}

// FUNCTION: MW2SHELL 0x1000725d
void VideoDriver::FUN_1000725d()
{
	FUN_10034f18(&m_screenView, 0, 0, &m_backView, 0, 0, -1);
}

// FUNCTION: MW2SHELL 0x10007293
void VideoDriver::FUN_10007293()
{
	FUN_10034f18(&m_backView, 0, 0, &m_screenView, 0, 0, -1);
}

// FUNCTION: MW2SHELL 0x100072c9
void VideoDriver::LoadPalette(MechS32 p_id)
{
	MechS32 size;
	undefined* data;

	if (g_pDatabaseMw2->GetDBItemLZ(p_id, (void**) &data, &size)) {
		return;
	}

	FUN_10037096(data, size, m_palette);

	if (ACQUIRE_FRAMEBUFFER() == 0) {
		FUN_10037014(&m_screenView, data);
	}

	ExpandRect(m_screenView.m_left, m_screenView.m_top, m_screenView.m_right, m_screenView.m_bottom);
	m_unk0x1e = 1;
	m_unk0x22 = 1;
	g_unk0x1005c2a4 = 1;
	HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, data);
}

// FUNCTION: MW2SHELL 0x100073b3
void VideoDriver::FUN_100073b3(
	undefined4 p_unk0x00,
	undefined4 p_unk0x04,
	MechS32 p_left,
	MechS32 p_top,
	MechS32 p_width,
	MechS32 p_height
)
{
	if (ACQUIRE_FRAMEBUFFER() == 0) {
		FUN_10032f84(&m_screenView, p_unk0x00, p_unk0x04, p_left, p_top);
	}

	ExpandRectBySize(p_left, p_top, p_width, p_height);
}

// FUNCTION: MW2SHELL 0x10007430
void VideoDriver::FUN_10007430(
	undefined4 p_unk0x00,
	undefined4 p_unk0x04,
	MechS32 p_left,
	MechS32 p_top,
	MechS32 p_width,
	MechS32 p_height
)
{
	if (!IntersectsRectBySize(p_left, p_top, p_width, p_height)) {
		return;
	}

	if (ACQUIRE_FRAMEBUFFER() == 0) {
		FUN_10032f84(&m_screenView, p_unk0x00, p_unk0x04, p_left, p_top);
	}

	ExpandRectBySize(p_left, p_top, p_width, p_height);
}

// Only the stack slots differ: the original puts cursor at [ebp-4] and acquired at [ebp-0xc];
// VC++ 4.1 assigns them [ebp-0xc] and [ebp-4] here. width stays at [ebp-8].
// FUNCTION: MW2SHELL 0x100074d2
MechS32 VideoDriver::FUN_100074d2(
	MechS32 p_left,
	MechS32 p_top,
	void* p_unk0x08,
	MechChar* p_text,
	undefined* p_unk0x10
)
{
	MechChar* cursor;
	MechS32 width;
	MechS32 acquired;

	if (p_text == NULL) {
		return 0;
	}

	width = 0;
	for (cursor = p_text; *cursor != '\0'; cursor++) {
		width += FUN_10036abc(p_unk0x08, *cursor);
	}

	if (width == 0) {
		return width;
	}

	if (g_unk0x1005c2a0 && !IntersectsRectBySize(p_left, p_top, width, ((VideoFontHeader*) p_unk0x08)->m_height)) {
		return width;
	}

	if (p_unk0x10 == NULL) {
		p_unk0x10 = g_unk0x10079998;
	}

	acquired = ACQUIRE_FRAMEBUFFER();
	if (acquired == 0) {
		FUN_10036c67(&m_screenView, p_left, p_top, p_unk0x08, p_text, p_unk0x10);
	}

	ExpandRectBySize(p_left, p_top, width, ((VideoFontHeader*) p_unk0x08)->m_height);
	return width;
}

// Only the stack slots differ: the original puts width at [ebp-4] and acquired at [ebp-8];
// VC++ 4.1 assigns them [ebp-8] and [ebp-4] here.
// FUNCTION: MW2SHELL 0x10007603
MechS32 VideoDriver::FUN_10007603(MechS32 p_left, MechS32 p_top, void* p_unk0x08, MechChar p_char, undefined* p_unk0x10)
{
	MechS32 width;
	MechS32 acquired;

	width = FUN_10036abc(p_unk0x08, p_char);
	if (g_unk0x1005c2a0 && !IntersectsRectBySize(p_left, p_top, width, ((VideoFontHeader*) p_unk0x08)->m_height)) {
		return width;
	}

	if (p_unk0x10 == NULL) {
		p_unk0x10 = g_unk0x10079998;
	}

	acquired = ACQUIRE_FRAMEBUFFER();
	if (acquired == 0) {
		FUN_10036adc(&m_screenView, p_left, p_top, p_unk0x08, p_char, p_unk0x10);
	}

	ExpandRectBySize(p_left, p_top, width, ((VideoFontHeader*) p_unk0x08)->m_height);
	return width;
}

// FUNCTION: MW2SHELL 0x100076e8
void VideoDriver::FUN_100076e8(TextGlyph* p_item, MechS32 p_unk0x16)
{
	if (p_unk0x16) {
		m_unk0x16->FUN_1003e171(p_item);
	}
	else {
		m_unk0x1a->FUN_1003e171(p_item);
	}
}

// FUNCTION: MW2SHELL 0x1000772d
void VideoDriver::FUN_1000772d(TextGlyph* p_item)
{
	m_unk0x16->FUN_1003e19b(p_item);
	m_unk0x1a->FUN_1003e19b(p_item);
}

// FUNCTION: MW2SHELL 0x10007763
void VideoDriver::FUN_10007763(MechS32 p_unk0x16)
{
	g_unk0x1005c2a0 = 1;

	if (p_unk0x16) {
		m_unk0x16->FUN_1003e286();
	}
	else {
		m_unk0x1a->FUN_1003e286();
	}

	g_unk0x1005c2a0 = 0;
}

// FUNCTION: MW2SHELL 0x100077b4
void VideoDriver::FUN_100077b4(MechU8 p_delete)
{
	m_unk0x16->FUN_1003e1e6(p_delete);
	m_unk0x1a->FUN_1003e1e6(p_delete);
}

// FUNCTION: MW2SHELL 0x100077ea
void VideoDriver::ActivateFramebuffer()
{
	if (ACQUIRE_FRAMEBUFFER() == 0) {
		FUN_10034e15(&m_screenView, 0);
	}

	g_currentRefreshMode->m_flip();
}
