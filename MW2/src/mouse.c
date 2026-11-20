#include "mouse.h"

#include "decomp.h"
#include "inputdeviceinfo.h"
#include "inputdriver.h"
#include "simmain.h"
#include "types.h"

#include <stdio.h>
#include <windows.h>

// The mouse input driver, the simulator's copy of the shell's. Without /Ob1, the __inline bounds
// test IsInsideWindow isn't expanded as in the shell: the compiler emits it after the unit's
// functions, in its own 16-byte aligned section.

__inline MechS32 IsInsideWindow(MechS32 p_x, MechS32 p_y);

// GLOBAL: MW2 0x100ad218
// GLOBAL: MW2MATROX 0x100aa148
// GLOBAL: MW23DFX 0x10250330
// GLOBAL: MW2ATI 0x10184640
// GLOBAL: MW2S3 0x10065008
// GLOBAL: MW2POWERVR 0x101af6a8
MechChar* g_mouseAxisNames[] = {"Mouse Down/Up Movement", "Mouse Left/Right Movement"};

// GLOBAL: MW2 0x100ad220
// GLOBAL: MW2MATROX 0x100aa150
// GLOBAL: MW23DFX 0x10250338
// GLOBAL: MW2ATI 0x10184648
// GLOBAL: MW2S3 0x10065010
// GLOBAL: MW2POWERVR 0x101af6b0
MechChar* g_mouseAxisTypes[] = {"Down/Up", "Left/Right"};

// GLOBAL: MW2 0x100ad228
// GLOBAL: MW2MATROX 0x100aa158
// GLOBAL: MW23DFX 0x10250340
// GLOBAL: MW2ATI 0x10184650
// GLOBAL: MW2S3 0x10065018
// GLOBAL: MW2POWERVR 0x101af6b8
MechChar* g_mouseButtonNames[] = {"Left button", "Middle button", "Right button", NULL};

// Three entries, no terminator: g_cursorClipped follows in the original.
// GLOBAL: MW2 0x100ad238
// GLOBAL: MW2MATROX 0x100aa168
// GLOBAL: MW23DFX 0x10250350
// GLOBAL: MW2ATI 0x10184660
// GLOBAL: MW2S3 0x10065028
// GLOBAL: MW2POWERVR 0x101af6c8
MechChar* g_mouseButtonTypes[] = {"LeftBtn", "MiddleBtn", "RightBtn"};

// GLOBAL: MW2 0x100ad244
// GLOBAL: MW2MATROX 0x100aa174
// GLOBAL: MW23DFX 0x1025035c
// GLOBAL: MW2ATI 0x1018466c
// GLOBAL: MW2S3 0x10065034
// GLOBAL: MW2POWERVR 0x101af6d4
BOOL g_cursorClipped = FALSE;

// GLOBAL: MW2 0x100ad248
// GLOBAL: MW2MATROX 0x100aa178
// GLOBAL: MW23DFX 0x10250360
// GLOBAL: MW2ATI 0x10184670
// GLOBAL: MW2S3 0x10065038
// GLOBAL: MW2POWERVR 0x101af6d8
undefined4 g_reclipCursor = 0;

MechS32 GetMouseDeviceCount(void);
MechS32 FillMouseDeviceInfo(MechS32 p_index, InputDeviceInfo* p_info);
MechS32 MouseOpenDevice(void);
MechS32 MouseCloseDevice(void);
MechS32 CenterCursor(undefined4 p_unk0x00, MechS32 p_axis);
MechS32 MousePoll(undefined4 p_unk0x00, MechS32* p_position, MechU32* p_buttons);
MechS32 MouseReadKeyCode(void);
MechS32 MouseFlushKeyCodes(void);
void GetClientScreenRect(RECT* p_rect, MechS32 p_width, MechS32 p_height);

// GLOBAL: MW2 0x100ad250
// GLOBAL: MW2MATROX 0x100aa180
// GLOBAL: MW23DFX 0x10250368
// GLOBAL: MW2ATI 0x10184678
// GLOBAL: MW2S3 0x10065040
// GLOBAL: MW2POWERVR 0x101af6e0
InputDriverModule g_mouseDriver = {
	GetMouseDeviceCount,
	FillMouseDeviceInfo,
	MouseOpenDevice,
	MouseCloseDevice,
	CenterCursor,
	MousePoll,
	MouseReadKeyCode,
	MouseFlushKeyCodes,
};

// GLOBAL: MW2 0x100ad270
// GLOBAL: MW2MATROX 0x100aa1a0
// GLOBAL: MW23DFX 0x10250388
// GLOBAL: MW2ATI 0x10184698
// GLOBAL: MW2S3 0x10065060
// GLOBAL: MW2POWERVR 0x101af700
MechChar g_mouseDeviceName[8] = "mouse";

// GLOBAL: MW2 0x100ad278
// GLOBAL: MW2MATROX 0x100aa1a8
// GLOBAL: MW23DFX 0x10250390
// GLOBAL: MW2ATI 0x101846a0
// GLOBAL: MW2S3 0x10065068
// GLOBAL: MW2POWERVR 0x101af708
MechChar g_mouseDisplayName[8] = "Mouse";

// GLOBAL: MW2 0x100ad280
// GLOBAL: MW2MATROX 0x100aa1b0
// GLOBAL: MW23DFX 0x10250398
// GLOBAL: MW2ATI 0x101846a8
// GLOBAL: MW2S3 0x10065070
// GLOBAL: MW2POWERVR 0x101af710
MechChar g_mouseTypeName[8] = "mouse";

// GLOBAL: MW2 0x100e9230
// GLOBAL: MW2MATROX 0x101d4710
// GLOBAL: MW23DFX 0x10159f60
// GLOBAL: MW2ATI 0x1008df60
// GLOBAL: MW2S3 0x1014d260
// GLOBAL: MW2POWERVR 0x100844a0
RECT g_cursorClipRect;

// FUNCTION: MW2 0x100688e0
// FUNCTION: MW2MATROX 0x10050950
// FUNCTION: MW23DFX 0x10036020
// FUNCTION: MW2ATI 0x10034c20
// FUNCTION: MW2S3 0x1001b260
// FUNCTION: MW2POWERVR 0x1002f6a0
MechS32 GetMouseDeviceCount(void)
{
	return 1;
}

// FUNCTION: MW2 0x100688f5
// FUNCTION: MW2MATROX 0x10050965
// FUNCTION: MW23DFX 0x10036030
// FUNCTION: MW2ATI 0x10034c30
// FUNCTION: MW2S3 0x1001b270
// FUNCTION: MW2POWERVR 0x1002f6b0
MechS32 FillMouseDeviceInfo(MechS32 p_index, InputDeviceInfo* p_info)
{
	p_info->m_axisCount = 2;
	p_info->m_buttonCount = 3;
	sprintf(p_info->m_shortName, g_mouseDeviceName);
	sprintf(p_info->m_displayName, g_mouseDisplayName);
	sprintf(p_info->m_matchName, g_mouseTypeName);
	p_info->m_axisNames = g_mouseAxisNames;
	p_info->m_axisShortNames = g_mouseAxisTypes;
	p_info->m_buttonNames = g_mouseButtonNames;
	p_info->m_buttonShortNames = g_mouseButtonTypes;
	p_info->m_driverData = NULL;

	return 0;
}

// FUNCTION: MW2 0x10068986
// FUNCTION: MW2MATROX 0x100509f6
// FUNCTION: MW23DFX 0x100360a0
// FUNCTION: MW2ATI 0x10034ca0
// FUNCTION: MW2S3 0x1001b2e0
// FUNCTION: MW2POWERVR 0x1002f720
MechS32 MouseOpenDevice(void)
{
	return 0;
}

// FUNCTION: MW2 0x10068998
// FUNCTION: MW2MATROX 0x10050a08
// FUNCTION: MW23DFX 0x100360b0
// FUNCTION: MW2ATI 0x10034cb0
// FUNCTION: MW2S3 0x1001b2f0
// FUNCTION: MW2POWERVR 0x1002f730
MechS32 MouseCloseDevice(void)
{
	ClipCursor(NULL);
	g_cursorClipped = FALSE;
	return 0;
}

// FUNCTION: MW2 0x100689bc
// FUNCTION: MW2MATROX 0x10050a2c
// FUNCTION: MW23DFX 0x100360d0
// FUNCTION: MW2ATI 0x10034cd0
// FUNCTION: MW2S3 0x1001b310
// FUNCTION: MW2POWERVR 0x1002f750
MechS32 CenterCursor(undefined4 p_unk0x00, MechS32 p_axis)
{
	POINT point;

	if (g_windowActive) {
		GetCursorPos(&point);
		ScreenToClient(g_gameWindow, &point);
		if (p_axis == 0) {
			point.y = g_gameWindowHeight / 2;
		}
		else if (p_axis == 1) {
			point.x = g_gameWindowWidth / 2;
		}

		ClientToScreen(g_gameWindow, &point);
		SetCursorPos(point.x, point.y);
	}

	return 0;
}

// FUNCTION: MW2 0x10068a49
// FUNCTION: MW2MATROX 0x10050ab9
// FUNCTION: MW23DFX 0x10036150
// FUNCTION: MW2ATI 0x10034d50
// FUNCTION: MW2S3 0x1001b390
// FUNCTION: MW2POWERVR 0x1002f7d0
MechS32 MousePoll(undefined4 p_unk0x00, MechS32* p_position, MechU32* p_buttons)
{
	MechS16 left;
	MechS16 middle;
	POINT point;
	MechS16 right;

	if (g_windowActive) {
		if (!g_cursorClipped || g_reclipCursor) {
			GetClientScreenRect(&g_cursorClipRect, g_gameWindowWidth, g_gameWindowHeight);
			ClipCursor(&g_cursorClipRect);
			CenterCursor(p_unk0x00, 0);
			CenterCursor(p_unk0x00, 1);
			g_cursorClipped = TRUE;
			g_reclipCursor = 0;
		}

		if (p_buttons) {
			left = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
			right = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
			middle = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;

			*p_buttons = (right << 2) | (middle << 1) | left;
		}

		if (p_position && GetCursorPos(&point)) {
			ScreenToClient(g_gameWindow, &point);
			if (IsInsideWindow(point.x, point.y)) {
				p_position[0] = ((point.y * 2 - g_gameWindowHeight) << 16) / g_gameWindowHeight;
				p_position[1] = ((point.x * 2 - g_gameWindowWidth) << 16) / g_gameWindowWidth;
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

// FUNCTION: MW2 0x10068c19
// FUNCTION: MW2MATROX 0x10050ccf
// FUNCTION: MW23DFX 0x100362e0
// FUNCTION: MW2ATI 0x10034ee0
// FUNCTION: MW2S3 0x1001b520
// FUNCTION: MW2POWERVR 0x1002f960
MechS32 MouseReadKeyCode(void)
{
	return 0;
}

// FUNCTION: MW2 0x10068c2b
// FUNCTION: MW2MATROX 0x10050ce1
// FUNCTION: MW23DFX 0x100362f0
// FUNCTION: MW2ATI 0x10034ef0
// FUNCTION: MW2S3 0x1001b530
// FUNCTION: MW2POWERVR 0x1002f970
MechS32 MouseFlushKeyCodes(void)
{
	return 0;
}

// FUNCTION: MW2 0x10068c3d
// FUNCTION: MW2MATROX 0x10050cf3
// FUNCTION: MW23DFX 0x10036300
// FUNCTION: MW2ATI 0x10034f00
// FUNCTION: MW2S3 0x1001b540
// FUNCTION: MW2POWERVR 0x1002f980
void GetClientScreenRect(RECT* p_rect, MechS32 p_width, MechS32 p_height)
{
	POINT point;

	point.x = point.y = 0;
	ClientToScreen(g_gameWindow, &point);
	p_rect->left = point.x;
	p_rect->top = point.y;

	point.x = p_width - 1;
	point.y = p_height - 1;
	ClientToScreen(g_gameWindow, &point);
	p_rect->right = point.x;
	p_rect->bottom = point.y;
}

// Whether a client point lies inside the game window.
// FUNCTION: MW2 0x10068cb0
__inline MechS32 IsInsideWindow(MechS32 p_x, MechS32 p_y)
{
	if (p_x < 0 || p_x >= g_gameWindowWidth) {
		return FALSE;
	}
	if (p_y < 0 || p_y >= g_gameWindowHeight) {
		return FALSE;
	}

	return TRUE;
}
