/* The input devices and INPUT.MAP. The unit is not named input.c: the CRT has an input.c (the
   scanf engine) whose line records reccmp confuses with it. */
#include "inputmap.h"

#include "decomp.h"
#include "error.h"
#include "inputdeviceinfo.h"
#include "inputdriver.h"
#include "joystick.h"
#include "keyboard.h"
#include "mouse.h"
#include "mw2log.h"
#include "refreshmode.h"
#include "simmain.h"
#include "types.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// GLOBAL: MW2 0x100b2514
MechS16 g_keyCode = 0;

// The outputs of the menu bindings (INPUT.MAP's menu_ sinks).

// GLOBAL: MW2 0x100b2588
MechS32 g_sinkMenuItem = 0;

// GLOBAL: MW2 0x100b258c
MechS32 g_sinkMenuValue = 0;

// GLOBAL: MW2 0x100b2592
MechS8 g_sinkMenuItemReset = 0;

// GLOBAL: MW2 0x100b2595
MechS8 g_sinkMenuValueReset = 0;

// GLOBAL: MW2 0x100b2596
MechS8 g_sinkMenuEnter = 0;

// GLOBAL: MW2 0x100b2597
MechS8 g_sinkMenuAbort = 0;

// GLOBAL: MW2 0x100b25a4
MechS32 g_gameplayInputEnabled = 1;

// GLOBAL: MW2 0x100b25a8
MechS32 g_keyboardDeviceIndex = 0;

// The line of INPUT.MAP being parsed.
// GLOBAL: MW2 0x100b25ac
MechS32 g_inputMapLine = 0;

// GLOBAL: MW2 0x100b7370
MechS32 g_inputDeviceCount = 0;

// The input driver classes, in the order RegisterInputDevice asks them for a device.
// GLOBAL: MW2 0x100b7378
InputDriverModule* g_inputDriverClasses[3] = {&g_keyboardDriver, &g_mouseDriver, &g_joystickDriver};

// The game key action of each key code (0: none), loaded from GAMEKEY.MAP.
// GLOBAL: MW2 0x100b8190
MechU8 g_gameKeyByKeyCode[0x800] = {0};

// The text of FUN_1007b7b1's message box.
// GLOBAL: MW2 0x100bf1d0
MechChar g_unk0x100bf1d0[0x400];

// GLOBAL: MW2 0x100bf5d0
MechS32 g_inputDevicePresent[5]; // length unknown

// GLOBAL: MW2 0x100bf5e8
InputDeviceInfo g_inputDeviceInfos[5]; // length unknown

// The INPUT.MAP name of each device.
// GLOBAL: MW2 0x100bf9c0
MechChar g_inputDeviceNames[5][0x28];

// GLOBAL: MW2 0x100bfa88
InputDriverModule* g_inputDrivers[5]; // length unknown

// Finds the device INPUT.MAP calls p_name among the drivers' devices and opens it. Returns its
// index, or -1 if it can't be opened; a missing device still takes an index.
// Stack-slot permutation: index, driver, i and count.
// FUNCTION: MW2 0x10079340
MechS32 RegisterInputDevice(MechChar* p_name)
{
	MechS32 index;
	MechS32 driver;
	MechS32 i;
	MechS32 count;

	for (driver = 0; driver < 3; driver++) {
		if (g_inputDriverClasses[driver]) {
			count = g_inputDriverClasses[driver]->m_getDeviceCount();
			if (count) {
				for (i = 0; i < count; i++) {
					index = g_inputDeviceCount;
					if (g_inputDeviceCount >= 5) {
						Error(0x11, "Too many input devices specified", 0);
						return -1;
					}

					if (!g_inputDriverClasses[driver]->m_fillDeviceInfo(i, &g_inputDeviceInfos[index])) {
						if (!_strcmpi(g_inputDeviceInfos[index].m_shortName, p_name)) {
							strcpy(g_inputDeviceNames[index], p_name);
							g_inputDrivers[index] = g_inputDriverClasses[driver];
							g_inputDeviceCount++;
							if (g_inputDrivers[index]->m_openDevice(&g_inputDeviceInfos[index])) {
								FUN_1007b7b1(0x70, NULL, g_inputDeviceInfos[index].m_displayName);
								g_inputDevicePresent[index] = FALSE;
								return -1;
							}
							else {
								g_inputDevicePresent[index] = TRUE;
								return index;
							}
						}
						else {
							g_inputDriverClasses[driver]->m_closeDevice(&g_inputDeviceInfos[index]);
						}
					}
				}
			}
		}
	}

	index = g_inputDeviceCount;
	g_inputDeviceCount++;
	strcpy(g_inputDeviceNames[index], p_name);
	g_inputDevicePresent[index] = FALSE;
	FUN_1007b7b1(0x6e, NULL, p_name);
	return -1;
}

// Returns the index of the device INPUT.MAP calls p_name, registering it the first time, or -1
// if it isn't there.
// Stack-slot permutation: index and i.
// FUNCTION: MW2 0x1007959c
MechS32 FindInputDevice(MechChar* p_name)
{
	MechS32 index;
	MechS32 i;

	for (i = 0; i < g_inputDeviceCount; i++) {
		if (!_strcmpi(g_inputDeviceNames[i], p_name)) {
			return g_inputDevicePresent[i] ? i : -1;
		}
	}

	index = RegisterInputDevice(p_name);
	if (!_strcmpi(p_name, "keyboard")) {
		if (index != -1) {
			g_keyboardDeviceIndex = index;
		}
		else {
			Error(0x11, "\nKeyboard initialization error", 0);
		}
	}

	return index;
}

// Returns the axis of device p_device that INPUT.MAP names p_name, a number or a short name, or
// -1.
// FUNCTION: MW2 0x1007966a
MechS32 FindInputAxis(MechS32 p_device, MechChar* p_name)
{
	MechS32 i;

	if (isdigit(*p_name)) {
		return atoi(p_name);
	}

	if (p_device == -1) {
		return -1;
	}

	if (!g_inputDeviceInfos[p_device].m_axisShortNames) {
		Error(0x11, "No input channel \"%s\". Line %d", p_name, g_inputMapLine, 0);
		return -1;
	}

	for (i = 0; i < g_inputDeviceInfos[p_device].m_axisCount; i++) {
		if (g_inputDeviceInfos[p_device].m_axisShortNames[i] &&
			!_strcmpi(g_inputDeviceInfos[p_device].m_axisShortNames[i], p_name)) {
			return i;
		}
	}

	FUN_1007b7b1(0x6f, p_name, g_inputDeviceInfos[p_device].m_displayName);
	return -1;
}

// Returns the button of device p_device that INPUT.MAP names p_name, a number or a short name,
// or -1.
// FUNCTION: MW2 0x100797de
MechS32 FindInputButton(MechS32 p_device, MechChar* p_name)
{
	MechS32 i;

	if (isdigit(*p_name)) {
		return atoi(p_name);
	}

	if (p_device == -1) {
		return -1;
	}

	if (!g_inputDeviceInfos[p_device].m_buttonShortNames) {
		Error(0x11, "No input channel \"%s\". Line %d", p_name, g_inputMapLine, 0);
		return -1;
	}

	for (i = 0; i < g_inputDeviceInfos[p_device].m_buttonCount; i++) {
		if (g_inputDeviceInfos[p_device].m_buttonShortNames[i] &&
			!_strcmpi(g_inputDeviceInfos[p_device].m_buttonShortNames[i], p_name)) {
			return i;
		}
	}

	FUN_1007b7b1(0x6f, p_name, g_inputDeviceInfos[p_device].m_displayName);
	return -1;
}

// Reads the next line of INPUT.MAP that isn't blank once its comment is cut off. Returns 0 at
// the end of the file.
// FUNCTION: MW2 0x100799b7
MechS32 ReadInputMapLine(MechChar* p_buffer, MechS32 p_size, FILE* p_file)
{
	MechChar* p;
	MechS32 found;

	found = FALSE;
	for (;;) {
		if (!fgets(p_buffer, p_size, p_file)) {
			return 0;
		}

		g_inputMapLine++;
		for (p = p_buffer; *p; p++) {
			if (*p == '\n' || *p == '#') {
				*p = '\0';
				break;
			}
			else if (!isspace(*p)) {
				found = TRUE;
			}
		}

		if (found) {
			return 1;
		}
	}
}

// STUB: MW2 0x1007a97a
MechS32 LoadInputMap(void)
{
	STUB(0x1007a97a);
	return 0;
}

// STUB: MW2 0x1007aba9
MechS32 LoadGamekeyMap(void)
{
	STUB(0x1007aba9);
	return 0;
}

// Loads INPUT.MAP and GAMEKEY.MAP and flushes the keyboard's key codes.
// FUNCTION: MW2 0x1007b177
void FirstInputs(void)
{
	LoadInputMap();
	LoadGamekeyMap();
	g_inputDrivers[g_keyboardDeviceIndex]->m_flushKeyCodes();
}

// STUB: MW2 0x1007b19b
void UpdateInputs(void)
{
	STUB(0x1007b19b);
}

// FUNCTION: MW2 0x1007b704
void CloseInputDevices(void)
{
	MechS32 i;

	i = g_inputDeviceCount;
	while (i--) {
		if (g_inputDevicePresent[i]) {
			g_inputDrivers[i]->m_closeDevice(&g_inputDeviceInfos[i]);
		}
	}
}

// FUNCTION: MW2 0x1007b768
void DisableGameplayInput(void)
{
	g_gameplayInputEnabled = FALSE;
}

// FUNCTION: MW2 0x1007b77d
void EnableGameplayInput(void)
{
	g_gameplayInputEnabled = TRUE;
}

// The game key action of p_keyCode.
// FUNCTION: MW2 0x1007b792
MechS16 LookupGameKey(MechS16 p_keyCode)
{
	return g_gameKeyByKeyCode[p_keyCode];
}

// Reports an input device error p_code in a message box: 0x6e a missing device, 0x6f a missing
// channel, 0x70 a device that doesn't open. Cancel exits the game.
// FUNCTION: MW2 0x1007b7b1
void FUN_1007b7b1(MechS32 p_code, MechChar* p_channel, MechChar* p_device)
{
	MechS32 hidden;

	hidden = FALSE;
	switch (p_code) {
	case 0x6e:
		sprintf(
			g_unk0x100bf1d0,
			"The input device \"%s\" does not exist or is not configured properly.\n\nTo eliminate the problem, "
			"you should check the Windows Joystick Control Panel settings, or reconfigure your Cockpit Controls "
			"in the Clan Hall without using \"%s\".\n\nPush OK to disable this device and continue, or push "
			"CANCEL to abort the mission.",
			p_device,
			p_device
		);
		break;
	case 0x6f:
		sprintf(
			g_unk0x100bf1d0,
			"The input channel \"%s\" on device \"%s\" does not exist.\n\nIf you have changed your joystick "
			"configuration, you should also reconfigure your Cockpit Controls in the Clan Hall.\n\nPush OK to "
			"disable this control and continue, or push CANCEL to abort the mission.",
			p_channel,
			p_device
		);
		break;
	case 0x70:
		sprintf(
			g_unk0x100bf1d0,
			"The input device \"%s\" is not connected properly.\n\nTo eliminate the problem, you should check "
			"that your joystick is plugged in correctly and check the Windows Control Panel settings. "
			"Alternatively, you could reconfigure your Cockpit Controls in the Clan Hall without using "
			"\"%s\".\n\nPush OK to disable this device and continue, or push CANCEL to abort the mission.",
			p_device,
			p_device
		);
		break;
	default:
		sprintf(
			g_unk0x100bf1d0,
			"An input device has caused an undefined error. Sorry, no other information is available.\nPush OK "
			"to ignore this error and continue, or push CANCEL to abort the mission."
		);
		break;
	}

	WriteToMw2Log(g_unk0x100bf1d0);
	if (ShowCursor(TRUE) <= 0) {
		hidden = TRUE;
		while (ShowCursor(TRUE) < 0) {
		}
	}

	if (g_windowMode == 1) {
		ShowWindow(g_gameWindow, SW_SHOWMINNOACTIVE);
	}

	if (MessageBox(g_gameWindow, g_unk0x100bf1d0, "MechWarrior2 Message", MB_OKCANCEL | MB_ICONASTERISK) == IDCANCEL) {
		FUN_1003ba07();
		exit(p_code);
	}

	if (g_windowMode == 1) {
		ShowWindow(g_gameWindow, SW_RESTORE);
	}

	if (hidden) {
		while (ShowCursor(FALSE) >= 0) {
		}
	}
}
