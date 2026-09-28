#ifndef DRAWMODE_H
#define DRAWMODE_H

#include "decomp.h"
#include "pixelbuffer.h"
#include "types.h"

#pragma pack(1)
// A display mode of one of the back ends in g_drawModeExtensions: how the framebuffer reaches
// the screen.
// SIZE 0x24
struct DrawMode {
	MechS32 m_index;                                                              // 0x00 — in g_drawModes
	MechS32 m_extension;                                                          // 0x04 — in g_drawModeExtensions
	MechS32 m_available;                                                          // 0x08 — cleared when m_begin fails
	MechU32 m_profileTime;                                                        // 0x0c
	MechS32 (*m_begin)(PixelBuffer* p_buffer, MechS32 p_width, MechS32 p_height); // 0x10
	MechS32 (*m_end)();                                                           // 0x14
	MechS32 (*m_flip)();                                                          // 0x18
	MechS32 (*m_blitRect)(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);    // 0x1c
	MechS32 (*m_stretchBlit)(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom); // 0x20
};
typedef struct DrawMode DrawMode;
#pragma pack()

#endif // DRAWMODE_H
