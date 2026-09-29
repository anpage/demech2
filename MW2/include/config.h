#ifndef CONFIG_H
#define CONFIG_H

#include "decomp.h"
#include "soundconfig.h"
#include "types.h"

#pragma pack(1)
// The difficulty settings, read from a .cfg file as one block.
// SIZE 0x17
typedef struct DifficultyCfg {
	undefined m_unk0x00;    // 0x00 — cleared in network games with more than one player
	undefined m_unk0x01;    // 0x01 — cleared in network games
	undefined m_unk0x02;    // 0x02 — set in network games with more than one player
	undefined m_unk0x03;    // 0x03 — set in network games with more than one player
	undefined m_unk0x04;    // 0x04 — set in network games with more than one player
	undefined m_unk0x05;    // 0x05 — 2 in network games
	undefined m_unk0x06[3]; // 0x06
	undefined m_unk0x09;    // 0x09 — set outside network games
	undefined m_unk0x0a;    // 0x0a
	undefined4 m_unk0x0b;   // 0x0b — cleared outside network games
	undefined4 m_unk0x0f;   // 0x0f — cleared outside network games
	undefined4 m_unk0x13;   // 0x13 — cleared outside network games
} DifficultyCfg;
#pragma pack()

// The functions and globals of config.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechChar g_gameDir[256];

	MechS32 LoadFile(MechChar* p_path, MechS32* p_size, void** p_data, MechU32* p_poolTag);
	MechS32 LoadDifficultyCfg(MechChar* p_name, DifficultyCfg** p_cfg);
	MechS32 LoadSndCfg(MechChar* p_name, SoundConfig** p_cfg);
	MechS32 SaveSndCfg(MechChar* p_name, SoundConfig* p_cfg);
	MechChar* BuildGamePath(MechChar* p_name);

#ifdef __cplusplus
}
#endif

#endif // CONFIG_H
