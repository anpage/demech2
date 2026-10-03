#ifndef BARGAUGES_H
#define BARGAUGES_H

#include "targeting.h"
#include "types.h"

// The functions and globals of bargauges.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_1004d020(void);
	void FUN_1004d175(PANE* p_target);
	void FUN_1004d310(PANE* p_target);
	void FUN_1004d48a(PANE* p_target);
	void FUN_1004d660(PANE* p_target);
	void FUN_1004d732(PANE* p_target, MechS32 p_x, MechS32 p_y, MechS32 p_width, MechS32 p_height, MechS32 p_color);
	void FUN_1004d8ae(PANE* p_target, MechS32 p_x, MechS32 p_y, MechS32 p_width, MechS32 p_height, MechS32 p_color);

#ifdef __cplusplus
}
#endif

#endif // BARGAUGES_H
