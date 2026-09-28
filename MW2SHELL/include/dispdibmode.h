#ifndef DISPDIBMODE_H
#define DISPDIBMODE_H

#include "drawmode.h"
#include "drawmodeextension.h"
#include "types.h"

// The functions and globals of dispdib.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern DrawModeExtension g_dispDibDrawModeExtension;
	extern DrawMode g_dispDibDrawMode;

#ifdef __cplusplus
}
#endif

#endif // DISPDIBMODE_H
