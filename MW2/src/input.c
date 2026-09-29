#include "input.h"

#include "decomp.h"
#include "inputdeviceinfo.h"
#include "inputdriver.h"
#include "types.h"

#include <windows.h>

// GLOBAL: MW2 0x100b2514
MechS16 g_keyCode = 0;

// The outputs of the menu bindings (INPUT.MAP's menu_ sinks).

// GLOBAL: MW2 0x100b2588
MechS32 g_sinkMenuItem = 0;

// GLOBAL: MW2 0x100b258c
MechS32 g_sinkMenuValue = 0;

// GLOBAL: MW2 0x100b2592
MechS8 g_sinkMenuItemReset = 0;

// GLOBAL: MW2 0x100b2595
MechS8 g_sinkMenuValueReset = 0;

// GLOBAL: MW2 0x100b2596
MechS8 g_sinkMenuEnter = 0;

// GLOBAL: MW2 0x100b2597
MechS8 g_sinkMenuAbort = 0;

// GLOBAL: MW2 0x100b25a4
MechS32 g_gameplayInputEnabled = 1;

// GLOBAL: MW2 0x100b25a8
MechS32 g_keyboardDeviceIndex = 0;

// GLOBAL: MW2 0x100b7370
MechS32 g_inputDeviceCount = 0;

// The game key action of each key code (0: none), loaded from GAMEKEY.MAP.
// GLOBAL: MW2 0x100b8190
MechU8 g_gameKeyByKeyCode[0x800] = {0};

// GLOBAL: MW2 0x100bf5d0
MechS32 g_inputDevicePresent[5]; // length unknown

// GLOBAL: MW2 0x100bf5e8
InputDeviceInfo g_inputDeviceInfos[5]; // length unknown

// GLOBAL: MW2 0x100bfa88
InputDriverModule* g_inputDrivers[5]; // length unknown

// STUB: MW2 0x1007a97a
MechS32 LoadInputMap(void)
{
	STUB(0x1007a97a);
	return 0;
}

// STUB: MW2 0x1007aba9
MechS32 LoadGamekeyMap(void)
{
	STUB(0x1007aba9);
	return 0;
}

// Loads INPUT.MAP and GAMEKEY.MAP and flushes the keyboard's key codes.
// FUNCTION: MW2 0x1007b177
void FirstInputs(void)
{
	LoadInputMap();
	LoadGamekeyMap();
	g_inputDrivers[g_keyboardDeviceIndex]->m_flushKeyCodes();
}

// STUB: MW2 0x1007b19b
void UpdateInputs(void)
{
	STUB(0x1007b19b);
}

// FUNCTION: MW2 0x1007b704
void CloseInputDevices(void)
{
	MechS32 i;

	i = g_inputDeviceCount;
	while (i--) {
		if (g_inputDevicePresent[i]) {
			g_inputDrivers[i]->m_closeDevice(&g_inputDeviceInfos[i]);
		}
	}
}

// FUNCTION: MW2 0x1007b768
void DisableGameplayInput(void)
{
	g_gameplayInputEnabled = FALSE;
}

// FUNCTION: MW2 0x1007b77d
void EnableGameplayInput(void)
{
	g_gameplayInputEnabled = TRUE;
}

// The game key action of p_keyCode.
// FUNCTION: MW2 0x1007b792
MechS16 LookupGameKey(MechS16 p_keyCode)
{
	return g_gameKeyByKeyCode[p_keyCode];
}
