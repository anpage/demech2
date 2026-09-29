#include "network.h"

#include "decomp.h"
#include "simmain.h"
#include "types.h"

#include <windows.h>

// STUB: MW2 0x1000e493
void FirstNetwork(struct NetLaunchInfo* p_unk0x00)
{
	STUB(0x1000e493);
}

// FUNCTION: MW2 0x1000e677
MechS32 FirstExternalCtrl(void)
{
	return 1;
}

// STUB: MW2 0x1000e68c
void UpdateNetwork(void)
{
	STUB(0x1000e68c);
}

// FUNCTION: MW2 0x1000eb05
void ShutdownNetwork(void)
{
	FUN_10010539();
	if (g_unk0x100a179c) {
		if (g_unk0x100a178c) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_unk0x100a178c);
		}

		if (g_unk0x100a1794) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_unk0x100a1794);
		}

		if (g_unk0x100a1798) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_unk0x100a1798);
		}

		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_unk0x101770cc);
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_unk0x101770a0);
	}
}

// STUB: MW2 0x10010539
void FUN_10010539(void)
{
	STUB(0x10010539);
}
