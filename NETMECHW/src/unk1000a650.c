#include "unk1000a650.h"

#include "decomp.h"
#include "mw2prj.h"
#include "prjfile.h"
#include "resourcecache.h"
#include "resourcename.h"
#include "types.h"

#include <mbstring.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// The mission texts: NTXT resources whose text ends at "&X". p_name's text lists missions a line
// each, starting with the mission's four-letter code; each mission's name is in <code>NAME and
// its briefing in <code>BRIE. Inline codes: "&1" to "&9" for a tab, "&&" for "&", "&D" ends the
// line without a line break.

// Fills the list box p_listBox with the names of the missions p_name lists, each with its line
// of the list as item data.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000a650
void FUN_1000a650(HWND p_listBox, MechChar* p_name)
{
	MechChar code[8];
	MechChar* text;
	MechChar* line;
	MechChar name[12];
	MechChar* missionName;
	MechS32 length;
	LRESULT index;

	line = LoadMissionText(p_name);
	text = line;
	line = strtok(line, "\n");
	while (line) {
		sprintf(code, "%.4s", line);
		sprintf(name, "%.4sNAME", code);
		missionName = LoadMissionText(name);
		length = strlen(missionName);
		missionName[length - 2] = '\0';

		index = SendMessage(p_listBox, LB_ADDSTRING, 0, (LPARAM) missionName);
		SendMessage(p_listBox, LB_SETITEMDATA, index, (LPARAM) line);
		free(missionName);
		line = strtok(NULL, "\n");
	}
}

// Shows the briefing of the mission p_mission in the edit control p_edit.
// FUNCTION: NETMECHW 0x1000a73b
void FUN_1000a73b(HWND p_edit, MechChar* p_mission)
{
	MechS32 tabStop;
	MechChar name[12];
	MechChar* text;

	tabStop = 60;
	sprintf(name, "%.4sBRIE", p_mission);
	text = LoadMissionText(name);
	FormatMissionText(text);
	SendMessage(p_edit, EM_SETTABSTOPS, 1, (LPARAM) &tabStop);
	SendMessage(p_edit, WM_SETTEXT, 0, (LPARAM) text);
	free(text);
}

// Returns a copy of the NTXT resource p_name, up to its "&X"; the caller frees it.
// The resource type's tag is the original's g_resourceTypeTags[c_resTagNtxt], which has no
// symbol in the original (mw2prj.c), so its three reads score as diffs. The stack slots of the
// locals differ too: VC++ 2.2 permutes them.
// FUNCTION: NETMECHW 0x1000a7b3
MechChar* LoadMissionText(MechChar* p_name)
{
	MechChar* data;
	MechS32 id;
	MechS32 size;
	MechS32 i;
	MechChar* text;

	id = FindResourceIdByName(0x10, p_name);
	size = GetArchiveItemSize(g_mw2PrjHandle, g_resourceTypeTags[c_resTagNtxt], id);
	data = (MechChar*) LoadCachedResource(g_mw2PrjHandle, id, g_resourceTypeTags[c_resTagNtxt], 0);

	for (i = 0; i < size; i++) {
		if (data[i] == '&' && data[i + 1] == 'X') {
			data[i] = '\0';
			break;
		}
	}

	text = (MechChar*) _mbsdup((unsigned char*) data);
	data[i] = '&';
	UnlockCachedResource(id, g_resourceTypeTags[c_resTagNtxt]);
	return text;
}

// Replaces the inline codes of the mission text p_text, in place.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000a893
void FormatMissionText(MechChar* p_text)
{
	MechS32 noBreak;
	MechChar* line;
	MechChar* copy;
	MechChar* code;

	noBreak = FALSE;
	if (!p_text) {
		return;
	}

	line = (MechChar*) _mbsdup((unsigned char*) p_text);
	copy = line;
	*p_text = '\0';
	line = strtok(line, "\n");
	while (line) {
		noBreak = FALSE;
		while (line) {
			code = strchr(line, '&');
			if (code) {
				*code = '\0';
				strcat(p_text, line);
				if (code[1] >= '1' && code[1] <= '9') {
					strcat(p_text, "\t");
					line = code + 2;
				}
				else if (code[1] == '&') {
					strcat(p_text, "&");
					line = code + 2;
				}
				else if (code[1] == 'D') {
					line = NULL;
					noBreak = TRUE;
				}
				else {
					line = code + 2;
				}
			}
			else {
				strcat(p_text, line);
				line = NULL;
			}
		}

		if (!noBreak) {
			strcat(p_text, "\r\n");
		}

		line = strtok(NULL, "\n");
	}

	free(copy);
}
