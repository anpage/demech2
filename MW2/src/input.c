#include "input.h"

#include "decomp.h"
#include "types.h"

#include <windows.h>

// GLOBAL: MW2 0x100b2514
MechS16 g_keyCode = 0;

// GLOBAL: MW2 0x100b25a4
MechS32 g_gameplayInputEnabled = 1;

// STUB: MW2 0x1007b177
void FirstInputs(void)
{
	STUB(0x1007b177);
}

// STUB: MW2 0x1007b19b
void UpdateInputs(void)
{
	STUB(0x1007b19b);
}

// STUB: MW2 0x1007b704
void CloseInputDevices(void)
{
	STUB(0x1007b704);
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
