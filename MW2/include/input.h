#ifndef INPUT_H
#define INPUT_H

#include "types.h"

// The functions and globals of input.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS16 g_keyCode;

	void FirstInputs(void);
	void UpdateInputs(void);
	void CloseInputDevices(void);
	void DisableGameplayInput(void);
	void EnableGameplayInput(void);

#ifdef __cplusplus
}
#endif

#endif // INPUT_H
