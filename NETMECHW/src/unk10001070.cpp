#include "unk10001070.h"

#include "types.h"
#include "unk10003660.h"

#include <ctype.h>
#include <string.h>
#include <windows.h>

// String and window helpers for the lobby's dialogs. A C++ unit: the void functions end with
// the jmp to the epilogue, and IsBlankString uses operator new.

// GLOBAL: NETMECHW 0x10023000
MechChar g_resStringIndex = 0;

// GLOBAL: NETMECHW 0x10023004
MechS32 g_busyCount = 0;

// GLOBAL: NETMECHW 0x1001c000
HCURSOR g_prevCursor;

// GLOBAL: NETMECHW 0x1001c008
MechChar g_resStrings[3][0x100];

// Loads a string resource into the next of three rotating buffers, so that a few results can
// be used at once.
// FUNCTION: NETMECHW 0x10001070
MechChar* LoadResString(UINT p_id)
{
	MechChar* string;

	strcpy(g_resStrings[g_resStringIndex], "");
	LoadString(g_hInstance, p_id, g_resStrings[g_resStringIndex], sizeof(g_resStrings[0]));
	string = g_resStrings[g_resStringIndex];

	if (g_resStringIndex > 1) {
		g_resStringIndex = 0;
	}
	else {
		g_resStringIndex = g_resStringIndex + 1;
	}

	return string;
}

// Matches except for the stack slots of desktop, desktopRect and rect, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000111a
void CenterWindow(HWND p_hWnd)
{
	HWND desktop;
	RECT desktopRect;
	RECT rect;

	desktop = GetDesktopWindow();
	GetWindowRect(p_hWnd, &rect);
	GetWindowRect(desktop, &desktopRect);
	rect.left = ((desktopRect.right - desktopRect.left) - (rect.right - rect.left)) / 2;
	rect.top = ((desktopRect.bottom - desktopRect.top) - (rect.bottom - rect.top)) / 2;
	SetWindowPos(p_hWnd, NULL, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

// Shows the hourglass while any caller is busy; calls nest.
// FUNCTION: NETMECHW 0x1000119a
void SetBusyCursor(MechS32 p_busy)
{
	if (p_busy) {
		if (!g_busyCount) {
			g_prevCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
		}

		g_busyCount++;
	}
	else if (g_busyCount && --g_busyCount == 0) {
		SetCursor(g_prevCursor);
	}
}

// Sizes p_hWnd to fit p_child's window rectangle in a bordered frame with a caption, and centers
// it on the desktop.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000120a
void FUN_1000120a(HWND p_hWnd, HWND p_child)
{
	WINDOWPLACEMENT placement;
	MechS32 screenHeight;
	MechS32 width;
	MechS32 height;
	HWND desktop;
	MechS32 screenWidth;
	MechS32 unk0x28;
	MechS32 unk0x24;
	RECT desktopRect;
	RECT rect;

	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);
	unk0x28 = 0;
	unk0x24 = 0;
	placement.length = sizeof(placement);
	GetWindowPlacement(p_hWnd, &placement);
	placement.length = sizeof(placement);

	GetWindowRect(p_child, &rect);
	width = rect.right - rect.left + GetSystemMetrics(SM_CXBORDER) * 2;
	height = rect.bottom - rect.top + GetSystemMetrics(SM_CYBORDER) * 2 + GetSystemMetrics(SM_CYCAPTION);

	desktop = GetDesktopWindow();
	GetWindowRect(desktop, &desktopRect);
	placement.rcNormalPosition.left = (desktopRect.right - desktopRect.left - width) / 2;
	placement.rcNormalPosition.top = (desktopRect.bottom - desktopRect.top - height) / 2;
	placement.rcNormalPosition.right = placement.rcNormalPosition.left + width;
	placement.rcNormalPosition.bottom = placement.rcNormalPosition.top + height;

	if (!IsWindowVisible(p_hWnd)) {
		placement.showCmd = SW_HIDE;
	}

	SetWindowPlacement(p_hWnd, &placement);
}

// Strips the leading whitespace: reversed, it is trailing.
// FUNCTION: NETMECHW 0x10001314
void TrimLeadingSpace(MechChar* p_string)
{
	MechU32 length;

	length = strlen(p_string);
	_strrev(p_string);

	while (length && isspace(p_string[length - 1])) {
		p_string[length - 1] = '\0';
		length--;
	}

	_strrev(p_string);
}

// Matches except for the stack slots of blank and copy, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x100013c2
MechS32 IsBlankString(MechChar* p_string)
{
	MechS32 blank;
	MechChar* copy;

	blank = FALSE;
	copy = new MechChar[strlen(p_string) + 1];
	strcpy(copy, p_string);
	TrimLeadingSpace(copy);

	if (*copy == '\0') {
		blank = TRUE;
	}
	else {
		blank = FALSE;
	}

	delete copy;
	return blank;
}
