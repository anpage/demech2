#ifndef TARGETPANEL_H
#define TARGETPANEL_H

#include "types.h"

struct CockpitPanel;

// The functions and globals of targetpanel.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_targetPanelMode;
	extern MechChar g_anonymousInstallationName[16];

	void DrawTargetPanelText(struct CockpitPanel* p_panel);
#ifdef MW2_POWERVR
	void FUN_10020450(struct CockpitPanel* p_panel);
	void FUN_100206e0(struct CockpitPanel* p_panel);
#endif
	void DrawTargetPanel(struct CockpitPanel* p_panel);
	void DrawTargetStatic(struct CockpitPanel* p_panel);
	void DrawTargetPanelStartup(struct CockpitPanel* p_panel);
	void DrawTargetPanelShutdown(struct CockpitPanel* p_panel);

#ifdef __cplusplus
}
#endif

#endif // TARGETPANEL_H
