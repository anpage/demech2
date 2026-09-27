#include "decomp.h"
#include "types.h"

#include <stdlib.h>

// GLOBAL: MW2SHELL 0x10063a54
undefined4* g_unk0x10063a54 = NULL;

// GLOBAL: MW2SHELL 0x10096860
undefined4 g_unk0x10096860;

// GLOBAL: MW2SHELL 0x10096864
undefined4 g_unk0x10096864;

// GLOBAL: MW2SHELL 0x10096868
undefined4 g_unk0x10096868;

// FUNCTION: MW2SHELL 0x100137aa
void FUN_100137aa(void)
{
	g_unk0x10063a54 = (undefined4*) calloc(0x3f1, 4);
}

// STUB: MW2SHELL 0x1001385c
void FUN_1001385c(void)
{
	STUB(0x1001385c);
}

// FUNCTION: MW2SHELL 0x10013907
void FUN_10013907(void)
{
	g_unk0x10096860 = 0;
	g_unk0x10096864 = 0;
	g_unk0x10096868 = 0;
	FUN_100137aa();
}

// FUNCTION: MW2SHELL 0x10013fa9
void* FUN_10013fa9(undefined4 p_size)
{
	return calloc(p_size, 1);
}
