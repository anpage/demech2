#ifndef DIRECTDRAW_H
#define DIRECTDRAW_H

#include "drawmode.h"
#include "drawmodeextension.h"
#include "types.h"

// The functions and globals of directdraw.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern DrawModeExtension g_unk0x10063230;
	extern DrawMode g_unk0x10063258;
	extern DrawMode g_unk0x10063280;
	extern DrawMode g_unk0x100632a8;
	extern DrawMode g_unk0x100632d0;

#ifdef __cplusplus
}
#endif

#endif // DIRECTDRAW_H
