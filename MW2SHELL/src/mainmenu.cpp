#include "audiosample.h"
#include "audiosubsystem.h"
#include "brasslantern0x414.h"
#include "campaignmission.h"
#include "decomp.h"
#include "mainmenubutton.h"
#include "menulist0x10d.h"
#include "tallowsign0x10.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "videodriver.h"

#include <stddef.h>

void* operator new(size_t);

extern AudioSubsystem* g_pAudioSubsystem;
extern VideoDriver* g_pVideoDriver;
extern BrassLantern0x414* g_unk0x1007120c;

// GLOBAL: MW2SHELL 0x1006ae74
void* g_unk0x1006ae74 = NULL;

// GLOBAL: MW2SHELL 0x1006ae7c
AudioSample* g_unk0x1006ae7c = NULL;

// GLOBAL: MW2SHELL 0x1006ae84
MechChar g_unk0x1006ae84[] = "amwlogo1";

// GLOBAL: MW2SHELL 0x10070820
MechChar g_unk0x10070820[0x18] = "~TRIALS OF GRIEVANCE";
// GLOBAL: MW2SHELL 0x10070838
MechChar g_unk0x10070838[0x10] = "~WOLF CLAN HALL";
// GLOBAL: MW2SHELL 0x10070848
MechChar g_unk0x10070848[0x18] = "~JADE FALCON CLAN HALL";
// GLOBAL: MW2SHELL 0x10070860
MechChar g_unk0x10070860[0x08] = "~EXIT";

// GLOBAL: MW2SHELL 0x1006f568
MainMenuButton g_mainMenuButtons[4] = {
	{0xdb, 0x126, 0x1aa, 0x1a3, 0x140, 0x18b, g_unk0x10070820},
	{0x1ab, 0xf5, 0x27a, 0x175, 0x20d, 0x176, g_unk0x10070838},
	{0x0a, 0xc5, 0xc8, 0x172, 0x7c, 0x176, g_unk0x10070848},
	{0, 0x1c2, 0x27f, 0x1df, 0x140, 0x1c7, g_unk0x10070860},
};

// GLOBAL: MW2SHELL 0x1006fc90
CampaignMission g_unk0x1006fc90[17] = {
	{"yellSCN1", 0, "Pyre Light"},
	{"oranSCN1", 0, "Flame Tongue "},
	{"tealSCN1", 0, "Blade Splint"},
	{"taupSCN1", 0, "Temper Edge"},
	{"jennSCN1", 1, "Trial 1"},
	{"sablSCN1", 0, "Sable Flame"},
	{"greySCN1", 0, "Burning Chrome"},
	{"browSCN1", 0, "Scorching Sand"},
	{"amy_SCN1", 1, "Trial 2"},
	{"silvSCN1", 0, "Silver Staff"},
	{"aquaSCN1", 0, "Aquiline Fire"},
	{"kim_SCN1", 1, "Trial 3"},
	{"cyanSCN1", 0, "Cold Crescent"},
	{"maroSCN1", 0, "Velvet Hammer"},
	{"goldSCN1", 0, "Golden Spade"},
	{"irenSCN1", 1, "Trial 4"},
	{NULL, 0, "Retired"},
};

// GLOBAL: MW2SHELL 0x1006fd30
CampaignMission g_unk0x1006fd30[17] = {
	{"pinkSCN1", 0, "Silent Thunder"},
	{"greeSCN1", 0, "Arkham Bridge"},
	{"red_SCN1", 0, "Mirror Cage"},
	{"fuchSCN1", 0, "Bone Machine"},
	{"cindSCN1", 1, "Trial 1"},
	{"rustSCN1", 0, "Bouk Obelisk"},
	{"umbeSCN1", 0, "Umber Wall"},
	{"tan_SCN1", 0, "Rogue Chariot"},
	{"heidSCN1", 1, "Trial 2"},
	{"plumSCN1", 0, "Plum Wine"},
	{"whitSCN1", 0, "Rust Heart"},
	{"jillSCN1", 1, "Trial 3"},
	{"puceSCN1", 0, "Armor Veil"},
	{"blonSCN1", 0, "Iron Piston"},
	{"bronSCN1", 0, "Bronze Anvil"},
	{"marySCN1", 1, "Trial 4"},
	{NULL, 0, "Retired"},
};

// GLOBAL: MW2SHELL 0x1006fdd0
CampaignMission* g_campaignMissions[2] = {g_unk0x1006fc90, g_unk0x1006fd30};

// The clan hall archive screen.
// GLOBAL: MW2SHELL 0x10070164
MechChar g_unk0x10070164[0x08] = "~EXIT";
// GLOBAL: MW2SHELL 0x1007016c
MechChar g_unk0x1007016c[0x0c] = "~PREV PAGE";
// GLOBAL: MW2SHELL 0x10070178
MechChar g_unk0x10070178[0x0c] = "~NEXT PAGE";
// GLOBAL: MW2SHELL 0x10070184
MechChar g_unk0x10070184[0x08] = "~BACK";

// GLOBAL: MW2SHELL 0x1007052c
MechChar g_unk0x1007052c[0x08] = "~EXIT";
// GLOBAL: MW2SHELL 0x10070534
MechChar g_unk0x10070534[0x0c] = "~PREV PAGE";
// GLOBAL: MW2SHELL 0x10070540
MechChar g_unk0x10070540[0x0c] = "~NEXT PAGE";
// GLOBAL: MW2SHELL 0x1007054c
MechChar g_unk0x1007054c[0x08] = "~BACK";

// GLOBAL: MW2SHELL 0x1006e4a0
MainMenuButton g_unk0x1006e4a0[4] = {
	{0, 440, 639, 479, 320, 455, g_unk0x10070164},
	{405, 372, 479, 405, 449, 354, g_unk0x1007016c},
	{480, 372, 556, 405, 511, 408, g_unk0x10070178},
	{67, 358, 120, 410, 92, 408, g_unk0x10070184},
};

// GLOBAL: MW2SHELL 0x1006ee20
MainMenuButton g_unk0x1006ee20[4] = {
	{0, 440, 639, 479, 320, 455, g_unk0x1007052c},
	{405, 364, 479, 397, 449, 346, g_unk0x10070534},
	{480, 364, 556, 397, 511, 400, g_unk0x10070540},
	{67, 358, 120, 410, 92, 400, g_unk0x1007054c},
};

// GLOBAL: MW2SHELL 0x1006fe70
TallowSign0x10 g_unk0x1006fe70[3] = {
	{g_unk0x1006e4a0, 4, 12, -1},
	{g_unk0x1006ee20, 4, 19, -1},
	{NULL, 0, 0, 0},
};

// The custom battle's star screen.
// GLOBAL: MW2SHELL 0x10070348
MechChar g_unk0x10070348[0x10] = "<~EXIT CONFIG";
// GLOBAL: MW2SHELL 0x10070358
MechChar g_unk0x10070358[0x0c] = "~MECH LAB";
// GLOBAL: MW2SHELL 0x10070364
MechChar g_unk0x10070364[0x10] = "NEXT FORMATION";
// GLOBAL: MW2SHELL 0x10070374
MechChar g_unk0x10070374[0x10] = "PREV FORMATION";
// GLOBAL: MW2SHELL 0x10070384
MechChar g_unk0x10070384[0x10] = "ADD STARMATE";
// GLOBAL: MW2SHELL 0x10070394
MechChar g_unk0x10070394[0x10] = "DELETE STARMATE";
// GLOBAL: MW2SHELL 0x100703a4
MechChar g_unk0x100703a4[0x0c] = "CHANGE MECH";
// GLOBAL: MW2SHELL 0x100703b0
MechChar g_unk0x100703b0[0x0c] = "CHANGE MECH";
// GLOBAL: MW2SHELL 0x100703bc
MechChar g_unk0x100703bc[0x0c] = "CHANGE MECH";

// GLOBAL: MW2SHELL 0x1007070c
MechChar g_unk0x1007070c[0x10] = "<~EXIT CONFIG";
// GLOBAL: MW2SHELL 0x1007071c
MechChar g_unk0x1007071c[0x0c] = "~MECH LAB";
// GLOBAL: MW2SHELL 0x10070728
MechChar g_unk0x10070728[0x10] = "NEXT FORMATION";
// GLOBAL: MW2SHELL 0x10070738
MechChar g_unk0x10070738[0x10] = "PREV FORMATION";
// GLOBAL: MW2SHELL 0x10070748
MechChar g_unk0x10070748[0x10] = "ADD STARMATE";
// GLOBAL: MW2SHELL 0x10070758
MechChar g_unk0x10070758[0x10] = "DELETE STARMATE";
// GLOBAL: MW2SHELL 0x10070768
MechChar g_unk0x10070768[0x0c] = "CHANGE MECH";
// GLOBAL: MW2SHELL 0x10070774
MechChar g_unk0x10070774[0x0c] = "CHANGE MECH";
// GLOBAL: MW2SHELL 0x10070780
MechChar g_unk0x10070780[0x0c] = "CHANGE MECH";

// GLOBAL: MW2SHELL 0x1006e9b0
MainMenuButton g_unk0x1006e9b0[9] = {
	{50, 445, 149, 469, 100, 450, g_unk0x10070348},
	{404, 414, 474, 474, 440, 460, g_unk0x10070358},
	{263, 425, 302, 469, 280, 465, g_unk0x10070364},
	{237, 425, 262, 469, 260, 465, g_unk0x10070374},
	{303, 425, 330, 469, 300, 465, g_unk0x10070384},
	{200, 425, 236, 469, 240, 465, g_unk0x10070394},
	{159, 195, 187, 229, 73, 234, g_unk0x100703a4},
	{391, 162, 419, 196, 305, 201, g_unk0x100703b0},
	{586, 218, 614, 252, 500, 257, g_unk0x100703bc},
};

// GLOBAL: MW2SHELL 0x1006f330
MainMenuButton g_unk0x1006f330[9] = {
	{50, 445, 149, 469, 100, 450, g_unk0x1007070c},
	{404, 414, 474, 474, 440, 460, g_unk0x1007071c},
	{263, 425, 302, 469, 280, 465, g_unk0x10070728},
	{237, 425, 262, 469, 260, 465, g_unk0x10070738},
	{303, 425, 330, 469, 300, 465, g_unk0x10070748},
	{200, 425, 236, 469, 240, 465, g_unk0x10070758},
	{125, 201, 153, 235, 39, 240, g_unk0x10070768},
	{358, 156, 386, 190, 272, 195, g_unk0x10070774},
	{573, 246, 601, 280, 487, 285, g_unk0x10070780},
};

// GLOBAL: MW2SHELL 0x1006fea0
TallowSign0x10 g_unk0x1006fea0[3] = {
	{g_unk0x1006e9b0, 9, 15, -1},
	{g_unk0x1006f330, 9, 22, -1},
	{g_unk0x1006e9b0, 9, 10, -1},
};

void FUN_10003175(MechS32, MechS32, MechS32, MechS32, MechS32);

// The original 0x10049c60 is the CRT operator new, already annotated in library_msvc.h.
void* AllocateAllowNew(MechS32 p_size)
{
	return ::operator new(p_size);
}
MechS32 FUN_100175e2(MechChar* p_name, MechS32 p_left, MechS32 p_top, MechU32 p_flags, MechU32 p_unk0x14);

extern void FUN_100108e5(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32));
void MainMenuCallback(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32);

// FUNCTION: MW2SHELL 0x1003dc10
void FUN_1003dc10(TMPackDataBase* p_database, MechS32*)
{
	void* audioData = NULL;
	MechS32 audioSize;

	FUN_10003175(1, 0, 0, 0, 100);
	p_database->GetDBItem(104, &audioData, &audioSize);
	g_unk0x1006ae7c = new AudioSample(g_pAudioSubsystem, audioData, audioSize);

	g_pVideoDriver->FUN_10006c50(p_database, 1);
	g_unk0x1006ae74 = new MenuList0x10d(g_pVideoDriver, g_unk0x1007120c, 0, g_mainMenuButtons, 3);

	FUN_100175e2(g_unk0x1006ae84, 0x6f, 0x21, 10, 0);
	g_unk0x1006ae7c->SetVolume(0x78);
	g_unk0x1006ae7c->Start();
	FUN_100108e5(MainMenuCallback);
}

// STUB: MW2SHELL 0x1003dd89
void MainMenuCallback(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32)
{
	STUB(0x1003dd89);
}
