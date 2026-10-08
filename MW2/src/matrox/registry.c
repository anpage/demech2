/* The Matrox edition's registry settings: DWORD values under the game's key in
   HKEY_LOCAL_MACHINE. One object of its own, after quadtree.c's. */
#include "matrox/registry.h"

#include "debugprint.h"
#include "decomp.h"
#include "types.h"

#include <windows.h>

// Reads the DWORD value p_name into p_value (0 when the value is missing). Returns whether the
// value was read. Stack-slot permutation: result, key, value and type.
// FUNCTION: MW2MATROX 0x100169e0
MechS32 ReadRegistryDword(LPCSTR p_name, DWORD* p_value)
{
	LONG result;
	HKEY key;
	DWORD value;
	DWORD type;
	DWORD size;

	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Activision\\MechWarrior 2\\1.0", 0, KEY_QUERY_VALUE, &key);
	if (result != ERROR_SUCCESS) {
		DebugPrint("Could not open MechWarrior 2 registry key: %d\n", result);
		return FALSE;
	}

	size = sizeof(DWORD);
	type = REG_DWORD;
	result = RegQueryValueEx(key, p_name, NULL, &type, (LPBYTE) &value, &size);
	if (result == ERROR_SUCCESS) {
		*p_value = value;
	}
	else {
		*p_value = 0;
	}

	RegCloseKey(key);
	return result == ERROR_SUCCESS;
}

// Writes p_value to the DWORD value p_name, creating the key if needed. Returns whether it was
// written. Stack-slot permutation: result and key.
// FUNCTION: MW2MATROX 0x10016a9d
MechS32 WriteRegistryDword(LPCSTR p_name, DWORD* p_value)
{
	LONG result;
	HKEY key;
	DWORD disposition;

	result = RegCreateKeyEx(
		HKEY_LOCAL_MACHINE,
		"Software\\Activision\\MechWarrior 2\\1.0",
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&key,
		&disposition
	);
	if (result != ERROR_SUCCESS) {
		DebugPrint("Could not open registry MechWarrior2 key: %d\n", result);
		return FALSE;
	}

	result = RegSetValueEx(key, p_name, 0, REG_DWORD, (BYTE*) p_value, sizeof(DWORD));
	if (result != ERROR_SUCCESS) {
		DebugPrint("Could not save '%s' registry MechWarrior2 key: %d\n", p_name, result);
		return FALSE;
	}

	RegCloseKey(key);
	return result == ERROR_SUCCESS;
}
