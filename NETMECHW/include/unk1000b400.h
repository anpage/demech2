#ifndef UNK1000B400_H
#define UNK1000B400_H

#include "decomp.h"
#include "types.h"

#include <dplay.h>

#pragma pack(1)

// The data of a settings change, a NetMessage tagged "SD", which the clients send to the host
// (FUN_1000c08c) and the host applies (FUN_1000bd99).
// SIZE 0x0d
struct SettingsMessage {
	enum {
		c_typeTeam = 0,   // switch the sender's team
		c_typeReady = 1,  // set the sender's ready bit to m_data.m_value
		c_typeAccept = 2, // the sender accepts the launch
		c_typeLaunch = 3, // the sender is launching
		c_typeMech = 4    // the sender's mech file, m_data.m_text
	};

	MechU32 m_type; // 0x00
	MechU8 m_flag;  // 0x04: bit 0 keeps the players' ready bits
	union {
		MechS32 m_value;
		MechChar m_text[8];
		MechU8 m_bytes[8];
	} m_data; // 0x05
};

#pragma pack()

// The functions and globals of unk1000b400.cpp that other units use.
void FUN_1000b400(DPID p_id, MechChar* p_name);
void FUN_1000b54e(DPID p_id);
void FUN_1000b6a6(DPID p_from, void* p_data);
void FUN_1000b6cb(void* p_settings);
void FUN_1000ba4d(DPID p_to);
void FUN_1000bbb0(DPID p_from, MechChar* p_data);
void FUN_1000bd99(DPID p_from, SettingsMessage* p_message);
void FUN_1000c072();
void FUN_1000c08c(MechU32 p_type, void* p_data, MechU32 p_size, MechU8 p_flag);
MechS32 FUN_1000c142(MechS32 p_index, MechChar* p_name);

#endif // UNK1000B400_H
