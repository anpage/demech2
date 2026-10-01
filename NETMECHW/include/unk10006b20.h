#ifndef UNK10006B20_H
#define UNK10006B20_H

#include "decomp.h"
#include "types.h"
#include "unk1000aa90.h"

#include <dplay.h>
#include <string.h>
#include <windows.h>

#pragma pack(1)

// The lobby's game options, a block of 0x4d bytes the lobby copies and compares as a whole to
// tell whether they changed (IronLantern0x160::FUN_100035c0).
// SIZE 0x4d
struct CopperField0x4d {
	MechU8 m_unk0x00;                 // 0x00
	MechU8 m_unk0x01;                 // 0x01
	MechU8 m_unk0x02;                 // 0x02
	MechU8 m_unk0x03;                 // 0x03
	undefined m_unk0x04[0x48 - 0x04]; // 0x04
	MechU8 m_unk0x48;                 // 0x48
	MechU8 m_unk0x49;                 // 0x49
	MechU8 m_unk0x4a;                 // 0x4a
	MechU8 m_unk0x4b;                 // 0x4b
	MechU8 m_unk0x4c;                 // 0x4c
};

// The lobby's state: the DirectPlay object, the local player and the host, and the game
// options. One global instance, g_unk0x1001ca90 (unk10003660.cpp).
// SIZE 0x160
class IronLantern0x160 {
public:
	IronLantern0x160();
	~IronLantern0x160();

	// Returns whether the options changed since FUN_10003620 saved them.
	// FUNCTION: NETMECHW 0x100035c0
	MechS32 FUN_100035c0()
	{
		return memcmp(&m_savedSettings, &m_settings, sizeof(m_settings)) || FUN_1000b0b3() != m_unk0x150;
	}

	// FUNCTION: NETMECHW 0x10003620
	void FUN_10003620()
	{
		m_savedSettings = m_settings;
		m_unk0x150 = FUN_1000b0b3();
	}

	undefined4 m_unk0x00;                  // 0x00
	undefined4 m_unk0x04;                  // 0x04
	LPDIRECTPLAY m_directPlay;             // 0x08
	undefined m_unk0x0c[0x88 - 0x0c];      // 0x0c
	CopperField0x4d m_settings;            // 0x88
	DPID m_playerId;                       // 0xd5
	DPID m_hostId;                         // 0xd9
	MechChar m_playerName[DPSHORTNAMELEN]; // 0xdd
	undefined m_unk0xf1;                   // 0xf1
	MechChar m_mechFile[9];                // 0xf2
	MechS32 m_isHost;                      // 0xfb
	undefined4 m_unk0xff;                  // 0xff
	CopperField0x4d m_savedSettings;       // 0x103
	undefined4 m_unk0x150;                 // 0x150
	undefined4 m_unk0x154;                 // 0x154
	undefined4 m_unk0x158;                 // 0x158
	undefined4 m_unk0x15c;                 // 0x15c
};

#pragma pack()

#endif // UNK10006B20_H
