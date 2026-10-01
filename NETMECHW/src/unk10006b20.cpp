#include "unk10006b20.h"

#include "decomp.h"
#include "types.h"
#include "unk10003660.h"

#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(CopperField0x4d, 0x4d)
DECOMP_SIZE_ASSERT(IronLantern0x160, 0x160)

// FUNCTION: NETMECHW 0x10006b20
IronLantern0x160::IronLantern0x160()
{
	DWORD size;
	MechChar name[0x100];

	size = sizeof(name);
	name[0] = '\0';
	if (!FUN_10005cdc(name, &size)) {
		GetUserName(name, &size);
	}

	m_unk0x04 = NULL;
	m_directPlay = NULL;
	strncpy(m_playerName, name, sizeof(m_playerName));
	strcpy(m_mechFile, "frm00std");
	m_hostId = 0;
	m_unk0xff = 0;
	m_isHost = FALSE;
	m_settings.m_unk0x00 = 0;
	m_settings.m_unk0x01 = 0;
	m_settings.m_unk0x02 = 0;
	m_settings.m_unk0x03 = 0;
	m_settings.m_options.m_byte = 0;
	m_settings.m_unk0x49 = 100;
	m_settings.m_unk0x4a = 1;
	m_settings.m_unk0x4b = 0x10;
	m_settings.m_unk0x4c = 1;
	m_unk0x154 = 0;
	m_unk0x158 = 0;
	m_unk0x15c = 0;
}

// FUNCTION: NETMECHW 0x10006cc1
IronLantern0x160::~IronLantern0x160()
{
}
