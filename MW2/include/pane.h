#ifndef PANE_H
#define PANE_H

#include "decomp.h"
#include "types.h"

typedef struct PixelBuffer PixelBuffer;

#pragma pack(1)
// A rectangle of a PixelBuffer, which the drawing routines (blit.asm, polyfill.asm) take. They
// are John Miles's VFX library: MW2's credits thank him for "the graphics and sound packages"
// and say "once we converted to the world of PANES, high res was easy", and VFX's VFX.H defines
// this struct as PANE (window, x0, y0, x1, y1), its PixelBuffer as WINDOW (buffer, x_max, y_max,
// stencil, shadow).
// SIZE 0x14
struct Pane {
	PixelBuffer* m_buffer; // 0x00
	MechS32 m_left;        // 0x04
	MechS32 m_top;         // 0x08
	MechS32 m_right;       // 0x0c
	MechS32 m_bottom;      // 0x10
};
typedef struct Pane Pane;
#pragma pack()

#endif // PANE_H
