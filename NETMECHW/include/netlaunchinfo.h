#ifndef NETLAUNCHINFO_H
#define NETLAUNCHINFO_H

#include "decomp.h"
#include "types.h"

#include <dplay.h>
#include <windows.h>

// What MECH2.EXE hands Launcher, and Launcher fills in for SimMain: the DirectPlay session,
// the local player's id, the ids of the players by slot, and the mission to run. MECH2.EXE
// and MW2.DLL declare the same layout.
typedef struct NetLaunchInfo {
	LPDIRECTPLAY m_directPlay;    // 0x00
	DPID m_localPlayerId;         // 0x04
	undefined4 m_unk0x08;         // 0x08
	undefined4 m_unk0x0c;         // 0x0c
	DPID* m_playerIds;            // 0x10
	undefined4 m_unk0x14;         // 0x14
	undefined4 m_unk0x18;         // 0x18
	undefined4 m_unk0x1c;         // 0x1c
	undefined4 m_missionNameSize; // 0x20
	MechChar* m_missionName;      // 0x24
} NetLaunchInfo;

#endif // NETLAUNCHINFO_H
