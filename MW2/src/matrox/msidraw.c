/* The Matrox edition's display back end and refresh modes, in place of 1.1's DirectDraw ones
   (directdraw.c): one object of its own, between ray.c's and door.c's. It keeps directdraw.c's
   debug messages ("DDRAW_Init() Called...") but opens the display through the A3D renderer over
   MSI95.DLL, which creates the window, and draws into two 16-bit frame buffers in the
   renderer's texture heap. All four refresh modes are the same: flipping and blitting do
   nothing here. */
#include "matrox/msidraw.h"

#include "brightness.h"
#include "debugprint.h"
#include "decomp.h"
#include "displaybackend.h"
#include "matrox/a3d.h"
#include "palettecolor.h"
#include "refreshmode.h"
#include "render.h"
#include "simmain.h"
#include "types.h"
#include "window.h"

#include <windows.h>

void MsiPumpMessages(void);
void DDRAW_Close(void);
MechS32 DDRAW_Init(WINDOW* p_buffer, MechS32 p_width, MechS32 p_height);
LRESULT CALLBACK DDRAW_WindowProc(HWND p_hWnd, UINT p_msg, WPARAM p_wParam, LPARAM p_lParam);
void MsiEnd(void);
MechS32 MsiFlip(void);
MechS32 MsiStretchBlit(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
MechS32 MsiBlitRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
MechS32 MsiAcquireFramebuffer(void);
MechS32 MsiSetPalette(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette, MechS32 p_allColors);
MechS32 MsiSetPaletteWithBrightness(PaletteColor* p_palette);
MechS32 MsiBlendPalettes(PaletteColor* p_palette, MechS32 p_steps);

// Never accessed: there is nothing to name it after.
// GLOBAL: MW2MATROX 0x100bbc04
undefined4 g_unk0x100bbc04 = 0;

// GLOBAL: MW2MATROX 0x100bbc08
MechS32 g_msiInitialized = FALSE;

// The texture heap, which starts with the frame buffers.
// GLOBAL: MW2MATROX 0x100bbc0c
undefined* g_msiHeap = NULL;

// The frame buffer being drawn, and the other one.
// GLOBAL: MW2MATROX 0x100bbc10
undefined* g_msiDrawBuffer = NULL;

// GLOBAL: MW2MATROX 0x100bbc14
undefined* g_msiOtherBuffer = NULL;

// The end functions return nothing.
// GLOBAL: MW2MATROX 0x100bbc18
DisplayBackend g_msiBackend = {
	c_displayBackendDirectDraw,
	c_windowModeFullscreen,
	WS_POPUP,
	DDRAW_Init,
	(MechS32 (*)()) DDRAW_Close,
	MsiSetPalette,
	MsiSetPaletteWithBrightness,
	MsiBlendPalettes,
	MsiAcquireFramebuffer,
	0
};

// GLOBAL: MW2MATROX 0x100bbc40
RefreshMode g_msiFlipRefreshMode =
	{0, c_displayBackendDirectDraw, 1, 0, DDRAW_Init, (MechS32 (*)()) MsiEnd, MsiFlip, MsiBlitRect, MsiStretchBlit};

// GLOBAL: MW2MATROX 0x100bbc68
RefreshMode g_msiBlitFlipRefreshMode =
	{1, c_displayBackendDirectDraw, 1, 0, DDRAW_Init, (MechS32 (*)()) MsiEnd, MsiFlip, MsiBlitRect, MsiStretchBlit};

// GLOBAL: MW2MATROX 0x100bbc90
RefreshMode g_msiVideoMemoryRefreshMode =
	{2, c_displayBackendDirectDraw, 1, 0, DDRAW_Init, (MechS32 (*)()) MsiEnd, MsiFlip, MsiBlitRect, MsiStretchBlit};

// GLOBAL: MW2MATROX 0x100bbcb8
RefreshMode g_msiSystemMemoryRefreshMode =
	{3, c_displayBackendDirectDraw, 1, 0, DDRAW_Init, (MechS32 (*)()) MsiEnd, MsiFlip, MsiBlitRect, MsiStretchBlit};

// The display's window, which the renderer creates: set by its WM_CREATE.
// GLOBAL: MW2MATROX 0x100bbcdc
HWND g_msiWindow = NULL;

// FUNCTION: MW2MATROX 0x10087f40
void MsiPumpMessages(void)
{
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

// FUNCTION: MW2MATROX 0x10087f81
void DDRAW_Close(void)
{
	if (!g_msiInitialized) {
		return;
	}

	DebugPrint("DDRAW_Close() Called...\n");
	FUN_10066c00(g_msiHeap);
	A3D_shutdown();
	g_msiInitialized = FALSE;
	g_msiWindow = NULL;
}

// FUNCTION: MW2MATROX 0x10087fd2
MechS32 DDRAW_Init(WINDOW* p_buffer, MechS32 p_width, MechS32 p_height)
{
	if (g_msiInitialized) {
		return 0;
	}

	DebugPrint("DDRAW_Init() Called...\n");
	if (g_currentDisplayBackend->m_id != c_displayBackendDirectDraw) {
		g_currentDisplayBackend->m_end();
		g_currentDisplayBackend = g_displayBackends[c_displayBackendDirectDraw];
		if (g_currentDisplayBackend->m_windowMode != g_windowMode) {
			AdjustWindowSize(g_currentDisplayBackend);
		}
	}

	g_msiWindow = NULL;
	if (A3D_Init(DDRAW_WindowProc, p_width, p_height)) {
		return -1;
	}

	while (g_msiWindow == NULL) {
		MsiPumpMessages();
	}

	SetActiveWindow(g_msiWindow);
	ReleaseCapture();
	SetCapture(g_msiWindow);

	g_msiHeap = FUN_10066c10();
	g_msiDrawBuffer = g_msiHeap;
	g_msiOtherBuffer = g_msiHeap + p_height * p_width * 2;

	p_buffer->m_buffer = g_msiDrawBuffer;
	p_buffer->m_xMax = p_width * 2 - 1;
	p_buffer->m_yMax = p_height - 1;
	p_buffer->m_shadow = 0;
	p_buffer->m_bitmapInfo = &g_bitmapInfo;
	g_msiInitialized = TRUE;

	DebugPrint("DDRAW_Init() Complete!\n");
	return 0;
}

// FUNCTION: MW2MATROX 0x10088120
LRESULT CALLBACK DDRAW_WindowProc(HWND p_hWnd, UINT p_msg, WPARAM p_wParam, LPARAM p_lParam)
{
	if (p_msg >= WM_KEYFIRST && p_msg <= WM_KEYLAST) {
		return SimWindowProc(p_hWnd, p_msg, p_wParam, p_lParam);
	}

	switch (p_msg) {
	case WM_CREATE:
		g_msiWindow = p_hWnd;
		return 0;
	case WM_DESTROY:
		DebugPrint("DDRAW_WindowProc(): WM_DESTROY\n");
		ReleaseCapture();
		g_msiWindow = NULL;
		PostQuitMessage(0);
		return 0;
	default:
		return SimWindowProc(p_hWnd, p_msg, p_wParam, p_lParam);
	}

	return DefWindowProc(p_hWnd, p_msg, p_wParam, p_lParam);
}

// FUNCTION: MW2MATROX 0x100881fd
void MsiEnd(void)
{
	DDRAW_Close();
}

// FUNCTION: MW2MATROX 0x1008820d
MechS32 MsiFlip(void)
{
	return 0;
}

// FUNCTION: MW2MATROX 0x1008821f
MechS32 MsiStretchBlit(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	return MsiFlip();
}

// FUNCTION: MW2MATROX 0x10088234
MechS32 MsiBlitRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	return 0;
}

// FUNCTION: MW2MATROX 0x10088246
void FUN_10088246(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	FUN_10066c40(g_msiHeap, g_msiDrawBuffer, p_left, p_top, p_right - p_left, p_bottom - p_top, 0xfffe);
}

// FUNCTION: MW2MATROX 0x10088280
void FUN_10088280(PANE* p_pane)
{
	FUN_10088246(p_pane->m_x0, p_pane->m_y0, p_pane->m_x1 + 1, p_pane->m_y1 + 1);
}

// Starts a frame and fills the frame buffer with the color 0xfffe, 64 bytes at a time.
// FUNCTION: MW2MATROX 0x100882b1
void MsiClearFrame(void)
{
	undefined* buffer;
	MechS32 count;

	FUN_1005f710();
	buffer = g_mainPixelBuffer.m_buffer;
	count = (g_mainPixelBuffer.m_xMax + 1) * (g_mainPixelBuffer.m_yMax + 1);

	__asm {
		mov eax, 0xfffefffe
		mov ebx, buffer
		mov ecx, ebx
		add ecx, count
jmp_100882e7:
		mov dword ptr [ebx], eax
		mov dword ptr [ebx + 4], eax
		mov dword ptr [ebx + 8], eax
		mov dword ptr [ebx + 0xc], eax
		mov dword ptr [ebx + 0x10], eax
		mov dword ptr [ebx + 0x14], eax
		mov dword ptr [ebx + 0x18], eax
		mov dword ptr [ebx + 0x1c], eax
		mov dword ptr [ebx + 0x20], eax
		mov dword ptr [ebx + 0x24], eax
		mov dword ptr [ebx + 0x28], eax
		mov dword ptr [ebx + 0x2c], eax
		mov dword ptr [ebx + 0x30], eax
		mov dword ptr [ebx + 0x34], eax
		mov dword ptr [ebx + 0x38], eax
		mov dword ptr [ebx + 0x3c], eax
		add ebx, 0x40
		cmp ebx, ecx
		jb jmp_100882e7
	}
}

// Ends the frame and draws the next one into the other frame buffer.
// FUNCTION: MW2MATROX 0x10088326
void MsiSwapBuffers(void)
{
	undefined* buffer;

	buffer = g_msiOtherBuffer;
	g_msiOtherBuffer = g_msiDrawBuffer;
	g_msiDrawBuffer = buffer;
	FUN_1005f760();
	g_mainPixelBuffer.m_buffer = g_msiDrawBuffer;
}

// FUNCTION: MW2MATROX 0x1008835d
MechS32 MsiAcquireFramebuffer(void)
{
	return 0;
}

// FUNCTION: MW2MATROX 0x1008836f
MechS32 MsiSetPalette(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette, MechS32 p_allColors)
{
	MechS32 i;

	if (p_palette == NULL || p_first < 0 || p_first > 0xff || p_count <= 0 || p_count > 0x100 - p_first) {
		return -1;
	}

	for (i = 0; i < p_count; i++) {
		g_paletteColors[p_first + i] = p_palette[i];
	}

	return 0;
}

// FUNCTION: MW2MATROX 0x1008840e
MechS32 MsiSetPaletteWithBrightness(PaletteColor* p_palette)
{
	MechS32 i;

	if (p_palette == NULL) {
		return -1;
	}

	for (i = 0; i < 0x100; i++) {
		g_paletteColorsPreBrightness[i] = p_palette[i];
		CopyPaletteColorWithBrightness(&p_palette[i], &g_paletteColors[i]);
	}

	return 0;
}

// DdrawPaletteFade's computation (directdraw.c) into a local table, without the delays: the
// 16-bit display has no palette to fade.
// FUNCTION: MW2MATROX 0x10088497
MechS32 MsiBlendPalettes(PaletteColor* p_palette, MechS32 p_steps)
{
	PALETTEENTRY entries[0x100];
	MechS32 i;
	MechS32 j;
	MechDouble deltas[0x100][3];

	if (p_palette == NULL) {
		return -1;
	}

	for (i = 0; i < 0x100; i++) {
		deltas[i][0] = (MechDouble) ((p_palette[i].m_red - g_paletteColors[i].m_red) * 4) / p_steps;
		deltas[i][1] = (MechDouble) ((p_palette[i].m_green - g_paletteColors[i].m_green) * 4) / p_steps;
		deltas[i][2] = (MechDouble) ((p_palette[i].m_blue - g_paletteColors[i].m_blue) * 4) / p_steps;
	}

	i = p_steps;
	while (i--) {
		j = 0x100;
		while (j--) {
			entries[j].peRed = (MechU8) ((p_steps - i) * deltas[j][0]) + g_paletteColors[j].m_red * 4;
			entries[j].peGreen = (MechU8) ((p_steps - i) * deltas[j][1]) + g_paletteColors[j].m_green * 4;
			entries[j].peBlue = (MechU8) ((p_steps - i) * deltas[j][2]) + g_paletteColors[j].m_blue * 4;
			if (i == 0) {
				g_paletteColors[j].m_red = entries[j].peRed >> 2;
				g_paletteColors[j].m_green = entries[j].peGreen >> 2;
				g_paletteColors[j].m_blue = entries[j].peBlue >> 2;
			}
		}
	}

	return 0;
}
