#ifndef INPUTMAP_H
#define INPUTMAP_H

#include "playersteering.h"
#include "types.h"

// The functions and globals of inputmap.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern PlayerSteering g_localSteering;
	extern MechS32 g_sinkPilotTilt;
	extern MechS32 g_sinkPilotPan;
	extern MechS32 g_sinkMenuItem;
	extern MechS32 g_sinkMenuValue;
	extern MechS8 g_sinkMenuItemReset;
	extern MechS8 g_sinkMenuValueReset;
	extern MechS8 g_sinkMenuEnter;
	extern MechS8 g_sinkMenuAbort;

	MechS32 RegisterInputDevice(MechChar* p_name);
	MechS32 FindInputDevice(MechChar* p_name);
	MechS32 FindInputAxis(MechS32 p_device, MechChar* p_name);
	MechS32 FindInputButton(MechS32 p_device, MechChar* p_name);
	MechS32 LoadInputMap(void);
	MechS32 LoadGamekeyMap(void);
	void FirstInputs(void);
	void UpdateInputs(void);
	void CloseInputDevices(void);
	void DisableGameplayInput(void);
	void EnableGameplayInput(void);
	MechS16 LookupGameKey(MechS16 p_keyCode);
	void FUN_1007b7b1(MechS32 p_code, MechChar* p_channel, MechChar* p_device);

#ifdef __cplusplus
}
#endif

#endif // INPUTMAP_H
