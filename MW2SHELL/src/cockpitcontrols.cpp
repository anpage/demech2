#include "brasslantern0x414.h"
#include "decomp.h"
#include "emberglyph0x3e.h"
#include "inputdevice.h"
#include "slatetab0x2c.h"
#include "types.h"
#include "videodriver.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

// The cockpit controls screen: four configurations of bindings from the game's controls to
// device axes and buttons, and the fields that show and edit them.

// One game control's binding in a configuration.
// SIZE 0x18
struct CpcBinding {
	MechS32 m_channelKind;  // 0x00 — 0: axis, 1: button, 2: axis with a button
	MechS32 m_deviceSlot;   // 0x04
	MechS32 m_flags;        // 0x08 — bits 0-2: modifier, bit 31: inverted
	undefined4 m_unk0x0c;   // 0x0c
	MechS32 m_controlIndex; // 0x10 — the axis or button
	MechS32 m_mode;         // 0x14 — the button of an axis with a button
};

DECOMP_SIZE_ASSERT(CpcBinding, 0x18)

enum CpcConfig {
	c_configCount = 4,
	c_bindingCount = 0x25,
	c_buttonRows = 18
};

enum CpcBindingFlags {
	c_flagModifierMask = 0x07
};

// Not an enumerator: the unsigned constant makes `^=` on the signed flags a load, xor and store.
#define CPC_FLAG_INVERTED 0x80000000

extern BrassLantern0x414* g_unk0x10071210;
extern VideoDriver* g_pVideoDriver;

EmberGlyph0x3e* FUN_1003ea0f(
	BrassLantern0x414* p_font,
	MechS32 p_left,
	MechS32 p_top,
	MechChar* p_text,
	undefined* p_colors,
	MechS32 p_maxLength,
	MechS32 p_maxWidth
);
void CpcLoadActiveDevices(undefined4 p_unk0x00);
void CpcAcceptAndCommit(undefined4 p_unk0x00);

// The binding selected for editing, -1 for none.
// GLOBAL: MW2SHELL 0x1006af18
MechS32 g_unk0x1006af18 = -1;

// Which part of the selected binding is edited: 0 the axis or button, 1 the axis's button.
// GLOBAL: MW2SHELL 0x1006af1c
MechS32 g_unk0x1006af1c = 0;

// GLOBAL: MW2SHELL 0x1006af20
MechS32 g_curInputDeviceIdx = -1;

// GLOBAL: MW2SHELL 0x1006af28
MechChar* g_unk0x1006af28[c_bindingCount] = {
	"Throttle ",      "Chassis ",        "Turret Turn",    "Turret Tilt",     "Eye Point ",      "Eye Point ",
	"Eye Point ",     "Recenter Torso",  "Glance Left",    "Glance Right",    "Glance Up",       "Glance Down",
	"Jump Jets On",   "Jump Jets Front", "Jump Jets Back", "Jump Jets Left",  "Jump Jets Right", "Fire Weapon",
	"Cycle Weapon",   "Cycle Group",     "Fire Group 1",   "Fire Group 2",    "Fire Group 3",    "Group Toggle",
	"Target Next",    "Target Prev",     "Target Reticle", "Target Friendly", "Nearest Enemy",   "Inspect Target",
	"Next Nav Point",
};

// The configuration shown.
// GLOBAL: MW2SHELL 0x1006b054
MechS32 g_unk0x1006b054 = 0;

// GLOBAL: MW2SHELL 0x10090ad0
CpcBinding g_cpcBindings[c_configCount][c_bindingCount];

// GLOBAL: MW2SHELL 0x1006b058
CpcBinding* g_pCpcBindings = g_cpcBindings[0];

// The first button shown in the device's button list.
// GLOBAL: MW2SHELL 0x1006b05c
MechS32 g_unk0x1006b05c = 0;

// GLOBAL: MW2SHELL 0x1006b064
MechS32 g_cpcConfigured = 0;

// GLOBAL: MW2SHELL 0x1006b068
MechS32 g_inputConfigChanged = 0;

// GLOBAL: MW2SHELL 0x1006b078
MechChar* g_unk0x1006b078[c_configCount] =
	{"Primary Controls", "Secondary Controls", "Tertiary Controls", "Quaternary Controls"};

// Axis directions, normal and inverted, per kind of axis.
// GLOBAL: MW2SHELL 0x1006b088
MechChar* g_unk0x1006b088[4][2] = {{"-/+", "+/-"}, {"L/R", "R/L"}, {"D/U", "U/D"}, {"I/O", "O/I"}};

// Modifier names, by the modifier bits of a binding.
// GLOBAL: MW2SHELL 0x1006b0a8
MechChar* g_unk0x1006b0a8[5] = {"~--", "~Ctrl", "~Alt", "~--", "~Shft"};

// GLOBAL: MW2SHELL 0x1006b0c0
MechChar* g_unk0x1006b0c0[5] = {"Default", "Custom 1", "Custom 2", "Custom 3", "Custom 4"};

// GLOBAL: MW2SHELL 0x100927d8
MechS32 g_inputDeviceActive[16];

// GLOBAL: MW2SHELL 0x10092818
MechChar g_unk0x10092818[0x100];

// Color map for disabled fields.
// GLOBAL: MW2SHELL 0x10092a18
undefined g_unk0x10092a18[0x100];

// Color map for the selected field.
// GLOBAL: MW2SHELL 0x10092b18
undefined g_unk0x10092b18[0x100];

// FUNCTION: MW2SHELL 0x1003e9d0
EmberGlyph0x3e* FUN_1003e9d0(SlateTab0x2c* p_tab)
{
	MechChar* text;

	text = (MechChar*) p_tab->m_unk0x24;
	return g_unk0x10071210->FUN_1000544e(p_tab->m_left, p_tab->m_top, text, p_tab->m_unk0x14);
}

// STUB: MW2SHELL 0x1003ea0f
EmberGlyph0x3e* FUN_1003ea0f(BrassLantern0x414*, MechS32, MechS32, MechChar*, undefined*, MechS32, MechS32)
{
	STUB(0x1003ea0f);
	return NULL;
}

// FUNCTION: MW2SHELL 0x1003eef3
EmberGlyph0x3e* FUN_1003eef3(SlateTab0x2c* p_tab)
{
	MechChar* text;

	text = (MechChar*) p_tab->m_unk0x24;
	return g_unk0x10071210->FUN_1000544e(p_tab->m_left, p_tab->m_top, text, p_tab->m_unk0x14);
}

// The device and axis or button bound to a control.
// Stack-slot permutation: device, label, colors and control.
// FUNCTION: MW2SHELL 0x1003ef32
EmberGlyph0x3e* FUN_1003ef32(SlateTab0x2c* p_tab)
{
	InputDevice* device;
	EmberGlyph0x3e* glyph;
	MechChar* label;
	undefined* colors;
	MechS32 control;

	control = g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_controlIndex;
	colors = p_tab->m_unk0x14;
	device = InputGetDevice(g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_deviceSlot);
	if ((MechS32) p_tab->m_unk0x24 == g_unk0x1006af18 && g_unk0x1006af1c == 0) {
		colors = g_unk0x10092b18;
	}

	if (device == NULL) {
		strcpy(g_unk0x10092818, "----");
	}
	else {
		if (control < 0) {
			label = "----";
		}
		else if (g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_channelKind == 0) {
			label = device->m_info.m_axisShortNames[control];
		}
		else {
			label = device->m_info.m_buttonShortNames[control];
		}

		if (label == NULL || *label == '\0' || *label == '*') {
			label = "----";
		}

		sprintf(
			g_unk0x10092818,
			"%s %s",
			strcmp(device->m_info.m_shortName, "keyboard") ? device->m_info.m_shortName : "key",
			label
		);
	}

	glyph = g_unk0x10071210->FUN_1000544e(p_tab->m_left, p_tab->m_top, g_unk0x10092818, colors);
	if (g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_channelKind != 2) {
		p_tab->m_width = 100;
	}
	else {
		p_tab->m_width = glyph->m_width;
	}

	return glyph;
}

// The button of a control bound to an axis with a button, placed after the previous field.
// Stack-slot permutation: device, glyph, colors and button.
// FUNCTION: MW2SHELL 0x1003f136
EmberGlyph0x3e* FUN_1003f136(SlateTab0x2c* p_tab)
{
	InputDevice* device;
	EmberGlyph0x3e* glyph;
	MechChar* label;
	undefined* colors;
	MechS32 button;

	button = g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_mode;
	colors = p_tab->m_unk0x14;
	device = InputGetDevice(g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_deviceSlot);
	if (device == NULL || g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_channelKind != 2) {
		return NULL;
	}

	if (p_tab[-1].m_glyph != NULL) {
		p_tab->m_left = p_tab[-1].m_glyph->m_right + 1;
	}
	if ((MechS32) p_tab->m_unk0x24 == g_unk0x1006af18 && g_unk0x1006af1c == 1) {
		colors = g_unk0x10092b18;
	}

	if (button < 0 || button > device->m_info.m_buttonCount) {
		label = "----";
	}
	else {
		label = device->m_info.m_buttonShortNames[button];
	}

	if (label == NULL || *label == '\0') {
		label = "----";
	}

	sprintf(g_unk0x10092818, "/%s", label);
	glyph = g_unk0x10071210->FUN_1000544e(p_tab->m_left, p_tab->m_top, g_unk0x10092818, colors);
	return glyph;
}

// FUNCTION: MW2SHELL 0x1003f283
EmberGlyph0x3e* FUN_1003f283(SlateTab0x2c* p_tab)
{
	return g_unk0x10071210
		->FUN_1000544e(p_tab->m_left, p_tab->m_top, g_unk0x1006af28[(MechS32) p_tab->m_unk0x24], p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x1003f2c0
EmberGlyph0x3e* FUN_1003f2c0(SlateTab0x2c* p_tab)
{
	sprintf(g_unk0x10092818, "%d", *(MechS32*) p_tab->m_unk0x24);
	return g_unk0x10071210->FUN_1000544e(p_tab->m_left, p_tab->m_top, g_unk0x10092818, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x1003f30f
EmberGlyph0x3e* FUN_1003f30f(SlateTab0x2c* p_tab)
{
	return g_unk0x10071210
		->FUN_1000544e(p_tab->m_left, p_tab->m_top, g_unk0x1006b078[g_unk0x1006b054], p_tab->m_unk0x14);
}

// Show the next configuration.
// FUNCTION: MW2SHELL 0x1003f34b
void FUN_1003f34b(SlateTab0x2c* p_tab)
{
	if (p_tab) {
	}

	g_unk0x1006b054++;
	if (g_unk0x1006b054 >= c_configCount) {
		g_unk0x1006b054 = 0;
	}

	g_pCpcBindings = g_cpcBindings[g_unk0x1006b054];
}

// An axis's direction, placed after the previous field.
// Stack-slot permutation: inverted and glyph.
// FUNCTION: MW2SHELL 0x1003f39e
EmberGlyph0x3e* FUN_1003f39e(SlateTab0x2c* p_tab)
{
	MechS32 inverted;
	EmberGlyph0x3e* glyph;

	inverted = g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_flags & CPC_FLAG_INVERTED ? 1 : 0;
	if (p_tab[-1].m_glyph != NULL) {
		p_tab->m_left = p_tab[-1].m_glyph->m_right + 1;
	}

	glyph =
		g_unk0x10071210
			->FUN_1000544e(p_tab->m_left, p_tab->m_top, g_unk0x1006b088[(MechS32) p_tab->m_unk0x28][inverted], NULL);
	return glyph;
}

// FUNCTION: MW2SHELL 0x1003f42e
void FUN_1003f42e(SlateTab0x2c* p_tab)
{
	g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_flags ^= CPC_FLAG_INVERTED;
}

// FUNCTION: MW2SHELL 0x1003f469
EmberGlyph0x3e* FUN_1003f469(SlateTab0x2c* p_tab)
{
	return g_unk0x10071210->FUN_1000544e(
		p_tab->m_left + p_tab->m_width / 2,
		p_tab->m_top,
		g_unk0x1006b0a8[g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_flags & c_flagModifierMask],
		NULL
	);
}

// Cycle a binding's modifier: none, 1, 4, none.
// FUNCTION: MW2SHELL 0x1003f4be
void FUN_1003f4be(SlateTab0x2c* p_tab)
{
	switch (g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_flags & c_flagModifierMask) {
	case 0:
		g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_flags |= 1;
		break;
	case 1:
		g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_flags &= ~1;
		g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_flags |= 4;
		break;
	case 4:
		g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_flags &= ~c_flagModifierMask;
		break;
	}
}

// FUNCTION: MW2SHELL 0x1003f576
void FUN_1003f576(SlateTab0x2c*)
{
	g_cpcConfigured = 1;
}

// Scroll the button list to the selected binding's button.
// FUNCTION: MW2SHELL 0x1003f590
void FUN_1003f590()
{
	MechS32 count;

	count = InputGetDevice(g_curInputDeviceIdx)->m_info.m_buttonCount;
	if (g_pCpcBindings[g_unk0x1006af18].m_deviceSlot == g_curInputDeviceIdx &&
		g_pCpcBindings[g_unk0x1006af18].m_channelKind != 0) {
		if (g_unk0x1006af1c == 1) {
			g_unk0x1006b05c = g_pCpcBindings[g_unk0x1006af18].m_mode;
		}
		else {
			g_unk0x1006b05c = g_pCpcBindings[g_unk0x1006af18].m_controlIndex;
		}

		if (g_unk0x1006b05c < 0) {
			g_unk0x1006b05c = 0;
		}
		if (g_unk0x1006b05c + c_buttonRows >= count) {
			g_unk0x1006b05c = count - c_buttonRows;
		}
		if (g_unk0x1006b05c < 0) {
			g_unk0x1006b05c = 0;
		}
	}
}

// Advance a binding to the device's next named axis or button (p_mode: the axis's button);
// past the last one, the binding loses its device.
// Stack-slot permutation: names, index and count.
// FUNCTION: MW2SHELL 0x1003f677
void FUN_1003f677(CpcBinding* p_binding, MechS32 p_mode)
{
	InputDevice* device;
	MechChar** names;
	MechS32 index;
	MechS32 count;

	device = InputGetDevice(p_binding->m_deviceSlot);
	if (p_binding->m_channelKind == 0) {
		names = device->m_info.m_axisShortNames;
		count = device->m_info.m_axisCount;
	}
	else {
		names = device->m_info.m_buttonShortNames;
		count = device->m_info.m_buttonCount;
	}

	if (!p_mode) {
		index = p_binding->m_controlIndex;
	}
	else {
		index = p_binding->m_mode;
	}

	if (index < 0) {
		index = -1;
	}

	while (++index < count && (names[index] == NULL || *names[index] == '\0' || *names[index] == '*')) {
	}

	if (index >= count) {
		p_binding->m_deviceSlot = -1;
		index = -1;
	}

	if (!p_mode) {
		p_binding->m_controlIndex = index;
	}
	else {
		p_binding->m_mode = index;
	}
}

// An axis of the current device.
// FUNCTION: MW2SHELL 0x1003fe7b
EmberGlyph0x3e* FUN_1003fe7b(SlateTab0x2c* p_tab)
{
	InputDevice* device;
	undefined* colors;

	device = InputGetDevice(g_curInputDeviceIdx);
	colors = p_tab->m_unk0x14;
	if ((MechS32) p_tab->m_unk0x24 >= device->m_info.m_axisCount) {
		return NULL;
	}
	if (device->m_info.m_axisShortNames[(MechS32) p_tab->m_unk0x24] == NULL) {
		return NULL;
	}
	if (device->m_info.m_axisNames[(MechS32) p_tab->m_unk0x24] == NULL) {
		return NULL;
	}
	if (*device->m_info.m_axisShortNames[(MechS32) p_tab->m_unk0x24] == '\0') {
		return NULL;
	}
	if (*device->m_info.m_axisNames[(MechS32) p_tab->m_unk0x24] == '\0') {
		return NULL;
	}

	if (g_unk0x1006af18 < 0 || g_pCpcBindings[g_unk0x1006af18].m_channelKind == 1) {
		colors = g_unk0x10092a18;
	}

	return g_unk0x10071210
		->FUN_1000544e(p_tab->m_left, p_tab->m_top, device->m_info.m_axisNames[(MechS32) p_tab->m_unk0x24], colors);
}

// A button of the current device, in the scrolled list.
// FUNCTION: MW2SHELL 0x1003ff95
EmberGlyph0x3e* FUN_1003ff95(SlateTab0x2c* p_tab)
{
	InputDevice* device;
	MechS32 index;
	MechChar* label;
	undefined* colors;

	device = InputGetDevice(g_curInputDeviceIdx);
	colors = p_tab->m_unk0x14;
	index = (MechS32) p_tab->m_unk0x24 + g_unk0x1006b05c;
	if (index >= device->m_info.m_buttonCount) {
		return NULL;
	}

	label = "----";
	if (device->m_info.m_buttonShortNames[index] == NULL || device->m_info.m_buttonNames[index] == NULL ||
		*device->m_info.m_buttonShortNames[index] == '\0' || *device->m_info.m_buttonNames[index] == '\0') {
		label = "----";
		colors = g_unk0x10092a18;
	}
	else {
		label = device->m_info.m_buttonNames[index];
		if (*device->m_info.m_buttonShortNames[index] == '*') {
			colors = g_unk0x10092a18;
		}
	}

	if (g_unk0x1006af18 < 0) {
		colors = g_unk0x10092a18;
	}

	return g_unk0x10071210->FUN_1000544e(p_tab->m_left, p_tab->m_top, label, colors);
}

// Bind the selected control to an axis of the current device.
// FUNCTION: MW2SHELL 0x100400b7
void FUN_100400b7(SlateTab0x2c* p_tab)
{
	InputDevice* device;

	device = InputGetDevice(g_curInputDeviceIdx);
	if (g_unk0x1006af18 < 0 || (MechS32) p_tab->m_unk0x24 >= device->m_info.m_axisCount) {
		return;
	}
	if (device->m_info.m_axisShortNames[(MechS32) p_tab->m_unk0x24] == NULL) {
		return;
	}
	if (device->m_info.m_axisNames[(MechS32) p_tab->m_unk0x24] == NULL) {
		return;
	}
	if (g_pCpcBindings[g_unk0x1006af18].m_channelKind == 1) {
		return;
	}

	g_pCpcBindings[g_unk0x1006af18].m_channelKind = 0;
	g_pCpcBindings[g_unk0x1006af18].m_deviceSlot = g_curInputDeviceIdx;
	g_pCpcBindings[g_unk0x1006af18].m_controlIndex = (MechS32) p_tab->m_unk0x24;
	g_pCpcBindings[g_unk0x1006af18].m_mode = -1;
	g_unk0x1006af1c = 0;
}

// The button list's scroll arrows: up for a negative step, down otherwise.
// Stack-slot permutation: colors and count.
// FUNCTION: MW2SHELL 0x100401b8
EmberGlyph0x3e* FUN_100401b8(SlateTab0x2c* p_tab)
{
	undefined* colors;
	MechS32 count;

	count = InputGetDevice(g_curInputDeviceIdx)->m_info.m_buttonCount;
	colors = p_tab->m_unk0x14;
	if (count <= c_buttonRows) {
		return NULL;
	}

	if ((MechS32) p_tab->m_unk0x24 < 0) {
		if (g_unk0x1006b05c == 0) {
			colors = g_unk0x10092a18;
		}
	}
	else if (g_unk0x1006b05c >= count - c_buttonRows) {
		colors = g_unk0x10092a18;
	}

	return g_unk0x10071210->FUN_1000544e(
		p_tab->m_left,
		p_tab->m_top,
		(MechChar*) ((MechS32) p_tab->m_unk0x24 >= 0 ? "\x02" : "\x01"),
		colors
	);
}

// Make an active device current.
// FUNCTION: MW2SHELL 0x10040a0d
void FUN_10040a0d(SlateTab0x2c* p_tab)
{
	if (g_inputDeviceActive[(MechS32) p_tab->m_unk0x24]) {
		if (InputGetDevice((MechS32) p_tab->m_unk0x24) != NULL) {
			g_curInputDeviceIdx = (MechS32) p_tab->m_unk0x24;
			g_unk0x1006b05c = 0;
		}
	}
}

// FUNCTION: MW2SHELL 0x10040a5d
EmberGlyph0x3e* FUN_10040a5d(SlateTab0x2c* p_tab)
{
	g_pVideoDriver->FUN_10006e51(p_tab->m_left, p_tab->m_top, p_tab->m_left, 0x1d6, 0x10);
	return NULL;
}

// FUNCTION: MW2SHELL 0x10040a94
EmberGlyph0x3e* FUN_10040a94(SlateTab0x2c* p_tab)
{
	MechS32 index;

	index = *(MechS32*) p_tab->m_unk0x24;
	if (index < 0 || index > 4) {
		index = 0;
	}

	return g_unk0x10071210->FUN_1000544e(p_tab->m_left, p_tab->m_top, g_unk0x1006b0c0[index], p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10040af7
void FUN_10040af7(SlateTab0x2c*)
{
	if (g_inputConfigChanged) {
		CpcLoadActiveDevices(0);
	}

	CpcAcceptAndCommit(0);
	g_cpcConfigured = 1;
}

// Text shown only while fewer than three devices are enumerated.
// FUNCTION: MW2SHELL 0x10040b32
EmberGlyph0x3e* FUN_10040b32(SlateTab0x2c* p_tab)
{
	MechChar* text;

	if (InputEnumDevices(FALSE) <= 2) {
		text = (MechChar*) p_tab->m_unk0x24;
	}
	else {
		text = NULL;
	}

	return g_unk0x10071210->FUN_1000544e(p_tab->m_left, p_tab->m_top, text, g_unk0x10092b18);
}

// Enumerate the devices again; the current device falls back to the last one.
// The original loads g_curInputDeviceIdx for the comparison; swapping the operands didn't flip it.
// FUNCTION: MW2SHELL 0x10040c21
void FUN_10040c21(SlateTab0x2c*)
{
	MechS32 count;

	count = InputEnumDevices(TRUE);
	if (count <= g_curInputDeviceIdx) {
		g_inputDeviceActive[g_curInputDeviceIdx] = 0;
		g_curInputDeviceIdx = count - 1;
	}

	CpcLoadActiveDevices(0);
}

// STUB: MW2SHELL 0x1004289c
void CpcLoadActiveDevices(undefined4)
{
	STUB(0x1004289c);
}

// STUB: MW2SHELL 0x10042940
void CpcAcceptAndCommit(undefined4)
{
	STUB(0x10042940);
}

// STUB: MW2SHELL 0x10042bcf
void OpenCockpitControls()
{
	STUB(0x10042bcf);
}
