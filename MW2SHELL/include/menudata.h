#ifndef MENUDATA_H
#define MENUDATA_H

#include "campaignmission.h"
#include "formation.h"
#include "mainmenubutton.h"
#include "menuscreen.h"
#include "types.h"

// The functions and globals of menudata.cpp that other units use.
extern MechS32 g_textTabStops[19];
extern char* g_unk0x1006e19c;
extern MechChar* g_unk0x1006e1a0[2];
extern Formation g_unk0x1006e1a8[6];
extern MainMenuButton g_mainMenuButtons[4];
extern MainMenuButton g_unk0x1006f618[0x19];
extern CampaignMission* g_campaignMissions[2];
extern MechChar** g_unk0x1006fe08[2];
extern MenuScreen g_unk0x1006fe10[3];
extern MenuScreen g_unk0x1006fe40[3];
extern MenuScreen g_unk0x1006fe70[3];
extern MenuScreen g_unk0x1006fea0[3];
extern MenuScreen g_unk0x1006fed0[3];
extern MenuScreen g_unk0x1006ff00[3];
extern MenuScreen g_unk0x1006ff30[3];
extern MenuScreen g_unk0x1006ff60[3];
extern MenuScreen g_unk0x1006ff90[3];
extern MenuScreen g_unk0x1006ffc0[3];
extern MenuScreen g_unk0x1006fff0[3];

#endif // MENUDATA_H
