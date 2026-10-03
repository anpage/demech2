#ifndef RENDER_H
#define RENDER_H

#include "decomp.h"
#include "rendertarget.h"
#include "types.h"
#include "window.h"

// SIZE 0x18
typedef struct GameWindowGeometry {
	MechS32 m_width;      // 0x00
	MechS32 m_height;     // 0x04
	undefined4 m_unk0x08; // 0x08
	MechS32 m_numColors;  // 0x0c
	undefined4 m_unk0x10; // 0x10
	undefined4 m_unk0x14; // 0x14
} GameWindowGeometry;

struct Shape;

// The functions and globals of render.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechChar* g_unk0x100a2458;
	extern void* g_unk0x100a245c;
	extern MechS32 g_unk0x100a2460;
	extern MechS32 g_unk0x100a2464;
	extern MechS32 g_unk0x100a2454;
	extern MechS32 g_drawModeIndex;
	extern MechS32 g_initDrawModeParam2;
	extern PANE g_unk0x100bdff8;
	extern PANE g_currentPane;
	extern WINDOW g_mainPixelBuffer;
	extern MechS32 g_unk0x100a246c;
	extern MechS32 g_unk0x100a2470;
	extern MechS32 g_unk0x100a2474;
	extern GameWindowGeometry* g_gameWindowGeometry;
	extern MechS32 g_screenHeight;
	extern MechS32 g_screenHeightMinus1;
	extern MechS32 g_screenPixelCount;
	extern MechS32 g_screenWidth;
	extern MechS32 g_screenWidthMinus1;
	extern MechS32 g_screenHalfWidth;
	extern MechS32 g_screenHalfHeight;
	extern undefined4 g_unk0x10176eb0;
	extern MechS32 g_unk0x10176ebc;
	extern MechS32 g_unk0x100a2468;
	extern MechS32 g_unk0x100a2480;

	MechS32 InitGameWindowGeometry(void);
	MechS32 InitDisplayGeometry(void);
	void FirstRender(void);
	void SecondRender(void);
	void FUN_10012afe(void);
	void FUN_10012dca(MechS32 p_value);
	void FUN_10012e00(void);
	void Blit(void);
	void ShutdownRender(void);
	undefined4 FUN_10012f14(void);
	void FUN_10012f29(undefined4 p_unk0x00, undefined4 p_value);
	void FUN_100131f1(struct Shape* p_root);

#ifdef __cplusplus
}
#endif

#endif // RENDER_H
