#ifndef MENUCONTROL_H
#define MENUCONTROL_H

#include "decomp.h"
#include "types.h"

struct MenuPage;

// A value a menu item edits (menucontrols.c: a slider, a list of choices or a text box): the
// value, the control's own data and the callbacks that read and apply it (the brightness menu's
// are GetBrightnessFraction, SetBrightnessFraction, PreviewBrightnessFraction and
// RestoreBrightness).
typedef struct MenuControl {
	MechU32 m_flags;  // 0x00 — 1: read the value through m_get every time
	MechS32 m_value;  // 0x04
	void* m_data;     // 0x08 — shape ids, MenuChoices or a MenuTextBox
	undefined4 m_arg; // 0x0c — passed to the callbacks
	void (*m_init)(struct MenuPage* p_page, struct MenuControl* p_control); // 0x10
	MechS32 (*m_get)(undefined4 p_arg);                                     // 0x14
	void (*m_set)(undefined4 p_arg, MechS32 p_value);                       // 0x18 — when the menu is accepted
	void (*m_preview)(undefined4 p_arg, MechS32 p_value);                   // 0x1c — when the value changes
	void (*m_cancel)(undefined4 p_arg);                                     // 0x20 — when the menu is cancelled
} MenuControl;

#endif // MENUCONTROL_H
