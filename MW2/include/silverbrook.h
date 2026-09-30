#ifndef SILVERBROOK_H
#define SILVERBROOK_H

#include "decomp.h"
#include "types.h"

// An entry of an AI maneuver table: a maneuver and the ones that may follow it.
// SIZE 0x12
typedef struct AmberGlade0x12 {
	MechS16 m_id;                     // 0x00
	MechS16 m_count;                  // 0x02 — how many of m_next are used
	undefined m_unk0x04[0x12 - 0x04]; // 0x04
} AmberGlade0x12;

// An AI maneuver table (FUN_10013d81): the maneuvers a mech class chooses among.
// SIZE 0x08
typedef struct SilverBrook0x08 {
	MechS16 m_count;           // 0x00
	MechS16 m_unk0x02;         // 0x02
	AmberGlade0x12* m_entries; // 0x04
} SilverBrook0x08;

#endif // SILVERBROOK_H
