#ifndef PIXELVIEW_H
#define PIXELVIEW_H

#include "decomp.h"
#include "types.h"

typedef struct PixelBuffer PixelBuffer;

#pragma pack(1)
// A rectangle of a PixelBuffer; the blit routines take views, not buffers.
// SIZE 0x14
struct PixelView {
	PixelBuffer* m_buffer; // 0x00
	MechS32 m_left;        // 0x04
	MechS32 m_top;         // 0x08
	MechS32 m_right;       // 0x0c
	MechS32 m_bottom;      // 0x10
};
typedef struct PixelView PixelView;
#pragma pack()

#endif // PIXELVIEW_H
