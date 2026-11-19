/* The cockpit's text readout and the layouts of its map and satellite views (g_cockpitLayouts),
   which cockpit.c draws. A data-only object: its data follows targeting.c's in 1.1, overlay.c's
   in the Matrox edition and gdi.c's in the 3Dfx edition. */
#include "cockpit.h"
#include "cockpitreadout.h"
#include "recttransition.h"
#include "targeting.h"
#ifdef MW2_LAYOUT_GAUGE_FUNCTIONS
#include "screenscale.h"
#endif
#include "types.h"

#include <stddef.h>

// The cockpit layouts' text buffers and saved viewports.

// GLOBAL: MW2 0x100e9450
// GLOBAL: MW23DFX 0x10244c30
// GLOBAL: MW2ATI 0x10178a80
MechChar g_satelliteRangeText[0x20];

// GLOBAL: MW2 0x100e9490
// GLOBAL: MW23DFX 0x10244b30
// GLOBAL: MW2ATI 0x101789a0
MechChar g_satelliteExtraText[0x20];

// GLOBAL: MW2 0x100e94b0
// GLOBAL: MW23DFX 0x10244bb0
// GLOBAL: MW2ATI 0x10178a00
PANE g_satelliteSavedViewport;

// GLOBAL: MW2 0x100e94d0
// GLOBAL: MW23DFX 0x10244ab0
// GLOBAL: MW2ATI 0x10178940
MechChar g_smallMapRangeText[0x20];

// GLOBAL: MW2 0x100e94f0
// GLOBAL: MW23DFX 0x10244b90
// GLOBAL: MW2ATI 0x101789e0
MechChar g_smallMapExtraText[0x20];

// GLOBAL: MW2 0x100e9510
// GLOBAL: MW23DFX 0x10244bf0
// GLOBAL: MW2ATI 0x10178a40
MechChar g_readoutText[0x20];

// GLOBAL: MW2 0x100e9530
// GLOBAL: MW23DFX 0x10244b50
// GLOBAL: MW2ATI 0x101789c0
PANE g_largeMapSavedViewport;

// GLOBAL: MW2 0x100e9550
// GLOBAL: MW23DFX 0x10244c10
// GLOBAL: MW2ATI 0x10178a60
MechChar g_largeMapRangeText[0x20];

// GLOBAL: MW2 0x100e9570
// GLOBAL: MW23DFX 0x10244af0
// GLOBAL: MW2ATI 0x10178960
MechChar g_bearingText[0x20];

// GLOBAL: MW2 0x100e9590
// GLOBAL: MW23DFX 0x10244bd0
// GLOBAL: MW2ATI 0x10178a20
PANE g_smallMapSavedViewport;

// GLOBAL: MW2 0x100e95d0
// GLOBAL: MW23DFX 0x10244b10
// GLOBAL: MW2ATI 0x10178980
MechChar g_largeMapExtraText[0x20];

// The cockpit views' layouts (g_cockpitLayouts): the map view of cockpit views 1 and 2 and the
// satellite view (4), with their labels, icons, colors, rectangles and transitions.

// GLOBAL: MW2 0x100aabb0
// GLOBAL: MW2MATROX 0x100bcdd0
// GLOBAL: MW23DFX 0x1024a228
// GLOBAL: MW2ATI 0x1017e070
MechChar g_readoutLabel[8] = "x";

// GLOBAL: MW2 0x100aabb8
// GLOBAL: MW2MATROX 0x100bcdd8
// GLOBAL: MW23DFX 0x1024a230
// GLOBAL: MW2ATI 0x1017e078
CockpitReadout g_readout = {1, 0, -1, g_readoutLabel, g_readoutText, {0x28f, 0x28f}};

// GLOBAL: MW2 0x100aabd4
// GLOBAL: MW2MATROX 0x100bcdf4
// GLOBAL: MW23DFX 0x1024a24c
// GLOBAL: MW2ATI 0x1017e094
CockpitReadout* g_cockpitReadout = &g_readout;

// GLOBAL: MW2 0x100aabd8
// GLOBAL: MW2MATROX 0x100bcdf8
// GLOBAL: MW23DFX 0x1024a250
// GLOBAL: MW2ATI 0x1017e098
MechChar g_smallMapExtraLabel[4] = "x";

// GLOBAL: MW2 0x100aabdc
// GLOBAL: MW2MATROX 0x100bcdfc
// GLOBAL: MW23DFX 0x1024a254
// GLOBAL: MW2ATI 0x1017e09c
MechChar g_smallMapRangeLabel[4] = "R: ";

// GLOBAL: MW2 0x100aabe0
// GLOBAL: MW2MATROX 0x100bce00
MechChar g_smallMapBearingLabel[12] = "Bearing: ";

// GLOBAL: MW2 0x100aabec
// GLOBAL: MW2MATROX 0x100bce0c
// GLOBAL: MW23DFX 0x1024a264
// GLOBAL: MW2ATI 0x1017e0ac
MechChar g_largeMapExtraLabel[4] = "x";

// GLOBAL: MW2 0x100aabf0
// GLOBAL: MW2MATROX 0x100bce10
// GLOBAL: MW23DFX 0x1024a268
// GLOBAL: MW2ATI 0x1017e0b0
MechChar g_largeMapRangeLabel[8] = "R: ";

// GLOBAL: MW2 0x100aabf8
// GLOBAL: MW2MATROX 0x100bce18
MechChar g_largeMapBearingLabel[12] = "Bearing: ";

// GLOBAL: MW2 0x100aac04
// GLOBAL: MW2MATROX 0x100bce24
// GLOBAL: MW23DFX 0x1024a27c
// GLOBAL: MW2ATI 0x1017e0c4
MechChar g_satelliteExtraLabel[4] = "x";

// GLOBAL: MW2 0x100aac08
// GLOBAL: MW2MATROX 0x100bce28
// GLOBAL: MW23DFX 0x1024a280
// GLOBAL: MW2ATI 0x1017e0c8
MechChar g_satelliteRangeLabel[8] = "Range: ";

// GLOBAL: MW2 0x100aac10
// GLOBAL: MW2MATROX 0x100bce30
// GLOBAL: MW23DFX 0x1024a288
// GLOBAL: MW2ATI 0x1017e0d0
MechChar g_bearingLabel[12] = "Bearing: ";

// GLOBAL: MW2 0x100aac1c
// GLOBAL: MW2MATROX 0x100bce3c
// GLOBAL: MW23DFX 0x1024a294
// GLOBAL: MW2ATI 0x1017e0dc
MechChar g_metersUnit[4] = "m";

// GLOBAL: MW2 0x100aac20
// GLOBAL: MW2MATROX 0x100bce40
// GLOBAL: MW23DFX 0x1024a298
// GLOBAL: MW2ATI 0x1017e0e0
MechChar g_kilometersUnit[8] = "km";

// The accelerated editions' map colors are other palette entries.
#ifdef MW2_BRIGHT_MAP_COLORS
// GLOBAL: MW2MATROX 0x100bce48
// GLOBAL: MW23DFX 0x1024a2a0
// GLOBAL: MW2ATI 0x1017e0e8
MechS32 g_smallMapColors[7][3] = {
	{0xb2, 0xb2, 0xb2},
	{0x7f, 0x7c, 0x82},
	{0x97, 0x94, 0x9a},
	{0xa0, 0x9d, 0xa3},
	{0x88, 0x8b, 0x88},
	{0xa6, 0x8e, 0xa6},
	{0xac, 0xa9, 0xaf}
};
#else
// GLOBAL: MW2 0x100aac28
MechS32 g_smallMapColors[7][3] = {
	{0xac, 0xac, 0xac},
	{0x7c, 0x79, 0x7f},
	{0x91, 0x8e, 0x94},
	{0x9a, 0x97, 0x9d},
	{0x85, 0x88, 0x85},
	{0xa0, 0x118, 0xa0},
	{0xa6, 0xa3, 0xa9}
};
#endif

#ifdef MW2_BRIGHT_MAP_COLORS
// GLOBAL: MW2MATROX 0x100bcea0
// GLOBAL: MW23DFX 0x1024a2f8
// GLOBAL: MW2ATI 0x1017e140
MechS32 g_largeMapColors[7][3] = {
	{0xb2, 0xb2, 0xb2},
	{0x7f, 0x7c, 0x82},
	{0x97, 0x94, 0x9a},
	{0xa0, 0x9d, 0xa3},
	{0x88, 0x8b, 0x88},
	{0xa6, 0x8e, 0xa6},
	{0xac, 0xa9, 0xaf}
};
#else
// GLOBAL: MW2 0x100aac80
MechS32 g_largeMapColors[7][3] = {
	{0xac, 0xac, 0xac},
	{0x7c, 0x79, 0x7f},
	{0x91, 0x8e, 0x94},
	{0x9a, 0x97, 0x9d},
	{0x85, 0x88, 0x85},
	{0xa0, 0x118, 0xa0},
	{0xa6, 0xa3, 0xa9}
};
#endif

#ifdef MW2_BRIGHT_MAP_COLORS
// GLOBAL: MW2MATROX 0x100bcef8
// GLOBAL: MW23DFX 0x1024a350
// GLOBAL: MW2ATI 0x1017e198
MechS32 g_satelliteColors[7][3] = {
	{0xb2, 0xb2, 0xb2},
	{0x7f, 0x7c, 0x82},
	{0x97, 0x94, 0x9a},
	{0xa0, 0x9d, 0xa3},
	{0x88, 0x8b, 0x88},
	{0xa6, 0x8e, 0xa6},
	{0xac, 0xa9, 0xaf}
};
#else
// GLOBAL: MW2 0x100aacd8
MechS32 g_satelliteColors[7][3] = {
	{0xac, 0xac, 0xac},
	{0x7c, 0x79, 0x7f},
	{0x91, 0x8e, 0x94},
	{0x9a, 0x97, 0x9d},
	{0x85, 0x88, 0x85},
	{0xa0, 0x118, 0xa0},
	{0xa6, 0xa3, 0xa9}
};
#endif

// GLOBAL: MW2 0x100aad30
// GLOBAL: MW2MATROX 0x100bcf50
// GLOBAL: MW23DFX 0x1024a3a8
// GLOBAL: MW2ATI 0x1017e1f0
MechS32 g_mapColors[13] = {0xe, 0xa, 6, 0xf, 0xb, 0xf5, 2, 3, 0xf9, 0xff, 0xf0, 1, 2};

// GLOBAL: MW2 0x100aad68
// GLOBAL: MW2MATROX 0x100bcf88
// GLOBAL: MW23DFX 0x1024a3e0
// GLOBAL: MW2ATI 0x1017e228
MechS32 g_smallMapAnims[5] = {-1, -1, -1, -1, -1};

// GLOBAL: MW2 0x100aad80
// GLOBAL: MW2MATROX 0x100bcfa0
// GLOBAL: MW23DFX 0x1024a3f8
// GLOBAL: MW2ATI 0x1017e240
MechS32 g_largeMapAnims[5] = {-1, -1, -1, -1, -1};

// GLOBAL: MW2 0x100aad98
// GLOBAL: MW2MATROX 0x100bcfb8
// GLOBAL: MW23DFX 0x1024a410
// GLOBAL: MW2ATI 0x1017e258
MechS32 g_satelliteAnims[5] = {-1, -1, -1, -1, -1};

// GLOBAL: MW2 0x100aadb0
// GLOBAL: MW23DFX 0x1024a428
// GLOBAL: MW2ATI 0x1017e270
PANE g_smallMapTransitionFirst = {NULL, 0x8000, 0x8000, 0x8000, 0x8000};

// GLOBAL: MW2 0x100aadc8
// GLOBAL: MW23DFX 0x1024a440
// GLOBAL: MW2ATI 0x1017e288
PANE g_smallMapTransitionSecond = {NULL, 0, 0, 0x10000, 0x10000};

// GLOBAL: MW2 0x100aade0
// GLOBAL: MW23DFX 0x1024a458
// GLOBAL: MW2ATI 0x1017e2a0
PANE g_smallMapTransitionRect = {NULL, 0, 0, 0, 0};

// GLOBAL: MW2 0x100aadf8
// GLOBAL: MW23DFX 0x1024a470
// GLOBAL: MW2ATI 0x1017e2b8
RectTransitionState g_mapTransitionState = {0, 0, 0};

// GLOBAL: MW2 0x100aae08
// GLOBAL: MW23DFX 0x1024a480
// GLOBAL: MW2ATI 0x1017e2c8
RectTransitionDef g_smallMapTransitionDef =
	{0xb5, &g_smallMapTransitionFirst, &g_smallMapTransitionSecond, &g_smallMapTransitionRect};

// GLOBAL: MW2 0x100aae18
// GLOBAL: MW23DFX 0x1024a490
// GLOBAL: MW2ATI 0x1017e2d8
RectTransition g_smallMapTransition = {&g_mapTransitionState, &g_smallMapTransitionDef};

// GLOBAL: MW2 0x100aae20
// GLOBAL: MW23DFX 0x1024a498
// GLOBAL: MW2ATI 0x1017e2e0
PANE g_largeMapTransitionFirst = {NULL, 0x8000, 0x8000, 0x8000, 0x8000};

// GLOBAL: MW2 0x100aae38
// GLOBAL: MW23DFX 0x1024a4b0
// GLOBAL: MW2ATI 0x1017e2f8
PANE g_largeMapTransitionSecond = {NULL, 0, 0, 0x10000, 0x10000};

// GLOBAL: MW2 0x100aae50
// GLOBAL: MW23DFX 0x1024a4c8
// GLOBAL: MW2ATI 0x1017e310
PANE g_largeMapTransitionRect = {NULL, 0, 0, 0, 0};

// GLOBAL: MW2 0x100aae68
// GLOBAL: MW23DFX 0x1024a4e0
// GLOBAL: MW2ATI 0x1017e328
RectTransitionDef g_largeMapTransitionDef =
	{0xb5, &g_largeMapTransitionFirst, &g_largeMapTransitionSecond, &g_largeMapTransitionRect};

// GLOBAL: MW2 0x100aae78
// GLOBAL: MW23DFX 0x1024a4f0
// GLOBAL: MW2ATI 0x1017e338
RectTransition g_largeMapTransition = {&g_mapTransitionState, &g_largeMapTransitionDef};

// GLOBAL: MW2 0x100aae80
// GLOBAL: MW23DFX 0x1024a4f8
// GLOBAL: MW2ATI 0x1017e340
PANE g_satelliteTransitionFirst = {NULL, 0x599a, 0x599a, 0xa666, 0xa666};

// GLOBAL: MW2 0x100aae98
// GLOBAL: MW23DFX 0x1024a510
// GLOBAL: MW2ATI 0x1017e358
PANE g_satelliteTransitionSecond = {NULL, 0, 0, 0x10000, 0x10000};

// GLOBAL: MW2 0x100aaeb0
// GLOBAL: MW23DFX 0x1024a528
// GLOBAL: MW2ATI 0x1017e370
PANE g_satelliteTransitionRect = {NULL, 0, 0, 0, 0};

// GLOBAL: MW2 0x100aaec8
// GLOBAL: MW23DFX 0x1024a540
// GLOBAL: MW2ATI 0x1017e388
RectTransitionState g_satelliteTransitionState = {0, 0, 0};

// GLOBAL: MW2 0x100aaed8
// GLOBAL: MW23DFX 0x1024a550
// GLOBAL: MW2ATI 0x1017e398
RectTransitionDef g_satelliteTransitionDef =
	{0x21f, &g_satelliteTransitionFirst, &g_satelliteTransitionSecond, &g_satelliteTransitionRect};

// GLOBAL: MW2 0x100aaee8
// GLOBAL: MW23DFX 0x1024a560
// GLOBAL: MW2ATI 0x1017e3a8
RectTransition g_satelliteTransition = {&g_satelliteTransitionState, &g_satelliteTransitionDef};

// GLOBAL: MW2 0x100aaef0
// GLOBAL: MW23DFX 0x1024a568
// GLOBAL: MW2ATI 0x1017e3b0
PANE g_smallMapViewport = {NULL, 0x51f, 0x51f, 0x428f, 0x570a};

// GLOBAL: MW2 0x100aaf08
// GLOBAL: MW2MATROX 0x100bd128
// GLOBAL: MW23DFX 0x1024a580
// GLOBAL: MW2ATI 0x1017e3c8
CockpitLayout g_smallMapLayout = {
	&g_smallMapViewport,
	&g_smallMapSavedViewport,
	8,
	{-1, -1},
	&g_smallMapTransition,
	0x30d40,
	-1,
	0x30d40,
	0xc350,
	0x61a80,
	0,
	1,
	g_smallMapExtraLabel,
	g_smallMapExtraText,
	g_smallMapRangeLabel,
	g_smallMapRangeText,
	g_bearingLabel,
	g_bearingText,
	-1,
	g_metersUnit,
	g_kilometersUnit,
	{0, 0},
	{0, 0xa3d},
	{0, 0},
	g_smallMapColors,
	g_mapColors,
	g_smallMapAnims,
#ifdef MW2_LAYOUT_GAUGE_FUNCTIONS
	{(CockpitGaugeFn) DrawGaugeEllipse,
	 (CockpitGaugeFn) IsInsideGaugeEllipse,
	 (CockpitGaugeFn) GetEllipseNeedleToward,
	 (CockpitGaugeFn) GetEllipseNeedleAt}
#else
	{(CockpitGaugeFn) 2, (CockpitGaugeFn) 4, (CockpitGaugeFn) 6, (CockpitGaugeFn) 8}
#endif
};

// GLOBAL: MW2 0x100aaf98
// GLOBAL: MW23DFX 0x1024a610
// GLOBAL: MW2ATI 0x1017e458
PANE g_largeMapViewport = {NULL, 0x2148, 0, 0xdeb8, 0x10000};

// GLOBAL: MW2 0x100aafb0
// GLOBAL: MW2MATROX 0x100bd1d0
// GLOBAL: MW23DFX 0x1024a628
// GLOBAL: MW2ATI 0x1017e470
CockpitLayout g_largeMapLayout = {
	&g_largeMapViewport,
	&g_largeMapSavedViewport,
	9,
	{-1, -1},
	&g_largeMapTransition,
	0x30d40,
	-1,
	0x30d40,
	0xc350,
	0x61a80,
	0,
	1,
	g_largeMapExtraLabel,
	g_largeMapExtraText,
	g_largeMapRangeLabel,
	g_largeMapRangeText,
	g_bearingLabel,
	g_bearingText,
	-1,
	g_metersUnit,
	g_kilometersUnit,
	{0, 0},
	{0x147b, 0x2148},
	{0, 0},
	g_largeMapColors,
	g_mapColors,
	g_largeMapAnims,
#ifdef MW2_LAYOUT_GAUGE_FUNCTIONS
	{(CockpitGaugeFn) DrawGaugeEllipse,
	 (CockpitGaugeFn) IsInsideGaugeEllipse,
	 (CockpitGaugeFn) GetEllipseNeedleToward,
	 (CockpitGaugeFn) GetEllipseNeedleAt}
#else
	{(CockpitGaugeFn) 2, (CockpitGaugeFn) 4, (CockpitGaugeFn) 6, (CockpitGaugeFn) 8}
#endif
};

// GLOBAL: MW2 0x100ab040
// GLOBAL: MW23DFX 0x1024a6b8
// GLOBAL: MW2ATI 0x1017e500
PANE g_satelliteViewport = {NULL, 0, 0, 0x10000, 0x10000};

// GLOBAL: MW2 0x100ab058
// GLOBAL: MW2MATROX 0x100bd278
// GLOBAL: MW23DFX 0x1024a6d0
// GLOBAL: MW2ATI 0x1017e518
CockpitLayout g_satelliteLayout = {
	&g_satelliteViewport,
	&g_satelliteSavedViewport,
	10,
	{0xe8, 0xe8},
	&g_satelliteTransition,
	0x186a0,
	-1,
	0x186a0,
	0x186a,
	0x186a0,
	0,
	1,
	g_satelliteExtraLabel,
	g_satelliteExtraText,
	g_satelliteRangeLabel,
	g_satelliteRangeText,
	g_bearingLabel,
	g_bearingText,
	-1,
	g_metersUnit,
	g_kilometersUnit,
	{0, 0},
	{0x28f, 0x28f},
	{0x28f, 0xccd},
	g_satelliteColors,
	g_mapColors,
	g_satelliteAnims,
#ifdef MW2_LAYOUT_GAUGE_FUNCTIONS
	{NULL, NULL, (CockpitGaugeFn) GetRectNeedleToward, (CockpitGaugeFn) GetRectNeedleAt}
#else
	{(CockpitGaugeFn) 0, (CockpitGaugeFn) 3, (CockpitGaugeFn) 5, (CockpitGaugeFn) 7}
#endif
};

// The layout of each cockpit view, NULL where it has none.
// GLOBAL: MW2 0x100ab0e8
// GLOBAL: MW2MATROX 0x100bd308
// GLOBAL: MW23DFX 0x1024a760
// GLOBAL: MW2ATI 0x1017e5a8
CockpitLayout* g_cockpitLayouts[6] = {NULL, &g_smallMapLayout, &g_largeMapLayout, NULL, &g_satelliteLayout, NULL};
