#include "decomp.h"
#include "types.h"

#include <stdio.h>
#include <string.h>

// Optional base directory for resource names.
// GLOBAL: MW2SHELL 0x10067708
MechChar g_unk0x10067708[0x100] = {0};

// Shared scratch buffer returned by the path helpers (overwritten on each call).
// GLOBAL: MW2SHELL 0x1008ff58
MechChar g_unk0x1008ff58[0x50];

// INPUT.MAP stream shared by the input-map writers.
// GLOBAL: MW2SHELL 0x10067840
FILE* g_unk0x10067840 = NULL;

// Prefix a bare name with the base directory; leave paths containing a separator unchanged.
// FUNCTION: MW2SHELL 0x10031a8f
MechChar* FUN_10031a8f(MechChar* p_name)
{
	MechS32 i;

	for (i = 0; i < 0x50; i++) {
		g_unk0x1008ff58[i] = '\0';
	}

	if (g_unk0x10067708[0] != '\0' && strchr(p_name, '\\') == NULL && strchr(p_name, '/') == NULL) {
		sprintf(g_unk0x1008ff58, "%s\\%s", g_unk0x10067708, p_name);
	}
	else {
		strcpy(g_unk0x1008ff58, p_name);
	}

	return g_unk0x1008ff58;
}

// Prefix a name with the base directory without checking for path separators.
// FUNCTION: MW2SHELL 0x10031b51
MechChar* FUN_10031b51(MechChar* p_name)
{
	MechS32 i;

	for (i = 0; i < 0x50; i++) {
		g_unk0x1008ff58[i] = '\0';
	}

	if (g_unk0x10067708[0] != '\0') {
		sprintf(g_unk0x1008ff58, "%s\\%s", g_unk0x10067708, p_name);
	}
	else {
		strcpy(g_unk0x1008ff58, p_name);
	}

	return g_unk0x1008ff58;
}

// FUNCTION: MW2SHELL 0x10031e32
MechS32 FUN_10031e32(void)
{
	return 5;
}

// FUNCTION: MW2SHELL 0x10031e7f
MechS32 FUN_10031e7f(void)
{
	return 0x35;
}

// FUNCTION: MW2SHELL 0x10031ece
void FUN_10031ece(void)
{
	if (g_unk0x10067840 != NULL) {
		return;
	}

	g_unk0x10067840 = fopen("INPUT.MAP", "w");
}

// FUNCTION: MW2SHELL 0x10032033
void FUN_10032033(void)
{
	if (g_unk0x10067840 == NULL) {
		return;
	}

	fclose(g_unk0x10067840);
	g_unk0x10067840 = NULL;
}
