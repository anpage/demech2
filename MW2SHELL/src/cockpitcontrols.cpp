#include "brasslantern0x414.h"
#include "decomp.h"
#include "emberglyph0x3e.h"
#include "hollowreed0x110.h"
#include "inputdevice.h"
#include "mousestate.h"
#include "shellmain.h"
#include "silverreel0x18.h"
#include "slatetab0x2c.h"
#include "types.h"
#include "videodriver.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

// The cockpit controls screen: four configurations of bindings from the game's controls to
// device axes and buttons, and the fields that show and edit them. The configurations are
// saved as .cpc files (giddi\configNN.cpc, and one per device) and written out as the sim's
// input.map.

// One game control's binding in a configuration.
// SIZE 0x18
struct CpcBinding {
	MechS32 m_channelKind;  // 0x00 — 0: axis, 1: button, 2: axis with a button
	MechS32 m_deviceSlot;   // 0x04
	MechS32 m_flags;        // 0x08 — bits 0-2: modifier, bit 31: inverted
	MechS32 m_modeFlags;    // 0x0c — the flags of the axis's button
	MechS32 m_controlIndex; // 0x10 — the axis or button
	MechS32 m_mode;         // 0x14 — the button of an axis with a button
};

DECOMP_SIZE_ASSERT(CpcBinding, 0x18)

// A device slot of a .cpc file: the device a binding's slot number stood for when it was saved.
// SIZE 0x10
struct CpcDeviceSlot {
	MechS32 m_deviceId;    // 0x00
	MechChar m_name[0x0c]; // 0x04
};

DECOMP_SIZE_ASSERT(CpcDeviceSlot, 0x10)

// A button with a centered caption. Nothing calls its two functions.
// SIZE 0x9c
struct PewterPlaque0x9c {
	MechS32 m_left;        // 0x00
	MechS32 m_top;         // 0x04
	MechS32 m_right;       // 0x08
	MechS32 m_bottom;      // 0x0c
	MechS32 m_textLeft;    // 0x10
	MechS32 m_textTop;     // 0x14
	MechChar m_text[0x80]; // 0x18
	undefined4 m_unk0x98;  // 0x98
};

DECOMP_SIZE_ASSERT(PewterPlaque0x9c, 0x9c)

enum CpcConfig {
	c_configCount = 4,
	c_bindingCount = 0x25,
	c_axisBindingCount = 7,
	c_buttonRows = 18,
	c_deviceSlotCount = 16,
	c_maxActiveDevices = 4
};

enum CpcBindingFlags {
	c_flagModifierMask = 0x07
};

// FUN_100431b4's messages.
enum CpcMessage {
	c_messageMissingJoystick = 100,
	c_messageRemapJoystick = 101,
	c_messageNoJoysticks = 102
};

// Not an enumerator: the unsigned constant makes `^=` on the signed flags a load, xor and store.
#define CPC_FLAG_INVERTED 0x80000000

extern BrassLantern0x414* g_unk0x1007120c;
extern BrassLantern0x414* g_unk0x10071210;
extern MouseState* g_pMouseState;
extern HollowReed0x110* g_unk0x100711f8;
extern HMENU g_windowMenu;
extern MechS32 g_menuDialogOpen;
extern PaletteColor g_unk0x10071378[0x100];
extern VideoDriver* g_pVideoDriver;
extern "C" HWND g_pWnd;

extern "C" void DebugPrint(const MechChar* p_format, ...);
MechS32 FUN_1000fe0d();
void FUN_100109a0(void (*p_callback)(MechS32));
void FUN_100109b8(void (*p_callback)(MechS32));
void FUN_1001661b();
void CpcScreenTick(MechS32 p_active);
SlateTab0x2c* FUN_1000b5ed(SlateTab0x2c* p_tabs, MechS32 p_x, MechS32 p_y);
extern "C" void InputFreeDevices(void);
void FUN_100078cd(SlateTab0x2c* p_tabs);
void FUN_100079f8(SlateTab0x2c* p_tabs);
void FUN_10007ac8(SlateTab0x2c* p_tabs);
MechS32 ShowDialog(const char* p_text, MechS32 p_unk0x04);

EmberGlyph0x3e* FUN_1003e9d0(SlateTab0x2c* p_tab);
MechS32 FUN_1003ea0f(
	BrassLantern0x414* p_font,
	MechS32 p_left,
	MechS32 p_top,
	MechChar* p_text,
	undefined* p_colors,
	MechS32 p_maxLength,
	MechS32 p_maxWidth
);
void FUN_1003ee3e(SlateTab0x2c* p_tab);
EmberGlyph0x3e* FUN_1003ef32(SlateTab0x2c* p_tab);
EmberGlyph0x3e* FUN_1003f136(SlateTab0x2c* p_tab);
EmberGlyph0x3e* FUN_1003f283(SlateTab0x2c* p_tab);
EmberGlyph0x3e* FUN_1003f30f(SlateTab0x2c* p_tab);
void FUN_1003f34b(SlateTab0x2c* p_tab);
EmberGlyph0x3e* FUN_1003f39e(SlateTab0x2c* p_tab);
void FUN_1003f42e(SlateTab0x2c* p_tab);
EmberGlyph0x3e* FUN_1003f469(SlateTab0x2c* p_tab);
void FUN_1003f4be(SlateTab0x2c* p_tab);
void FUN_1003f576(SlateTab0x2c* p_tab);
void FUN_1003f78e(SlateTab0x2c* p_tab);
void FUN_1003fbb3(SlateTab0x2c* p_tab);
EmberGlyph0x3e* FUN_1003fe7b(SlateTab0x2c* p_tab);
EmberGlyph0x3e* FUN_1003ff95(SlateTab0x2c* p_tab);
void FUN_100400b7(SlateTab0x2c* p_tab);
EmberGlyph0x3e* FUN_100401b8(SlateTab0x2c* p_tab);
void FUN_10040272(SlateTab0x2c* p_tab);
void FUN_1004034e(SlateTab0x2c* p_tab);
EmberGlyph0x3e* FUN_1004059e(SlateTab0x2c* p_tab);
EmberGlyph0x3e* FUN_1004067c(SlateTab0x2c* p_tab);
EmberGlyph0x3e* FUN_1004079f(SlateTab0x2c* p_tab);
void InputToggleDeviceActive(SlateTab0x2c* p_tab);
void FUN_10040a0d(SlateTab0x2c* p_tab);
EmberGlyph0x3e* FUN_10040a5d(SlateTab0x2c* p_tab);
EmberGlyph0x3e* FUN_10040a94(SlateTab0x2c* p_tab);
void FUN_10040af7(SlateTab0x2c* p_tab);
EmberGlyph0x3e* FUN_10040b32(SlateTab0x2c* p_tab);
void FUN_10040b8e(SlateTab0x2c* p_tab);
void FUN_10040c21(SlateTab0x2c* p_tab);
void FUN_10040c72(SlateTab0x2c* p_tab);
void FUN_10040cc7(SlateTab0x2c* p_tab);
MechS32 CpcCheckControlCount();
void CpcRemapDeviceSlots(CpcBinding* p_bindings);
void FUN_1004207e(MechS32 p_index, CpcBinding* p_binding);
void CpcLoadConfigSlot(SlateTab0x2c* p_tab);
void CpcSaveConfigSlot(SlateTab0x2c* p_tab);
void CpcLoadActiveDevices(SlateTab0x2c* p_tab);
void CpcAcceptAndCommit(SlateTab0x2c* p_tab);
void FUN_100431b4(MechS32 p_message, undefined4 p_unk0x04, MechChar* p_name);

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

// The sim's name of each control, for input.map.
// GLOBAL: MW2SHELL 0x1006afc0
MechChar* g_unk0x1006afc0[c_bindingCount] = {
	"throttle",
	"legs_pan_delta",
	"torso_pan",
	"torso_tilt",
	"pilot_pan",
	"pilot_tilt",
	"zoom_factor",
	"torso_tilt_reset",
	"glance_left",
	"glance_right",
	"glance_up",
	"glance_down",
	"jumpjet_enabled",
	"jumpjet_fire_forward",
	"jumpjet_fire_backward",
	"jumpjet_fire_left",
	"jumpjet_fire_right",
	"weapon_fire",
	"weapon_cycle",
	"weapon_cycle_group",
	"weapon_fire_group_1",
	"weapon_fire_group_2",
	"weapon_fire_group_3",
	"toggle_group_fire",
	"advance_target",
	"previous_target",
	"target_reticle",
	"target_friendly",
	"nearest_enemy",
	"inspect_target",
	"advance_nav",
};

// The configuration shown.
// GLOBAL: MW2SHELL 0x1006b054
MechS32 g_unk0x1006b054 = 0;

// The bindings of the four configurations, one after the other.
// GLOBAL: MW2SHELL 0x10090ad0
CpcBinding g_cpcBindings[c_configCount * c_bindingCount];

// GLOBAL: MW2SHELL 0x1006b058
CpcBinding* g_pCpcBindings = g_cpcBindings;

// The first button shown in the device's button list.
// GLOBAL: MW2SHELL 0x1006b05c
MechS32 g_unk0x1006b05c = 0;

// GLOBAL: MW2SHELL 0x1006b060
MechS32 g_curCpcConfigSlot = 1;

// GLOBAL: MW2SHELL 0x1006b064
MechS32 g_cpcConfigured = 0;

// GLOBAL: MW2SHELL 0x1006b068
MechS32 g_inputConfigChanged = 0;

// GLOBAL: MW2SHELL 0x1006b06c
MechS32 g_unk0x1006b06c = 0;

// GLOBAL: MW2SHELL 0x1006b070
MechS32 g_activeInputDeviceCount = 0;

// The device slot of the bindings CpcCheckControlCount adds for the legs' pan.
// GLOBAL: MW2SHELL 0x1006b074
MechS32 g_unk0x1006b074 = -1;

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

// GLOBAL: MW2SHELL 0x10090690
MechChar g_cpcConfigName[0x40];

// GLOBAL: MW2SHELL 0x100906d0
MechChar g_unk0x100906d0[0x400];

// GLOBAL: MW2SHELL 0x100918b0
SilverReel0x18* g_unk0x100918b0;

// GLOBAL: MW2SHELL 0x100918b8
MechChar g_unk0x100918b8[0x40];

// The bindings of one device's .cpc file.
// GLOBAL: MW2SHELL 0x100918f8
CpcBinding g_cpcDeviceFileBindings[c_configCount * c_bindingCount];

// GLOBAL: MW2SHELL 0x100927d8
MechS32 g_inputDeviceActive[c_deviceSlotCount];

// GLOBAL: MW2SHELL 0x10092818
MechChar g_unk0x10092818[0x100];

// GLOBAL: MW2SHELL 0x10092918
CpcDeviceSlot g_cpcDeviceSlots[c_deviceSlotCount];

// Color map for disabled fields.
// GLOBAL: MW2SHELL 0x10092a18
undefined g_unk0x10092a18[0x100];

// Color map for the selected field.
// GLOBAL: MW2SHELL 0x10092b18
undefined g_unk0x10092b18[0x100];

// A field's m_top: a packed row and offset below the previous field (see SlateTab0x2c).
#define CPC_ROW(row, offset) ((MechS32) (0x80000000 | ((row) << 4) | (offset)))
#define CPC_TAB(left, top, width, draw, click, data, next)                                                             \
	{left, top, width, -1, 0, NULL, NULL, draw, click, (void*) (data), (SlateTab0x2c*) (next)}
#define CPC_TEXT(left, top, width, draw, click, data)                                                                  \
	{left, top, width, -1, 0, g_unk0x10092a18, NULL, draw, click, (void*) (data), NULL}
#define CPC_END {-1, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL}

// One control bound to an axis: its name, direction, modifier, axis and button. kind picks the
// direction names.
#define CPC_AXIS_BINDING(top, index, kind)                                                                             \
	CPC_TEXT(175, top, -1, FUN_1003f283, NULL, index),                                                                 \
		CPC_TAB(225, CPC_ROW(0, 0), -1, FUN_1003f39e, FUN_1003f42e, index, kind),                                      \
		CPC_TAB(260, CPC_ROW(0, 0), 20, FUN_1003f469, FUN_1003f4be, index, 0),                                         \
		CPC_TAB(285, CPC_ROW(0, 0), 100, FUN_1003ef32, FUN_1003f78e, index, 0),                                        \
		CPC_TAB(345, CPC_ROW(0, 0), 50, FUN_1003f136, FUN_1003fbb3, index, 0)

// One control bound to a button: its name, modifier and button.
#define CPC_BUTTON_BINDING(index)                                                                                      \
	CPC_TEXT(175, CPC_ROW(1, 1), -1, FUN_1003f283, NULL, index),                                                       \
		CPC_TAB(260, CPC_ROW(0, 0), 20, FUN_1003f469, FUN_1003f4be, index, 0),                                         \
		CPC_TAB(285, CPC_ROW(0, 0), 100, FUN_1003ef32, FUN_1003f78e, index, 0)

// The fields of one of the device's axes or buttons, or one input device.
#define CPC_LIST(top, draw, click, index) CPC_TAB(480, top, 100, draw, click, index, 0)
#define CPC_DEVICE(left, top, draw, click, index) CPC_TAB(left, top, 100, draw, click, index, 0)

// The bindings screen.
// GLOBAL: MW2SHELL 0x1006b0d8
SlateTab0x2c g_unk0x1006b0d8[169] = {
	CPC_TEXT(172, 0x78, -1, FUN_10040a5d, NULL, 1),
	CPC_TEXT(477, 0x78, -1, FUN_10040a5d, NULL, 1),
	CPC_TEXT(175, 0x6e, -1, FUN_1003e9d0, NULL, "GAME CONTROLS"),
	CPC_TAB(325, CPC_ROW(0, 0), -1, FUN_1003f30f, FUN_1003f34b, 0, 0),
	CPC_AXIS_BINDING(CPC_ROW(2, 2), 0, 0),
	CPC_AXIS_BINDING(CPC_ROW(1, 1), 1, 1),
	CPC_AXIS_BINDING(CPC_ROW(1, 1), 2, 1),
	CPC_AXIS_BINDING(CPC_ROW(1, 1), 3, 2),
	CPC_AXIS_BINDING(CPC_ROW(1, 1), 4, 1),
	CPC_AXIS_BINDING(CPC_ROW(1, 1), 5, 2),
	CPC_AXIS_BINDING(CPC_ROW(1, 1), 6, 3),
	CPC_BUTTON_BINDING(7),
	CPC_BUTTON_BINDING(8),
	CPC_BUTTON_BINDING(9),
	CPC_BUTTON_BINDING(10),
	CPC_BUTTON_BINDING(11),
	CPC_BUTTON_BINDING(12),
	CPC_BUTTON_BINDING(13),
	CPC_BUTTON_BINDING(14),
	CPC_BUTTON_BINDING(15),
	CPC_BUTTON_BINDING(16),
	CPC_BUTTON_BINDING(17),
	CPC_BUTTON_BINDING(18),
	CPC_BUTTON_BINDING(19),
	CPC_BUTTON_BINDING(20),
	CPC_BUTTON_BINDING(21),
	CPC_BUTTON_BINDING(22),
	CPC_BUTTON_BINDING(23),
	CPC_BUTTON_BINDING(24),
	CPC_BUTTON_BINDING(25),
	CPC_BUTTON_BINDING(26),
	CPC_BUTTON_BINDING(27),
	CPC_BUTTON_BINDING(28),
	CPC_BUTTON_BINDING(29),
	CPC_BUTTON_BINDING(30),
	CPC_TEXT(480, 0x6e, -1, FUN_1004059e, NULL, -1),
	CPC_TEXT(480, CPC_ROW(2, 2), -1, FUN_1003e9d0, NULL, "Directional"),
	CPC_LIST(CPC_ROW(2, 2), FUN_1003fe7b, FUN_100400b7, 0),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003fe7b, FUN_100400b7, 1),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003fe7b, FUN_100400b7, 2),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003fe7b, FUN_100400b7, 3),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003fe7b, FUN_100400b7, 4),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003fe7b, FUN_100400b7, 5),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003fe7b, FUN_100400b7, 6),
	CPC_TEXT(480, CPC_ROW(2, 2), -1, FUN_1003e9d0, NULL, "Buttons"),
	CPC_TAB(600, CPC_ROW(2, 2), 10, FUN_100401b8, FUN_10040272, -1, 0),
	CPC_LIST(CPC_ROW(0, 0), FUN_1003ff95, FUN_1004034e, 0),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003ff95, FUN_1004034e, 1),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003ff95, FUN_1004034e, 2),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003ff95, FUN_1004034e, 3),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003ff95, FUN_1004034e, 4),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003ff95, FUN_1004034e, 5),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003ff95, FUN_1004034e, 6),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003ff95, FUN_1004034e, 7),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003ff95, FUN_1004034e, 8),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003ff95, FUN_1004034e, 9),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003ff95, FUN_1004034e, 10),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003ff95, FUN_1004034e, 11),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003ff95, FUN_1004034e, 12),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003ff95, FUN_1004034e, 13),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003ff95, FUN_1004034e, 14),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003ff95, FUN_1004034e, 15),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003ff95, FUN_1004034e, 16),
	CPC_LIST(CPC_ROW(1, 1), FUN_1003ff95, FUN_1004034e, 17),
	CPC_TAB(600, CPC_ROW(0, 0), 10, FUN_100401b8, FUN_10040272, 1, 0),
	CPC_TEXT(0, 0x6e, -1, FUN_1003e9d0, NULL, "INPUT DEVICES"),
	CPC_DEVICE(0, CPC_ROW(2, 2), FUN_1004067c, FUN_10040a0d, 0),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004067c, FUN_10040a0d, 1),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004067c, FUN_10040a0d, 2),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004067c, FUN_10040a0d, 3),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004067c, FUN_10040a0d, 4),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004067c, FUN_10040a0d, 5),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004067c, FUN_10040a0d, 6),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004067c, FUN_10040a0d, 7),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004067c, FUN_10040a0d, 8),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004067c, FUN_10040a0d, 9),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004067c, FUN_10040a0d, 10),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004067c, FUN_10040a0d, 11),
	CPC_TEXT(0, CPC_ROW(2, 2), -1, FUN_1003e9d0, NULL, "Current Config:"),
	CPC_TEXT(90, CPC_ROW(0, 0), -1, FUN_10040a94, NULL, &g_curCpcConfigSlot),
	CPC_TAB(20, CPC_ROW(1, 1), 150, FUN_1003e9d0, FUN_1003ee3e, g_cpcConfigName, 0),
	CPC_TAB(0, CPC_ROW(2, 2), -1, FUN_1003e9d0, CpcLoadActiveDevices, "RESET DEFAULTS", 0),
	CPC_TAB(0, CPC_ROW(2, 2), -1, FUN_1003e9d0, CpcLoadConfigSlot, "Load Custom 1", 1),
	CPC_TAB(0, CPC_ROW(1, 1), -1, FUN_1003e9d0, CpcLoadConfigSlot, "Load Custom 2", 2),
	CPC_TAB(0, CPC_ROW(1, 1), -1, FUN_1003e9d0, CpcLoadConfigSlot, "Load Custom 3", 3),
	CPC_TAB(0, CPC_ROW(1, 1), -1, FUN_1003e9d0, CpcLoadConfigSlot, "Load Custom 4", 4),
	CPC_TAB(0, CPC_ROW(2, 2), -1, FUN_1003e9d0, CpcSaveConfigSlot, "Save Custom 1", 1),
	CPC_TAB(0, CPC_ROW(1, 1), -1, FUN_1003e9d0, CpcSaveConfigSlot, "Save Custom 2", 2),
	CPC_TAB(0, CPC_ROW(1, 1), -1, FUN_1003e9d0, CpcSaveConfigSlot, "Save Custom 3", 3),
	CPC_TAB(0, CPC_ROW(1, 1), -1, FUN_1003e9d0, CpcSaveConfigSlot, "Save Custom 4", 4),
	CPC_TAB(0, CPC_ROW(2, 2), -1, FUN_1003e9d0, CpcAcceptAndCommit, "ACCEPT CONFIG AND EXIT", 0),
	CPC_TAB(0, CPC_ROW(2, 2), -1, FUN_1003e9d0, FUN_1003f576, "ABORT", 0),
	CPC_END,
};

// The input devices screen.
// GLOBAL: MW2SHELL 0x1006cde8
SlateTab0x2c g_unk0x1006cde8[24] = {
	CPC_TEXT(172, 0x78, -1, FUN_10040a5d, NULL, 1),
	CPC_TEXT(477, 0x78, -1, FUN_10040a5d, NULL, 1),
	CPC_TEXT(0, 0x6e, -1, FUN_1003e9d0, NULL, "Current Config:"),
	CPC_TEXT(80, CPC_ROW(0, 0), -1, FUN_1003e9d0, NULL, g_cpcConfigName),
	CPC_TEXT(0, CPC_ROW(2, 2), -1, FUN_1003e9d0, NULL, "SELECT INPUT DEVICES"),
	CPC_DEVICE(0, CPC_ROW(2, 2), FUN_1004079f, InputToggleDeviceActive, 0),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004079f, InputToggleDeviceActive, 1),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004079f, InputToggleDeviceActive, 2),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004079f, InputToggleDeviceActive, 3),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004079f, InputToggleDeviceActive, 4),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004079f, InputToggleDeviceActive, 5),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004079f, InputToggleDeviceActive, 6),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004079f, InputToggleDeviceActive, 7),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004079f, InputToggleDeviceActive, 8),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004079f, InputToggleDeviceActive, 9),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004079f, InputToggleDeviceActive, 10),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_1004079f, InputToggleDeviceActive, 11),
	CPC_DEVICE(0, CPC_ROW(1, 1), FUN_10040b32, FUN_10040b8e, "No joysticks configured"),
	CPC_DEVICE(0, CPC_ROW(2, 2), FUN_1003e9d0, FUN_10040cc7, "JOYSTICK CONTROL PANEL"),
	CPC_DEVICE(0, CPC_ROW(2, 2), FUN_1003e9d0, FUN_10040c21, "REFRESH"),
	CPC_DEVICE(0, 0x190, FUN_1003e9d0, FUN_10040af7, "ACCEPT"),
	CPC_DEVICE(0, 0x1a4, FUN_1003e9d0, FUN_10040c72, "CUSTOM CONFIGURATION"),
	CPC_DEVICE(0, 0x1b8, FUN_1003e9d0, FUN_1003f576, "ABORT"),
	CPC_END,
};

#undef CPC_ROW
#undef CPC_TAB
#undef CPC_TEXT
#undef CPC_END
#undef CPC_AXIS_BINDING
#undef CPC_BUTTON_BINDING
#undef CPC_LIST
#undef CPC_DEVICE

// The axes and buttons written to the last input.map.
// GLOBAL: MW2SHELL 0x1006d208
MechS32 g_cpcAnalogCount = 0;

// GLOBAL: MW2SHELL 0x1006d20c
MechS32 g_cpcDiscreteCount = 0;

// FUNCTION: MW2SHELL 0x1003e9d0
EmberGlyph0x3e* FUN_1003e9d0(SlateTab0x2c* p_tab)
{
	MechChar* text;

	text = (MechChar*) p_tab->m_unk0x24;
	return g_unk0x10071210->FUN_1000544e(p_tab->m_left, p_tab->m_top, text, p_tab->m_unk0x14);
}

// The text being edited by FUN_1003ea0f, with its cursor.
// GLOBAL: MW2SHELL 0x100926d8
MechChar g_unk0x100926d8[0x100];

// Edits p_text with a cursor ('_') drawn after it, like FUN_10044451 but keeping the screen's
// video running. Return or a click store the text and return 1; Escape stores it and returns
// 0. When the window closes it gives up without a return value.
// Not 100%: the stack slots of the locals are permuted.
// FUNCTION: MW2SHELL 0x1003ea0f
MechS32 FUN_1003ea0f(
	BrassLantern0x414* p_font,
	MechS32 p_left,
	MechS32 p_top,
	MechChar* p_text,
	undefined* p_colors,
	MechS32 p_maxLength,
	MechS32 p_maxWidth
)
{
	MechS32 key;
	MechS32 width;
	MechS32 length;
	EmberGlyph0x3e* glyph;

	glyph = NULL;
	length = strlen(p_text);
	strcpy(g_unk0x100926d8, p_text);
	strcat(g_unk0x100926d8, "_");
	width = p_font->FUN_100053be(g_unk0x100926d8);
	glyph = p_font->FUN_10005522(p_left, p_top, g_unk0x100926d8, p_colors);

	for (;;) {
		if (g_unk0x100918b0) {
			g_unk0x100918b0->FUN_1001630b();
		}

		if (!FUN_1000fe0d()) {
			break;
		}

		g_pMouseState->ReadMouseState();
		g_pVideoDriver->DrawShell();
		if (g_pMouseState->GetLeftPressed() == 1) {
			g_unk0x100926d8[length] = '\0';
			if (glyph) {
				delete glyph;
			}

			strcpy(p_text, g_unk0x100926d8);
			return 1;
		}

		if (g_unk0x100711f8->FUN_10044189()) {
			switch (g_unk0x100711f8->m_key) {
			case 8:
				if (length == 0) {
					break;
				}

				length--;
				g_unk0x100926d8[length] = '_';
				g_unk0x100926d8[length + 1] = '\0';
				if (glyph) {
					delete glyph;
				}

				glyph = p_font->FUN_10005522(p_left, p_top, g_unk0x100926d8, p_colors);
				break;
			case 0x0d:
				g_unk0x100926d8[length] = '\0';
				if (glyph) {
					delete glyph;
				}

				strcpy(p_text, g_unk0x100926d8);
				return 1;
			case 0x1b:
				g_unk0x100926d8[length] = '\0';
				if (glyph) {
					delete glyph;
				}

				strcpy(p_text, g_unk0x100926d8);
				return 0;
			default:
				key = g_unk0x100711f8->m_key;
				if (key < 0x20 || key > 0x7f || key == 0x7e) {
					break;
				}

				if (length == p_maxLength) {
					break;
				}

				if (!p_font->FUN_10005424(key)) {
					break;
				}

				g_unk0x100926d8[length] = key;
				length++;
				g_unk0x100926d8[length] = '_';
				g_unk0x100926d8[length + 1] = '\0';

				if (p_font->FUN_100053be(g_unk0x100926d8) < p_maxWidth) {
					if (glyph) {
						delete glyph;
					}

					glyph = p_font->FUN_10005522(p_left, p_top, g_unk0x100926d8, p_colors);
				}
				else {
					length--;
					g_unk0x100926d8[length] = '_';
					g_unk0x100926d8[length + 1] = '\0';
				}
				break;
			}
		}
	}
}

// Edit a field's text in place, then show it.
// FUNCTION: MW2SHELL 0x1003ee3e
void FUN_1003ee3e(SlateTab0x2c* p_tab)
{
	if (p_tab->m_glyph != NULL) {
		delete p_tab->m_glyph;
	}

	FUN_1003ea0f(
		g_unk0x1007120c,
		p_tab->m_left,
		p_tab->m_top,
		(MechChar*) p_tab->m_unk0x24,
		p_tab->m_unk0x14,
		0x3e,
		p_tab->m_width
	);
	p_tab->m_glyph =
		g_unk0x10071210->FUN_1000544e(p_tab->m_left, p_tab->m_top, (MechChar*) p_tab->m_unk0x24, p_tab->m_unk0x14);
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

	g_pCpcBindings = &g_cpcBindings[g_unk0x1006b054 * c_bindingCount];
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

// Click on a binding's axis or button: select it, or advance it to the current device's next one.
// A right click clears the binding.
// FUNCTION: MW2SHELL 0x1003f78e
void FUN_1003f78e(SlateTab0x2c* p_tab)
{
	InputDevice* device;

	device = InputGetDevice(g_curInputDeviceIdx);
	if (g_pMouseState->GetRightPressed() == 1) {
		g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_deviceSlot = -1;
		if (g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_channelKind == 2) {
			g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_channelKind = 0;
		}

		g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_flags = 0;
		g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_modeFlags = 0;
		g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_controlIndex = -1;
		g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_mode = -1;
		g_unk0x1006af18 = (MechS32) p_tab->m_unk0x24;
		g_unk0x1006af1c = 0;
		return;
	}

	if ((MechS32) p_tab->m_unk0x24 == g_unk0x1006af18 && g_unk0x1006af1c == 0) {
		switch (g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_channelKind) {
		case 0:
			if (device->m_info.m_axisCount == 0) {
				break;
			}

			if (g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_deviceSlot != g_curInputDeviceIdx) {
				g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_deviceSlot = g_curInputDeviceIdx;
				g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_controlIndex = 0;
			}
			else {
				FUN_1003f677(&g_pCpcBindings[(MechS32) p_tab->m_unk0x24], 0);
			}
			break;
		case 1:
			if (device->m_info.m_buttonCount == 0) {
				break;
			}

			if (g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_deviceSlot != g_curInputDeviceIdx) {
				g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_deviceSlot = g_curInputDeviceIdx;
				g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_controlIndex = -1;
			}

			FUN_1003f677(&g_pCpcBindings[(MechS32) p_tab->m_unk0x24], 0);
			break;
		case 2:
			if (g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_deviceSlot != g_curInputDeviceIdx) {
				if (device->m_info.m_buttonCount != 0) {
					g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_deviceSlot = g_curInputDeviceIdx;
					g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_controlIndex = -1;
					g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_mode = -1;
					FUN_1003f677(&g_pCpcBindings[(MechS32) p_tab->m_unk0x24], 0);
				}
				else if (device->m_info.m_axisCount != 0) {
					g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_channelKind = 0;
					g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_deviceSlot = g_curInputDeviceIdx;
					g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_controlIndex = -1;
					g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_mode = -1;
					FUN_1003f677(&g_pCpcBindings[(MechS32) p_tab->m_unk0x24], 0);
				}
			}
			else {
				FUN_1003f677(&g_pCpcBindings[(MechS32) p_tab->m_unk0x24], 0);
				if (g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_controlIndex < 0) {
					if (g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_mode < 0) {
						g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_channelKind = 0;
					}
					else {
						g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_deviceSlot = g_curInputDeviceIdx;
					}
				}
			}
			break;
		}
	}
	else {
		g_unk0x1006af18 = (MechS32) p_tab->m_unk0x24;
		g_unk0x1006af1c = 0;
	}

	FUN_1003f590();
}

// Click on the button of a binding to an axis with a button: select it, or advance it to the
// current device's next button. A right click clears the binding.
// FUNCTION: MW2SHELL 0x1003fbb3
void FUN_1003fbb3(SlateTab0x2c* p_tab)
{
	InputDevice* device;

	device = InputGetDevice(g_curInputDeviceIdx);
	if (g_pMouseState->GetRightPressed() == 1) {
		g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_deviceSlot = -1;
		if (g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_channelKind == 2) {
			g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_channelKind = 0;
		}

		g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_flags = 0;
		g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_modeFlags = 0;
		g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_controlIndex = -1;
		g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_mode = -1;
		g_unk0x1006af18 = (MechS32) p_tab->m_unk0x24;
		g_unk0x1006af1c = 0;
		return;
	}

	if ((MechS32) p_tab->m_unk0x24 == g_unk0x1006af18 && g_unk0x1006af1c == 1) {
		if (g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_deviceSlot != g_curInputDeviceIdx) {
			if (device->m_info.m_buttonCount != 0) {
				g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_deviceSlot = g_curInputDeviceIdx;
				g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_controlIndex = -1;
				g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_mode = 0;
			}
			else if (device->m_info.m_axisCount != 0) {
				g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_deviceSlot = g_curInputDeviceIdx;
				g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_channelKind = 0;
				g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_controlIndex = 0;
				g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_mode = -1;
				g_unk0x1006af1c = 0;
			}
		}
		else {
			FUN_1003f677(&g_pCpcBindings[(MechS32) p_tab->m_unk0x24], 1);
			if (g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_mode < 0) {
				if (g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_controlIndex < 0) {
					g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_channelKind = 0;
					g_unk0x1006af1c = 0;
				}
				else {
					g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_deviceSlot = g_curInputDeviceIdx;
				}
			}
		}
	}
	else if (g_pCpcBindings[(MechS32) p_tab->m_unk0x24].m_channelKind == 2) {
		g_unk0x1006af18 = (MechS32) p_tab->m_unk0x24;
		g_unk0x1006af1c = 1;
	}

	FUN_1003f590();
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
// The original loads the index before the name table in m_axisShortNames[...] and m_axisNames[...];
// it matched until the unit grew, so the order follows the unit's symbol count.
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

// Scroll the button list while a mouse button is held; the right button scrolls twice as fast.
// Stack-slot permutation: count and step.
// FUNCTION: MW2SHELL 0x10040272
void FUN_10040272(SlateTab0x2c* p_tab)
{
	MechS32 count;
	MechS32 step;

	count = InputGetDevice(g_curInputDeviceIdx)->m_info.m_buttonCount;
	step = (MechS32) p_tab->m_unk0x24;
	if (g_pMouseState->m_rightDown == 1) {
		step <<= 1;
	}

	do {
		if ((g_unk0x1006b05c += step) < 0) {
			g_unk0x1006b05c = 0;
		}
		if (g_unk0x1006b05c + c_buttonRows >= count) {
			g_unk0x1006b05c = count - c_buttonRows;
		}
		if (g_unk0x1006b05c < 0) {
			g_unk0x1006b05c = 0;
		}

		FUN_100079f8(g_unk0x1006b0d8);
		if (g_unk0x100918b0 != NULL) {
			g_unk0x100918b0->FUN_1001630b();
		}

		g_pMouseState->ReadMouseState();
	} while (g_pMouseState->m_leftDown == 1 || g_pMouseState->m_rightDown == 1);
}

// Bind the selected control, or the selected axis's button, to a button of the current device.
// FUNCTION: MW2SHELL 0x1004034e
void FUN_1004034e(SlateTab0x2c* p_tab)
{
	InputDevice* device;
	MechS32 index;

	device = InputGetDevice(g_curInputDeviceIdx);
	index = (MechS32) p_tab->m_unk0x24 + g_unk0x1006b05c;
	if (g_unk0x1006af18 < 0 || index >= device->m_info.m_buttonCount) {
		return;
	}
	if (device->m_info.m_buttonShortNames[index] == NULL) {
		return;
	}
	if (device->m_info.m_buttonNames[index] == NULL) {
		return;
	}
	if (*device->m_info.m_buttonShortNames[index] == '\0') {
		return;
	}
	if (*device->m_info.m_buttonNames[index] == '\0') {
		return;
	}
	if (*device->m_info.m_buttonShortNames[index] == '*') {
		return;
	}

	if (g_unk0x1006af1c == 1) {
		if (g_pCpcBindings[g_unk0x1006af18].m_deviceSlot != g_curInputDeviceIdx) {
			g_pCpcBindings[g_unk0x1006af18].m_deviceSlot = g_curInputDeviceIdx;
			g_pCpcBindings[g_unk0x1006af18].m_controlIndex = -1;
		}

		g_pCpcBindings[g_unk0x1006af18].m_mode = index;
		if (g_pCpcBindings[g_unk0x1006af18].m_controlIndex < 0) {
			g_unk0x1006af1c = 0;
		}
	}
	else {
		if (g_pCpcBindings[g_unk0x1006af18].m_channelKind == 0) {
			g_pCpcBindings[g_unk0x1006af18].m_channelKind = 2;
			g_pCpcBindings[g_unk0x1006af18].m_mode = -1;
		}
		if (g_pCpcBindings[g_unk0x1006af18].m_deviceSlot != g_curInputDeviceIdx) {
			g_pCpcBindings[g_unk0x1006af18].m_mode = -1;
		}

		g_pCpcBindings[g_unk0x1006af18].m_deviceSlot = g_curInputDeviceIdx;
		g_pCpcBindings[g_unk0x1006af18].m_controlIndex = index;
		if (g_pCpcBindings[g_unk0x1006af18].m_channelKind == 2 && g_pCpcBindings[g_unk0x1006af18].m_mode < 0) {
			g_unk0x1006af1c = 1;
		}
	}
}

// The current device's name: "Joystick N" for a joystick.
// Stack-slot permutation: name and number.
// FUNCTION: MW2SHELL 0x1004059e
EmberGlyph0x3e* FUN_1004059e(SlateTab0x2c* p_tab)
{
	undefined* colors; // Set and never read.
	size_t number;
	MechChar name[12];
	InputDevice* device;

	colors = p_tab->m_unk0x14;
	device = InputGetDevice(g_curInputDeviceIdx);
	if (device == NULL) {
		return NULL;
	}

	if (strncmp(device->m_info.m_shortName, "joystick", 8) == 0) {
		number = strcspn(device->m_info.m_shortName, "1234567890");
		sprintf(name, "Joystick %s", device->m_info.m_shortName + number);
	}
	else {
		strcpy(name, device->m_info.m_displayName);
	}

	return g_unk0x10071210->FUN_1000544e(p_tab->m_left, p_tab->m_top, name, p_tab->m_unk0x14);
}

// An input device of the bindings screen, its name shortened to fit.
// FUNCTION: MW2SHELL 0x1004067c
EmberGlyph0x3e* FUN_1004067c(SlateTab0x2c* p_tab)
{
	InputDevice* device;
	undefined* colors;
	MechS32 index;

	colors = p_tab->m_unk0x14;
	if ((MechS32) p_tab->m_unk0x24 >= 0 && !g_inputDeviceActive[(MechS32) p_tab->m_unk0x24]) {
		colors = g_unk0x10092a18;
	}
	if ((MechS32) p_tab->m_unk0x24 == g_curInputDeviceIdx) {
		colors = g_unk0x10092b18;
	}

	if ((MechS32) p_tab->m_unk0x24 < 0) {
		index = g_curInputDeviceIdx;
	}
	else {
		index = (MechS32) p_tab->m_unk0x24;
	}

	device = InputGetDevice(index);
	if (device == NULL) {
		return NULL;
	}

	while (g_unk0x10071210->FUN_100053be(device->m_info.m_displayName) > 175) {
		strcpy(device->m_info.m_displayName + strlen(device->m_info.m_displayName) - 4, "...");
	}

	return g_unk0x10071210->FUN_1000544e(p_tab->m_left, p_tab->m_top, device->m_info.m_displayName, colors);
}

// An input device of the devices screen: highlighted when active, disabled while four are.
// FUNCTION: MW2SHELL 0x1004079f
EmberGlyph0x3e* FUN_1004079f(SlateTab0x2c* p_tab)
{
	InputDevice* device;
	undefined* colors;
	MechS32 index;

	colors = p_tab->m_unk0x14;
	if ((MechS32) p_tab->m_unk0x24 >= 0 && g_inputDeviceActive[(MechS32) p_tab->m_unk0x24]) {
		colors = g_unk0x10092b18;
	}
	else if (g_activeInputDeviceCount >= c_maxActiveDevices) {
		colors = g_unk0x10092a18;
	}

	if ((MechS32) p_tab->m_unk0x24 < 0) {
		index = g_curInputDeviceIdx;
	}
	else {
		index = (MechS32) p_tab->m_unk0x24;
	}

	device = InputGetDevice(index);
	if (device == NULL) {
		return NULL;
	}

	while (g_unk0x10071210->FUN_100053be(device->m_info.m_displayName) > 175) {
		strcpy(device->m_info.m_displayName + strlen(device->m_info.m_displayName) - 4, "...");
	}

	return g_unk0x10071210->FUN_1000544e(p_tab->m_left, p_tab->m_top, device->m_info.m_displayName, colors);
}

// Activate or deactivate a device; the keyboard stays as it is, and at most four are active.
// FUNCTION: MW2SHELL 0x100408c2
void InputToggleDeviceActive(SlateTab0x2c* p_tab)
{
	MechS32 count;

	count = InputEnumDevices(FALSE);
	if (InputEnumDevices(TRUE) != count) {
		CpcLoadActiveDevices(NULL);
	}

	if (InputGetDevice((MechS32) p_tab->m_unk0x24) != NULL) {
		if (strcmp(g_cpcDeviceSlots[(MechS32) p_tab->m_unk0x24].m_name, "keyboard")) {
			if (g_inputDeviceActive[(MechS32) p_tab->m_unk0x24]) {
				g_activeInputDeviceCount--;
				g_inputConfigChanged = 1;
				g_inputDeviceActive[(MechS32) p_tab->m_unk0x24] = 0;
				if ((MechS32) p_tab->m_unk0x24 == g_curInputDeviceIdx) {
					while (!g_inputDeviceActive[--g_curInputDeviceIdx]) {
					}
				}
			}
			else if (g_activeInputDeviceCount < c_maxActiveDevices) {
				g_activeInputDeviceCount++;
				g_inputConfigChanged = 1;
				g_inputDeviceActive[(MechS32) p_tab->m_unk0x24] = 1;
				g_curInputDeviceIdx = (MechS32) p_tab->m_unk0x24;
			}
		}
	}
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
		CpcLoadActiveDevices(NULL);
	}

	CpcAcceptAndCommit(NULL);
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

// Enumerate the devices again, and explain when there is no joystick.
// FUNCTION: MW2SHELL 0x10040b8e
void FUN_10040b8e(SlateTab0x2c*)
{
	MechS32 count;

	count = InputEnumDevices(FALSE);
	if (InputEnumDevices(TRUE) != count) {
		CpcLoadActiveDevices(NULL);
		count = InputEnumDevices(FALSE);
		if (count <= g_curInputDeviceIdx) {
			g_inputDeviceActive[g_curInputDeviceIdx] = 0;
			g_curInputDeviceIdx = count - 1;
		}
	}

	if (InputEnumDevices(FALSE) <= 2) {
		FUN_100431b4(c_messageNoJoysticks, 0, NULL);
	}
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

	CpcLoadActiveDevices(NULL);
}

// Leave the devices screen for the bindings screen.
// FUNCTION: MW2SHELL 0x10040c72
void FUN_10040c72(SlateTab0x2c*)
{
	if (g_inputConfigChanged) {
		CpcLoadActiveDevices(NULL);
	}

	FUN_10007ac8(g_unk0x1006cde8);
	FUN_100078cd(g_unk0x1006b0d8);
	g_unk0x1006b06c = 1;
	g_cpcConfigured = 0;
}

// Open the Windows joystick control panel.
// Stack-slot permutation: created and processInfo.
// FUNCTION: MW2SHELL 0x10040cc7
void FUN_10040cc7(SlateTab0x2c*)
{
	STARTUPINFO startupInfo;
	BOOL created;
	PROCESS_INFORMATION processInfo;
	MechChar* commandLine;

	memset(&startupInfo, 0, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	commandLine = (MechChar*) HeapAlloc(g_hPrimaryHeap, HEAP_NO_SERIALIZE, MAX_PATH);
	if (commandLine != NULL) {
		GetWindowsDirectory(commandLine, MAX_PATH);
		strcat(commandLine, "\\control.exe joy.cpl");
		created = CreateProcess(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
		if (!created) {
			DebugPrint("CreateProcess failed: %d\n", GetLastError());
		}

		HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, commandLine);
	}
}

// Write a binding's modifier keys to input.map: pressed for bits 0-2, released for bits 4-6.
// FUNCTION: MW2SHELL 0x10040d89
void FUN_10040d89(FILE* p_file, CpcBinding* p_binding)
{
	if (p_binding->m_flags & 1) {
		fprintf(p_file, "\t+ keyboard\tControl\n");
	}
	else if (p_binding->m_flags & 0x10) {
		fprintf(p_file, "\t- keyboard\tControl\n");
	}

	if (p_binding->m_flags & 2) {
		fprintf(p_file, "\t+ keyboard\tAlt\n");
	}
	else if (p_binding->m_flags & 0x20) {
		fprintf(p_file, "\t- keyboard\tAlt\n");
	}

	if (p_binding->m_flags & 4) {
		fprintf(p_file, "\t+ keyboard\tShift\n");
	}
	else if (p_binding->m_flags & 0x40) {
		fprintf(p_file, "\t- keyboard\tShift\n");
	}
}

// Write an axis binding to input.map.
// Stack-slot permutation: device and axis.
// FUNCTION: MW2SHELL 0x10040e5c
void FUN_10040e5c(FILE* p_file, MechChar* p_name, CpcBinding* p_binding)
{
	InputDevice* device;
	MechS32 axis;

	device = InputGetDevice(p_binding->m_deviceSlot);
	axis = p_binding->m_controlIndex;
	if (axis < 0) {
		return;
	}

	g_cpcAnalogCount++;
	fprintf(p_file, "%s {\n", p_name);
	fprintf(
		p_file,
		"\t%c %s\t%s\n",
		p_binding->m_flags & CPC_FLAG_INVERTED ? '-' : '+',
		device->m_info.m_shortName,
		device->m_info.m_axisShortNames[axis]
	);
	FUN_10040d89(p_file, p_binding);
	fprintf(p_file, "}\n");
}

// Write a button binding to input.map; a jump jet button also enables the jump jets.
// FUNCTION: MW2SHELL 0x10040f14
void FUN_10040f14(FILE* p_file, MechChar* p_name, CpcBinding* p_binding)
{
	InputDevice* device;
	MechS32 button;

	device = InputGetDevice(p_binding->m_deviceSlot);
	button = p_binding->m_controlIndex;
	if (button < 0) {
		return;
	}

	g_cpcDiscreteCount++;
	fprintf(p_file, "%s {\n", p_name);
	fprintf(p_file, "\t+ %s\t%s\n", device->m_info.m_shortName, device->m_info.m_buttonShortNames[button]);
	FUN_10040d89(p_file, p_binding);
	fprintf(p_file, "}\n");
	if (strncmp(p_name, "jumpjet_fire_", 13) == 0) {
		FUN_10040f14(p_file, "jumpjet_enabled", p_binding);
	}
}

// Write a binding to an axis with a button to input.map, as two buttons: the control's _minus
// and _plus.
// Stack-slot permutation: saved, second, first and delta.
// FUNCTION: MW2SHELL 0x10040fe2
void FUN_10040fe2(FILE* p_file, MechChar* p_name, CpcBinding* p_binding)
{
	CpcBinding saved;
	MechChar second[80];
	MechChar first[80];
	MechChar* delta;

	saved = *p_binding;
	strcpy(first, p_name);
	delta = strstr(first, "_delta");
	if (delta != NULL) {
		*delta = '\0';
	}

	strcpy(second, first);
	if (p_binding->m_flags & CPC_FLAG_INVERTED) {
		strcat(second, "_minus");
		strcat(first, "_plus");
	}
	else {
		strcat(first, "_minus");
		strcat(second, "_plus");
	}

	FUN_10040f14(p_file, first, p_binding);
	p_binding->m_controlIndex = p_binding->m_mode;
	p_binding->m_flags = p_binding->m_modeFlags;
	FUN_10040f14(p_file, second, p_binding);
	*p_binding = saved;
}

// Write a binding to input.map, with the sim controls that follow it.
// FUNCTION: MW2SHELL 0x10041168
void FUN_10041168(FILE* p_file, MechChar* p_name, CpcBinding* p_binding)
{
	if (p_binding->m_controlIndex < 0) {
		return;
	}

	if (p_binding->m_channelKind == 0) {
		FUN_10040e5c(p_file, p_name, p_binding);
	}
	else if (p_binding->m_channelKind == 1) {
		FUN_10040f14(p_file, p_name, p_binding);
	}
	else if (p_binding->m_channelKind == 2) {
		FUN_10040fe2(p_file, p_name, p_binding);
	}

	if (!strcmp(p_name, "pilot_tilt")) {
		FUN_10041168(p_file, "track_height_delta", p_binding);
	}
	else if (!strcmp(p_name, "pilot_pan")) {
		FUN_10041168(p_file, "eyepoint_pan_delta", p_binding);
	}
	else if (!strcmp(p_name, "zoom_factor")) {
		p_binding->m_flags ^= CPC_FLAG_INVERTED;
		FUN_10041168(p_file, "track_distance_delta", p_binding);
		p_binding->m_flags ^= CPC_FLAG_INVERTED;
	}
	else if (!strcmp(p_name, "torso_tilt_reset")) {
		FUN_10041168(p_file, "pilot_tilt_reset", p_binding);
		FUN_10041168(p_file, "torso_pan_reset", p_binding);
		FUN_10041168(p_file, "pilot_pan_reset", p_binding);
	}
	else if (!strcmp(p_name, "glance_up")) {
		FUN_10041168(p_file, "track_height_minus", p_binding);
	}
	else if (!strcmp(p_name, "glance_down")) {
		FUN_10041168(p_file, "track_height_plus", p_binding);
	}
	else if (!strcmp(p_name, "glance_right")) {
		FUN_10041168(p_file, "eyepoint_pan_plus", p_binding);
	}
	else if (!strcmp(p_name, "glance_left")) {
		FUN_10041168(p_file, "eyepoint_pan_minus", p_binding);
	}
}

// Works out the modifier flags each binding must exclude (the modifiers of the other bindings of
// the same axis or button), writes the bindings to temp.map, and returns whether the sim can
// take their analog and discrete counts.
// Not 100%: the stack slots of the locals are permuted.
// FUNCTION: MW2SHELL 0x1004154b
MechS32 CpcCheckControlCount()
{
	MechS32 mask;
	CpcBinding binding;
	MechU32 i;
	MechU32 j;
	FILE* file;

	for (i = 0; i < c_configCount * c_bindingCount; i++) {
		switch (g_cpcBindings[i].m_channelKind) {
		case 0:
		case 1:
			g_cpcBindings[i].m_modeFlags = 0;
			break;
		case 2:
			if (g_cpcBindings[i].m_mode == g_cpcBindings[i].m_controlIndex) {
				g_cpcBindings[i].m_modeFlags = (g_cpcBindings[i].m_flags << 4) & 0x70;
			}
			else {
				g_cpcBindings[i].m_modeFlags = g_cpcBindings[i].m_flags;
			}
		}
	}

	for (i = 0; i < c_configCount * c_bindingCount; i++) {
		if (g_cpcBindings[i].m_deviceSlot < 0) {
			continue;
		}

		mask = (g_cpcBindings[i].m_flags << 4) & 0x70;
		if (mask) {
			switch (g_cpcBindings[i].m_channelKind) {
			case 0:
				for (j = 0; j < c_configCount * c_bindingCount; j++) {
					if (j == i) {
						continue;
					}
					if (g_cpcBindings[j].m_deviceSlot < 0) {
						continue;
					}
					if (g_cpcBindings[j].m_channelKind != 0) {
						continue;
					}
					if (g_cpcBindings[j].m_controlIndex != g_cpcBindings[i].m_controlIndex) {
						continue;
					}

					g_cpcBindings[j].m_flags |= mask;
				}
				break;
			case 1:
			case 2:
				for (j = 0; j < c_configCount * c_bindingCount; j++) {
					if (j == i) {
						continue;
					}
					if (g_cpcBindings[j].m_deviceSlot < 0) {
						continue;
					}
					if (g_cpcBindings[j].m_channelKind == 0) {
						continue;
					}

					if (g_cpcBindings[j].m_controlIndex == g_cpcBindings[i].m_controlIndex) {
						g_cpcBindings[j].m_flags |= mask;
					}
					if (g_cpcBindings[j].m_channelKind == 2 &&
						g_cpcBindings[j].m_mode == g_cpcBindings[i].m_controlIndex) {
						g_cpcBindings[j].m_modeFlags |= mask;
					}
					if (g_cpcBindings[j].m_channelKind == 2 &&
						g_cpcBindings[i].m_mode == g_cpcBindings[j].m_controlIndex) {
						g_cpcBindings[j].m_flags |= (g_cpcBindings[i].m_modeFlags << 4) & 0x70;
					}
					if (g_cpcBindings[j].m_channelKind == 2 && g_cpcBindings[j].m_mode == g_cpcBindings[i].m_mode) {
						g_cpcBindings[j].m_modeFlags |= (g_cpcBindings[i].m_modeFlags << 4) & 0x70;
					}
				}
				break;
			}
		}
	}

	file = fopen("temp.map", "w");
	g_cpcAnalogCount = 0;
	g_cpcDiscreteCount = 0;
	if (file) {
		fprintf(file, "# mw2shell CockPit Config generated map file\n");
		for (i = 0; i < c_configCount * c_bindingCount; i++) {
			if (g_cpcBindings[i].m_deviceSlot >= 0) {
				FUN_10041168(file, g_unk0x1006afc0[i % c_bindingCount], &g_cpcBindings[i]);
				g_cpcBindings[i].m_flags &= CPC_FLAG_INVERTED | c_flagModifierMask;
				g_cpcBindings[i].m_modeFlags = 0;
			}
		}

		binding.m_channelKind = 1;
		binding.m_deviceSlot = g_unk0x1006b074;
		binding.m_flags = 0;
		binding.m_mode = -1;
		binding.m_modeFlags = -1;
		binding.m_controlIndex = 0x66;
		FUN_10041168(file, "legs_pan_minus", &binding);
		FUN_10041168(file, "jumpjet_enabled", &binding);
		binding.m_controlIndex = 0x64;
		FUN_10041168(file, "legs_pan_plus", &binding);
		FUN_10041168(file, "jumpjet_enabled", &binding);
		fprintf(file, "# analog count = %d\n", g_cpcAnalogCount);
		fprintf(file, "# discrete count = %d\n", g_cpcDiscreteCount);
		fclose(file);
	}
	else {
		ShowDialog("Error: Could not write map file.#Ok", 0);
	}

	return g_cpcAnalogCount <= 40 && g_cpcDiscreteCount <= 90;
}

// Mark the keyboard and the devices the bindings use active, drop the bindings of missing
// devices, then keep at most four devices active; the last one becomes current.
// FUNCTION: MW2SHELL 0x10041abc
void CpcValidateActiveDevices()
{
	InputDevice* device;
	MechU32 index;

	for (index = 0; (MechS32) index < c_deviceSlotCount; index++) {
		device = InputGetDevice(index);
		if (device != NULL && !strcmp(device->m_info.m_shortName, "keyboard")) {
			g_inputDeviceActive[index] = 1;
		}
		else {
			g_inputDeviceActive[index] = 0;
		}
	}

	for (index = 0; index < c_configCount * c_bindingCount; index++) {
		if (g_cpcBindings[index].m_deviceSlot >= 0) {
			g_curInputDeviceIdx = g_cpcBindings[index].m_deviceSlot;
			if (InputGetDevice(g_curInputDeviceIdx) == NULL) {
				FUN_1004207e(index, &g_cpcBindings[index]);
			}
			else {
				g_inputDeviceActive[g_curInputDeviceIdx] = 1;
			}
		}
	}

	g_activeInputDeviceCount = 0;
	for (index = 0; (MechS32) index < c_deviceSlotCount; index++) {
		device = InputGetDevice(index);
		if (device != NULL && g_inputDeviceActive[index] && g_activeInputDeviceCount < c_maxActiveDevices) {
			g_activeInputDeviceCount++;
			g_curInputDeviceIdx = index;
		}
		else {
			g_inputDeviceActive[index] = 0;
		}
	}
}

// Maps the device slots of loaded bindings onto the devices present: by name, then for a
// missing joystick onto an unused one (explaining either way), and drops the bindings whose
// device, axis or button is gone. Then records the present devices in the slots.
// Not 100%: the stack slots of the locals are permuted.
// FUNCTION: MW2SHELL 0x10041c7e
void CpcRemapDeviceSlots(CpcBinding* p_bindings)
{
	InputDevice* device;
	MechU32 i;
	MechS32 j;
	MechS32 k;
	MechS32 missing[c_deviceSlotCount];

	for (i = 0; (MechS32) i < c_deviceSlotCount; i++) {
		if (g_cpcDeviceSlots[i].m_deviceId != -1) {
			missing[i] = 1;
			g_cpcDeviceSlots[i].m_deviceId = -1;
		}
		else {
			missing[i] = 0;
		}
	}

	for (i = 0; (MechS32) i < c_deviceSlotCount; i++) {
		device = InputGetDevice(i);
		if (!device) {
			break;
		}

		for (j = 0; j < c_deviceSlotCount; j++) {
			if (!device->m_info.m_matchName[0]) {
				break;
			}

			if (!_strcmpi(g_cpcDeviceSlots[j].m_name, device->m_info.m_matchName) &&
				g_cpcDeviceSlots[i].m_deviceId == -1) {
				g_cpcDeviceSlots[j].m_deviceId = i;
				break;
			}
		}
	}

	for (i = 0; (MechS32) i < c_deviceSlotCount; i++) {
		if (missing[i] && g_cpcDeviceSlots[i].m_deviceId == -1 && g_cpcDeviceSlots[i].m_name[0]) {
			for (j = 0; j < c_deviceSlotCount; j++) {
				device = InputGetDevice(j);
				if (device && device->m_info.m_shortName[0] == 'j') {
					for (k = 0; k < c_deviceSlotCount; k++) {
						if (g_cpcDeviceSlots[k].m_deviceId == j) {
							break;
						}
					}

					if (k == c_deviceSlotCount) {
						FUN_100431b4(
							c_messageRemapJoystick,
							(undefined4) g_cpcDeviceSlots[i].m_name,
							device->m_info.m_displayName
						);
						g_cpcDeviceSlots[i].m_deviceId = j;
						break;
					}
				}
			}

			if (j == c_deviceSlotCount) {
				FUN_100431b4(c_messageMissingJoystick, (undefined4) g_cpcDeviceSlots[i].m_name, NULL);
			}
		}
	}

	for (i = 0; i < c_configCount * c_bindingCount; i++) {
		if (p_bindings[i].m_deviceSlot >= 0) {
			device = InputGetDevice(g_cpcDeviceSlots[p_bindings[i].m_deviceSlot].m_deviceId);
			if (device) {
				if (p_bindings[i].m_channelKind == 0 && p_bindings[i].m_mode == -1) {
					if (p_bindings[i].m_controlIndex >= device->m_info.m_axisCount) {
						p_bindings[i].m_deviceSlot = -1;
					}
				}
				else {
					if (p_bindings[i].m_controlIndex >= device->m_info.m_buttonCount) {
						p_bindings[i].m_deviceSlot = -1;
					}
				}

				if (p_bindings[i].m_deviceSlot >= 0) {
					p_bindings[i].m_deviceSlot = g_cpcDeviceSlots[p_bindings[i].m_deviceSlot].m_deviceId;
				}
			}
		}
	}

	for (i = 0; (MechS32) i < c_deviceSlotCount; i++) {
		device = InputGetDevice(i);
		if (device) {
			g_cpcDeviceSlots[i].m_deviceId = i;
			strncpy(g_cpcDeviceSlots[i].m_name, device->m_info.m_matchName, 0x10);
		}
		else {
			g_cpcDeviceSlots[i].m_deviceId = -1;
			g_cpcDeviceSlots[i].m_name[0] = '\0';
		}
	}
}

// Reset a binding: the first seven controls are axes, the rest buttons.
// FUNCTION: MW2SHELL 0x1004207e
void FUN_1004207e(MechS32 p_index, CpcBinding* p_binding)
{
	if (p_binding != NULL) {
		p_binding->m_deviceSlot = -1;
		if (p_index >= c_axisBindingCount) {
			p_binding->m_channelKind = 1;
		}
		else {
			p_binding->m_channelKind = 0;
		}

		p_binding->m_flags = 0;
		p_binding->m_modeFlags = 0;
		p_binding->m_controlIndex = -1;
		p_binding->m_mode = -1;
	}
}

// Reset every binding of the four configurations.
// FUNCTION: MW2SHELL 0x100420eb
void FUN_100420eb(SlateTab0x2c* p_tab)
{
	MechS32 config;
	MechU32 index;

	if (p_tab) {
	}

	for (config = 0; config < c_configCount; config++) {
		for (index = 0; index < c_bindingCount; index++) {
			FUN_1004207e(index, &g_cpcBindings[config * c_bindingCount + index]);
		}
	}

	g_unk0x1006b054 = 0;
	g_pCpcBindings = &g_cpcBindings[g_unk0x1006b054 * c_bindingCount];
	g_curCpcConfigSlot = 0;
}

// Reset the bindings to the current device.
// FUNCTION: MW2SHELL 0x10042199
void FUN_10042199(SlateTab0x2c* p_tab)
{
	MechS32 config;
	MechU32 index;

	if (p_tab) {
	}

	for (config = 0; config < c_configCount; config++) {
		for (index = 0; index < c_bindingCount; index++) {
			if (g_cpcBindings[config * c_bindingCount + index].m_deviceSlot != g_curInputDeviceIdx) {
				continue;
			}

			g_cpcBindings[config * c_bindingCount + index].m_deviceSlot = -1;
			if ((MechS32) index >= c_axisBindingCount) {
				g_cpcBindings[config * c_bindingCount + index].m_channelKind = 1;
			}
			else {
				g_cpcBindings[config * c_bindingCount + index].m_channelKind = 0;
			}

			g_cpcBindings[config * c_bindingCount + index].m_flags = 0;
			g_cpcBindings[config * c_bindingCount + index].m_modeFlags = 0;
			g_cpcBindings[config * c_bindingCount + index].m_controlIndex = 0;
			g_cpcBindings[config * c_bindingCount + index].m_mode = 0;
		}
	}

	g_unk0x1006b054 = 0;
	g_pCpcBindings = &g_cpcBindings[g_unk0x1006b054 * c_bindingCount];
}

// Load the current device's .cpc file and merge its bindings into the free ones.
// Stack-slot permutation: path, other, config, index and binding.
// FUNCTION: MW2SHELL 0x10042314
void CpcLoadDeviceFile(SlateTab0x2c* p_tab)
{
	FILE* file;
	MechChar path[32];
	MechS32 other;
	MechS32 config;
	MechU32 index;
	MechS32 binding;

	if (p_tab) {
	}

	sprintf(path, "giddi\\%s.cpc", InputGetDevice(g_curInputDeviceIdx)->m_info.m_matchName);
	file = fopen(path, "rb");
	if (file == NULL) {
		return;
	}

	fread(g_cpcConfigName, sizeof(g_cpcConfigName), 1, file);
	fread(g_cpcDeviceSlots, sizeof(CpcDeviceSlot), c_deviceSlotCount, file);
	fread(g_cpcDeviceFileBindings, sizeof(g_cpcDeviceFileBindings), 1, file);
	fclose(file);
	CpcRemapDeviceSlots(g_cpcDeviceFileBindings);

	for (config = 0; config < c_configCount; config++) {
		for (index = 0; index < c_bindingCount; index++) {
			binding = config * c_bindingCount + index;
			if (g_cpcDeviceFileBindings[binding].m_deviceSlot == g_curInputDeviceIdx) {
				for (other = 0; other < c_configCount; other++) {
					if (g_cpcBindings[other * c_bindingCount + index].m_deviceSlot ==
							g_cpcDeviceFileBindings[binding].m_deviceSlot &&
						g_cpcBindings[other * c_bindingCount + index].m_channelKind ==
							g_cpcDeviceFileBindings[binding].m_channelKind &&
						g_cpcBindings[other * c_bindingCount + index].m_flags ==
							g_cpcDeviceFileBindings[binding].m_flags &&
						g_cpcBindings[other * c_bindingCount + index].m_controlIndex ==
							g_cpcDeviceFileBindings[binding].m_controlIndex &&
						g_cpcBindings[other * c_bindingCount + index].m_mode ==
							g_cpcDeviceFileBindings[binding].m_mode &&
						g_cpcBindings[other * c_bindingCount + index].m_modeFlags ==
							g_cpcDeviceFileBindings[binding].m_modeFlags) {
						break;
					}

					if (g_cpcBindings[other * c_bindingCount + index].m_deviceSlot < 0) {
						g_cpcBindings[other * c_bindingCount + index] = g_cpcDeviceFileBindings[binding];
						break;
					}
				}
			}
		}
	}

	g_unk0x1006b054 = 0;
	g_pCpcBindings = &g_cpcBindings[g_unk0x1006b054 * c_bindingCount];
}

// Load giddi\configNN.cpc: slot 0 without a field, the field's slot, or the current slot.
// Stack-slot permutation: path and slot.
// FUNCTION: MW2SHELL 0x100425d3
void CpcLoadConfigSlot(SlateTab0x2c* p_tab)
{
	FILE* file;
	MechChar path[32];
	MechS32 slot;

	if (p_tab == NULL) {
		slot = 0;
	}
	else if ((MechS32) p_tab->m_unk0x28 < 0) {
		slot = g_curCpcConfigSlot;
	}
	else {
		slot = g_curCpcConfigSlot = (MechS32) p_tab->m_unk0x28;
	}

	sprintf(path, "giddi\\config%02d.cpc", slot);
	file = fopen(path, "rb");
	if (file == NULL) {
		return;
	}

	fread(g_cpcConfigName, sizeof(g_cpcConfigName), 1, file);
	fread(g_cpcDeviceSlots, sizeof(CpcDeviceSlot), c_deviceSlotCount, file);
	fread(g_cpcBindings, sizeof(g_cpcBindings), 1, file);
	fclose(file);
	CpcRemapDeviceSlots(g_cpcBindings);
	CpcValidateActiveDevices();
	g_unk0x1006b054 = 0;
	g_pCpcBindings = &g_cpcBindings[g_unk0x1006b054 * c_bindingCount];
}

// Save giddi\configNN.cpc, naming a custom slot's configuration after the slot.
// FUNCTION: MW2SHELL 0x100426e7
void CpcSaveConfigSlot(SlateTab0x2c* p_tab)
{
	FILE* file;
	MechS32 slot;

	if (p_tab == NULL) {
		slot = 0;
	}
	else if ((MechS32) p_tab->m_unk0x28 < 0) {
		slot = g_curCpcConfigSlot;
	}
	else {
		slot = g_curCpcConfigSlot = (MechS32) p_tab->m_unk0x28;
	}

	sprintf(g_unk0x100918b8, "giddi\\config%02d.cpc", slot);
	if (slot != 0) {
		if (!strcmp("Default Config", g_cpcConfigName)) {
			sprintf(g_cpcConfigName, "Custom Config #%d", slot);
		}
		else if (!strncmp("Custom Config #", g_cpcConfigName, 15)) {
			sprintf(g_cpcConfigName, "Custom Config #%d", slot);
		}
	}

	file = fopen(g_unk0x100918b8, "wb");
	if (file != NULL) {
		fwrite(g_cpcConfigName, sizeof(g_cpcConfigName), 1, file);
		fwrite(g_cpcDeviceSlots, sizeof(CpcDeviceSlot), c_deviceSlotCount, file);
		fwrite(g_cpcBindings, sizeof(g_cpcBindings), 1, file);
		fclose(file);
	}

	fclose(file);
	if (slot != 0) {
		sprintf(g_unk0x100918b8, "Configuration %d Saved.#Ok", slot);
		ShowDialog(g_unk0x100918b8, 0);
	}
}

// Reset the bindings and load the .cpc file of every active device.
// FUNCTION: MW2SHELL 0x1004289c
void CpcLoadActiveDevices(SlateTab0x2c* p_tab)
{
	MechS32 current;

	current = g_curInputDeviceIdx;
	if (p_tab) {
	}

	FUN_100420eb(NULL);
	for (g_curInputDeviceIdx = 0; g_curInputDeviceIdx < c_deviceSlotCount; g_curInputDeviceIdx++) {
		if (g_inputDeviceActive[g_curInputDeviceIdx]) {
			CpcLoadDeviceFile(NULL);
		}
	}

	strcpy(g_cpcConfigName, "Default Config");
	g_curInputDeviceIdx = current;
}

// Install the new input.map (keeping the old one as input.bak) and save the configuration.
// FUNCTION: MW2SHELL 0x10042940
void CpcAcceptAndCommit(SlateTab0x2c* p_tab)
{
	if (p_tab) {
	}

	if (!CpcCheckControlCount()) {
		ShowDialog("Error: Sim can not|handle that many controls.#Ok", 0);
		return;
	}

	remove("input.bak");
	rename("input.map", "input.bak");
	rename("temp.map", "input.map");
	CpcSaveConfigSlot(NULL);
	ShowDialog("Cockpit Control Configured.#Ok", 0);
	g_cpcConfigured = 1;
}

// Save the current device's bindings to its .cpc file.
// Stack-slot permutation: path, config, index and binding.
// FUNCTION: MW2SHELL 0x100429cf
void CpcSaveDeviceFile(SlateTab0x2c* p_tab)
{
	FILE* file;
	MechChar path[32];
	MechS32 config;
	MechU32 index;
	MechS32 binding;

	if (p_tab) {
	}

	for (config = 0; config < c_configCount; config++) {
		for (index = 0; index < c_bindingCount; index++) {
			binding = config * c_bindingCount + index;
			if (g_cpcBindings[binding].m_deviceSlot == g_curInputDeviceIdx) {
				g_cpcDeviceFileBindings[binding] = g_cpcBindings[binding];
			}
			else {
				g_cpcDeviceFileBindings[binding].m_deviceSlot = -1;
				if ((MechS32) index >= c_axisBindingCount) {
					g_cpcDeviceFileBindings[binding].m_channelKind = 1;
				}
				else {
					g_cpcDeviceFileBindings[binding].m_channelKind = 0;
				}

				g_cpcDeviceFileBindings[binding].m_flags = 0;
				g_cpcDeviceFileBindings[binding].m_modeFlags = 0;
				g_cpcDeviceFileBindings[binding].m_controlIndex = 0;
				g_cpcDeviceFileBindings[binding].m_mode = 0;
			}
		}
	}

	sprintf(path, "giddi\\%s.cpc", InputGetDevice(g_curInputDeviceIdx)->m_info.m_shortName);
	file = fopen(path, "wb");
	if (file != NULL) {
		fwrite(g_cpcConfigName, sizeof(g_cpcConfigName), 1, file);
		fwrite(g_cpcDeviceSlots, sizeof(CpcDeviceSlot), c_deviceSlotCount, file);
		fwrite(g_cpcDeviceFileBindings, sizeof(g_cpcDeviceFileBindings), 1, file);
		fclose(file);
	}

	fclose(file);
}

// FUNCTION: MW2SHELL 0x10042b99
void FUN_10042b99(SlateTab0x2c* p_tab)
{
	if (p_tab) {
	}

	if (!CpcCheckControlCount()) {
		ShowDialog("Error: Sim can not|handle that many controls.#Ok", 0);
	}
}

// Opens the cockpit controls screen over the current one: its palette and logo, the devices
// (the keyboard is required), the configuration of the first slot, and its fields.
// FUNCTION: MW2SHELL 0x10042bcf
void OpenCockpitControls()
{
	InputDevice* device;
	MechS32 i;

	g_pVideoDriver->GetPalette(g_unk0x10071378);
	g_pVideoDriver->LoadPalette(4);
	g_pVideoDriver->ActivateFramebuffer();
	g_unk0x100918b0 = NULL;
	g_unk0x100918b0 = new SilverReel0x18("amwlogo1", 0x78, 4);
	g_pVideoDriver->DrawShell();
	g_pVideoDriver->m_unk0x3a6 = 0;
	g_curCpcConfigSlot = 0;
	if (!InputEnumDevices(1)) {
		return;
	}

	g_unk0x1006b074 = -1;
	for (i = 0; i < c_deviceSlotCount; i++) {
		device = InputGetDevice(i);
		if (device) {
			g_cpcDeviceSlots[i].m_deviceId = i;
			strcpy(g_cpcDeviceSlots[i].m_name, device->m_info.m_matchName);
			if (!strcmp(device->m_info.m_matchName, "keyboard")) {
				g_unk0x1006b074 = i;
				g_curInputDeviceIdx = i;
				g_inputDeviceActive[i] = 1;
			}
			else {
				g_inputDeviceActive[i] = 0;
			}
		}
		else {
			g_cpcDeviceSlots[i].m_deviceId = -1;
			strcpy(g_cpcDeviceSlots[i].m_name, "");
			g_inputDeviceActive[i] = 0;
		}
	}

	if (g_unk0x1006b074 < 0) {
		ShowDialog("Error: keyboard not initialized.#Ok", 0);
		InputFreeDevices();
		return;
	}

	strcpy(g_cpcConfigName, "NO CONFIG");
	FUN_100420eb(NULL);
	CpcLoadConfigSlot(NULL);
	CpcValidateActiveDevices();
	g_unk0x1006b05c = 0;
	g_unk0x1006af18 = -1;
	g_unk0x1006af1c = 0;
	g_unk0x1006b054 = 0;
	g_pCpcBindings = &g_cpcBindings[g_unk0x1006b054 * c_bindingCount];
	g_unk0x100711f8->FUN_100440ed();

	g_unk0x10092b18[0] = 0xff;
	g_unk0x10092b18[1] = 0x10;
	g_unk0x10092a18[0] = 0xff;
	g_unk0x10092a18[1] = 7;
	for (i = 2; i < 0x100; i++) {
		g_unk0x10092b18[i] = i;
		g_unk0x10092a18[i] = i;
	}

	FUN_100078cd(g_unk0x1006cde8);
	g_unk0x1006b06c = 0;
	g_cpcConfigured = 0;
	g_inputConfigChanged = 0;
	FUN_100109a0(CpcScreenTick);
}

// The cockpit controls screen's frame, run over the screen below: the clicks on its fields (the
// right button too on the bindings page, g_unk0x1006b06c). Closes the screen when p_active is
// cleared, once configured, on the key code 3 or on a right click on the first page.
// FUNCTION: MW2SHELL 0x10042f65
void CpcScreenTick(MechS32 p_active)
{
	SlateTab0x2c* tab;

	if (p_active) {
		if (g_unk0x100918b0) {
			g_unk0x100918b0->FUN_1001630b();
		}

		if (g_pMouseState->GetLeftPressed() == 1 || (g_unk0x1006b06c == 1 && g_pMouseState->GetRightPressed() == 1)) {
			tab = FUN_1000b5ed(
				g_unk0x1006b06c ? g_unk0x1006b0d8 : g_unk0x1006cde8,
				g_pMouseState->m_x,
				g_pMouseState->m_y
			);
			if (tab && tab->m_unk0x20) {
				tab->m_unk0x20(tab);
				FUN_100079f8(g_unk0x1006b06c ? g_unk0x1006b0d8 : g_unk0x1006cde8);
			}
		}
	}

	if (!p_active || g_cpcConfigured || g_unk0x100711f8->FUN_10044189() == 3 ||
		(!g_unk0x1006b06c && g_pMouseState->GetRightPressed() == 1)) {
		FUN_100109b8(CpcScreenTick);
		EnableMenuItem(g_windowMenu, 0x9c4b, MF_ENABLED);
		g_menuDialogOpen = 0;
		FUN_10007ac8(g_unk0x1006b06c ? g_unk0x1006b0d8 : g_unk0x1006cde8);
		if (g_unk0x100918b0) {
			delete g_unk0x100918b0;
		}
		g_unk0x100918b0 = NULL;
		InputFreeDevices();
		g_pVideoDriver->m_unk0x3a6 = -1;
		g_pVideoDriver->FUN_100071ad(0, 0, 640, 480);
		FUN_1001661b();
		g_pVideoDriver->SetPalette(g_unk0x10071378, 1);
		if (p_active) {
			g_pVideoDriver->DrawShell();
			g_pVideoDriver->FUN_100071ad(0, 0, 640, 480);
			FUN_1001661b();
		}
	}
}

// Explain a problem with the input devices in a message box.
// FUNCTION: MW2SHELL 0x100431b4
void FUN_100431b4(MechS32 p_message, undefined4, MechChar* p_name)
{
	// Set and never read.
	undefined4 result = 0;

	switch (p_message) {
	case c_messageNoJoysticks:
		sprintf(
			g_unk0x100906d0,
			"There are no joystick devices currently configured in the Windows Joystick Control Panel.  You must "
			"configure, calibrate, and test your joystick in the Control Panel before MechWarrior 2 can use it."
		);
		break;
	case c_messageMissingJoystick:
		sprintf(
			g_unk0x100906d0,
			"The current Cockpit Controls configuration includes a joystick which no longer exists in the system "
			"or is not configured properly.\n\nTo eliminate the problem, you can chose \"ABORT\" from the Cockpit "
			"Controls screen and check the Windows Joystick Control Panel settings.\n\nOtherwise, this device will "
			"be ignored.  If you choose \"ACCEPT\",\nit will be removed from the Cockpit Controls configuration."
		);
		break;
	case c_messageRemapJoystick:
		sprintf(
			g_unk0x100906d0,
			"The current Cockpit Controls configuration includes a joystick which no longer exists in the "
			"system.\n\nMechWarrior 2 will attempt to remap its controls to the \"%s\" from the Windows Joystick "
			"Control Panel.",
			p_name
		);
		break;
	default:
		sprintf(
			g_unk0x100906d0,
			"An input device has caused an undefined error. Sorry, no other information is available."
		);
		break;
	}

	MessageBox(g_pWnd, g_unk0x100906d0, "MechWarrior 2 Message", MB_ICONASTERISK);
}

// Lay out a button: its rectangle around a center, and its caption centered in it.
// FUNCTION: MW2SHELL 0x10043280
void FUN_10043280(
	PewterPlaque0x9c* p_plaque,
	MechS32 p_centerX,
	MechS32 p_centerY,
	MechS32 p_width,
	MechS32 p_height,
	BrassLantern0x414* p_font
)
{
	p_plaque->m_textLeft = p_centerX;
	p_plaque->m_textTop = p_centerY;
	p_plaque->m_left = p_plaque->m_textLeft - p_width / 2;
	p_plaque->m_right = p_plaque->m_left + p_width - 1;
	p_plaque->m_top = p_plaque->m_textTop - p_height / 2;
	p_plaque->m_bottom = p_plaque->m_top + p_height - 1;
	p_plaque->m_textTop -= p_font->m_unk0x40c / 2;
	p_plaque->m_textLeft -= p_font->FUN_100053be(p_plaque->m_text) / 2;
	p_plaque->m_unk0x98 = 0;
}

// Whether a point lies in a button.
// FUNCTION: MW2SHELL 0x10043333
MechS32 FUN_10043333(PewterPlaque0x9c* p_plaque, MechS32 p_x, MechS32 p_y)
{
	return p_x >= p_plaque->m_left && p_x <= p_plaque->m_right && p_y >= p_plaque->m_top && p_y <= p_plaque->m_bottom;
}
