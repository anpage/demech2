#include "joystick.h"

#include "debugprint.h"
#include "decomp.h"
#include "inputdeviceinfo.h"
#include "inputdriver.h"
#include "simmain.h"
#include "types.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

// The joystick input driver, on the multimedia joystick API.

MechS32 GetJoystickDeviceCount(void);
MechS32 FillJoystickDeviceInfo(MechS32 p_index, InputDeviceInfo* p_info);
MechS32 JoystickOpenDevice(InputDeviceInfo* p_info);
MechS32 JoystickCloseDevice(InputDeviceInfo* p_info);
MechS32 JoystickCenterAxis(void);
MechS32 JoystickPoll(void);
MechS32 JoystickReadKeyCode(void);
MechS32 JoystickFlushKeyCodes(void);

// GLOBAL: MW2 0x100a6f58
InputDriverModule g_joystickDriver = {
	GetJoystickDeviceCount,
	FillJoystickDeviceInfo,
	JoystickOpenDevice,
	JoystickCloseDevice,
	JoystickCenterAxis,
	JoystickPoll,
	JoystickReadKeyCode,
	JoystickFlushKeyCodes,
};

// The registry key and value FUN_1004ae29 reads.
// GLOBAL: MW2 0x100be8a0
MechChar g_unk0x100be8a0[0x40];

// GLOBAL: MW2 0x100be8e0
MechChar g_unk0x100be8e0[0x100];

// FUNCTION: MW2 0x10049e70
MechS32 GetJoystickDeviceCount(void)
{
	return joyGetNumDevs();
}

// STUB: MW2 0x10049e86
MechS32 FillJoystickDeviceInfo(MechS32 p_index, InputDeviceInfo* p_info)
{
	STUB(0x10049e86);
	return 0;
}

// STUB: MW2 0x1004a3be
MechS32 JoystickOpenDevice(InputDeviceInfo* p_info)
{
	STUB(0x1004a3be);
	return 0;
}

// Frees the names and the driver data FillJoystickDeviceInfo and JoystickOpenDevice allocated.
// FUNCTION: MW2 0x1004a947
MechS32 JoystickCloseDevice(InputDeviceInfo* p_info)
{
	if (p_info) {
		if (p_info->m_axisNames) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_info->m_axisNames);
		}

		if (p_info->m_axisShortNames) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_info->m_axisShortNames);
		}

		if (p_info->m_buttonNames) {
			if (*p_info->m_buttonNames) {
				HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, *p_info->m_buttonNames);
			}

			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_info->m_buttonNames);
		}

		if (p_info->m_buttonShortNames) {
			if (*p_info->m_buttonShortNames) {
				HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, *p_info->m_buttonShortNames);
			}

			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_info->m_buttonShortNames);
		}

		if (p_info->m_driverData) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_info->m_driverData);
		}
	}

	return 0;
}

// FUNCTION: MW2 0x1004aa59
MechS32 JoystickCenterAxis(void)
{
	return 0;
}

// STUB: MW2 0x1004aa6b
MechS32 JoystickPoll(void)
{
	STUB(0x1004aa6b);
	return 0;
}

// FUNCTION: MW2 0x1004ad42
MechS32 JoystickReadKeyCode(void)
{
	return 2;
}

// FUNCTION: MW2 0x1004ad57
MechS32 JoystickFlushKeyCodes(void)
{
	return 2;
}

// Scales a joystick axis reading about p_center to -0x10000..0x10000 by p_scale, with a dead
// zone of p_deadZone on either side.
// FUNCTION: MW2 0x1004ad6c
MechS32 FUN_1004ad6c(MechS32 p_value, MechS32 p_deadZone, MechS32 p_center, MechDouble p_scale)
{
	if ((p_value -= p_center) < 0) {
		if (p_value < -p_deadZone) {
			p_value += p_deadZone;
			p_value = p_value * p_scale;
			if (p_value < -0x10000) {
				p_value = -0x10000;
			}
		}
		else {
			p_value = 0;
		}
	}
	else if (p_value > p_deadZone) {
		p_value -= p_deadZone;
		p_value = p_value * p_scale;
		if (p_value > 0x10000) {
			p_value = 0x10000;
		}
	}
	else {
		p_value = 0;
	}

	return p_value;
}

// Reads the OEM name of joystick p_index of the driver p_driverKey from the registry into
// p_name. Returns TRUE on success.
// FUNCTION: MW2 0x1004ae29
BOOL FUN_1004ae29(MechS32 p_index, MechChar* p_driverKey, MechChar* p_name, MechU32 p_size)
{
	HKEY key;
	DWORD type;
	DWORD size;
	LONG result;

	sprintf(
		g_unk0x100be8e0,
		"System\\CurrentControlSet\\Control\\MediaResources\\Joystick\\%s\\CurrentJoystickSettings",
		p_driverKey
	);
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_unk0x100be8e0, 0, KEY_QUERY_VALUE, &key);
	if (result != ERROR_SUCCESS) {
		DebugPrint("Could not open registry joystick current config: %d\n", result);
		return FALSE;
	}

	sprintf(g_unk0x100be8e0, "Joystick%dOEMName", p_index + 1);
	size = sizeof(g_unk0x100be8a0);
	type = REG_SZ;
	result = RegQueryValueEx(key, g_unk0x100be8e0, NULL, &type, (LPBYTE) g_unk0x100be8a0, &size);
	if (result == ERROR_SUCCESS) {
		RegCloseKey(key);
		sprintf(
			g_unk0x100be8e0,
			"System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\%s",
			g_unk0x100be8a0
		);
		result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_unk0x100be8e0, 0, KEY_QUERY_VALUE, &key);
		if (result == ERROR_SUCCESS) {
			size = sizeof(g_unk0x100be8a0);
			result = RegQueryValueEx(key, "OEMName", NULL, &type, (LPBYTE) g_unk0x100be8a0, &size);
			if (result == ERROR_SUCCESS) {
				strncpy(p_name, g_unk0x100be8a0, p_size);
			}
		}
	}

	RegCloseKey(key);
	return result == ERROR_SUCCESS;
}

// Picks the name INPUT.MAP knows the joystick by: from its display name, or from the axes, hat
// and buttons it reports.
// FUNCTION: MW2 0x1004af8c
void FUN_1004af8c(InputDeviceInfo* p_info, JOYCAPS* p_caps)
{
	MechU16 caps;

	caps = p_caps->wCaps;
	if (strstr(p_info->m_displayName, "Tracker")) {
		strcpy(p_info->m_matchName, "tracker");
	}
	else if (
		strstr(p_info->m_displayName, "SideWinder") ||
		((caps & JOYCAPS_HASZ) && (caps & JOYCAPS_HASR) && (caps & JOYCAPS_HASPOV) && (caps & JOYCAPS_POV4DIR) &&
		 p_caps->wNumButtons == 8)
	) {
		strcpy(p_info->m_matchName, "sidewind");
	}
	else if (
		strstr(p_info->m_displayName, "Flightstick Pro") ||
		((caps & JOYCAPS_HASZ) && (caps & JOYCAPS_HASPOV) && (caps & JOYCAPS_POV4DIR) && p_caps->wNumButtons == 4)
	) {
		strcpy(p_info->m_matchName, "fltstick");
	}
	else if (
		strstr(p_info->m_displayName, "Thrustmaster") ||
		((caps & JOYCAPS_HASR) && (caps & JOYCAPS_HASPOV) && (caps & JOYCAPS_POV4DIR) && p_caps->wNumButtons == 4)
	) {
		strcpy(p_info->m_matchName, "tmaster");
	}
	else {
		strcpy(p_info->m_matchName, "joystick");
	}
}
