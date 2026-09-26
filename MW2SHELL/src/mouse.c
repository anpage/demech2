#include "decomp.h"
#include "inputdriver.h"
#include "types.h"

#include <windows.h>

extern MechS32 g_fWindowActive;
extern MechS32 g_windowHeight;
extern MechS32 g_windowWidth;
extern HWND g_pWnd;

// FUN_10046bd9's bounds test keeps a jmp per return: an /Ob1-expanded inline function.
__inline MechS32 IsInsideWindow(POINT* p_point)
{
	if (p_point->x < 0 || p_point->x >= g_windowWidth) {
		return FALSE;
	}
	if (p_point->y < 0 || p_point->y >= g_windowHeight) {
		return FALSE;
	}

	return TRUE;
}

// GLOBAL: MW2SHELL 0x10071d44
BOOL g_cursorClipped = FALSE;

// GLOBAL: MW2SHELL 0x10071d48
undefined4 g_unk0x10071d48 = 0;

MechS32 GetMouseDeviceCount(void);
MechS32 FillMouseDeviceInfo(MechS32 p_index, void* p_info);
MechS32 FUN_10046b16(void);
MechS32 FUN_10046b28(void);
MechS32 CenterCursor(undefined4 p_unk0x00, MechS32 p_axis);
MechS32 FUN_10046bd9(undefined4 p_unk0x00, MechS32* p_position, MechU32* p_buttons);
MechS32 FUN_10046def(void);
MechS32 FUN_10046e01(void);
void FUN_10046e13(RECT* p_rect, MechS32 p_width, MechS32 p_height);

// GLOBAL: MW2SHELL 0x10071d50
InputDriverModule g_mouseDriver = {
	GetMouseDeviceCount,
	FillMouseDeviceInfo,
	FUN_10046b16,
	FUN_10046b28,
	CenterCursor,
	FUN_10046bd9,
	FUN_10046def,
	FUN_10046e01,
};

// GLOBAL: MW2SHELL 0x10095ec0
RECT g_cursorClipRect;

// FUNCTION: MW2SHELL 0x10046a70
MechS32 GetMouseDeviceCount(void)
{
	return 1;
}

// STUB: MW2SHELL 0x10046a85
MechS32 FillMouseDeviceInfo(MechS32 p_index, void* p_info)
{
	STUB(0x10046a85);
	return 0;
}

// FUNCTION: MW2SHELL 0x10046b16
MechS32 FUN_10046b16(void)
{
	return 0;
}

// FUNCTION: MW2SHELL 0x10046b28
MechS32 FUN_10046b28(void)
{
	ClipCursor(NULL);
	g_cursorClipped = FALSE;
	return 0;
}

// FUNCTION: MW2SHELL 0x10046b4c
MechS32 CenterCursor(undefined4 p_unk0x00, MechS32 p_axis)
{
	POINT point;

	if (g_fWindowActive) {
		GetCursorPos(&point);
		ScreenToClient(g_pWnd, &point);
		if (p_axis == 0) {
			point.y = g_windowHeight / 2;
		}
		else if (p_axis == 1) {
			point.x = g_windowWidth / 2;
		}

		ClientToScreen(g_pWnd, &point);
		SetCursorPos(point.x, point.y);
	}

	return 0;
}

// FUNCTION: MW2SHELL 0x10046bd9
MechS32 FUN_10046bd9(undefined4 p_unk0x00, MechS32* p_position, MechU32* p_buttons)
{
	MechS16 left;
	MechS16 middle;
	POINT point;
	MechS16 right;

	if (g_fWindowActive) {
		if (!g_cursorClipped || g_unk0x10071d48) {
			FUN_10046e13(&g_cursorClipRect, g_windowWidth, g_windowHeight);
			ClipCursor(&g_cursorClipRect);
			CenterCursor(p_unk0x00, 0);
			CenterCursor(p_unk0x00, 1);
			g_cursorClipped = TRUE;
			g_unk0x10071d48 = 0;
		}

		if (p_buttons) {
			if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
				left = 1;
			}
			else {
				left = 0;
			}
			if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
				right = 1;
			}
			else {
				right = 0;
			}
			if (GetAsyncKeyState(VK_MBUTTON) & 0x8000) {
				middle = 1;
			}
			else {
				middle = 0;
			}

			*p_buttons = (right << 2) | (middle << 1) | left;
		}

		if (p_position && GetCursorPos(&point)) {
			ScreenToClient(g_pWnd, &point);
			if (IsInsideWindow(&point)) {
				p_position[0] = ((point.y * 2 - g_windowHeight) << 16) / g_windowHeight;
				p_position[1] = ((point.x * 2 - g_windowWidth) << 16) / g_windowWidth;
			}
		}
	}
	else {
		if (g_cursorClipped) {
			ClipCursor(NULL);
			g_cursorClipped = FALSE;
		}

		*p_buttons = 0;
		return 0;
	}

	return 0;
}

// FUNCTION: MW2SHELL 0x10046def
MechS32 FUN_10046def(void)
{
	return 0;
}

// FUNCTION: MW2SHELL 0x10046e01
MechS32 FUN_10046e01(void)
{
	return 0;
}

// FUNCTION: MW2SHELL 0x10046e13
void FUN_10046e13(RECT* p_rect, MechS32 p_width, MechS32 p_height)
{
	POINT point;

	point.x = point.y = 0;
	ClientToScreen(g_pWnd, &point);
	p_rect->left = point.x;
	p_rect->top = point.y;

	point.x = p_width - 1;
	point.y = p_height - 1;
	ClientToScreen(g_pWnd, &point);
	p_rect->right = point.x;
	p_rect->bottom = point.y;
}
