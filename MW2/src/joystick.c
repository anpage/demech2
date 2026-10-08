#include "joystick.h"

#include "debugprint.h"
#include "decomp.h"
#include "inputdeviceinfo.h"
#include "inputdriver.h"
#include "joystickdata.h"
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
MechS32 JoystickPoll(JoystickData* p_data, MechS32* p_axes, MechU32* p_buttons);
MechS32 JoystickReadKeyCode(void);
MechS32 JoystickFlushKeyCodes(void);
MechS32 ScaleJoystickAxis(MechS32 p_value, MechS32 p_deadZone, MechS32 p_center, MechDouble p_scale);
BOOL ReadJoystickOemName(MechS32 p_index, MechChar* p_driverKey, MechChar* p_name, MechU32 p_size);
void ChooseJoystickMapName(InputDeviceInfo* p_info, JOYCAPS* p_caps);

// The names FillJoystickDeviceInfo gives the joystick's axes and POV directions, and their short
// names in INPUT.MAP.

// GLOBAL: MW2 0x100a6e08
// GLOBAL: MW2MATROX 0x100a5608
MechChar g_downUpAxisName[] = "Down/Up Movement";

// GLOBAL: MW2 0x100a6e20
// GLOBAL: MW2MATROX 0x100a5620
MechChar g_leftRightAxisName[] = "Left/Right Movement";

// GLOBAL: MW2 0x100a6e38
// GLOBAL: MW2MATROX 0x100a5638
MechChar g_throttleAxisName[] = "Throttle Control";

// GLOBAL: MW2 0x100a6e50
// GLOBAL: MW2MATROX 0x100a5650
MechChar g_rudderAxisName[] = "Rudder Movement";

// GLOBAL: MW2 0x100a6e60
// GLOBAL: MW2MATROX 0x100a5660
MechChar g_fifthAxisName[] = "5th axis Movement";

// GLOBAL: MW2 0x100a6e78
// GLOBAL: MW2MATROX 0x100a5678
MechChar g_sixthAxisName[] = "6th axis Movement";

// GLOBAL: MW2 0x100a6e90
// GLOBAL: MW2MATROX 0x100a5690
MechChar g_downUpAxisShortName[] = "Down/Up";

// GLOBAL: MW2 0x100a6e98
// GLOBAL: MW2MATROX 0x100a5698
MechChar g_leftRightAxisShortName[] = "Left/Right";

// GLOBAL: MW2 0x100a6ea8
// GLOBAL: MW2MATROX 0x100a56a8
MechChar g_throttleAxisShortName[] = "Throttle";

// GLOBAL: MW2 0x100a6eb8
// GLOBAL: MW2MATROX 0x100a56b8
MechChar g_rudderAxisShortName[] = "Rudder";

// GLOBAL: MW2 0x100a6ec0
// GLOBAL: MW2MATROX 0x100a56c0
MechChar g_fifthAxisShortName[] = "5thAxis";

// GLOBAL: MW2 0x100a6ec8
// GLOBAL: MW2MATROX 0x100a56c8
MechChar g_sixthAxisShortName[] = "6thAxis";

// GLOBAL: MW2 0x100a6ed0
// GLOBAL: MW2MATROX 0x100a56d0
MechChar g_headRollAxisName[] = "Left/Right Head Roll";

// GLOBAL: MW2 0x100a6ee8
// GLOBAL: MW2MATROX 0x100a56e8
MechChar g_headRollAxisShortName[] = "HeadRoll";

// GLOBAL: MW2 0x100a6ef8
// GLOBAL: MW2MATROX 0x100a56f8
MechChar g_hatUpName[] = "Hat Up";

// GLOBAL: MW2 0x100a6f00
// GLOBAL: MW2MATROX 0x100a5700
MechChar g_hatRightName[] = "Hat Right";

// GLOBAL: MW2 0x100a6f10
// GLOBAL: MW2MATROX 0x100a5710
MechChar g_hatDownName[] = "Hat Down";

// GLOBAL: MW2 0x100a6f20
// GLOBAL: MW2MATROX 0x100a5720
MechChar g_hatLeftName[] = "Hat Left";

// GLOBAL: MW2 0x100a6f30
// GLOBAL: MW2MATROX 0x100a5730
MechChar g_hatUpShortName[] = "HatUp";

// GLOBAL: MW2 0x100a6f38
// GLOBAL: MW2MATROX 0x100a5738
MechChar g_hatRightShortName[] = "HatRight";

// GLOBAL: MW2 0x100a6f48
// GLOBAL: MW2MATROX 0x100a5748
MechChar g_hatDownShortName[] = "HatDown";

// GLOBAL: MW2 0x100a6f50
// GLOBAL: MW2MATROX 0x100a5750
MechChar g_hatLeftShortName[] = "HatLeft";

// GLOBAL: MW2 0x100a6f58
// GLOBAL: MW2MATROX 0x100a5758
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

#ifdef MW2_MATROX
// The Matrox edition's JoystickPoll sums each axis over the reads between two polls that read the
// joystick, then keeps the sums as the axes it returns until the next.
// GLOBAL: MW2MATROX 0x100a5778
MechS32 g_joystickAxes[6] = {0};

// GLOBAL: MW2MATROX 0x100a5790
MechS32 g_joystickAxisSums[6] = {0};
#endif

// The registry key and value ReadJoystickOemName reads.
// GLOBAL: MW2 0x100be8a0
// GLOBAL: MW2MATROX 0x100c1ea0
MechChar g_joystickOemName[0x40];

// GLOBAL: MW2 0x100be8e0
// GLOBAL: MW2MATROX 0x100c1ee0
MechChar g_joystickRegistryKey[0x100];

// FUNCTION: MW2 0x10049e70
// FUNCTION: MW2MATROX 0x10025f40
MechS32 GetJoystickDeviceCount(void)
{
	return joyGetNumDevs();
}

// Fills p_info for joystick p_index: its OEM name from the registry (or "Joystick n"), its short
// name, its driver data and the names of its axes (a tracker's third axis is head roll) and
// buttons, the POV directions after the buttons. Returns 1 on failure.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10049e86
// FUNCTION: MW2MATROX 0x10025f56
MechS32 FillJoystickDeviceInfo(MechS32 p_index, InputDeviceInfo* p_info)
{
	JOYCAPS caps;
	MechChar** axisShortNames;
	MechChar** buttonNames;
	MechS32 i;
	MechChar** axisNames;
	MechChar** buttonShortNames;
	JoystickData* data;

	if (joyGetDevCaps(p_index, &caps, sizeof(caps))) {
		return 1;
	}

	p_info->m_axisNames = axisNames =
		HeapAlloc(g_primaryHeap, HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, 6 * sizeof(MechChar*));
	p_info->m_axisShortNames = axisShortNames =
		HeapAlloc(g_primaryHeap, HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, 6 * sizeof(MechChar*));
	p_info->m_buttonNames = buttonNames =
		HeapAlloc(g_primaryHeap, HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, 36 * sizeof(MechChar*));
	p_info->m_buttonShortNames = buttonShortNames =
		HeapAlloc(g_primaryHeap, HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, 36 * sizeof(MechChar*));
	if (!axisNames || !axisShortNames || !buttonNames || !buttonShortNames) {
		JoystickCloseDevice(p_info);
		return 1;
	}

	data = HeapAlloc(g_primaryHeap, HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, sizeof(JoystickData));
	if (!data) {
		return 1;
	}
	else {
		data->m_id = p_index;
		p_info->m_driverData = data;
	}

	if (!ReadJoystickOemName(p_index, caps.szRegKey, p_info->m_displayName, sizeof(p_info->m_displayName))) {
		sprintf(p_info->m_displayName, "Joystick %d", p_index + 1);
	}

	sprintf(p_info->m_shortName, "joystick%d", p_index + 1);
	ChooseJoystickMapName(p_info, &caps);
	axisNames[0] = g_downUpAxisName;
	axisShortNames[0] = g_downUpAxisShortName;
	axisNames[1] = g_leftRightAxisName;
	axisShortNames[1] = g_leftRightAxisShortName;
	i = 2;
	if (caps.wCaps & JOYCAPS_HASZ) {
		if (!memcmp(p_info->m_matchName, "tracker", 8)) {
			axisNames[i] = g_headRollAxisName;
			axisShortNames[i] = g_headRollAxisShortName;
			i++;
		}
		else {
			axisNames[i] = g_throttleAxisName;
			axisShortNames[i] = g_throttleAxisShortName;
			i++;
		}
	}

	if (caps.wCaps & JOYCAPS_HASR) {
		axisNames[i] = g_rudderAxisName;
		axisShortNames[i] = g_rudderAxisShortName;
		i++;
	}

	if (caps.wCaps & JOYCAPS_HASU) {
		axisNames[i] = g_fifthAxisName;
		axisShortNames[i] = g_fifthAxisShortName;
		i++;
	}

	if (caps.wCaps & JOYCAPS_HASV) {
		axisNames[i] = g_sixthAxisName;
		axisShortNames[i] = g_sixthAxisShortName;
		i++;
	}

	p_info->m_axisCount = i;
	buttonNames[0] = HeapAlloc(g_primaryHeap, HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, 32 * 12);
	buttonShortNames[0] = HeapAlloc(g_primaryHeap, HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, 32 * 12);
	if (!buttonNames[0] || !buttonShortNames[0]) {
		JoystickCloseDevice(p_info);
		return 1;
	}

	for (i = 0; i < (MechS32) caps.wNumButtons; i++) {
		sprintf(buttonNames[i], "Button %d", i + 1);
		sprintf(buttonShortNames[i], "Button%d", i + 1);
		buttonNames[i + 1] = buttonNames[i] + 12;
		buttonShortNames[i + 1] = buttonShortNames[i] + 12;
	}

	i = caps.wNumButtons;
	if (caps.wCaps & JOYCAPS_HASPOV) {
		buttonNames[i] = g_hatUpName;
		buttonShortNames[i] = g_hatUpShortName;
		i++;
		buttonNames[i] = g_hatRightName;
		buttonShortNames[i] = g_hatRightShortName;
		i++;
		buttonNames[i] = g_hatDownName;
		buttonShortNames[i] = g_hatDownShortName;
		i++;
		buttonNames[i] = g_hatLeftName;
		buttonShortNames[i] = g_hatLeftShortName;
		i++;
	}

	p_info->m_buttonCount = i;
	return 0;
}

// Opens the joystick: fails (1) without driver data, capabilities or while unplugged; otherwise
// sets the JOYINFOEX flags to read with, each axis's center, dead zone (a sixteenth of its range,
// an eighth for the rudder) and scale to +-0x10000, and the buttons the POV directions set: the
// four after the device's own. The Matrox edition reads the extended position only, without
// setting its size.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1004a3be
// FUNCTION: MW2MATROX 0x10026380
MechS32 JoystickOpenDevice(InputDeviceInfo* p_info)
{
	JoystickData* data;
	JOYCAPS caps;
#ifndef MW2_MATROX
	JOYINFO info;
#endif
	MechDouble range;
	MechS32 button;
	JOYINFOEX infoEx;

	data = p_info->m_driverData;
#ifdef MW2_MATROX
	if (!data || joyGetDevCaps(data->m_id, &caps, sizeof(caps)) ||
		joyGetPosEx(data->m_id, &infoEx) == JOYERR_UNPLUGGED) {
		return 1;
	}
#else
	if (!data) {
		return 1;
	}

	if (joyGetDevCaps(data->m_id, &caps, sizeof(caps))) {
		return 1;
	}

	if (caps.wNumAxes <= 3 && caps.wMaxButtons <= 4) {
		if (joyGetPos(data->m_id, &info) == JOYERR_UNPLUGGED) {
			return 1;
		}
	}
	else {
		memset(&infoEx, 0, sizeof(infoEx));
		infoEx.dwSize = sizeof(infoEx);
		if (joyGetPosEx(data->m_id, &infoEx) == JOYERR_UNPLUGGED) {
			return 1;
		}
	}
#endif

	data->m_flags = 0x483;
	data->m_centers[0] = caps.wXmin + (MechS32) ((range = caps.wXmax - caps.wXmin + 1) / 2.0);
	data->m_deadZones[0] = range / 16.0;
	data->m_scales[0] = 131072.0 / range;
	data->m_centers[1] = caps.wYmin + (MechS32) ((range = caps.wYmax - caps.wYmin + 1) / 2.0);
	data->m_deadZones[1] = range / 16.0;
	data->m_scales[1] = 131072.0 / range;
	if (caps.wCaps & JOYCAPS_HASZ) {
		data->m_flags |= JOY_RETURNZ;
		data->m_centers[2] = caps.wZmin + (MechS32) ((range = caps.wZmax - caps.wZmin + 1) / 2.0);
		data->m_deadZones[2] = range / 16.0;
		data->m_scales[2] = 131072.0 / range;
	}

	if (caps.wCaps & JOYCAPS_HASR) {
		data->m_flags |= JOY_RETURNR;
		data->m_centers[3] = caps.wRmin + (MechS32) ((range = caps.wRmax - caps.wRmin + 1) / 2.0);
		data->m_deadZones[3] = range / 8.0;
		data->m_scales[3] = 131072.0 / range;
	}

	if (caps.wCaps & JOYCAPS_HASU) {
		data->m_flags |= JOY_RETURNU;
		data->m_centers[4] = caps.wUmin + (MechS32) ((range = caps.wUmax - caps.wUmin + 1) / 2.0);
		data->m_deadZones[4] = range / 16.0;
		data->m_scales[4] = 131072.0 / range;
	}

	if (caps.wCaps & JOYCAPS_HASV) {
		data->m_flags |= JOY_RETURNV;
		data->m_centers[5] = caps.wVmin + (MechS32) ((range = caps.wVmax - caps.wVmin + 1) / 2.0);
		data->m_deadZones[5] = range / 16.0;
		data->m_scales[5] = 131072.0 / range;
	}

	if (caps.wCaps & JOYCAPS_HASPOV) {
		data->m_flags |= JOY_RETURNPOV;
		button = caps.wNumButtons;
		if (button >= 32) {
			data->m_povButtons[0] = 1;
		}
		else {
			data->m_povButtons[0] = 0;
		}

		data->m_povMasks[0] = 1 << (button % 32);
		button++;
		if (button >= 32) {
			data->m_povButtons[1] = 1;
		}
		else {
			data->m_povButtons[1] = 0;
		}

		data->m_povMasks[1] = 1 << (button % 32);
		button++;
		if (button >= 32) {
			data->m_povButtons[2] = 1;
		}
		else {
			data->m_povButtons[2] = 0;
		}

		data->m_povMasks[2] = 1 << (button % 32);
		button++;
		if (button >= 32) {
			data->m_povButtons[3] = 1;
		}
		else {
			data->m_povButtons[3] = 0;
		}

		data->m_povMasks[3] = 1 << (button % 32);
	}

	return 0;
}

// Frees the names and the driver data FillJoystickDeviceInfo and JoystickOpenDevice allocated.
// FUNCTION: MW2 0x1004a947
// FUNCTION: MW2MATROX 0x1002699d
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
// FUNCTION: MW2MATROX 0x10026aaf
MechS32 JoystickCenterAxis(void)
{
	return 0;
}

#ifdef MW2_MATROX
#include "joystickscale.c"
#endif

// Reads the joystick into the six axes (Y and X first, then Z, R, U and V as far as the device has
// them) and the two button words, the POV hat setting the buttons its directions map to.
// Returns 1 without somewhere to put them, else 0.
// The Matrox edition's differs from the original only in its stack slots (i and info).
// FUNCTION: MW2 0x1004aa6b
// FUNCTION: MW2MATROX 0x10026b7e
MechS32 JoystickPoll(JoystickData* p_data, MechS32* p_axes, MechU32* p_buttons)
{
#ifdef MW2_MATROX
	MMRESULT result;
	MechS32 i;
	JOYINFOEX info;

	if (!p_buttons || !p_axes) {
		return 1;
	}

	if (!g_windowActive) {
		p_buttons[0] = p_buttons[1] = 0;
		for (i = 0; i < 6; i++) {
			p_axes[i] = 0;
		}

		return 0;
	}

	info.dwFlags = p_data->m_flags;
	result = joyGetPosEx(p_data->m_id, &info);
	if (result == JOYERR_NOERROR) {
		p_buttons[0] = p_buttons[1] = 0;
		for (i = 0; i < 6; i++) {
			p_axes[i] = 0;
		}

		p_buttons[0] = info.dwButtons;
		if (p_data->m_flags & JOY_RETURNPOV) {
			switch (info.dwPOV) {
			case JOY_POVFORWARD:
				p_buttons[p_data->m_povButtons[0]] |= p_data->m_povMasks[0];
				break;
			case JOY_POVRIGHT:
				p_buttons[p_data->m_povButtons[1]] |= p_data->m_povMasks[1];
				break;
			case JOY_POVBACKWARD:
				p_buttons[p_data->m_povButtons[2]] |= p_data->m_povMasks[2];
				break;
			case JOY_POVLEFT:
				p_buttons[p_data->m_povButtons[3]] |= p_data->m_povMasks[3];
				break;
			}
		}

		g_joystickAxisSums[0] +=
			ScaleJoystickAxis(info.dwYpos, p_data->m_deadZones[1], p_data->m_centers[1], p_data->m_scales[1]);
		g_joystickAxisSums[1] +=
			ScaleJoystickAxis(info.dwXpos, p_data->m_deadZones[0], p_data->m_centers[0], p_data->m_scales[0]);
		i = 2;
		if (p_data->m_flags & JOY_RETURNZ) {
			g_joystickAxisSums[i] +=
				ScaleJoystickAxis(info.dwZpos, p_data->m_deadZones[2], p_data->m_centers[2], p_data->m_scales[2]);
			i++;
		}

		if (p_data->m_flags & JOY_RETURNR) {
			g_joystickAxisSums[i] +=
				ScaleJoystickAxis(info.dwRpos, p_data->m_deadZones[3], p_data->m_centers[3], p_data->m_scales[3]);
			i++;
		}

		if (p_data->m_flags & JOY_RETURNU) {
			g_joystickAxisSums[i] +=
				ScaleJoystickAxis(info.dwUpos, p_data->m_deadZones[4], p_data->m_centers[4], p_data->m_scales[4]);
			i++;
		}

		if (p_data->m_flags & JOY_RETURNV) {
			g_joystickAxisSums[i] +=
				ScaleJoystickAxis(info.dwVpos, p_data->m_deadZones[5], p_data->m_centers[5], p_data->m_scales[5]);
			i++;
		}

		for (i = 0; i < 6; i++) {
			g_joystickAxes[i] = g_joystickAxisSums[i];
			g_joystickAxisSums[i] = 0;
		}
	}
	else {
	}

	p_axes[0] = g_joystickAxes[0];
	p_axes[1] = g_joystickAxes[1];
	i = 2;
	if (p_data->m_flags & JOY_RETURNZ) {
		p_axes[i] = g_joystickAxes[i];
		i++;
	}

	if (p_data->m_flags & JOY_RETURNR) {
		p_axes[i] = g_joystickAxes[i];
		i++;
	}

	if (p_data->m_flags & JOY_RETURNU) {
		p_axes[i] = g_joystickAxes[i];
		i++;
	}

	if (p_data->m_flags & JOY_RETURNV) {
		p_axes[i] = g_joystickAxes[i];
		i++;
	}

	return 0;
#else
	MechS32 i;
	JOYINFOEX info;

	memset(&info, 0, sizeof(info));
	info.dwSize = sizeof(info);
	if (!p_buttons || !p_axes) {
		return 1;
	}

	for (i = 0; i < 6; i++) {
		p_axes[i] = 0;
	}

	p_buttons[0] = p_buttons[1] = 0;
	if (!g_windowActive) {
		return 0;
	}

	info.dwFlags = p_data->m_flags;
	if (joyGetPosEx(p_data->m_id, &info) == JOYERR_NOERROR) {
		p_buttons[0] = info.dwButtons;
		if (p_data->m_flags & JOY_RETURNPOV) {
			switch (info.dwPOV) {
			case JOY_POVFORWARD:
				p_buttons[p_data->m_povButtons[0]] |= p_data->m_povMasks[0];
				break;
			case JOY_POVRIGHT:
				p_buttons[p_data->m_povButtons[1]] |= p_data->m_povMasks[1];
				break;
			case JOY_POVBACKWARD:
				p_buttons[p_data->m_povButtons[2]] |= p_data->m_povMasks[2];
				break;
			case JOY_POVLEFT:
				p_buttons[p_data->m_povButtons[3]] |= p_data->m_povMasks[3];
				break;
			}
		}

		p_axes[0] = ScaleJoystickAxis(info.dwYpos, p_data->m_deadZones[1], p_data->m_centers[1], p_data->m_scales[1]);
		p_axes[1] = ScaleJoystickAxis(info.dwXpos, p_data->m_deadZones[0], p_data->m_centers[0], p_data->m_scales[0]);
		i = 2;
		if (p_data->m_flags & JOY_RETURNZ) {
			p_axes[i] =
				ScaleJoystickAxis(info.dwZpos, p_data->m_deadZones[2], p_data->m_centers[2], p_data->m_scales[2]);
			i++;
		}

		if (p_data->m_flags & JOY_RETURNR) {
			p_axes[i] =
				ScaleJoystickAxis(info.dwRpos, p_data->m_deadZones[3], p_data->m_centers[3], p_data->m_scales[3]);
			i++;
		}

		if (p_data->m_flags & JOY_RETURNU) {
			p_axes[i] =
				ScaleJoystickAxis(info.dwUpos, p_data->m_deadZones[4], p_data->m_centers[4], p_data->m_scales[4]);
			i++;
		}

		if (p_data->m_flags & JOY_RETURNV) {
			p_axes[i] =
				ScaleJoystickAxis(info.dwVpos, p_data->m_deadZones[5], p_data->m_centers[5], p_data->m_scales[5]);
			i++;
		}
	}

	return 0;
#endif
}

// FUNCTION: MW2 0x1004ad42
// FUNCTION: MW2MATROX 0x10026f79
MechS32 JoystickReadKeyCode(void)
{
	return 2;
}

// FUNCTION: MW2 0x1004ad57
// FUNCTION: MW2MATROX 0x10026f8e
MechS32 JoystickFlushKeyCodes(void)
{
	return 2;
}

#ifndef MW2_MATROX
#include "joystickscale.c"
#endif

// Reads the OEM name of joystick p_index of the driver p_driverKey from the registry into
// p_name. Returns TRUE on success.
// FUNCTION: MW2 0x1004ae29
// FUNCTION: MW2MATROX 0x10026fa3
BOOL ReadJoystickOemName(MechS32 p_index, MechChar* p_driverKey, MechChar* p_name, MechU32 p_size)
{
	HKEY key;
	DWORD type;
	DWORD size;
	LONG result;

	sprintf(
		g_joystickRegistryKey,
		"System\\CurrentControlSet\\Control\\MediaResources\\Joystick\\%s\\CurrentJoystickSettings",
		p_driverKey
	);
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_joystickRegistryKey, 0, KEY_QUERY_VALUE, &key);
	if (result != ERROR_SUCCESS) {
		DebugPrint("Could not open registry joystick current config: %d\n", result);
		return FALSE;
	}

	sprintf(g_joystickRegistryKey, "Joystick%dOEMName", p_index + 1);
	size = sizeof(g_joystickOemName);
	type = REG_SZ;
	result = RegQueryValueEx(key, g_joystickRegistryKey, NULL, &type, (LPBYTE) g_joystickOemName, &size);
	if (result == ERROR_SUCCESS) {
		RegCloseKey(key);
		sprintf(
			g_joystickRegistryKey,
			"System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM\\%s",
			g_joystickOemName
		);
		result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_joystickRegistryKey, 0, KEY_QUERY_VALUE, &key);
		if (result == ERROR_SUCCESS) {
			size = sizeof(g_joystickOemName);
			result = RegQueryValueEx(key, "OEMName", NULL, &type, (LPBYTE) g_joystickOemName, &size);
			if (result == ERROR_SUCCESS) {
				strncpy(p_name, g_joystickOemName, p_size);
			}
		}
	}

	RegCloseKey(key);
	return result == ERROR_SUCCESS;
}

// Picks the name INPUT.MAP knows the joystick by: from its display name, or from the axes, hat
// and buttons it reports.
// FUNCTION: MW2 0x1004af8c
// FUNCTION: MW2MATROX 0x10027106
void ChooseJoystickMapName(InputDeviceInfo* p_info, JOYCAPS* p_caps)
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
