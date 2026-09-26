#include "decomp.h"
#include "tinwhistle0x3c.h"
#include "tmpackdatabase.h"
#include "types.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

extern "C" void FUN_10015c90(const MechChar* p_format, ...);

// GLOBAL: MW2SHELL 0x100946d0
TinWhistle0x3c g_pilotRoster[20];

// STUB: MW2SHELL 0x10029010
void FUN_10029010(TMPackDataBase* p_database, MechS32 p_campaign, WPARAM p_wParam)
{
	STUB(0x10029010);
}

// Stack-slot permutation: file and i swap [ebp-N] slots with the original.
// FUNCTION: MW2SHELL 0x1002da80
void LoadPilotRoster()
{
	TinWhistle0x3c* pilot;
	FILE* file;
	MechS32 i;

	file = fopen("MW2REG.CFG", "rb");
	if (file == NULL) {
		for (i = 0; i < 20; i++) {
			pilot = &g_pilotRoster[i];
			pilot->m_unk0x00 = 0;
			pilot->m_unk0x04 = 0;
			if (i >= 10) {
				pilot->m_unk0x08 = 1;
			}
			else {
				pilot->m_unk0x08 = 0;
			}
			pilot->m_unk0x0c = 0;
			pilot->m_unk0x10 = 0;
			pilot->m_unk0x14 = 0;
			pilot->m_unk0x18 = 0;
			pilot->m_unk0x1c = 0;
			pilot->m_unk0x20 = 0;
			pilot->m_unk0x24 = 0;
			strcpy(pilot->m_unk0x28, "");
		}
	}
	else {
		fread(g_pilotRoster, 0x3c, 20, file);
		fclose(file);
	}

	for (i = 0; i < 20; i++) {
		pilot = &g_pilotRoster[i];
		pilot->m_unk0x38 = 0;
	}
}

// FUNCTION: MW2SHELL 0x1002dbec
void SavePilotRoster()
{
	FILE* file;

	file = fopen("MW2REG.CFG", "wb");
	if (file == NULL) {
		FUN_10015c90("Error Writing Career File\n");
		return;
	}

	fwrite(g_pilotRoster, 0x3c, 20, file);
	fclose(file);
}
