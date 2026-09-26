#include "decomp.h"
#include "types.h"

#include <mbstring.h>
#include <stdlib.h>

void FUN_10013907(void);
void* FUN_10013fa9(undefined4 p_size);
void FUN_1002fb90(void* (*p_alloc)(undefined4), void (*p_free)(void*));
MechS32 FUN_1002fcdc(char* p_name, undefined p_unk0x04);
MechS32 FUN_1002ffc9(MechS32 p_handle);
MechS32 FUN_1003024f(MechS32 p_handle);
char* FUN_10031a8f(char* p_name);
void* FUN_1003c061(undefined4 p_size);
void FUN_1003c07d(void* p_mem);

// GLOBAL: MW2SHELL 0x1006aac4
MechS32 g_unk0x1006aac4 = -1;

// GLOBAL: MW2SHELL 0x1006aac8
char* g_unk0x1006aac8 = NULL;

// FUNCTION: MW2SHELL 0x1003bfb0
MechS32 FUN_1003bfb0(void)
{
	MechS32 result;

	result = TRUE;
	FUN_1002fb90(FUN_1003c061, FUN_1003c07d);
	FUN_10013907();

	if (g_unk0x1006aac8 == NULL) {
		g_unk0x1006aac8 = (char*) _mbsdup((unsigned char*) FUN_10031a8f("mw2.prj"));
	}

	g_unk0x1006aac4 = FUN_1002fcdc(g_unk0x1006aac8, 0);
	if (g_unk0x1006aac4 != -1) {
		FUN_1003024f(g_unk0x1006aac4);
	}
	else {
		result = FALSE;
	}

	return result;
}

// FUNCTION: MW2SHELL 0x1003c048
void FUN_1003c048(void)
{
	FUN_1002ffc9(g_unk0x1006aac4);
}

// FUNCTION: MW2SHELL 0x1003c061
void* FUN_1003c061(undefined4 p_size)
{
	return FUN_10013fa9(p_size);
}

// FUNCTION: MW2SHELL 0x1003c07d
void FUN_1003c07d(void* p_mem)
{
	free(p_mem);
}
