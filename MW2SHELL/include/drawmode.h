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
	undefined4 m_unk0x00;                                                                       // 0x00
	undefined4 m_unk0x04;                                                                       // 0x04
	undefined4 m_unk0x08;                                                                       // 0x08
	undefined4 m_unk0x0c;                                                                       // 0x0c
	MechS32 (*m_begin)(PixelBuffer* p_buffer, MechS32 p_width, MechS32 p_height);               // 0x10
	MechS32 (*m_end)();                                                                         // 0x14
	MechS32 (*m_flip)();                                                                        // 0x18
	MechS32 (*m_blitRect)(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);    // 0x1c
	MechS32 (*m_stretchBlit)(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom); // 0x20
};
typedef struct DrawMode DrawMode;
#pragma pack()

#endif // DRAWMODE_H
