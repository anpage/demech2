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
	// The option check boxes (controls 0x421 to 0x426), read as bits and written as a byte.
	// SIZE 0x01
	struct Options {
		MechU8 m_option0 : 1; // 0x421
		MechU8 m_option1 : 1; // 0x422
		MechU8 m_option2 : 1; // 0x423
		MechU8 m_option3 : 1; // 0x424
		MechU8 m_option4 : 1; // 0x425
		MechU8 m_option5 : 1; // 0x426
		MechU8 m_option6 : 1; // cleared by FUN_10009615
		MechU8 m_option7 : 1; // cleared by FUN_10009615
	};

	// The options as a byte or as bits.
	// SIZE 0x01
	union OptionFlags {
		MechU8 m_byte;
		Options m_bits;
	};

	MechU8 m_unk0x00;       // 0x00
	MechU8 m_unk0x01;       // 0x01: the players on team 1, a bit per slot
	MechU8 m_unk0x02;       // 0x02: the players who are ready, a bit per slot
	MechU8 m_unk0x03;       // 0x03: the players who accepted the launch, a bit per slot
	MechChar m_mechs[8][8]; // 0x04: each slot's mech file, without terminator
	MechChar m_unk0x44[4];  // 0x44
	OptionFlags m_options;  // 0x48
	MechU8 m_unk0x49;       // 0x49
	MechU8 m_unk0x4a;       // 0x4a
	MechU8 m_unk0x4b;       // 0x4b
	MechU8 m_unk0x4c;       // 0x4c
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
		return memcmp(&m_savedSettings, &m_settings, sizeof(m_settings)) || FUN_1000b0b3() != m_savedPlayerCount;
	}

	// FUNCTION: NETMECHW 0x10003620
	void FUN_10003620()
	{
		m_savedSettings = m_settings;
		m_savedPlayerCount = FUN_1000b0b3();
	}

	// Marks the local player ready in the saved options (FUN_1000e58d).
	// FUNCTION: NETMECHW 0x1000f050
	void FUN_1000f050() { m_savedSettings.m_unk0x02 |= 1 << FUN_1000b02b(m_playerId); }

	// Marks the local player not ready in the saved options (FUN_1000e58d).
	// FUNCTION: NETMECHW 0x1000f0a0
	void FUN_1000f0a0() { m_savedSettings.m_unk0x02 &= ~(MechU8) (1 << FUN_1000b02b(m_playerId)); }

	undefined4 m_unk0x00;                  // 0x00
	LPGUID m_unk0x04;                      // 0x04: the service provider (DirectPlayCreate)
	LPDIRECTPLAY m_directPlay;             // 0x08
	DPSESSIONDESC m_sessionDesc;           // 0x0c
	CopperField0x4d m_settings;            // 0x88
	DPID m_playerId;                       // 0xd5
	DPID m_hostId;                         // 0xd9
	MechChar m_playerName[DPSHORTNAMELEN]; // 0xdd
	undefined m_unk0xf1;                   // 0xf1
	MechChar m_mechFile[9];                // 0xf2
	MechS32 m_isHost;                      // 0xfb
	undefined4 m_unk0xff;                  // 0xff
	CopperField0x4d m_savedSettings;       // 0x103
	MechS32 m_savedPlayerCount;            // 0x150
	MechS32 m_unk0x154;                    // 0x154
	MechS32 m_unk0x158;                    // 0x158
	MechS32 m_unk0x15c;                    // 0x15c: the current pane (FUN_10005023)
};

#pragma pack()

#endif // UNK10006B20_H
