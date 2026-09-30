#ifndef GARNETFRAME_H
#define GARNETFRAME_H

#include "types.h"

// A rectangle of the cockpit layout resource (FUN_10070e22): its corner and size.
// SIZE 0x8
typedef struct GarnetFrame0x8 {
	MechS16 m_x;      // 0x00
	MechS16 m_y;      // 0x02
	MechS16 m_width;  // 0x04
	MechS16 m_height; // 0x06
} GarnetFrame0x8;

#endif // GARNETFRAME_H
