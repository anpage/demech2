#include "decomp.h"
#include "tmpackdatabase.h"
#include "types.h"

#include <stdio.h>

// Copies p_src to p_dst with every run of spaces and control characters turned into one space.
// FUNCTION: MW2SHELL 0x10001c28
void FUN_10001c28(MechChar* p_dst, MechChar* p_src)
{
	MechS32 space = FALSE;

	while (*p_src) {
		if (*p_src <= ' ') {
			if (!space) {
				*p_dst++ = ' ';
				space = TRUE;
			}
		}
		else {
			space = FALSE;
			*p_dst++ = *p_src;
		}
		p_src++;
	}
	*p_dst = '\0';
}

// Reads the simulator's mission results.
// FUNCTION: MW2SHELL 0x100021a6
void ReadMissionResults(void* p_results)
{
	FILE* file = NULL;

	file = fopen("MW2MSN.CFG", "rb");
	if (file == NULL) {
		return;
	}

	fread(p_results, 0x9d4, 1, file);
	fclose(file);
}

// STUB: MW2SHELL 0x100024a3
void DrawMissionDebrief(TMPackDataBase* p_database, MechS32 p_campaign, char** p_scenario)
{
	STUB(0x100024a3);
}
