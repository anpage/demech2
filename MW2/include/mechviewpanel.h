#ifndef MECHVIEWPANEL_H
#define MECHVIEWPANEL_H

#include "cockpitpanel.h"
#include "rendersettings.h"
#include "types.h"

// The functions and globals of mechviewpanel.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100a88e8;

	void FUN_100509a0(void);
	void DrawMechViewPanel(CockpitPanel* p_panel);
	void FUN_10050dc3(RenderSettings* p_saved);
	void DrawMechViewStatic(CockpitPanel* p_panel);
	void FUN_10050e6c(CockpitPanel* p_panel, MechS32 p_color, MechS32 p_unk0x08);
	void DrawMechViewStartup(CockpitPanel* p_panel);
	void DrawMechViewShutdown(CockpitPanel* p_panel);

#ifdef __cplusplus
}
#endif

#endif // MECHVIEWPANEL_H
