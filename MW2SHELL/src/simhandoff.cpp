#include "simhandoff.h"

#include "decomp.h"
#include "mechvariant.h"
#include "pilotrecord.h"
#include "refreshmode.h"
#include "shellglobals.h"
#include "simhandoffstate.h"
#include "types.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(SimHandoffState, 0x218)

// The mission's name, from its briefing file.
// GLOBAL: MW2SHELL 0x1006a550
MechChar g_unk0x1006a550[0x10] = "xxxxxxxx.xxx";

// GLOBAL: MW2SHELL 0x10090288
SimHandoffState g_unk0x10090288;

// Reads the shell's state back from mw2prm.cfg after a mission. With p_fromSim, posts the saved
// message to the shell window; otherwise it returns to the campaign's start (2, no pilot, no
// scenario).
// FUNCTION: MW2SHELL 0x10039b50
void ReadSimHandoff(BOOL p_fromSim, MechS32* p_campaign, MechU8* p_pilotChosen, char** p_scenario)
{
	MechS32 i;
	FILE* file;

	file = fopen("mw2prm.cfg", "rb");
	if (!file) {
		return;
	}

	if (fread(&g_unk0x10090288, sizeof(g_unk0x10090288), 1, file) != 1) {
		fclose(file);
		return;
	}
	fclose(file);

	*p_campaign = g_unk0x10090288.m_unk0x04;
	*p_pilotChosen = g_unk0x10090288.m_unk0x08;
	*p_scenario = g_unk0x10090288.m_unk0x118;
	for (i = 0; g_unk0x10090288.m_unk0x118[i] > ' '; i++) {
	}
	g_unk0x10090288.m_unk0x118[i] = '\0';

	if (g_unk0x10090288.m_unk0x114 >= 0) {
		g_pCurrentPilot = &g_pilotRoster[g_unk0x10090288.m_unk0x114];
	}
	else {
		g_pCurrentPilot = NULL;
	}
	RestoreStars();

	if (p_fromSim) {
		PostMessage(g_pWnd, g_unk0x10090288.m_unk0x00, 0x410, 0);
	}
	else {
		*p_campaign = 2;
		*p_pilotChosen = 0;
		*p_scenario = NULL;
	}
}

// Saves the shell's state to mw2prm.cfg before a mission: the message to post on return, the
// campaign, the pilot, and the simulator's command line (the scenario and "-b=" the mission's
// name).
// FUNCTION: MW2SHELL 0x10039c92
void WriteSimHandoff(UINT p_msg, MechS32 p_campaign, MechU8 p_pilotChosen, const char* p_scenario)
{
	FILE* file;

	g_unk0x10090288.m_unk0x00 = p_msg;
	g_unk0x10090288.m_unk0x04 = p_campaign;
	g_unk0x10090288.m_unk0x08 = p_pilotChosen;
	strcpy(g_unk0x10090288.m_unk0x118, p_scenario);
	strcat(g_unk0x10090288.m_unk0x118, " -b=");
	strcat(g_unk0x10090288.m_unk0x118, g_unk0x1006a550);

	if (g_pCurrentPilot) {
		g_unk0x10090288.m_unk0x114 = g_pCurrentPilot - g_pilotRoster;
	}
	else {
		g_unk0x10090288.m_unk0x114 = -1;
	}
	SaveStars();

	if (p_msg != 0x414 && p_msg != 0x402) {
		WriteStarFiles();
	}

	file = fopen("mw2prm.cfg", "wb");
	if (!file) {
		return;
	}

	fwrite(&g_unk0x10090288, sizeof(g_unk0x10090288), 1, file);
	fclose(file);
}
