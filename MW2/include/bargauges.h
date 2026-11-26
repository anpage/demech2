#ifndef BARGAUGES_H
#define BARGAUGES_H

#include "targeting.h"
#include "types.h"

#ifdef MW2_MFD_PANELS
struct CockpitPanel;
#endif

// The functions and globals of bargauges.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void ScaleBarGauges(void);
#ifdef MW2_MFD_PANELS
	void ScaleHeatBar(struct CockpitPanel* p_panel);
	void ScaleHeatRateBar(struct CockpitPanel* p_panel);
	void ScaleThrottleGauge(struct CockpitPanel* p_panel);
	void ScaleJumpFuelBar(struct CockpitPanel* p_panel);
	void DrawDialTick(PANE* p_pane, MechScalar p_angle, MechS32 p_color, PANE* p_inner);
	void DrawHeatBar(struct CockpitPanel* p_panel);
	void DrawHeatRateBar(struct CockpitPanel* p_panel);
	void DrawThrottleGauge(struct CockpitPanel* p_panel);
	void DrawJumpFuelBar(struct CockpitPanel* p_panel);
#else
void DrawHeatBar(PANE* p_target);
void DrawHeatRateBar(PANE* p_target);
void DrawThrottleGauge(PANE* p_target);
void DrawJumpFuelBar(PANE* p_target);
#endif
	void DrawVerticalBar(PANE* p_target, MechS32 p_x, MechS32 p_y, MechS32 p_width, MechS32 p_height, MechS32 p_color);
	void DrawHorizontalBar(
		PANE* p_target,
		MechS32 p_x,
		MechS32 p_y,
		MechS32 p_width,
		MechS32 p_height,
		MechS32 p_color
	);

#ifdef __cplusplus
}
#endif

#endif // BARGAUGES_H
