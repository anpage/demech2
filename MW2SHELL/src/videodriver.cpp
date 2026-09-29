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

// Set while RedrawGlyphs runs: text outside the dirty rectangle is skipped.
// GLOBAL: MW2SHELL 0x1005c2a0
MechS32 g_redrawingGlyphs = 0;

// Set by LoadPalette: DrawShell loads the new palette between two flips of a black one.
// GLOBAL: MW2SHELL 0x1005c2a4
MechS32 g_clearPaletteOnDraw = 0;

// The black palette of ClearPalette, and LoadBackground's picture palette.
// GLOBAL: MW2SHELL 0x10079698
PaletteColor g_tempPalette[0x100];

// The text colors when a glyph has none: each color maps to itself, but 0 is transparent.
// GLOBAL: MW2SHELL 0x10079998
MechU8 g_defaultColorMap[0x100];

// 0 once the framebuffer can be drawn to; -1 while the window is inactive.
#define ACQUIRE_FRAMEBUFFER() (g_fWindowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1)

// FUNCTION: MW2SHELL 0x10005e70
void QuitWithVDriverError(MechS32 p_code)
{
	ShutdownRefreshMode();

	if (g_pAudioSubsystem) {
		delete g_pAudioSubsystem;
	}

	CloseAllVideos();
	DebugPrint("vdriver error\n");
	exit(p_code + 1);
}

// FUNCTION: MW2SHELL 0x10005eea
void ClearPalette()
{
	memset(g_tempPalette, 0, sizeof(g_tempPalette));
	g_currentDisplayBackend->m_setPalette(0, 0x100, g_tempPalette, 1);
}

// Matches except for the operand order of m_width * m_height in the back buffer allocation
// (the original loads m_height first). It tracks the number of symbols declared ahead of this
// unit: declaring AudioSubsystem's seventh new method flipped it (while fixing RestoreBackground's
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
	m_pictureMaxX = m_width - 1;
	m_pictureMaxY = m_height - 1;
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

	m_overlayGlyphs = new TextGlyphList();
	m_glyphs = new TextGlyphList();

	g_defaultColorMap[0] = 0xff;
	for (i = 1; i < 0x100; i++) {
		g_defaultColorMap[i] = i;
	}

	m_unk0x3a2 = 0;
	m_restoreColor = -1;
}

// FUNCTION: MW2SHELL 0x10006202
VideoDriver::~VideoDriver()
{
	delete m_overlayGlyphs;
	delete m_glyphs;
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
	g_currentDisplayBackend->m_setPalette(0, 0x100, m_palette, m_allColors);
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

	if (m_paletteChanged) {
		if (m_allColors) {
			if (g_clearPaletteOnDraw) {
				ClearPalette();
				g_currentRefreshMode->m_flip();
				g_currentDisplayBackend->m_setPalette(0, 0x100, m_palette, m_allColors);
				g_currentRefreshMode->m_flip();
				g_clearPaletteOnDraw = 0;
			}
			else {
				ActivateFramebuffer();
				g_currentDisplayBackend->m_setPalette(0, 0x100, m_palette, m_allColors);
				memcpy(m_screenBuffer.m_pixels, m_backBuffer.m_pixels, m_width * m_height);
				g_currentRefreshMode->m_flip();
			}
		}
		else if (m_fmvFirstFrame) {
			memcpy(m_backBuffer.m_pixels, m_screenBuffer.m_pixels, m_width * m_height);
			ActivateFramebuffer();
			g_currentDisplayBackend->m_setPalette(0, 0x100, m_palette, m_allColors);
			memcpy(m_screenBuffer.m_pixels, m_backBuffer.m_pixels, m_width * m_height);
			g_currentRefreshMode->m_flip();
			m_fmvFirstFrame = 0;
		}
		else {
			g_currentDisplayBackend->m_setPalette(0, 0x100, m_palette, m_allColors);
			g_currentRefreshMode
				->m_blitRect(m_screenView.m_left, m_screenView.m_top, m_screenView.m_right, m_screenView.m_bottom);
		}

		m_paletteChanged = 0;
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
// at (160, 140) with g_littleMovies.
// Operand order: the original loads m_height first in both m_width * m_height (as in
// FUN_10005f21; the source operand order doesn't flip it).
// FUNCTION: MW2SHELL 0x10006842
void VideoDriver::DrawFmv()
{
	if (m_paletteChanged) {
		if (m_fmvFirstFrame) {
			memcpy(m_backBuffer.m_pixels, m_screenBuffer.m_pixels, m_width * m_height);
			ActivateFramebuffer();
			g_currentDisplayBackend->m_setPalette(0, 0x100, m_palette, m_allColors);
			memcpy(m_screenBuffer.m_pixels, m_backBuffer.m_pixels, m_width * m_height);
			m_fmvFirstFrame = 0;
		}
		else {
			g_currentDisplayBackend->m_setPalette(0, 0x100, m_palette, m_allColors);
		}

		m_paletteChanged = 0;
	}

	if (m_dirtyView.m_left <= m_dirtyView.m_right && m_dirtyView.m_top <= m_dirtyView.m_bottom) {
		if (g_littleMovies) {
			GdiBlitCentered(m_dirtyView.m_left, m_dirtyView.m_top, m_dirtyView.m_right, m_dirtyView.m_bottom);
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
void VideoDriver::SetPalette(PaletteColor* p_palette, undefined4 p_allColors)
{
	memcpy(m_palette, p_palette, sizeof(m_palette));
	m_paletteChanged = 1;
	m_allColors = p_allColors;
}

// FUNCTION: MW2SHELL 0x10006a6d
void VideoDriver::LoadPicturePalette(undefined* p_data, MechS32 p_size, PaletteColor* p_palette)
{
	ReadPicturePalette(p_data, p_size, p_palette);
}

// FUNCTION: MW2SHELL 0x10006a99
void VideoDriver::ReadPictureSize(MechS32* p_maxX, MechS32* p_maxY, undefined* p_data, MechS32 p_size, MechS32 p_type)
{
	if (p_size) {
	}

	switch (p_type) {
	case 2:
		m_pictureSize = GetPictureSize(p_data);
		break;
	default:
		m_pictureSize = 0;
		QuitWithVDriverError(0);
	}

	*p_maxX = (m_pictureSize >> 16) - 1;
	*p_maxY = (m_pictureSize & 0xffff) - 1;
}

// Loads a picture, with its palette, and draws it to the screen and the background.
// FUNCTION: MW2SHELL 0x10006b21
void VideoDriver::ShowPicture(
	undefined* p_picture,
	MechS32 p_pictureLength,
	MechS32 p_pictureType,
	MechU8 p_unk0x08,
	MechU8 p_unk0x09
)
{
	m_picture = p_picture;
	m_pictureLength = p_pictureLength;
	m_pictureType = p_pictureType;
	m_unk0x08 = p_unk0x08;
	m_unk0x09 = p_unk0x09;

	ReadPictureSize(&m_pictureMaxX, &m_pictureMaxY, m_picture, m_pictureLength, m_pictureType);
	LoadPicturePalette(m_picture, m_pictureLength, m_palette);
	m_paletteChanged = 1;
	m_allColors = 1;

	if (ACQUIRE_FRAMEBUFFER() == 0) {
		BlitPicture(&m_screenView, m_picture);
		BlitView(&m_screenView, 0, 0, &m_backView, 0, 0, -1);
	}

	ExpandRect(m_screenView.m_left, m_screenView.m_top, m_screenView.m_right, m_screenView.m_bottom);
}

// FUNCTION: MW2SHELL 0x10006c50
void VideoDriver::LoadBackground(TMPackDataBase* p_database, MechS32 p_id)
{
	MechS32 size;
	undefined* data;

	if (p_database->GetDBItemLZ(p_id, (void**) &data, &size)) {
		return;
	}

	ReadPicturePalette(data, size, g_tempPalette);
	if (memcmp(g_tempPalette, m_palette, sizeof(m_palette))) {
		memcpy(m_palette, g_tempPalette, sizeof(m_palette));
		m_paletteChanged = 1;
		m_allColors = 1;
	}

	if (m_paletteChanged) {
		BlitPicture(&m_backView, data);
		DrawShell();
	}
	else if (ACQUIRE_FRAMEBUFFER() == 0) {
		BlitPicture(&m_screenView, data);
		memcpy(m_backBuffer.m_pixels, m_screenBuffer.m_pixels, m_width * m_height);
		ExpandRect(m_screenView.m_left, m_screenView.m_top, m_screenView.m_right, m_screenView.m_bottom);
	}

	HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, data);
}

// FUNCTION: MW2SHELL 0x10006da9
void VideoDriver::DrawPicture(undefined* p_data, MechS32 p_type, PixelView* p_view)
{
	switch (p_type) {
	case 2:
		if (ACQUIRE_FRAMEBUFFER() == 0) {
			BlitPicture(p_view, p_data);
		}
		break;
	default:
		QuitWithVDriverError(0);
	}

	ExpandRect(m_screenView.m_left, m_screenView.m_top, m_screenView.m_right, m_screenView.m_bottom);
}

// FUNCTION: MW2SHELL 0x10006e51
void VideoDriver::DrawLine(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom, MechS32 p_color)
{
	if (ACQUIRE_FRAMEBUFFER() == 0) {
		BlitLine(&m_screenView, p_left, p_top, p_right, p_bottom, 0, p_color);
	}

	ExpandRect(p_left, p_top, p_right, p_bottom);
}

// Draws a block of pixels (a video's frame) to the screen.
// FUNCTION: MW2SHELL 0x10006ed4
void VideoDriver::DrawPixels(undefined* p_pixels, MechS32 p_left, MechS32 p_top, MechS32 p_width, MechS32 p_height)
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
		BlitView(&view, 0, 0, &m_screenView, p_left, p_top, -1);
	}

	ExpandRectBySize(p_left, p_top, p_width, p_height);
}

// DrawPixels, skipped unless the block meets the dirty rectangle.
// FUNCTION: MW2SHELL 0x10006f87
void VideoDriver::DrawPixelsClipped(
	undefined* p_pixels,
	MechS32 p_left,
	MechS32 p_top,
	MechS32 p_width,
	MechS32 p_height
)
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
		BlitView(&view, 0, 0, &m_screenView, p_left, p_top, -1);
	}

	ExpandRectBySize(p_left, p_top, p_width, p_height);
}

// The same as DrawPixels.
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
		BlitView(&view, 0, 0, &m_screenView, p_left, p_top, -1);
	}

	ExpandRectBySize(p_left, p_top, p_width, p_height);
}

// FUNCTION: MW2SHELL 0x10007112
void VideoDriver::ReadPixels(undefined* p_pixels, MechS32 p_left, MechS32 p_top, MechS32 p_width, MechS32 p_height)
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
		BlitView(&m_screenView, p_left, p_top, &view, 0, 0, -1);
	}
}

// FUNCTION: MW2SHELL 0x100071ad
void VideoDriver::RestoreBackground(MechS32 p_left, MechS32 p_top, MechS32 p_width, MechS32 p_height)
{
	PixelView view;

	view.m_left = p_left;
	view.m_top = p_top;
	view.m_right = p_left + p_width - 1;
	view.m_bottom = p_top + p_height - 1;
	view.m_buffer = &m_backBuffer;

	if (ACQUIRE_FRAMEBUFFER() == 0) {
		BlitView(&view, 0, 0, &m_screenView, p_left, p_top, m_restoreColor);
	}

	ExpandRectBySize(p_left, p_top, p_width, p_height);
}

// FUNCTION: MW2SHELL 0x1000725d
void VideoDriver::CopyScreenToBackground()
{
	BlitView(&m_screenView, 0, 0, &m_backView, 0, 0, -1);
}

// FUNCTION: MW2SHELL 0x10007293
void VideoDriver::CopyBackgroundToScreen()
{
	BlitView(&m_backView, 0, 0, &m_screenView, 0, 0, -1);
}

// FUNCTION: MW2SHELL 0x100072c9
void VideoDriver::LoadPalette(MechS32 p_id)
{
	MechS32 size;
	undefined* data;

	if (g_pDatabaseMw2->GetDBItemLZ(p_id, (void**) &data, &size)) {
		return;
	}

	ReadPicturePalette(data, size, m_palette);

	if (ACQUIRE_FRAMEBUFFER() == 0) {
		BlitPicture(&m_screenView, data);
	}

	ExpandRect(m_screenView.m_left, m_screenView.m_top, m_screenView.m_right, m_screenView.m_bottom);
	m_paletteChanged = 1;
	m_allColors = 1;
	g_clearPaletteOnDraw = 1;
	HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, data);
}

// Draws frame p_frame of an SHP animation to the screen.
// FUNCTION: MW2SHELL 0x100073b3
void VideoDriver::DrawShpFrame(
	undefined4 p_shp,
	undefined4 p_frame,
	MechS32 p_left,
	MechS32 p_top,
	MechS32 p_width,
	MechS32 p_height
)
{
	if (ACQUIRE_FRAMEBUFFER() == 0) {
		BlitShpFrame(&m_screenView, p_shp, p_frame, p_left, p_top);
	}

	ExpandRectBySize(p_left, p_top, p_width, p_height);
}

// DrawShpFrame, skipped unless the frame meets the dirty rectangle.
// FUNCTION: MW2SHELL 0x10007430
void VideoDriver::DrawShpFrameClipped(
	undefined4 p_shp,
	undefined4 p_frame,
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
		BlitShpFrame(&m_screenView, p_shp, p_frame, p_left, p_top);
	}

	ExpandRectBySize(p_left, p_top, p_width, p_height);
}

// Only the stack slots differ: the original puts cursor at [ebp-4] and acquired at [ebp-0xc];
// VC++ 4.1 assigns them [ebp-0xc] and [ebp-4] here. width stays at [ebp-8].
// FUNCTION: MW2SHELL 0x100074d2
MechS32 VideoDriver::DrawString(MechS32 p_left, MechS32 p_top, void* p_font, MechChar* p_text, undefined* p_colorMap)
{
	MechChar* cursor;
	MechS32 width;
	MechS32 acquired;

	if (p_text == NULL) {
		return 0;
	}

	width = 0;
	for (cursor = p_text; *cursor != '\0'; cursor++) {
		width += FontGetCharWidth(p_font, *cursor);
	}

	if (width == 0) {
		return width;
	}

	if (g_redrawingGlyphs && !IntersectsRectBySize(p_left, p_top, width, ((VideoFontHeader*) p_font)->m_height)) {
		return width;
	}

	if (p_colorMap == NULL) {
		p_colorMap = g_defaultColorMap;
	}

	acquired = ACQUIRE_FRAMEBUFFER();
	if (acquired == 0) {
		BlitString(&m_screenView, p_left, p_top, p_font, p_text, p_colorMap);
	}

	ExpandRectBySize(p_left, p_top, width, ((VideoFontHeader*) p_font)->m_height);
	return width;
}

// Only the stack slots differ: the original puts width at [ebp-4] and acquired at [ebp-8];
// VC++ 4.1 assigns them [ebp-8] and [ebp-4] here.
// FUNCTION: MW2SHELL 0x10007603
MechS32 VideoDriver::DrawChar(MechS32 p_left, MechS32 p_top, void* p_font, MechChar p_char, undefined* p_colorMap)
{
	MechS32 width;
	MechS32 acquired;

	width = FontGetCharWidth(p_font, p_char);
	if (g_redrawingGlyphs && !IntersectsRectBySize(p_left, p_top, width, ((VideoFontHeader*) p_font)->m_height)) {
		return width;
	}

	if (p_colorMap == NULL) {
		p_colorMap = g_defaultColorMap;
	}

	acquired = ACQUIRE_FRAMEBUFFER();
	if (acquired == 0) {
		BlitChar(&m_screenView, p_left, p_top, p_font, p_char, p_colorMap);
	}

	ExpandRectBySize(p_left, p_top, width, ((VideoFontHeader*) p_font)->m_height);
	return width;
}

// FUNCTION: MW2SHELL 0x100076e8
void VideoDriver::AddGlyph(TextGlyph* p_item, MechS32 p_overlay)
{
	if (p_overlay) {
		m_overlayGlyphs->Add(p_item);
	}
	else {
		m_glyphs->Add(p_item);
	}
}

// FUNCTION: MW2SHELL 0x1000772d
void VideoDriver::RemoveGlyph(TextGlyph* p_item)
{
	m_overlayGlyphs->Remove(p_item);
	m_glyphs->Remove(p_item);
}

// FUNCTION: MW2SHELL 0x10007763
void VideoDriver::RedrawGlyphs(MechS32 p_overlay)
{
	g_redrawingGlyphs = 1;

	if (p_overlay) {
		m_overlayGlyphs->DrawAll();
	}
	else {
		m_glyphs->DrawAll();
	}

	g_redrawingGlyphs = 0;
}

// FUNCTION: MW2SHELL 0x100077b4
void VideoDriver::ClearGlyphs(MechU8 p_delete)
{
	m_overlayGlyphs->Clear(p_delete);
	m_glyphs->Clear(p_delete);
}

// FUNCTION: MW2SHELL 0x100077ea
void VideoDriver::ActivateFramebuffer()
{
	if (ACQUIRE_FRAMEBUFFER() == 0) {
		FUN_10034e15(&m_screenView, 0);
	}

	g_currentRefreshMode->m_flip();
}
