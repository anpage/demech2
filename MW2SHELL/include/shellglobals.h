#ifndef SHELLGLOBALS_H
#define SHELLGLOBALS_H

#include "audiosubsystem.h"
#include "decomp.h"
#include "font.h"
#include "keyboardinput.h"
#include "mousestate.h"
#include "palettecolor.h"
#include "pilotrecord.h"
#include "projectarchive.h"
#include "soundconfig.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "videodriver.h"

// The functions and globals of shellglobals.cpp that other units use.
extern KeyboardInput* g_keyboardInput;
extern AudioSubsystem* g_pAudioSubsystem;
extern void* g_unk0x10071200;
extern MouseState* g_pMouseState;
extern VideoDriver* g_pVideoDriver;
extern Font* g_defaultFont;
extern Font* g_textFont;
extern Font* g_titleFont;
extern Font* g_buttonFont;
extern Font* g_unk0x1007121c;
extern Font* g_unk0x10071220;
extern Font* g_archiveFont;
extern Font* g_bodyFont;
extern TMPackDataBase* g_pDatabaseMw2;
extern ProjectArchive* g_projectArchive;
extern MechS32 g_fAudio;
extern MechS32 g_fDigitalAudio;
extern MechS32 g_unk0x1007123c;
extern MechS32 g_unk0x10071240;
extern MechU32 g_unk0x10071248;
extern MechU8 g_fDrawFmv;
extern MechChar g_szDataDrivePath[4];
extern MechChar* g_rankNames[10];
extern MechChar* g_clanNames[6];
extern MechS32 g_unk0x10071298[18];
extern MechS32 g_unk0x100712e0[18];
extern MechS32 g_unk0x10071328[18];
extern PilotRecord* g_pCurrentPilot;
extern MechS32 g_unk0x10071374;
extern PaletteColor g_unk0x10071378[0x100];
extern SoundConfig g_soundConfig;
extern undefined g_unk0x100716b8[0x17];
extern PilotRecord g_pilotRoster[20];

#endif // SHELLGLOBALS_H
