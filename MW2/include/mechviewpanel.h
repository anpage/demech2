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

	extern MechS32 g_mechViewStatic;

	void CycleMechViewMode(void);
	void DrawMechViewPanel(CockpitPanel* p_panel);
	void SetMechViewRenderSettings(RenderSettings* p_saved);
	void DrawMechViewStatic(CockpitPanel* p_panel);
	void DrawMechViewFrame(CockpitPanel* p_panel, MechS32 p_color, MechS32 p_shapeId);
	void DrawMechViewStartup(CockpitPanel* p_panel);
	void DrawMechViewShutdown(CockpitPanel* p_panel);
#ifdef MW2_POWERVR
	// The PowerVR edition's handlers before and after the scene is rendered (CockpitPanel::m_unk0x90
	// and m_unk0x94), which place the inset's camera and set its qualities.
	void FUN_10010b10(CockpitPanel* p_panel);
	void FUN_10010d50(CockpitPanel* p_panel);
#endif

#ifdef __cplusplus
}
#endif

#endif // MECHVIEWPANEL_H
