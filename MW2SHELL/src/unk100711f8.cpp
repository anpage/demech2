#include "unk100711f8.h"

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

// The shell's global state. The original links this data as an object of its own (or as
// glassbanner0x35c.cpp's data, which has none otherwise), between page.cpp and unk100460a0.c:
// its globals, then the rank and clan names they point to; the pilot roster is its .bss.

// GLOBAL: MW2SHELL 0x100711f8
HollowReed0x110* g_unk0x100711f8 = NULL;

// GLOBAL: MW2SHELL 0x100711fc
AudioSubsystem* g_pAudioSubsystem = NULL;

// GLOBAL: MW2SHELL 0x10071200
void* g_unk0x10071200 = NULL;

// GLOBAL: MW2SHELL 0x10071204
MouseState* g_pMouseState = NULL;

// GLOBAL: MW2SHELL 0x10071208
VideoDriver* g_pVideoDriver = NULL;

// GLOBAL: MW2SHELL 0x1007120c
BrassLantern0x414* g_unk0x1007120c = NULL;

// GLOBAL: MW2SHELL 0x10071210
BrassLantern0x414* g_unk0x10071210 = NULL;

// GLOBAL: MW2SHELL 0x10071214
BrassLantern0x414* g_unk0x10071214 = NULL;

// GLOBAL: MW2SHELL 0x10071218
BrassLantern0x414* g_unk0x10071218 = NULL;

// GLOBAL: MW2SHELL 0x1007121c
BrassLantern0x414* g_unk0x1007121c = NULL;

// GLOBAL: MW2SHELL 0x10071220
BrassLantern0x414* g_unk0x10071220 = NULL;

// GLOBAL: MW2SHELL 0x10071224
BrassLantern0x414* g_unk0x10071224 = NULL;

// GLOBAL: MW2SHELL 0x10071228
BrassLantern0x414* g_unk0x10071228 = NULL;

// GLOBAL: MW2SHELL 0x1007122c
TMPackDataBase* g_pDatabaseMw2 = NULL;

// GLOBAL: MW2SHELL 0x10071230
CedarKnot0x10* g_unk0x10071230 = NULL;

// GLOBAL: MW2SHELL 0x10071234
MechS32 g_fAudio = 1;

// GLOBAL: MW2SHELL 0x10071238
MechS32 g_fDigitalAudio = 1;

// GLOBAL: MW2SHELL 0x1007123c
MechS32 g_unk0x1007123c = 1;

// GLOBAL: MW2SHELL 0x10071240
MechS32 g_unk0x10071240 = 1;

// The flags the shell opens Smacker movies with.
// GLOBAL: MW2SHELL 0x10071248
MechU32 g_unk0x10071248 = 0;

// GLOBAL: MW2SHELL 0x1007124c
MechU8 g_fDrawFmv = 0;

// GLOBAL: MW2SHELL 0x10071250
MechChar g_szDataDrivePath[4] = "A:\\";

// The pilot roster clamps rank + 1 and rank + 2 to index 9: the NULL after Khan.
// GLOBAL: MW2SHELL 0x10071258
MechChar* g_rankNames[10] = {
	"Mechwarrior",
	"Star Commander",
	"Nova Commander",
	"Star Captain",
	"Nova Captain",
	"Star Colonel",
	"Nova Colonel",
	"Galaxy Commander",
	"Khan",
	NULL,
};

// GLOBAL: MW2SHELL 0x10071280
MechChar* g_unk0x10071280[6] = {"Wolf", "Jade Falcon", "Ghost Bear", "Smoke Jaguar", "Nova Cat", "Steel Vipers"};

// The song of each shell message from 0x406 up, per campaign: a database item (plus the base),
// 0 to keep the current one, 0x20000000 to stop the music. 0x10000000 restarts the song.
// GLOBAL: MW2SHELL 0x10071298
MechS32 g_unk0x10071298[18] =
	{0x23, 0, 0, 0, 0x20000000, 0, 0, 0x23, 0x20000000, 0x23, 0x20000000, 0, 0, 0x23, 0, 0x20000000, 0x20000000, 0};

// GLOBAL: MW2SHELL 0x100712e0
MechS32 g_unk0x100712e0[18] = {
	0x25,
	0x24,
	0,
	0,
	0x20000000,
	0x24,
	0,
	0x23,
	0x20000000,
	0x25,
	0x20000000,
	0x25,
	0x24,
	0x25,
	0x26,
	0x20000000,
	0x20000000,
	0
};

// GLOBAL: MW2SHELL 0x10071328
MechS32 g_unk0x10071328[18] = {
	0x28,
	0x27,
	0,
	0,
	0x20000000,
	0x27,
	0,
	0x23,
	0x20000000,
	0x28,
	0x20000000,
	0x28,
	0x27,
	0x28,
	0x29,
	0x20000000,
	0x20000000,
	0
};

// GLOBAL: MW2SHELL 0x10071370
TinWhistle0x3c* g_pCurrentPilot = NULL;

// Set when a new pilot is registered, for the clan hall's welcome.
// GLOBAL: MW2SHELL 0x10071374
MechS32 g_unk0x10071374 = 0;

// GLOBAL: MW2SHELL 0x10071378
PaletteColor g_unk0x10071378[0x100] = {0};

// The sound settings (MW2SND.CFG).
// GLOBAL: MW2SHELL 0x10071678
ChimeLedger0x3c g_soundConfig = {0x10000, 0x10000, 0x10000, 0x10000, 0xf, 1, 1, 1, 1, 1, 8, 0, {0}};

// GLOBAL: MW2SHELL 0x100716b8
undefined g_unk0x100716b8[0x17] = {0, 0, 1, 1, 1, 1, 0, 0, 0, 1};

// GLOBAL: MW2SHELL 0x100946d0
TinWhistle0x3c g_pilotRoster[20];
