#include "cockpitpanel.h"

#include "cobaltharbor.h"
#include "decomp.h"
#include "recttransition.h"
#include "rendertarget.h"
#include "types.h"

#include <string.h>

DECOMP_SIZE_ASSERT(CobaltHarbor0x88, 0x88)

// FUNCTION: MW2 0x100746c0
void FUN_100746c0(CobaltHarbor0x88* p_panel)
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
	p_panel->m_init = FUN_100746c0;
	p_panel->m_unk0x4c = FUN_100747c9;
	p_panel->m_setUnk0x08 = FUN_100747d4;
	p_panel->m_setUnk0x0c = FUN_100747e8;
	p_panel->m_setName = FUN_100747fc;
	p_panel->m_setTarget = FUN_10074823;
	p_panel->m_setUnk0x34 = FUN_10074879;
	p_panel->m_setTransition = FUN_1007488d;
	p_panel->m_setRect = FUN_100748a1;
	p_panel->m_setUnk0x06 = FUN_100748d4;
	p_panel->m_enable = FUN_100748e9;
	p_panel->m_disable = FUN_100748fd;
	p_panel->m_unk0x78 = NULL;
	p_panel->m_unk0x7c = NULL;
	p_panel->m_unk0x80 = NULL;
	p_panel->m_unk0x84 = NULL;
}

// FUNCTION: MW2 0x100747c9
void FUN_100747c9(CobaltHarbor0x88* p_panel)
{
}

// FUNCTION: MW2 0x100747d4
void FUN_100747d4(CobaltHarbor0x88* p_panel, undefined4 p_unk0x08)
{
	p_panel->m_unk0x08 = p_unk0x08;
}

// FUNCTION: MW2 0x100747e8
void FUN_100747e8(CobaltHarbor0x88* p_panel, MechS32 p_unk0x0c)
{
	p_panel->m_unk0x0c = p_unk0x0c;
}

// FUNCTION: MW2 0x100747fc
void FUN_100747fc(CobaltHarbor0x88* p_panel, const MechChar* p_name)
{
	strncpy(p_panel->m_name, p_name, sizeof(p_panel->m_name));
	p_panel->m_name[sizeof(p_panel->m_name) - 1] = '\0';
}

// FUNCTION: MW2 0x10074823
void FUN_10074823(CobaltHarbor0x88* p_panel, PANE* p_target)
{
	p_panel->m_target = p_target;
	p_panel->m_x = p_target->m_x0;
	p_panel->m_y = p_target->m_y0;
	p_panel->m_width = p_target->m_x1 - p_target->m_x0 + 1;
	p_panel->m_height = p_target->m_y1 - p_target->m_y0 + 1;
}

// FUNCTION: MW2 0x10074879
void FUN_10074879(CobaltHarbor0x88* p_panel, Point* p_unk0x34)
{
	p_panel->m_unk0x34 = p_unk0x34;
}

// FUNCTION: MW2 0x1007488d
void FUN_1007488d(CobaltHarbor0x88* p_panel, RectTransition* p_transition)
{
	p_panel->m_transition = p_transition;
}

// FUNCTION: MW2 0x100748a1
void FUN_100748a1(CobaltHarbor0x88* p_panel, MechS32 p_x, MechS32 p_y, MechS32 p_width, MechS32 p_height)
{
	p_panel->m_x = p_x;
	p_panel->m_y = p_y;
	p_panel->m_width = p_width;
	p_panel->m_height = p_height;
}

// FUNCTION: MW2 0x100748d4
void FUN_100748d4(CobaltHarbor0x88* p_panel, MechS32 p_unk0x06)
{
	p_panel->m_unk0x06 = p_unk0x06;
}

// FUNCTION: MW2 0x100748e9
void FUN_100748e9(CobaltHarbor0x88* p_panel)
{
	p_panel->m_enabled = 1;
}

// FUNCTION: MW2 0x100748fd
void FUN_100748fd(CobaltHarbor0x88* p_panel)
{
	p_panel->m_enabled = 0;
}
