#ifndef DRAWMODEEXT_H
#define DRAWMODEEXT_H

#include "decomp.h"
#include "types.h"

typedef enum {
	c_windowModeUnknown = 0,
	c_windowModeFullscreen = 1,
	c_windowModeWindowed = 2
} WindowMode;

// The palette and buffer half of a draw mode (g_currentDrawModeExtension).
typedef struct DrawModeExtension {
	MechS32 m_index;                                                                                         // 0x00
	WindowMode m_windowMode;                                                                                 // 0x04
	MechU32 m_windowStyle;                                                                                   // 0x08
	void* m_drawModeBegin;                                                                                   // 0x0c
	void* m_drawModeEnd;                                                                                     // 0x10
	void (*m_setPalette)(undefined4 p_unk0x00, undefined4 p_unk0x04, void* p_unk0x08, undefined4 p_unk0x0c); // 0x14
	void (*m_setPaletteWithBrightness)(void* p_palette);                                                     // 0x18
	void (*m_paletteFade)(void* p_palette, MechS32 p_steps);                                                 // 0x1c
	MechS32 (*m_lockBuffer)(void);                                                                           // 0x20
	undefined4 m_unk0x24;                                                                                    // 0x24
} DrawModeExtension;

#endif // DRAWMODEEXT_H
