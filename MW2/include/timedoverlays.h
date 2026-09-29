#ifndef TIMEDOVERLAYS_H
#define TIMEDOVERLAYS_H

#include "types.h"

// The functions and globals of timedoverlays.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_1006ee60(void);
	MechS32 ShowInGameMessage(MechChar* p_text, MechS32 p_font, MechS32 p_duration, MechS32 p_priority);
	void DrawTimedOverlays(void);
	void FUN_1006f28f(MechS32 p_background, MechS32 p_font, MechChar* p_text, MechS32 p_x, MechS32 p_y);
	void FUN_1006f3ea(MechS32 p_background, MechS32 p_font, MechS32 p_id, MechS32 p_x, MechS32 p_y);

#ifdef __cplusplus
}
#endif

#endif // TIMEDOVERLAYS_H
