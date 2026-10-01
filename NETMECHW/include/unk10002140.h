#ifndef UNK10002140_H
#define UNK10002140_H

#include "decomp.h"
#include "types.h"

#include <dplay.h>
#include <windows.h>

// A lobby message: a two-letter tag, then the tag's data.
// SIZE 0x200
struct NetMessage {
	enum {
		c_tagCP = 0x5043, // "CP"
		c_tagMI = 0x494d, // "MI"
		c_tagMQ = 0x514d, // "MQ"
		c_tagSD = 0x4453, // "SD"
		c_tagST = 0x5453, // "ST"
		c_tagXX = 0x5858  // "XX", echoed back to the sender
	};

	MechU16 m_tag;                  // 0x00
	undefined m_data[0x200 - 0x02]; // 0x02
};

// The functions and globals of unk10002140.cpp that other units use.
typedef void (*NetMessageHandler)(void* p_message, DPID p_from);

DWORD WINAPI ReceiveThread(LPVOID p_handler);
void HostMessageHandler(void* p_message, DPID p_from);
void ClientMessageHandler(void* p_message, DPID p_from);

#endif // UNK10002140_H
