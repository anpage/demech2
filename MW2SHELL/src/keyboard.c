/* The keyboard object is shared with MW2.DLL (KeyboardReadKeyCode is byte-identical); both
   targets keep their own copy until one source matches both. */
#include "decomp.h"
#include "inputdriver.h"
#include "types.h"

#include <ctype.h>
#include <windows.h>

enum KeyCodeBuffer {
	c_keyCodeBufferSize = 64
};

// Key code modifier bits.
enum KeyCodeModifier {
	c_keyCodeControl = 0x100,
	c_keyCodeShift = 0x200,
	c_keyCodeAlt = 0x400
};

// g_keyModifiers bits.
enum KeyModifier {
	c_modifierShift = 0x400000,
	c_modifierControl = 0x800000,
	c_modifierAlt = 0x1000000
};

// GLOBAL: MW2SHELL 0x10079690
undefined4 g_keyCodeWriteIndex;

// GLOBAL: MW2SHELL 0x1007968c
undefined4 g_keyCodeReadIndex;

// A ring buffer of key codes, filled by KeyboardQueueKeyCode.
// GLOBAL: MW2SHELL 0x10096eb0
MechS16 g_keyCodes[c_keyCodeBufferSize];

// One bit per (remapped) scan code, set while the key is down.
// GLOBAL: MW2SHELL 0x10096ea0
undefined4 g_keyStates[4];

// GLOBAL: MW2SHELL 0x10096eac
undefined4 g_keyModifiers;

// clang-format off
// The key code for each virtual key; 0 falls back to MapVirtualKey.
// GLOBAL: MW2SHELL 0x1005bed8
MechS16 g_keyCodeMap[256] = {
	0,     0,     0,     0,     0,     0,     0,     0,     0x08,  0x09,  0,     0,     0,     0x0d,  0,     0,
	0,     0,     0,     0x1ff, 0,     0,     0,     0,     0,     0,     0,     0x1b,  0,     0,     0,     0,
	0x20,  0xc4,  0xc5,  0xc3,  0xc2,  0xc9,  0xc6,  0xc8,  0xc7,  0,     0,     0,     0,     0xc0,  0xc1,  0,
	0x30,  0x31,  0x32,  0x33,  0x34,  0x35,  0x36,  0x37,  0x38,  0x39,  0,     0,     0,     0,     0,     0,
	0,     0x61,  0x62,  0x63,  0x64,  0x65,  0x66,  0x67,  0x68,  0x69,  0x6a,  0x6b,  0x6c,  0x6d,  0x6e,  0x6f,
	0x70,  0x71,  0x72,  0x73,  0x74,  0x75,  0x76,  0x77,  0x78,  0x79,  0x7a,  0,     0,     0,     0,     0,
	0xc0,  0xc3,  0xc7,  0xc5,  0xc9,  0,     0xc8,  0xc2,  0xc6,  0xc4,  0xcb,  0xcd,  0,     0xcc,  0x2e,  0xca,
	0xb1,  0xb2,  0xb3,  0xb4,  0xb5,  0xb6,  0xb7,  0xb8,  0xb9,  0xba,  0xbb,  0xbc,
};
// clang-format on

// clang-format off
// The key state index of the extended (0xe0-prefixed) scan codes.
// GLOBAL: MW2SHELL 0x1005c0d8
MechS32 g_extendedScanCodeMap[0x59] = {
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0x55,  0x70,  0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0x59,  0,     0,     0x71,  0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0x45,  0,     0x5e,  0x5f,  0x60,  0,     0x61,  0,     0x62,  0,     0x63,
	0x64,  0x65,  0x66,  0x67,  0,     0,     0,     0x57,  0x58,
};
// clang-format on

MechS32 GetKeyboardDeviceCount(void);
MechS32 FillKeyboardDeviceInfo(MechS32 p_index, void* p_info);
MechS32 FUN_10004b06(void);
MechS32 FUN_10004b18(void);
MechS32 FUN_10004b2a(void);
MechS32 FUN_10004b3c(undefined4 p_unk0x00, undefined4 p_unk0x04, undefined4* p_keyStates);
MechS32 KeyboardReadKeyCode(MechS16* p_keyCode);
MechS32 KeyboardFlushKeyCodes(void);
void HandleKeyboardMessages(UINT p_msg, WPARAM p_wParam, LPARAM p_lParam);
void KeyboardQueueKeyCode(WPARAM p_virtualKey, LPARAM p_lParam);
void KeyboardRecordKeyState(WPARAM p_virtualKey, MechU32 p_lParam, BOOL p_pressed);

// GLOBAL: MW2SHELL 0x1005c240
InputDriverModule g_keyboardDriver = {
	GetKeyboardDeviceCount,
	FillKeyboardDeviceInfo,
	FUN_10004b06,
	FUN_10004b18,
	FUN_10004b2a,
	FUN_10004b3c,
	KeyboardReadKeyCode,
	KeyboardFlushKeyCodes,
};

// FUNCTION: MW2SHELL 0x10004a60
MechS32 GetKeyboardDeviceCount(void)
{
	return 1;
}

// STUB: MW2SHELL 0x10004a75
MechS32 FillKeyboardDeviceInfo(MechS32 p_index, void* p_info)
{
	STUB(0x10004a75);
	return 0;
}

// FUNCTION: MW2SHELL 0x10004b06
MechS32 FUN_10004b06(void)
{
	return 0;
}

// FUNCTION: MW2SHELL 0x10004b18
MechS32 FUN_10004b18(void)
{
	return 0;
}

// FUNCTION: MW2SHELL 0x10004b2a
MechS32 FUN_10004b2a(void)
{
	return 0;
}

// FUNCTION: MW2SHELL 0x10004b3c
MechS32 FUN_10004b3c(undefined4 p_unk0x00, undefined4 p_unk0x04, undefined4* p_keyStates)
{
	MechS32 i;

	if (!(g_keyModifiers & c_modifierAlt) && p_keyStates) {
		for (i = 0; i < 4; i++) {
			p_keyStates[i] = g_keyStates[i];
		}
	}

	return 0;
}

// FUNCTION: MW2SHELL 0x10004b99
MechS32 KeyboardReadKeyCode(MechS16* p_keyCode)
{
	if (g_keyCodeReadIndex == g_keyCodeWriteIndex) {
		*p_keyCode = 0;
	}
	else {
		*p_keyCode = g_keyCodes[g_keyCodeReadIndex];
		g_keyCodeReadIndex++;
		if (g_keyCodeReadIndex == c_keyCodeBufferSize) {
			g_keyCodeReadIndex = 0;
		}
	}

	return 0;
}

// FUNCTION: MW2SHELL 0x10004bf9
MechS32 KeyboardFlushKeyCodes(void)
{
	MSG msg;

	if (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)) {
		HandleKeyboardMessages(msg.message, msg.wParam, msg.lParam);
	}

	g_keyCodeReadIndex = g_keyCodeWriteIndex = 0;
	return 0;
}

// FUNCTION: MW2SHELL 0x10004c56
void HandleKeyboardMessages(UINT p_msg, WPARAM p_wParam, LPARAM p_lParam)
{
	MSG msg;
	BOOL done;

	done = FALSE;
	do {
		switch (p_msg) {
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			KeyboardRecordKeyState(p_wParam, p_lParam, TRUE);
			KeyboardQueueKeyCode(p_wParam, p_lParam);
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			KeyboardRecordKeyState(p_wParam, p_lParam, FALSE);
			break;
		}

		if (p_msg >= WM_SYSKEYDOWN) {
			DefWindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
		}

		if (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)) {
			TranslateMessage(&msg);
			p_msg = msg.message;
			p_wParam = msg.wParam;
			p_lParam = msg.lParam;
		}
		else {
			done = TRUE;
		}
	} while (!done);
}

// FUNCTION: MW2SHELL 0x10004d61
void KeyboardQueueKeyCode(WPARAM p_virtualKey, LPARAM p_lParam)
{
	MechS16 keyCode;

	keyCode = g_keyCodeMap[p_virtualKey];
	if (keyCode == 0) {
		keyCode = (MechChar) MapVirtualKey(p_virtualKey, 2);
		keyCode = tolower(keyCode);
	}

	if (keyCode != 0) {
		if (GetKeyState(VK_CONTROL) & 0x1000) {
			keyCode |= c_keyCodeControl;
		}
		if (GetKeyState(VK_MENU) & 0x1000) {
			keyCode |= c_keyCodeAlt;
		}
		if (GetKeyState(VK_SHIFT) & 0x1000) {
			keyCode |= c_keyCodeShift;
		}

		g_keyCodes[g_keyCodeWriteIndex] = keyCode;
		g_keyCodeWriteIndex++;
		if (g_keyCodeWriteIndex == c_keyCodeBufferSize) {
			g_keyCodeWriteIndex = 0;
		}
		else if (g_keyCodeReadIndex == g_keyCodeWriteIndex) {
			g_keyCodeReadIndex++;
			if (g_keyCodeReadIndex == c_keyCodeBufferSize) {
				g_keyCodeReadIndex = 0;
			}
		}
	}
}

// FUNCTION: MW2SHELL 0x10004e80
void KeyboardRecordKeyState(WPARAM p_virtualKey, MechU32 p_lParam, BOOL p_pressed)
{
	MechS32 key;

	key = (p_lParam >> 16) & 0xff;
	if (key != 0 && ((p_lParam >> 16) & 0x100) && key < 0x59) {
		key = g_extendedScanCodeMap[key];
	}

	if (key != 0) {
		key--;
		if (p_pressed) {
			g_keyStates[key / 32] |= 1 << (key & 0x1f);
		}
		else {
			g_keyStates[key / 32] &= ~(1 << (key & 0x1f));
		}

		switch (key) {
		case 0x29:
		case 0x35:
			if (p_pressed) {
				g_keyModifiers |= c_modifierShift;
			}
			else {
				g_keyModifiers &= ~c_modifierShift;
			}
			break;
		case 0x1c:
		case 0x6f:
			if (p_pressed) {
				g_keyModifiers |= c_modifierControl;
			}
			else {
				g_keyModifiers &= ~c_modifierControl;
			}
			break;
		case 0x37:
		case 0x70:
			if (p_pressed) {
				g_keyModifiers |= c_modifierAlt;
			}
			else {
				g_keyModifiers &= ~c_modifierAlt;
			}
			break;
		}
	}
}

// FUNCTION: MW2SHELL 0x10005045
void FUN_10005045(void)
{
	MechS32 i;

	for (i = 0; i < 4; i++) {
		g_keyStates[i] = 0;
	}
}

// FUNCTION: MW2SHELL 0x1000507f
MechS16 KeyboardPollKeyCode(void)
{
	MSG msg;
	MechS16 keyCode;

	KeyboardReadKeyCode(&keyCode);
	if (keyCode == 0 && PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)) {
		HandleKeyboardMessages(msg.message, msg.wParam, msg.lParam);
		KeyboardReadKeyCode(&keyCode);
	}

	return keyCode;
}
