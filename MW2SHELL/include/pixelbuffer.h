#ifndef PIXELBUFFER_H
#define PIXELBUFFER_H

#include "decomp.h"
#include "types.h"

#pragma pack(1)
// SIZE 0x14
struct PixelBuffer {
	undefined* m_pixels;  // 0x00
	MechS32 m_maxX;       // 0x04
	MechS32 m_maxY;       // 0x08
	undefined4 m_unk0x0c; // 0x0c
	undefined4 m_unk0x10; // 0x10
};
typedef struct PixelBuffer PixelBuffer;
#pragma pack()

#endif // PIXELBUFFER_H
