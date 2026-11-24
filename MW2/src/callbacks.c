/* Timed callbacks: a list of TimedCallback entries, each calling its function every m_period
   ticks of the game clock until the function returns 0. */
#include "callbacks.h"

#include "clock.h"
#include "decomp.h"
#include "poolsizes.h"
#include "simmain.h"
#include "staticmem.h"
#include "types.h"

#include <windows.h>

DECOMP_SIZE_ASSERT(TimedCallback, 0x18)

// The callback being created, run or removed.
// GLOBAL: MW2 0x100ba5d4
// GLOBAL: MW2GBL 0x1009f0e0
// GLOBAL: MW2PREMERCS 0x100b0444
// GLOBAL: MW2MATROX 0x100a5948
// GLOBAL: MW23DFX 0x1024abc8
// GLOBAL: MW2ATI 0x1017f018
// GLOBAL: MW2S3 0x100759e8
// GLOBAL: MW2POWERVR 0x101a9d68
TimedCallback* g_currentCallback = NULL;

// FUNCTION: MW2 0x1007d2e0
// FUNCTION: MW2GBL 0x1001baf0
// FUNCTION: MW2PREMERCS 0x10026e70
// FUNCTION: MW2MATROX 0x100272b0
// FUNCTION: MW23DFX 0x10008270
// FUNCTION: MW2ATI 0x10008ab0
// FUNCTION: MW2S3 0x1003b820
// FUNCTION: MW2POWERVR 0x10007820
TimedCallback* GetCurrentCallback(void)
{
	return g_currentCallback;
}

// FUNCTION: MW2 0x1007d2f5
// FUNCTION: MW2GBL 0x1001bb05
// FUNCTION: MW2PREMERCS 0x10026e85
// FUNCTION: MW2MATROX 0x100272c5
// FUNCTION: MW23DFX 0x10008280
// FUNCTION: MW2ATI 0x10008ac0
// FUNCTION: MW2S3 0x1003b830
// FUNCTION: MW2POWERVR 0x10007830
TimedCallback* CreateDetachedTask(TimedCallback** p_list, TimedCallbackFn p_fn, MechS32 p_period, MechChar* p_data)
{
	TimedCallback* callback;

	callback = StaticPoolAlloc(sizeof(TimedCallback), g_staticPoolTags[3]);
	if (callback == NULL) {
		return NULL;
	}

	callback->m_fn = p_fn;
	callback->m_data = NULL;
	callback->m_period = p_period;
	callback->m_lastTime = g_currentClock;
	callback->m_nextTime = callback->m_lastTime;
	callback->m_next = *p_list;
	*p_list = callback;
	g_currentCallback = *p_list;
	if (p_fn != NULL && !p_fn(0, p_data, g_currentClock, p_period)) {
		RemoveTask(p_list, g_currentCallback);
		return NULL;
	}

	return callback;
}

// FUNCTION: MW2 0x1007d3bf
// FUNCTION: MW2GBL 0x1001bbcf
// FUNCTION: MW2PREMERCS 0x10026f4f
// FUNCTION: MW2MATROX 0x1002738f
// FUNCTION: MW23DFX 0x10008300
// FUNCTION: MW2ATI 0x10008b40
// FUNCTION: MW2S3 0x1003b8b0
// FUNCTION: MW2POWERVR 0x100078b0
void RemoveTask(TimedCallback** p_list, TimedCallback* p_callback)
{
	TimedCallback* callback;

	if (p_callback == NULL) {
		return;
	}

	if (*p_list == p_callback) {
		*p_list = p_callback->m_next;
	}
	else {
		for (callback = *p_list; callback != NULL; callback = callback->m_next) {
			if (callback->m_next == p_callback) {
				callback->m_next = p_callback->m_next;
				break;
			}
		}
	}

	g_currentCallback = p_callback;
	p_callback->m_fn(2, NULL, g_currentClock, p_callback->m_period);
	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_callback->m_data);
}

// FUNCTION: MW2 0x1007d475
// FUNCTION: MW2GBL 0x1001bc85
// FUNCTION: MW2PREMERCS 0x10027005
// FUNCTION: MW2MATROX 0x10027445
// FUNCTION: MW23DFX 0x10008360
// FUNCTION: MW2ATI 0x10008ba0
// FUNCTION: MW2S3 0x1003b910
// FUNCTION: MW2POWERVR 0x10007910
void RemoveAllTasks(TimedCallback** p_list)
{
	TimedCallback* callback;

	for (callback = *p_list; callback != NULL; callback = callback->m_next) {
		RemoveTask(p_list, callback);
	}
}

// FUNCTION: MW2 0x1007d4b8
// FUNCTION: MW2GBL 0x1001bcc8
// FUNCTION: MW2PREMERCS 0x10027048
// FUNCTION: MW2MATROX 0x10027488
// FUNCTION: MW23DFX 0x10008380
// FUNCTION: MW2ATI 0x10008bc0
// FUNCTION: MW2S3 0x1003b930
// FUNCTION: MW2POWERVR 0x10007930
void SignalTask(TimedCallback** p_list, TimedCallback* p_callback)
{
	p_callback->m_fn(-1, NULL, g_currentClock, p_callback->m_period);
}

// FUNCTION: MW2 0x1007d4dc
// FUNCTION: MW2GBL 0x1001bcec
// FUNCTION: MW2PREMERCS 0x1002706c
// FUNCTION: MW2MATROX 0x100274ac
// FUNCTION: MW23DFX 0x100083a0
// FUNCTION: MW2ATI 0x10008be0
// FUNCTION: MW2S3 0x1003b950
// FUNCTION: MW2POWERVR 0x10007950
void SignalAllTasks(TimedCallback** p_list)
{
	TimedCallback* callback;

	for (callback = *p_list; callback != NULL; callback = callback->m_next) {
		SignalTask(p_list, callback);
	}
}

// FUNCTION: MW2 0x1007d51f
// FUNCTION: MW2GBL 0x1001bd2f
// FUNCTION: MW2PREMERCS 0x100270af
// FUNCTION: MW2MATROX 0x100274ef
// FUNCTION: MW23DFX 0x100083c0
// FUNCTION: MW2ATI 0x10008c00
// FUNCTION: MW2S3 0x1003b970
// FUNCTION: MW2POWERVR 0x10007970
void** GetCallbackData(TimedCallback* p_callback)
{
	return &p_callback->m_data;
}

// FUNCTION: MW2 0x1007d535
// FUNCTION: MW2GBL 0x1001bd45
// FUNCTION: MW2PREMERCS 0x100270c5
// FUNCTION: MW2MATROX 0x10027505
// FUNCTION: MW23DFX 0x100083d0
// FUNCTION: MW2ATI 0x10008c10
// FUNCTION: MW2S3 0x1003b980
// FUNCTION: MW2POWERVR 0x10007980
void RunTimedCallbacks(TimedCallback** p_list)
{
	for (g_currentCallback = *p_list; g_currentCallback != NULL; g_currentCallback = g_currentCallback->m_next) {
		if (g_currentCallback->m_fn != NULL && g_currentCallback->m_nextTime <= g_currentClock) {
			g_currentCallback->m_nextTime = g_currentCallback->m_period + g_currentClock;
			if (!g_currentCallback->m_fn(1, NULL, g_currentClock, g_currentCallback->m_period)) {
				RemoveTask(p_list, g_currentCallback);
			}
			else {
				g_currentCallback->m_lastTime = g_currentClock;
			}
		}
	}
}

// FUNCTION: MW2 0x1007d5f1
// FUNCTION: MW2GBL 0x1001be01
// FUNCTION: MW2PREMERCS 0x10027181
// FUNCTION: MW2MATROX 0x100275c1
// FUNCTION: MW23DFX 0x10008450
// FUNCTION: MW2ATI 0x10008c90
// FUNCTION: MW2S3 0x1003ba00
// FUNCTION: MW2POWERVR 0x10007a00
MechS32 GetTimedCallbackSize(void)
{
	return sizeof(TimedCallback);
}
