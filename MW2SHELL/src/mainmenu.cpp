#include "audiosample.h"
#include "audiosubsystem.h"
#include "brasslantern0x414.h"
#include "decomp.h"
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

struct MainMenuButton {
	MechS32 m_unk0x00;   // 0x00
	MechS32 m_unk0x04;   // 0x04
	MechS32 m_unk0x08;   // 0x08
	MechS32 m_unk0x0c;   // 0x0c
	MechS32 m_unk0x10;   // 0x10
	MechS32 m_unk0x14;   // 0x14
	MechChar* m_unk0x18; // 0x18
};

// GLOBAL: MW2SHELL 0x1006f568
MainMenuButton g_mainMenuButtons[4] = {
	{0xdb, 0x126, 0x1aa, 0x1a3, 0x140, 0x18b, g_unk0x10070820},
	{0x1ab, 0xf5, 0x27a, 0x175, 0x20d, 0x176, g_unk0x10070838},
	{0x0a, 0xc5, 0xc8, 0x172, 0x7c, 0x176, g_unk0x10070848},
	{0, 0x1c2, 0x27f, 0x1df, 0x140, 0x1c7, g_unk0x10070860},
};

// STUB: MW2SHELL 0x10003175
void FUN_10003175(MechS32, MechS32, MechS32, MechS32, MechS32)
{
	STUB(0x10003175);
}

// The original 0x10049c60 is the CRT operator new, already annotated in library_msvc.h.
void* AllocateAllowNew(MechS32 p_size)
{
	return ::operator new(p_size);
}
// SIZE 0x10d
class RandomName0x10d {
public:
	RandomName0x10d(
		VideoDriver* p_videoDriver,
		void* p_shellSelected,
		MechS32 p_unk0x0c,
		MainMenuButton* p_buttons,
		MechS32 p_count
	);

private:
	undefined m_unk0x00[0x10d]; // 0x00
};
DECOMP_SIZE_ASSERT(RandomName0x10d, 0x10d)
// STUB: MW2SHELL 0x100175e2
MechS32 FUN_100175e2(MechChar*, MechS32, MechS32, MechU32, MechU32)
{
	STUB(0x100175e2);
	return -1;
}

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
	g_unk0x1006ae74 = new RandomName0x10d(g_pVideoDriver, g_unk0x1007120c, 0, g_mainMenuButtons, 3);

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

// STUB: MW2SHELL 0x100485f0
RandomName0x10d::RandomName0x10d(VideoDriver*, void*, MechS32, MainMenuButton*, MechS32)
{
	STUB(0x100485f0);
}
