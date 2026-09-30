#ifndef ERROR_H
#define ERROR_H

#include "types.h"

// The functions and globals of error.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void Error(MechS32 p_code, const char* p_format, ...);
	void FUN_1003ba07(void);
	void FUN_1003ba3a(const char** p_args);
	MechChar* FUN_1003bad1(MechChar* p_title, MechS32 p_code, const char** p_args);
	void FUN_1003bbb1(const char** p_args);

#ifdef __cplusplus
}
#endif

#endif // ERROR_H
