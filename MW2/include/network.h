#ifndef NETWORK_H
#define NETWORK_H

#include "types.h"

struct NetLaunchInfo;

// The functions and globals of network.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FirstNetwork(struct NetLaunchInfo* p_unk0x00);
	MechS32 FirstExternalCtrl(void);
	void UpdateNetwork(void);
	void ShutdownNetwork(void);
	void FUN_10010539(void);

#ifdef __cplusplus
}
#endif

#endif // NETWORK_H
