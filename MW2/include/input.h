#ifndef INPUT_H
#define INPUT_H

#include "types.h"

// The functions and globals of input.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS16 g_keyCode;
	extern MechS32 g_sinkMenuItem;
	extern MechS32 g_sinkMenuValue;
	extern MechS8 g_sinkMenuItemReset;
	extern MechS8 g_sinkMenuValueReset;
	extern MechS8 g_sinkMenuEnter;
	extern MechS8 g_sinkMenuAbort;

	MechS32 LoadInputMap(void);
	MechS32 LoadGamekeyMap(void);
	void FirstInputs(void);
	void UpdateInputs(void);
	void CloseInputDevices(void);
	void DisableGameplayInput(void);
	void EnableGameplayInput(void);
	MechS16 LookupGameKey(MechS16 p_keyCode);

#ifdef __cplusplus
}
#endif

#endif // INPUT_H
