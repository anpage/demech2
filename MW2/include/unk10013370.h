#ifndef UNK10013370_H
#define UNK10013370_H

#include "decomp.h"
#include "types.h"

// An entry of the sound table FUN_10013370 unpacks into g_soundInfo.
// SIZE 0x08
typedef struct SoundTableEntry {
	MechS16 m_id;       // 0x00 — the sound resource
	MechS16 m_priority; // 0x02
	MechS16 m_maxCount; // 0x04
	MechS16 m_rate;     // 0x06
} SoundTableEntry;

// The functions and globals of unk10013370.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern SoundTableEntry g_soundTable[0x8c];

	void FUN_10013370(void);

#ifdef __cplusplus
}
#endif

#endif // UNK10013370_H
