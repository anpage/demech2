#ifndef ERROR_H
#define ERROR_H

#include "types.h"

// The functions and globals of error.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void Error(MechS32 p_unk0x00, const char* p_unk0x04, ...);

#ifdef __cplusplus
}
#endif

#endif // ERROR_H
