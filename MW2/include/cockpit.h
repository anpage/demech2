#ifndef COCKPIT_H
#define COCKPIT_H

#include "decomp.h"
#include "point.h"
#include "rendertarget.h"
#include "types.h"

// A gauge-drawing function of a cockpit layout.
typedef void (*CockpitGaugeFn)();

// Rectangles of a cockpit view beyond its viewport (CockpitLayout's helpers).
typedef struct CockpitLayoutRects {
	undefined4 m_unk0x00;    // 0x00
	RenderTarget* m_unk0x04; // 0x04
	RenderTarget* m_unk0x08; // 0x08
	RenderTarget* m_unk0x0c; // 0x0c
} CockpitLayoutRects;

typedef struct CockpitLayoutExtra {
	undefined4 m_unk0x00;        // 0x00
	CockpitLayoutRects* m_rects; // 0x04
} CockpitLayoutExtra;

// The layout of one cockpit view (4: the satellite view).
typedef struct CockpitLayout {
	RenderTarget* m_viewport;                // 0x00 — in 16.16 fractions of the screen
	undefined4 m_unk0x04;                    // 0x04
	MechS32 m_renderTargetSlot;              // 0x08 — in g_renderTargets
	undefined4 m_unk0x0c[(0x14 - 0x0c) / 4]; // 0x0c
	CockpitLayoutExtra* m_extra;             // 0x14
	MechS32 m_unk0x18;                       // 0x18 — the range the readout shows
	MechS32 m_unk0x1c;                       // 0x1c — the range it last formatted
	undefined4 m_unk0x20[(0x30 - 0x20) / 4]; // 0x20
	MechS32 m_unk0x30;                       // 0x30 — its font, from g_unk0x100e9614
	undefined4 m_unk0x34[(0x3c - 0x34) / 4]; // 0x34
	MechChar* m_unk0x3c;                     // 0x3c — the range label
	MechChar* m_unk0x40;                     // 0x40 — the range text
	MechChar* m_unk0x44;                     // 0x44 — the heading label
	MechChar* m_unk0x48;                     // 0x48 — the heading text
	MechS32 m_unk0x4c;                       // 0x4c — the heading it last formatted
	MechChar* m_unk0x50;                     // 0x50 — the unit of short ranges
	MechChar* m_unk0x54;                     // 0x54 — the unit of long ranges
	Point m_unk0x58;                         // 0x58
	Point m_unk0x60;                         // 0x60
	Point m_unk0x68;                         // 0x68
	undefined4 m_unk0x70[(0x7c - 0x70) / 4]; // 0x70
	CockpitGaugeFn m_gauges[4];              // 0x7c — indices in g_cockpitGauges until loaded
} CockpitLayout;

// The functions and globals of cockpit.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x10109c5c;
	extern MechS32 g_unk0x10109c60;
	extern MechS32 g_cockpitLayoutIndex;
	extern MechS32 g_unk0x10109c68;
	extern MechS32 g_unk0x10109c6c;

	void LoadCockpitLayout(MechS32 p_cockpit, CockpitLayout* p_layout);
	void FUN_1003dd82(void);
	MechS32 FUN_1003ddd7(void);
	void FUN_1003e03c(void);
	void FUN_1003e06c(void);
	void DrawMapViewText(CockpitLayout* p_layout);
	void FUN_1003ee26(void);
	MechS32 FUN_1003ee69(void);
	void FUN_1003ee92(void);
	void FUN_1003eeaf(void);
	void FUN_1003ef07(MechS32 p_unk0x00);
	void FUN_1003f8d1(void);

#ifdef __cplusplus
}
#endif

#endif // COCKPIT_H
