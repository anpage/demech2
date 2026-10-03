#include "cockpitpanel.h"

#include "decomp.h"
#include "recttransition.h"
#include "targeting.h"
#include "types.h"

#include <string.h>

DECOMP_SIZE_ASSERT(CockpitPanel, 0x88)

// FUNCTION: MW2 0x100746c0
void InitCockpitPanel(CockpitPanel* p_panel)
{
	p_panel->m_self = p_panel;
	p_panel->m_unk0x06 = 0;
	p_panel->m_unk0x08 = 0;
	p_panel->m_unk0x0c = -1;
	memset(p_panel->m_name, 0, sizeof(p_panel->m_name));
	p_panel->m_x = 0;
	p_panel->m_y = 0;
	p_panel->m_width = 0;
	p_panel->m_height = 0;
	p_panel->m_init = InitCockpitPanel;
	p_panel->m_unk0x4c = FUN_100747c9;
	p_panel->m_setUnk0x08 = FUN_100747d4;
	p_panel->m_setUnk0x0c = FUN_100747e8;
	p_panel->m_setName = SetCockpitPanelName;
	p_panel->m_setTarget = SetCockpitPanelTarget;
	p_panel->m_setUnk0x34 = FUN_10074879;
	p_panel->m_setTransition = SetCockpitPanelTransition;
	p_panel->m_setRect = SetCockpitPanelRect;
	p_panel->m_setUnk0x06 = FUN_100748d4;
	p_panel->m_enable = EnableCockpitPanel;
	p_panel->m_disable = DisableCockpitPanel;
	p_panel->m_unk0x78 = NULL;
	p_panel->m_unk0x7c = NULL;
	p_panel->m_unk0x80 = NULL;
	p_panel->m_unk0x84 = NULL;
}

// FUNCTION: MW2 0x100747c9
void FUN_100747c9(CockpitPanel* p_panel)
{
}

// FUNCTION: MW2 0x100747d4
void FUN_100747d4(CockpitPanel* p_panel, undefined4 p_unk0x08)
{
	p_panel->m_unk0x08 = p_unk0x08;
}

// FUNCTION: MW2 0x100747e8
void FUN_100747e8(CockpitPanel* p_panel, MechS32 p_unk0x0c)
{
	p_panel->m_unk0x0c = p_unk0x0c;
}

// FUNCTION: MW2 0x100747fc
void SetCockpitPanelName(CockpitPanel* p_panel, const MechChar* p_name)
{
	strncpy(p_panel->m_name, p_name, sizeof(p_panel->m_name));
	p_panel->m_name[sizeof(p_panel->m_name) - 1] = '\0';
}

// FUNCTION: MW2 0x10074823
void SetCockpitPanelTarget(CockpitPanel* p_panel, PANE* p_target)
{
	p_panel->m_target = p_target;
	p_panel->m_x = p_target->m_x0;
	p_panel->m_y = p_target->m_y0;
	p_panel->m_width = p_target->m_x1 - p_target->m_x0 + 1;
	p_panel->m_height = p_target->m_y1 - p_target->m_y0 + 1;
}

// FUNCTION: MW2 0x10074879
void FUN_10074879(CockpitPanel* p_panel, Point* p_unk0x34)
{
	p_panel->m_unk0x34 = p_unk0x34;
}

// FUNCTION: MW2 0x1007488d
void SetCockpitPanelTransition(CockpitPanel* p_panel, RectTransition* p_transition)
{
	p_panel->m_transition = p_transition;
}

// FUNCTION: MW2 0x100748a1
void SetCockpitPanelRect(CockpitPanel* p_panel, MechS32 p_x, MechS32 p_y, MechS32 p_width, MechS32 p_height)
{
	p_panel->m_x = p_x;
	p_panel->m_y = p_y;
	p_panel->m_width = p_width;
	p_panel->m_height = p_height;
}

// FUNCTION: MW2 0x100748d4
void FUN_100748d4(CockpitPanel* p_panel, MechS32 p_unk0x06)
{
	p_panel->m_unk0x06 = p_unk0x06;
}

// FUNCTION: MW2 0x100748e9
void EnableCockpitPanel(CockpitPanel* p_panel)
{
	p_panel->m_enabled = 1;
}

// FUNCTION: MW2 0x100748fd
void DisableCockpitPanel(CockpitPanel* p_panel)
{
	p_panel->m_enabled = 0;
}
