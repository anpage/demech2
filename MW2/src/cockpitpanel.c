#include "cockpitpanel.h"

#include "decomp.h"
#include "recttransition.h"
#include "targeting.h"
#include "types.h"

#include <string.h>

#if defined(MW2_POWERVR)
DECOMP_SIZE_ASSERT(CockpitPanel, 0x98)
#elif defined(MW2_COCKPIT_PANEL_UNK_MEMBERS)
DECOMP_SIZE_ASSERT(CockpitPanel, 0xf0)
#else
DECOMP_SIZE_ASSERT(CockpitPanel, 0x88)
#endif

// FUNCTION: MW2 0x100746c0
// FUNCTION: MW2GBL 0x10016d30
// FUNCTION: MW2PREMERCS 0x100533d0
// FUNCTION: MW2MATROX 0x1001c7d0
// FUNCTION: MW23DFX 0x10023440
// FUNCTION: MW2ATI 0x10023700
// FUNCTION: MW2S3 0x10037870
// FUNCTION: MW2POWERVR 0x10022940
void InitCockpitPanel(CockpitPanel* p_panel)
{
	p_panel->m_self = p_panel;
	p_panel->m_damage = 0;
	p_panel->m_lightUpTime = 0;
	p_panel->m_weapon = -1;
	memset(p_panel->m_name, 0, sizeof(p_panel->m_name));
#ifdef MW2_POWERVR
	p_panel->m_unk0x30 = 0;
	p_panel->m_view = NULL;
#endif
	p_panel->m_x = 0;
	p_panel->m_y = 0;
	p_panel->m_width = 0;
	p_panel->m_height = 0;
	p_panel->m_init = InitCockpitPanel;
	p_panel->m_shutdown = ShutdownCockpitPanel;
	p_panel->m_setLightUpTime = SetCockpitPanelLightUpTime;
	p_panel->m_setWeapon = SetCockpitPanelWeapon;
	p_panel->m_setName = SetCockpitPanelName;
	p_panel->m_setTarget = SetCockpitPanelTarget;
#ifdef MW2_COCKPIT_PANEL_UNK_MEMBERS
	p_panel->m_setMfdMode = SetCockpitPanelMfdMode;
#endif
	p_panel->m_setTextOrigin = SetCockpitPanelTextOrigin;
	p_panel->m_setTransition = SetCockpitPanelTransition;
	p_panel->m_setRect = SetCockpitPanelRect;
	p_panel->m_setDamage = SetCockpitPanelDamage;
	p_panel->m_enable = EnableCockpitPanel;
	p_panel->m_disable = DisableCockpitPanel;
	p_panel->m_drawStartup = NULL;
	p_panel->m_draw = NULL;
	p_panel->m_drawShutdown = NULL;
	p_panel->m_drawStatic = NULL;
#ifdef MW2_POWERVR
	p_panel->m_unk0x90 = NULL;
	p_panel->m_unk0x94 = NULL;
#endif
}

// FUNCTION: MW2 0x100747c9
// FUNCTION: MW2GBL 0x10016e3d
// FUNCTION: MW2PREMERCS 0x10053520
// FUNCTION: MW2MATROX 0x1001c8d9
// FUNCTION: MW23DFX 0x100234e0
// FUNCTION: MW2ATI 0x100237a0
// FUNCTION: MW2S3 0x10037910
// FUNCTION: MW2POWERVR 0x10022a00
void ShutdownCockpitPanel(CockpitPanel* p_panel)
{
}

// FUNCTION: MW2 0x100747d4
// FUNCTION: MW2GBL 0x10016e48
// FUNCTION: MW2PREMERCS 0x1005352b
// FUNCTION: MW2MATROX 0x1001c8e4
// FUNCTION: MW23DFX 0x100234f0
// FUNCTION: MW2ATI 0x100237b0
// FUNCTION: MW2S3 0x10037920
// FUNCTION: MW2POWERVR 0x10022a10
void SetCockpitPanelLightUpTime(CockpitPanel* p_panel, undefined4 p_lightUpTime)
{
	p_panel->m_lightUpTime = p_lightUpTime;
}

// FUNCTION: MW2 0x100747e8
// FUNCTION: MW2GBL 0x10016e5c
// FUNCTION: MW2PREMERCS 0x1005353f
// FUNCTION: MW2MATROX 0x1001c8f8
// FUNCTION: MW23DFX 0x10023500
// FUNCTION: MW2ATI 0x100237c0
// FUNCTION: MW2S3 0x10037930
// FUNCTION: MW2POWERVR 0x10022a20
void SetCockpitPanelWeapon(CockpitPanel* p_panel, MechS32 p_weapon)
{
	p_panel->m_weapon = p_weapon;
}

// FUNCTION: MW2 0x100747fc
// FUNCTION: MW2GBL 0x10016e70
// FUNCTION: MW2PREMERCS 0x10053553
// FUNCTION: MW2MATROX 0x1001c90c
// FUNCTION: MW23DFX 0x10023510
// FUNCTION: MW2ATI 0x100237d0
// FUNCTION: MW2S3 0x10037940
// FUNCTION: MW2POWERVR 0x10022a30
void SetCockpitPanelName(CockpitPanel* p_panel, const MechChar* p_name)
{
	strncpy(p_panel->m_name, p_name, sizeof(p_panel->m_name));
	p_panel->m_name[sizeof(p_panel->m_name) - 1] = '\0';
}

// FUNCTION: MW2 0x10074823
// FUNCTION: MW2GBL 0x10016e97
// FUNCTION: MW2PREMERCS 0x1005357a
// FUNCTION: MW2MATROX 0x1001c933
// FUNCTION: MW23DFX 0x10023530
// FUNCTION: MW2ATI 0x100237f0
// FUNCTION: MW2S3 0x10037960
// FUNCTION: MW2POWERVR 0x10022a50
void SetCockpitPanelTarget(CockpitPanel* p_panel, PANE* p_target)
{
	p_panel->m_target = p_target;
	p_panel->m_x = p_target->m_x0;
	p_panel->m_y = p_target->m_y0;
	p_panel->m_width = p_target->m_x1 - p_target->m_x0 + 1;
	p_panel->m_height = p_target->m_y1 - p_target->m_y0 + 1;
}

#ifdef MW2_COCKPIT_PANEL_UNK_MEMBERS
// FUNCTION: MW2PREMERCS 0x100535df
void SetCockpitPanelMfdMode(CockpitPanel* p_panel, MechS32 p_mfdMode)
{
	p_panel->m_mfdMode = p_mfdMode;
}
#endif

// FUNCTION: MW2 0x10074879
// FUNCTION: MW2GBL 0x10016eed
// FUNCTION: MW2PREMERCS 0x100535f3
// FUNCTION: MW2MATROX 0x1001c989
// FUNCTION: MW23DFX 0x10023560
// FUNCTION: MW2ATI 0x10023820
// FUNCTION: MW2S3 0x10037990
// FUNCTION: MW2POWERVR 0x10022a80
void SetCockpitPanelTextOrigin(CockpitPanel* p_panel, Point* p_textOrigin)
{
	p_panel->m_textOrigin = p_textOrigin;
}

// FUNCTION: MW2 0x1007488d
// FUNCTION: MW2GBL 0x10016f01
// FUNCTION: MW2PREMERCS 0x1005360a
// FUNCTION: MW2MATROX 0x1001c99d
// FUNCTION: MW23DFX 0x10023570
// FUNCTION: MW2ATI 0x10023830
// FUNCTION: MW2S3 0x100379a0
// FUNCTION: MW2POWERVR 0x10022a90
void SetCockpitPanelTransition(CockpitPanel* p_panel, RectTransition* p_transition)
{
	p_panel->m_transition = p_transition;
}

// FUNCTION: MW2 0x100748a1
// FUNCTION: MW2GBL 0x10016f15
// FUNCTION: MW2PREMERCS 0x10053621
// FUNCTION: MW2MATROX 0x1001c9b1
// FUNCTION: MW23DFX 0x10023580
// FUNCTION: MW2ATI 0x10023840
// FUNCTION: MW2S3 0x100379b0
// FUNCTION: MW2POWERVR 0x10022aa0
void SetCockpitPanelRect(CockpitPanel* p_panel, MechS32 p_x, MechS32 p_y, MechS32 p_width, MechS32 p_height)
{
	p_panel->m_x = p_x;
	p_panel->m_y = p_y;
	p_panel->m_width = p_width;
	p_panel->m_height = p_height;
}

// FUNCTION: MW2 0x100748d4
// FUNCTION: MW2GBL 0x10016f48
// FUNCTION: MW2PREMERCS 0x10053660
// FUNCTION: MW2MATROX 0x1001c9e4
// FUNCTION: MW23DFX 0x100235b0
// FUNCTION: MW2ATI 0x10023870
// FUNCTION: MW2S3 0x100379e0
// FUNCTION: MW2POWERVR 0x10022ad0
void SetCockpitPanelDamage(CockpitPanel* p_panel, MechS32 p_damage)
{
	p_panel->m_damage = p_damage;
}

// FUNCTION: MW2 0x100748e9
// FUNCTION: MW2GBL 0x10016f5d
// FUNCTION: MW2PREMERCS 0x10053675
// FUNCTION: MW2MATROX 0x1001c9f9
// FUNCTION: MW23DFX 0x100235c0
// FUNCTION: MW2ATI 0x10023880
// FUNCTION: MW2S3 0x100379f0
// FUNCTION: MW2POWERVR 0x10022ae0
void EnableCockpitPanel(CockpitPanel* p_panel)
{
	p_panel->m_enabled = 1;
}

// FUNCTION: MW2 0x100748fd
// FUNCTION: MW2GBL 0x10016f71
// FUNCTION: MW2PREMERCS 0x10053689
// FUNCTION: MW2MATROX 0x1001ca0d
// FUNCTION: MW23DFX 0x100235d0
// FUNCTION: MW2ATI 0x10023890
// FUNCTION: MW2S3 0x10037a00
// FUNCTION: MW2POWERVR 0x10022af0
void DisableCockpitPanel(CockpitPanel* p_panel)
{
	p_panel->m_enabled = 0;
}
