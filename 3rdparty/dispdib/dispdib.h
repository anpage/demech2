/* DisplayDib (DISPDIB.DLL) window class declarations: only what the game uses. Win32 programs
   drive DisplayDib through a "DisplayDibWindow" created by the DLL and WM_COPYDATA messages;
   the Win95-era SDK's DISPDIB.H wraps this in __inline helpers, which the game's /Ob1 build
   expands at every call site. Modern SDKs no longer ship the header. */
#ifndef DISPDIB_H
#define DISPDIB_H

#include <windows.h>

#define DISPLAYDIB_NOWAIT 0x0040
#define DISPLAYDIB_DONTLOCKTASK 0x0200

#define DISPLAYDIB_WINDOW_CLASS "DisplayDibWindow"
#define DISPLAYDIB_DLL "DISPDIB.DLL"

#define DDM_SETFMT (WM_USER + 0)
#define DDM_DRAW (WM_USER + 1)
#define DDM_CLOSE (WM_USER + 2)

// windowsx.h's GetWindowInstance. 64-bit SDKs only define the pointer-sized form.
#ifdef GWLP_HINSTANCE
#define DisplayDibGetWindowInstance(hwnd) ((HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE))
#else
#define DisplayDibGetWindowInstance(hwnd) ((HINSTANCE) GetWindowLong(hwnd, GWL_HINSTANCE))
#endif

// Sends a DDM_ message whose lParam is a pointer, through WM_COPYDATA.
__inline UINT DisplayDibWindowMessage(HWND p_hwnd, UINT p_msg, WPARAM p_wParam, LPARAM p_lParam, DWORD p_size)
{
	COPYDATASTRUCT cds;

	cds.dwData = MAKELONG(p_msg, p_wParam);
	cds.cbData = p_lParam ? p_size : 0;
	cds.lpData = (LPVOID) p_lParam;
	return (UINT) SendMessage(p_hwnd, WM_COPYDATA, (WPARAM) (HWND) NULL, (LPARAM) (LPVOID) &cds);
}

// Loads DISPDIB.DLL (which registers the window class) and creates a screen-sized window.
__inline HWND DisplayDibWindowCreateEx(HWND p_hwndParent, HINSTANCE p_hInstance, DWORD p_style)
{
	DWORD show = 2;
	DWORD zero = 0;
	LPVOID params[4] = {NULL, &zero, &show, 0};

	if ((UINT) LoadModule(DISPLAYDIB_DLL, &params) < (UINT) HINSTANCE_ERROR) {
		return NULL;
	}

	return CreateWindow(
		DISPLAYDIB_WINDOW_CLASS,
		"",
		p_style,
		0,
		0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		p_hwndParent,
		NULL,
		(p_hInstance ? p_hInstance : DisplayDibGetWindowInstance(p_hwndParent)),
		NULL
	);
}

#define DisplayDibWindowCreate(hwndParent, hInstance) DisplayDibWindowCreateEx(hwndParent, hInstance, WS_POPUP)
#define DisplayDibWindowSetFmt(hwnd, lpbi)                                                                             \
	DisplayDibWindowMessage(                                                                                           \
		hwnd,                                                                                                          \
		DDM_SETFMT,                                                                                                    \
		0,                                                                                                             \
		(LPARAM) (LPVOID) (lpbi),                                                                                      \
		sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)                                                               \
	)
#define DisplayDibWindowDraw(hwnd, flags, bits, size)                                                                  \
	DisplayDibWindowMessage(hwnd, DDM_DRAW, (WPARAM) (UINT) (flags), (LPARAM) (LPVOID) (bits), (DWORD) (size))
#define DisplayDibWindowBegin(hwnd) ShowWindow(hwnd, SW_SHOWNORMAL)
#define DisplayDibWindowEnd(hwnd) ShowWindow(hwnd, SW_HIDE)
#define DisplayDibWindowClose(hwnd) SendMessage(hwnd, DDM_CLOSE, 0, 0)

#endif /* DISPDIB_H */
