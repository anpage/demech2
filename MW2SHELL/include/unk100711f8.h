#ifndef UNK100711F8_H
#define UNK100711F8_H

#include "audiosubsystem.h"
#include "brasslantern0x414.h"
#include "cedarknot0x10.h"
#include "chimeledger0x3c.h"
#include "decomp.h"
#include "hollowreed0x110.h"
#include "mousestate.h"
#include "palettecolor.h"
#include "tinwhistle0x3c.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "videodriver.h"

// The functions and globals of unk100711f8.cpp that other units use.
extern HollowReed0x110* g_unk0x100711f8;
extern AudioSubsystem* g_pAudioSubsystem;
extern void* g_unk0x10071200;
extern MouseState* g_pMouseState;
extern VideoDriver* g_pVideoDriver;
extern BrassLantern0x414* g_unk0x1007120c;
extern BrassLantern0x414* g_unk0x10071210;
extern BrassLantern0x414* g_unk0x10071214;
extern BrassLantern0x414* g_unk0x10071218;
extern BrassLantern0x414* g_unk0x1007121c;
extern BrassLantern0x414* g_unk0x10071220;
extern BrassLantern0x414* g_unk0x10071224;
extern BrassLantern0x414* g_unk0x10071228;
extern TMPackDataBase* g_pDatabaseMw2;
extern CedarKnot0x10* g_unk0x10071230;
extern MechS32 g_fAudio;
extern MechS32 g_fDigitalAudio;
extern MechS32 g_unk0x1007123c;
extern MechS32 g_unk0x10071240;
extern MechU32 g_unk0x10071248;
extern MechU8 g_fDrawFmv;
extern MechChar g_szDataDrivePath[4];
extern MechChar* g_rankNames[10];
extern MechChar* g_unk0x10071280[6];
extern MechS32 g_unk0x10071298[18];
extern MechS32 g_unk0x100712e0[18];
extern MechS32 g_unk0x10071328[18];
extern TinWhistle0x3c* g_pCurrentPilot;
extern MechS32 g_unk0x10071374;
extern PaletteColor g_unk0x10071378[0x100];
extern ChimeLedger0x3c g_soundConfig;
extern undefined g_unk0x100716b8[0x17];
extern TinWhistle0x3c g_pilotRoster[20];

#endif // UNK100711F8_H
