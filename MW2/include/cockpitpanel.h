#ifndef COCKPITPANEL_H
#define COCKPITPANEL_H

#include "decomp.h"
#include "point.h"
#include "targeting.h"
#include "types.h"

typedef struct CockpitPanel CockpitPanel;

/* One of the 26 cockpit panels FUN_1006fca5 allocates (0x100c3280): a named rectangle of a
   pane with a table of methods. InitCockpitPanel sets the defaults, and callers replace
   some of the methods and the handlers at 0x78-0x84. */
// SIZE 0x88
struct CockpitPanel {
	CockpitPanel* m_self;                                                 // 0x00
	MechS16 m_enabled;                                                    // 0x04
	MechS16 m_unk0x06;                                                    // 0x06
	MechS32 m_unk0x08;                                                    // 0x08
	MechS32 m_unk0x0c;                                                    // 0x0c
	MechChar m_name[0x20];                                                // 0x10
	PANE* m_target;                                                       // 0x30
	struct Point* m_unk0x34;                                              // 0x34 — where its text goes
	struct RectTransition* m_transition;                                  // 0x38
	MechS32 m_unk0x3c;                                                    // 0x3c — the view mode it last drew
	MechS16 m_x;                                                          // 0x40
	MechS16 m_y;                                                          // 0x42
	MechS16 m_width;                                                      // 0x44
	MechS16 m_height;                                                     // 0x46
	void (*m_init)(CockpitPanel*);                                        // 0x48
	void (*m_unk0x4c)(CockpitPanel*);                                     // 0x4c
	void (*m_setUnk0x08)(CockpitPanel*, undefined4);                      // 0x50
	void (*m_setUnk0x0c)(CockpitPanel*, MechS32);                         // 0x54
	void (*m_setName)(CockpitPanel*, const MechChar*);                    // 0x58
	void (*m_setTarget)(CockpitPanel*, PANE*);                            // 0x5c
	void (*m_setUnk0x34)(CockpitPanel*, struct Point*);                   // 0x60
	void (*m_setTransition)(CockpitPanel*, struct RectTransition*);       // 0x64
	void (*m_setRect)(CockpitPanel*, MechS32, MechS32, MechS32, MechS32); // 0x68
	void (*m_setUnk0x06)(CockpitPanel*, MechS32);                         // 0x6c
	void (*m_enable)(CockpitPanel*);                                      // 0x70
	void (*m_disable)(CockpitPanel*);                                     // 0x74
	void (*m_unk0x78)();                                                  // 0x78
	void (*m_unk0x7c)();                                                  // 0x7c
	void (*m_unk0x80)();                                                  // 0x80
	void (*m_unk0x84)();                                                  // 0x84
};

// The functions and globals of cockpitpanel.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void InitCockpitPanel(CockpitPanel* p_panel);
	void FUN_100747c9(CockpitPanel* p_panel);
	void FUN_100747d4(CockpitPanel* p_panel, undefined4 p_unk0x08);
	void FUN_100747e8(CockpitPanel* p_panel, MechS32 p_unk0x0c);
	void SetCockpitPanelName(CockpitPanel* p_panel, const MechChar* p_name);
	void SetCockpitPanelTarget(CockpitPanel* p_panel, PANE* p_target);
	void FUN_10074879(CockpitPanel* p_panel, Point* p_unk0x34);
	void SetCockpitPanelTransition(CockpitPanel* p_panel, struct RectTransition* p_transition);
	void SetCockpitPanelRect(CockpitPanel* p_panel, MechS32 p_x, MechS32 p_y, MechS32 p_width, MechS32 p_height);
	void FUN_100748d4(CockpitPanel* p_panel, MechS32 p_unk0x06);
	void EnableCockpitPanel(CockpitPanel* p_panel);
	void DisableCockpitPanel(CockpitPanel* p_panel);

#ifdef __cplusplus
}
#endif

#endif // COCKPITPANEL_H
