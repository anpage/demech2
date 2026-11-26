#ifndef COCKPITPANEL_H
#define COCKPITPANEL_H

#include "decomp.h"
#include "point.h"
#include "targeting.h"
#include "types.h"

typedef struct CockpitPanel CockpitPanel;

/* One of the 26 cockpit panels InitCockpitPanels allocates (g_cockpitPanels): a named rectangle
   of a pane with a table of methods. InitCockpitPanel sets the defaults, and InitCockpitPanels
   installs the drawing handlers, which UpdateCockpit calls by the local mech's power state. The
   PowerVR edition's are 0x98 bytes: a panel with a 3D inset has a view of the renderer's after its
   name, and two more handlers at the end, so that the members after the name are 8 bytes further
   on there. The view is a void*: a struct A3DView tag declared here, ahead of the units that
   include this header without accel/a3d.h, moves their code. */
// SIZE 0x88
struct CockpitPanel {
	CockpitPanel* m_self;  // 0x00
	MechS16 m_enabled;     // 0x04
	MechS16 m_damage;      // 0x06 — hits step it (DamageCockpitPanels); the panel flickers or shows static as it rises
	MechS32 m_lightUpTime; // 0x08 — the clock time the panel lights up at on startup
	MechS32 m_weapon;      // 0x0c — the weapon a weapon panel shows, -1: none
	MechChar m_name[0x20]; // 0x10
#ifdef MW2_POWERVR
	// In the PowerVR edition these two put every member after them 8 bytes further on than the
	// offsets given, which are the other editions'.
	MechS32 m_unk0x30; // 0x30 — set once the view's camera is placed
	void* m_view;      // 0x34 — the renderer's A3DView of the panel's inset, or NULL
#endif
#ifdef MW2_COCKPIT_PANEL_UNK_MEMBERS
	// The early Mercenaries build's panels are 0xf0 bytes: an MFD's mode comes first, set by a
	// method of its own after m_setTarget, then the MFD's gauges, so the members from m_x on sit
	// 0x64 bytes further than their comments say, and from m_setTextOrigin on 0x68.
	MechS32 m_mfdMode; // 0x30 — an MFD's mode (DrawMechViewPanel), 0: off
	// Each MFD's gauges (ScaleBarGauges), in pixels of its pane: the throttle gauge's size and
	// frame, the left of its bar and its zero; the jump fuel bar's place, size and frame; the heat
	// and heat rate bars' places and sizes.
	struct Point m_throttleSize;         // 0x34
	MechS32 m_throttleFrameLeft;         // 0x3c
	MechS32 m_throttleFrameTop;          // 0x40
	MechS32 m_throttleFrameRight;        // 0x44
	MechS32 m_throttleFrameBottom;       // 0x48
	MechS32 m_throttleBarLeft;           // 0x4c
	MechS32 m_throttleZeroY;             // 0x50
	struct Point m_jumpFuelPosition;     // 0x54
	struct Point m_jumpFuelSize;         // 0x5c
	MechS32 m_jumpFuelFrameLeft;         // 0x64
	MechS32 m_jumpFuelFrameTop;          // 0x68
	MechS32 m_jumpFuelFrameRight;        // 0x6c
	MechS32 m_jumpFuelFrameBottom;       // 0x70
	struct Point m_heatPosition;         // 0x74
	struct Point m_heatSize;             // 0x7c
	struct Point m_heatRatePosition;     // 0x84
	struct Point m_heatRateSize;         // 0x8c
	PANE* m_target;                      // 0x94
	struct Point* m_textOrigin;          // 0x98
	struct RectTransition* m_transition; // 0x9c
	MechS32 m_lastPowerState;            // 0xa0
#else
	PANE* m_target;                      // 0x30
	struct Point* m_textOrigin;          // 0x34 — where its text goes
	struct RectTransition* m_transition; // 0x38
	MechS32 m_lastPowerState;            // 0x3c — the power state it last drew in (g_cockpitPowerState)
#endif
	MechS16 m_x;                                         // 0x40
	MechS16 m_y;                                         // 0x42
	MechS16 m_width;                                     // 0x44
	MechS16 m_height;                                    // 0x46
	void (*m_init)(CockpitPanel*);                       // 0x48
	void (*m_shutdown)(CockpitPanel*);                   // 0x4c — ShutdownCockpitPanels calls it
	void (*m_setLightUpTime)(CockpitPanel*, undefined4); // 0x50
	void (*m_setWeapon)(CockpitPanel*, MechS32);         // 0x54
	void (*m_setName)(CockpitPanel*, const MechChar*);   // 0x58
	void (*m_setTarget)(CockpitPanel*, PANE*);           // 0x5c
#ifdef MW2_COCKPIT_PANEL_UNK_MEMBERS
	void (*m_setMfdMode)(CockpitPanel*, MechS32); // 0xc4 in the early Mercenaries build
#endif
	void (*m_setTextOrigin)(CockpitPanel*, struct Point*);                // 0x60
	void (*m_setTransition)(CockpitPanel*, struct RectTransition*);       // 0x64
	void (*m_setRect)(CockpitPanel*, MechS32, MechS32, MechS32, MechS32); // 0x68
	void (*m_setDamage)(CockpitPanel*, MechS32);                          // 0x6c
	void (*m_enable)(CockpitPanel*);                                      // 0x70
	void (*m_disable)(CockpitPanel*);                                     // 0x74
	void (*m_drawStartup)();  // 0x78 — while the mech starts up (power state 1)
	void (*m_draw)();         // 0x7c — while it runs (2)
	void (*m_drawShutdown)(); // 0x80 — in the other states
	void (*m_drawStatic)();   // 0x84 — the panel as static; nothing calls it through here
#ifdef MW2_POWERVR
	void (*m_unk0x90)(CockpitPanel*); // 0x90 — before the scene is rendered: fills or places the view
	void (*m_unk0x94)(CockpitPanel*); // 0x94 — after: undoes or completes what m_unk0x90 set
#endif
};

// The cockpit panels, by index in g_cockpitPanels: one per weapon slot from c_panelFirstWeapon on,
// 14 in GBL (MW2_FOURTEEN_WEAPON_SLOTS, which takes the others 4 further).
enum {
	c_panelRadar = 0, // its damage also breaks up the map views
	c_panelMechView = 2,
	c_panelFirstWeapon = 3, // to 12 (GBL: 16)
#ifdef MW2_FOURTEEN_WEAPON_SLOTS
	c_panelTarget = 17,
	c_panelTargetText = 18,
	c_panelObjectives = 19,
	c_panelNetwork = 20,
	c_panelAutopilot = 21,
	c_panelSpeed = 22,
	c_panelMasc = 23,
	c_panelHeat = 24,
	c_panelHeatRate = 25,
	c_panelJets = 26,
	c_panelAltimeter = 27,
	c_panelCompass = 28,
	c_panelKills = 29,
#ifdef MW2_THIRTY_FOUR_PANELS
	c_panelCount = 34
#else
	c_panelCount = 30
#endif
#else
	c_panelTarget = 13,
	c_panelTargetText = 14,
	c_panelObjectives = 15,
	c_panelNetwork = 16,
	c_panelAutopilot = 17,
	c_panelSpeed = 18,
	c_panelMasc = 19,
	c_panelHeat = 20,
	c_panelHeatRate = 21,
	c_panelJets = 22,
	c_panelAltimeter = 23,
	c_panelCompass = 24,
	c_panelKills = 25,
	c_panelCount = 26
#endif
};

// The functions and globals of cockpitpanel.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void InitCockpitPanel(CockpitPanel* p_panel);
	void ShutdownCockpitPanel(CockpitPanel* p_panel);
	void SetCockpitPanelLightUpTime(CockpitPanel* p_panel, undefined4 p_lightUpTime);
	void SetCockpitPanelWeapon(CockpitPanel* p_panel, MechS32 p_weapon);
	void SetCockpitPanelName(CockpitPanel* p_panel, const MechChar* p_name);
	void SetCockpitPanelTarget(CockpitPanel* p_panel, PANE* p_target);
#ifdef MW2_COCKPIT_PANEL_UNK_MEMBERS
	void SetCockpitPanelMfdMode(CockpitPanel* p_panel, MechS32 p_mfdMode);
#endif
	void SetCockpitPanelTextOrigin(CockpitPanel* p_panel, Point* p_textOrigin);
	void SetCockpitPanelTransition(CockpitPanel* p_panel, struct RectTransition* p_transition);
	void SetCockpitPanelRect(CockpitPanel* p_panel, MechS32 p_x, MechS32 p_y, MechS32 p_width, MechS32 p_height);
	void SetCockpitPanelDamage(CockpitPanel* p_panel, MechS32 p_damage);
	void EnableCockpitPanel(CockpitPanel* p_panel);
	void DisableCockpitPanel(CockpitPanel* p_panel);

#ifdef __cplusplus
}
#endif

#endif // COCKPITPANEL_H
