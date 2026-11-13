#ifndef ACCEL_REGISTRY_H
#define ACCEL_REGISTRY_H

#include "types.h"

#include <windows.h>

// The functions of accel/registry.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechS32 ReadRegistryDword(LPCSTR p_name, DWORD* p_value);
	MechS32 WriteRegistryDword(LPCSTR p_name, DWORD* p_value);

#ifdef __cplusplus
}
#endif

#endif // ACCEL_REGISTRY_H
