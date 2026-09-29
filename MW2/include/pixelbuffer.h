#ifndef PIXELBUFFER_H
#define PIXELBUFFER_H

#include "decomp.h"
#include "types.h"

// A block of pixels a render target draws into (the main one at 0x10176ef0).
// SIZE 0x14
typedef struct PixelBuffer {
	void* m_data;         // 0x00
	undefined4 m_unk0x04; // 0x04
	undefined4 m_unk0x08; // 0x08
	undefined4 m_unk0x0c; // 0x0c
	undefined4 m_unk0x10; // 0x10
} PixelBuffer;

#endif // PIXELBUFFER_H
