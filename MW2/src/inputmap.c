/* The input devices and INPUT.MAP. The unit is not named input.c: the CRT has an input.c (the
   scanf engine) whose line records reccmp confuses with it. */
#include "inputmap.h"

#include "analogbinding.h"
#include "clock.h"
#include "decomp.h"
#include "discretebinding.h"
#include "error.h"
#include "fixedfloat.h"
#include "gamekeymodifier.h"
#include "gamekeyname.h"
#include "inputaxis.h"
#include "inputcondition.h"
#include "inputdeviceinfo.h"
#include "inputdevicestate.h"
#include "inputdriver.h"
#include "inputsink.h"
#include "joystick.h"
#include "keyboard.h"
#include "mouse.h"
#include "mw2log.h"
#include "refreshmode.h"
#include "simmain.h"
#include "statuspanels.h"
#include "types.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// The local player's steering: the outputs of INPUT.MAP's sinks. The Matrox edition leaves it and
// the sinks below uninitialized (in .bss), where 1.1 initializes them.
#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4c0
// GLOBAL: MW23DFX 0x10159e50
// GLOBAL: MW2ATI 0x1008de50
// GLOBAL: MW2S3 0x100ab570
PlayerSteering g_localSteering;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b0650
PlayerSteering g_localSteering = {0};
#else
// GLOBAL: MW2 0x100b2500
// GLOBAL: MW2GBL 0x100a5800
// GLOBAL: MW2PREMERCS 0x100b6540
PlayerSteering g_localSteering = {0};
#endif

// The outputs of INPUT.MAP's sinks past the steering. pilot_tilt and pilot_pan are the cockpit
// view's look offsets, zoom_factor the view scale (16.16, reset to 1 by FirstEyepoint), and the
// glance sinks look aside while one is held.
#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f480
// GLOBAL: MW23DFX 0x10159e10
// GLOBAL: MW2ATI 0x1008de10
// GLOBAL: MW2S3 0x100ab530
MechScalar g_sinkPilotTilt;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b0698
MechScalar g_sinkPilotTilt = 0;
#else
// GLOBAL: MW2 0x100b2548
// GLOBAL: MW2GBL 0x100a5848
// GLOBAL: MW2PREMERCS 0x100b6588
MechS32 g_sinkPilotTilt = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f484
// GLOBAL: MW23DFX 0x10159e14
// GLOBAL: MW2ATI 0x1008de14
// GLOBAL: MW2S3 0x100ab534
MechScalar g_sinkPilotPan;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b069c
MechScalar g_sinkPilotPan = 0;
#else
// GLOBAL: MW2 0x100b254c
// GLOBAL: MW2GBL 0x100a584c
// GLOBAL: MW2PREMERCS 0x100b658c
MechS32 g_sinkPilotPan = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f488
// GLOBAL: MW23DFX 0x10159e18
// GLOBAL: MW2ATI 0x1008de18
// GLOBAL: MW2S3 0x100ab538
MechScalar g_sinkEyepointTilt;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06a0
MechScalar g_sinkEyepointTilt = 0;
#else
// GLOBAL: MW2 0x100b2550
// GLOBAL: MW2GBL 0x100a5850
// GLOBAL: MW2PREMERCS 0x100b6590
MechS32 g_sinkEyepointTilt = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f48c
// GLOBAL: MW23DFX 0x10159e1c
// GLOBAL: MW2ATI 0x1008de1c
// GLOBAL: MW2S3 0x100ab53c
MechScalar g_sinkEyepointPanDelta;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06a4
MechScalar g_sinkEyepointPanDelta = 0;
#else
// GLOBAL: MW2 0x100b2554
// GLOBAL: MW2GBL 0x100a5854
// GLOBAL: MW2PREMERCS 0x100b6594
MechS32 g_sinkEyepointPanDelta = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f490
// GLOBAL: MW23DFX 0x10159e20
// GLOBAL: MW2ATI 0x1008de20
// GLOBAL: MW2S3 0x100ab540
MechScalar g_sinkEyepointSlideDelta;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06a8
MechScalar g_sinkEyepointSlideDelta = 0;
#else
// GLOBAL: MW2 0x100b2558
// GLOBAL: MW2GBL 0x100a5858
// GLOBAL: MW2PREMERCS 0x100b6598
MechS32 g_sinkEyepointSlideDelta = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f494
// GLOBAL: MW23DFX 0x10159e24
// GLOBAL: MW2ATI 0x1008de24
// GLOBAL: MW2S3 0x100ab544
MechScalar g_sinkTrackDistanceDelta;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06ac
MechScalar g_sinkTrackDistanceDelta = 0;
#else
// GLOBAL: MW2 0x100b255c
// GLOBAL: MW2GBL 0x100a585c
// GLOBAL: MW2PREMERCS 0x100b659c
MechS32 g_sinkTrackDistanceDelta = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f498
// GLOBAL: MW23DFX 0x10159e28
// GLOBAL: MW2ATI 0x1008de28
// GLOBAL: MW2S3 0x100ab548
MechScalar g_sinkTrackHeightDelta;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06b0
MechScalar g_sinkTrackHeightDelta = 0;
#else
// GLOBAL: MW2 0x100b2560
// GLOBAL: MW2GBL 0x100a5860
// GLOBAL: MW2PREMERCS 0x100b65a0
MechS32 g_sinkTrackHeightDelta = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f49c
// GLOBAL: MW23DFX 0x10159e2c
// GLOBAL: MW2ATI 0x1008de2c
// GLOBAL: MW2S3 0x100ab54c
MechScalar g_sinkZoomFactor;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06b4
MechScalar g_sinkZoomFactor = 0;
#else
// GLOBAL: MW2 0x100b2564
// GLOBAL: MW2GBL 0x100a5864
// GLOBAL: MW2PREMERCS 0x100b65a4
MechS32 g_sinkZoomFactor = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4a0
// GLOBAL: MW23DFX 0x10159e30
// GLOBAL: MW2ATI 0x1008de30
// GLOBAL: MW2S3 0x100ab550
MechS8 g_sinkPilotTiltPlus;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06b8
MechS8 g_sinkPilotTiltPlus = 0;
#else
// GLOBAL: MW2 0x100b2568
// GLOBAL: MW2GBL 0x100a5868
// GLOBAL: MW2PREMERCS 0x100b65a8
MechS8 g_sinkPilotTiltPlus = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4a1
// GLOBAL: MW23DFX 0x10159e31
// GLOBAL: MW2ATI 0x1008de31
// GLOBAL: MW2S3 0x100ab551
MechS8 g_sinkPilotTiltMinus;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06b9
MechS8 g_sinkPilotTiltMinus = 0;
#else
// GLOBAL: MW2 0x100b2569
// GLOBAL: MW2GBL 0x100a5869
// GLOBAL: MW2PREMERCS 0x100b65a9
MechS8 g_sinkPilotTiltMinus = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4a2
// GLOBAL: MW23DFX 0x10159e32
// GLOBAL: MW2ATI 0x1008de32
// GLOBAL: MW2S3 0x100ab552
MechS8 g_sinkPilotTiltReset;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06ba
MechS8 g_sinkPilotTiltReset = 0;
#else
// GLOBAL: MW2 0x100b256a
// GLOBAL: MW2GBL 0x100a586a
// GLOBAL: MW2PREMERCS 0x100b65aa
MechS8 g_sinkPilotTiltReset = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4a3
// GLOBAL: MW23DFX 0x10159e33
// GLOBAL: MW2ATI 0x1008de33
// GLOBAL: MW2S3 0x100ab553
MechS8 g_sinkPilotPanPlus;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06bb
MechS8 g_sinkPilotPanPlus = 0;
#else
// GLOBAL: MW2 0x100b256b
// GLOBAL: MW2GBL 0x100a586b
// GLOBAL: MW2PREMERCS 0x100b65ab
MechS8 g_sinkPilotPanPlus = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4a4
// GLOBAL: MW23DFX 0x10159e34
// GLOBAL: MW2ATI 0x1008de34
// GLOBAL: MW2S3 0x100ab554
MechS8 g_sinkPilotPanMinus;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06bc
MechS8 g_sinkPilotPanMinus = 0;
#else
// GLOBAL: MW2 0x100b256c
// GLOBAL: MW2GBL 0x100a586c
// GLOBAL: MW2PREMERCS 0x100b65ac
MechS8 g_sinkPilotPanMinus = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4a5
// GLOBAL: MW23DFX 0x10159e35
// GLOBAL: MW2ATI 0x1008de35
// GLOBAL: MW2S3 0x100ab555
MechS8 g_sinkPilotPanReset;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06bd
MechS8 g_sinkPilotPanReset = 0;
#else
// GLOBAL: MW2 0x100b256d
// GLOBAL: MW2GBL 0x100a586d
// GLOBAL: MW2PREMERCS 0x100b65ad
MechS8 g_sinkPilotPanReset = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4a6
// GLOBAL: MW23DFX 0x10159e36
// GLOBAL: MW2ATI 0x1008de36
// GLOBAL: MW2S3 0x100ab556
volatile MechS8 g_sinkGlanceLeft;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06be
volatile MechS8 g_sinkGlanceLeft = 0;
#else
// GLOBAL: MW2 0x100b256e
// GLOBAL: MW2GBL 0x100a586e
// GLOBAL: MW2PREMERCS 0x100b65ae
volatile MechS8 g_sinkGlanceLeft = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4a7
// GLOBAL: MW23DFX 0x10159e37
// GLOBAL: MW2ATI 0x1008de37
// GLOBAL: MW2S3 0x100ab557
volatile MechS8 g_sinkGlanceRight;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06bf
volatile MechS8 g_sinkGlanceRight = 0;
#else
// GLOBAL: MW2 0x100b256f
// GLOBAL: MW2GBL 0x100a586f
// GLOBAL: MW2PREMERCS 0x100b65af
volatile MechS8 g_sinkGlanceRight = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4a8
// GLOBAL: MW23DFX 0x10159e38
// GLOBAL: MW2ATI 0x1008de38
// GLOBAL: MW2S3 0x100ab558
volatile MechS8 g_sinkGlanceUp;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06c0
volatile MechS8 g_sinkGlanceUp = 0;
#else
// GLOBAL: MW2 0x100b2570
// GLOBAL: MW2GBL 0x100a5870
// GLOBAL: MW2PREMERCS 0x100b65b0
volatile MechS8 g_sinkGlanceUp = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4a9
// GLOBAL: MW23DFX 0x10159e39
// GLOBAL: MW2ATI 0x1008de39
// GLOBAL: MW2S3 0x100ab559
volatile MechS8 g_sinkGlanceDown;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06c1
volatile MechS8 g_sinkGlanceDown = 0;
#else
// GLOBAL: MW2 0x100b2571
// GLOBAL: MW2GBL 0x100a5871
// GLOBAL: MW2PREMERCS 0x100b65b1
volatile MechS8 g_sinkGlanceDown = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4aa
// GLOBAL: MW23DFX 0x10159e3a
// GLOBAL: MW2ATI 0x1008de3a
// GLOBAL: MW2S3 0x100ab55a
MechS8 g_sinkEyepointTiltPlus;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06c2
MechS8 g_sinkEyepointTiltPlus = 0;
#else
// GLOBAL: MW2 0x100b2572
// GLOBAL: MW2GBL 0x100a5872
// GLOBAL: MW2PREMERCS 0x100b65b2
MechS8 g_sinkEyepointTiltPlus = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4ab
// GLOBAL: MW23DFX 0x10159e3b
// GLOBAL: MW2ATI 0x1008de3b
// GLOBAL: MW2S3 0x100ab55b
MechS8 g_sinkEyepointTiltMinus;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06c3
MechS8 g_sinkEyepointTiltMinus = 0;
#else
// GLOBAL: MW2 0x100b2573
// GLOBAL: MW2GBL 0x100a5873
// GLOBAL: MW2PREMERCS 0x100b65b3
MechS8 g_sinkEyepointTiltMinus = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4ac
// GLOBAL: MW23DFX 0x10159e3c
// GLOBAL: MW2ATI 0x1008de3c
// GLOBAL: MW2S3 0x100ab55c
MechS8 g_sinkEyepointTiltReset;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06c4
MechS8 g_sinkEyepointTiltReset = 0;
#else
// GLOBAL: MW2 0x100b2574
// GLOBAL: MW2GBL 0x100a5874
// GLOBAL: MW2PREMERCS 0x100b65b4
MechS8 g_sinkEyepointTiltReset = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4ad
// GLOBAL: MW23DFX 0x10159e3d
// GLOBAL: MW2ATI 0x1008de3d
// GLOBAL: MW2S3 0x100ab55d
MechS8 g_sinkEyepointPanPlus;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06c5
MechS8 g_sinkEyepointPanPlus = 0;
#else
// GLOBAL: MW2 0x100b2575
// GLOBAL: MW2GBL 0x100a5875
// GLOBAL: MW2PREMERCS 0x100b65b5
MechS8 g_sinkEyepointPanPlus = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4ae
// GLOBAL: MW23DFX 0x10159e3e
// GLOBAL: MW2ATI 0x1008de3e
// GLOBAL: MW2S3 0x100ab55e
MechS8 g_sinkEyepointPanMinus;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06c6
MechS8 g_sinkEyepointPanMinus = 0;
#else
// GLOBAL: MW2 0x100b2576
// GLOBAL: MW2GBL 0x100a5876
// GLOBAL: MW2PREMERCS 0x100b65b6
MechS8 g_sinkEyepointPanMinus = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4af
// GLOBAL: MW23DFX 0x10159e3f
// GLOBAL: MW2ATI 0x1008de3f
// GLOBAL: MW2S3 0x100ab55f
MechS8 g_sinkEyepointPanReset;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06c7
MechS8 g_sinkEyepointPanReset = 0;
#else
// GLOBAL: MW2 0x100b2577
// GLOBAL: MW2GBL 0x100a5877
// GLOBAL: MW2PREMERCS 0x100b65b7
MechS8 g_sinkEyepointPanReset = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4b0
// GLOBAL: MW23DFX 0x10159e40
// GLOBAL: MW2ATI 0x1008de40
// GLOBAL: MW2S3 0x100ab560
MechS8 g_sinkEyepointSlidePlus;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06c8
MechS8 g_sinkEyepointSlidePlus = 0;
#else
// GLOBAL: MW2 0x100b2578
// GLOBAL: MW2GBL 0x100a5878
// GLOBAL: MW2PREMERCS 0x100b65b8
MechS8 g_sinkEyepointSlidePlus = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4b1
// GLOBAL: MW23DFX 0x10159e41
// GLOBAL: MW2ATI 0x1008de41
// GLOBAL: MW2S3 0x100ab561
MechS8 g_sinkEyepointSlideMinus;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06c9
MechS8 g_sinkEyepointSlideMinus = 0;
#else
// GLOBAL: MW2 0x100b2579
// GLOBAL: MW2GBL 0x100a5879
// GLOBAL: MW2PREMERCS 0x100b65b9
MechS8 g_sinkEyepointSlideMinus = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4b2
// GLOBAL: MW23DFX 0x10159e42
// GLOBAL: MW2ATI 0x1008de42
// GLOBAL: MW2S3 0x100ab562
MechS8 g_sinkTrackDistancePlus;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06ca
MechS8 g_sinkTrackDistancePlus = 0;
#else
// GLOBAL: MW2 0x100b257a
// GLOBAL: MW2GBL 0x100a587a
// GLOBAL: MW2PREMERCS 0x100b65ba
MechS8 g_sinkTrackDistancePlus = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4b3
// GLOBAL: MW23DFX 0x10159e43
// GLOBAL: MW2ATI 0x1008de43
// GLOBAL: MW2S3 0x100ab563
MechS8 g_sinkTrackDistanceMinus;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06cb
MechS8 g_sinkTrackDistanceMinus = 0;
#else
// GLOBAL: MW2 0x100b257b
// GLOBAL: MW2GBL 0x100a587b
// GLOBAL: MW2PREMERCS 0x100b65bb
MechS8 g_sinkTrackDistanceMinus = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4b4
// GLOBAL: MW23DFX 0x10159e44
// GLOBAL: MW2ATI 0x1008de44
// GLOBAL: MW2S3 0x100ab564
MechS8 g_sinkTrackHeightPlus;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06cc
MechS8 g_sinkTrackHeightPlus = 0;
#else
// GLOBAL: MW2 0x100b257c
// GLOBAL: MW2GBL 0x100a587c
// GLOBAL: MW2PREMERCS 0x100b65bc
MechS8 g_sinkTrackHeightPlus = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4b5
// GLOBAL: MW23DFX 0x10159e45
// GLOBAL: MW2ATI 0x1008de45
// GLOBAL: MW2S3 0x100ab565
MechS8 g_sinkTrackHeightMinus;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06cd
MechS8 g_sinkTrackHeightMinus = 0;
#else
// GLOBAL: MW2 0x100b257d
// GLOBAL: MW2GBL 0x100a587d
// GLOBAL: MW2PREMERCS 0x100b65bd
MechS8 g_sinkTrackHeightMinus = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4b6
// GLOBAL: MW23DFX 0x10159e46
// GLOBAL: MW2ATI 0x1008de46
// GLOBAL: MW2S3 0x100ab566
MechS8 g_sinkZoomFactorPlus;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06ce
MechS8 g_sinkZoomFactorPlus = 0;
#else
// GLOBAL: MW2 0x100b257e
// GLOBAL: MW2GBL 0x100a587e
// GLOBAL: MW2PREMERCS 0x100b65be
MechS8 g_sinkZoomFactorPlus = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4b7
// GLOBAL: MW23DFX 0x10159e47
// GLOBAL: MW2ATI 0x1008de47
// GLOBAL: MW2S3 0x100ab567
MechS8 g_sinkZoomFactorMinus;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06cf
MechS8 g_sinkZoomFactorMinus = 0;
#else
// GLOBAL: MW2 0x100b257f
// GLOBAL: MW2GBL 0x100a587f
// GLOBAL: MW2PREMERCS 0x100b65bf
MechS8 g_sinkZoomFactorMinus = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f4b8
// GLOBAL: MW23DFX 0x10159e48
// GLOBAL: MW2ATI 0x1008de48
// GLOBAL: MW2S3 0x100ab568
MechS8 g_sinkZoomFactorReset;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06d0
MechS8 g_sinkZoomFactorReset = 0;
#else
// GLOBAL: MW2 0x100b2580
// GLOBAL: MW2GBL 0x100a5880
// GLOBAL: MW2PREMERCS 0x100b65c0
MechS8 g_sinkZoomFactorReset = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f470
// GLOBAL: MW23DFX 0x10159e00
// GLOBAL: MW2ATI 0x1008de00
// GLOBAL: MW2S3 0x100ab5c0
MechScalar g_sinkMenuItem;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06d8
MechScalar g_sinkMenuItem = 0;
#else
// GLOBAL: MW2 0x100b2588
// GLOBAL: MW2GBL 0x100a5888
// GLOBAL: MW2PREMERCS 0x100b65c8
MechScalar g_sinkMenuItem = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f474
// GLOBAL: MW23DFX 0x10159e04
// GLOBAL: MW2ATI 0x1008de04
// GLOBAL: MW2S3 0x100ab5c4
MechScalar g_sinkMenuValue;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06dc
MechScalar g_sinkMenuValue = 0;
#else
// GLOBAL: MW2 0x100b258c
// GLOBAL: MW2GBL 0x100a588c
// GLOBAL: MW2PREMERCS 0x100b65cc
MechScalar g_sinkMenuValue = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f47a
// GLOBAL: MW23DFX 0x10159e0a
// GLOBAL: MW2ATI 0x1008de0a
// GLOBAL: MW2S3 0x100ab5ca
MechS8 g_sinkMenuItemReset;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06e2
MechS8 g_sinkMenuItemReset = 0;
#else
// GLOBAL: MW2 0x100b2592
// GLOBAL: MW2GBL 0x100a5892
// GLOBAL: MW2PREMERCS 0x100b65d2
MechS8 g_sinkMenuItemReset = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f47d
// GLOBAL: MW23DFX 0x10159e0d
// GLOBAL: MW2ATI 0x1008de0d
// GLOBAL: MW2S3 0x100ab5cd
MechS8 g_sinkMenuValueReset;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06e5
MechS8 g_sinkMenuValueReset = 0;
#else
// GLOBAL: MW2 0x100b2595
// GLOBAL: MW2GBL 0x100a5895
// GLOBAL: MW2PREMERCS 0x100b65d5
MechS8 g_sinkMenuValueReset = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f47e
// GLOBAL: MW23DFX 0x10159e0e
// GLOBAL: MW2ATI 0x1008de0e
// GLOBAL: MW2S3 0x100ab5ce
MechS8 g_sinkMenuEnter;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06e6
MechS8 g_sinkMenuEnter = 0;
#else
// GLOBAL: MW2 0x100b2596
// GLOBAL: MW2GBL 0x100a5896
// GLOBAL: MW2PREMERCS 0x100b65d6
MechS8 g_sinkMenuEnter = 0;
#endif

#if defined(MW2_ACCEL) && !defined(MW2_POWERVR)
// GLOBAL: MW2MATROX 0x1012f47f
// GLOBAL: MW23DFX 0x10159e0f
// GLOBAL: MW2ATI 0x1008de0f
// GLOBAL: MW2S3 0x100ab5cf
MechS8 g_sinkMenuAbort;
#elif defined(MW2_POWERVR)
// GLOBAL: MW2POWERVR 0x101b06e7
MechS8 g_sinkMenuAbort = 0;
#else
// GLOBAL: MW2 0x100b2597
// GLOBAL: MW2GBL 0x100a5897
// GLOBAL: MW2PREMERCS 0x100b65d7
MechS8 g_sinkMenuAbort = 0;
#endif

// GLOBAL: MW2 0x100b2598
// GLOBAL: MW2GBL 0x100a5898
// GLOBAL: MW2PREMERCS 0x100b65d8
// GLOBAL: MW2MATROX 0x100b1fc0
// GLOBAL: MW23DFX 0x10251170
// GLOBAL: MW2ATI 0x10185480
// GLOBAL: MW2S3 0x1006af78
// GLOBAL: MW2POWERVR 0x101b06e8
MechS32 g_analogBindingCount = 0;

// GLOBAL: MW2 0x100b259c
// GLOBAL: MW2GBL 0x100a589c
// GLOBAL: MW2PREMERCS 0x100b65dc
// GLOBAL: MW2MATROX 0x100b1fc4
// GLOBAL: MW23DFX 0x10251174
// GLOBAL: MW2ATI 0x10185484
// GLOBAL: MW2S3 0x1006af7c
// GLOBAL: MW2POWERVR 0x101b06ec
MechS32 g_inputAxisCount = 0;

// GLOBAL: MW2 0x100b25a0
// GLOBAL: MW2GBL 0x100a58a0
// GLOBAL: MW2PREMERCS 0x100b65e0
// GLOBAL: MW2MATROX 0x100b1fc8
// GLOBAL: MW23DFX 0x10251178
// GLOBAL: MW2ATI 0x10185488
// GLOBAL: MW2S3 0x1006af80
// GLOBAL: MW2POWERVR 0x101b06f0
MechS32 g_discreteBindingCount = 0;

// GLOBAL: MW2 0x100b25a4
// GLOBAL: MW2GBL 0x100a58a4
// GLOBAL: MW2PREMERCS 0x100b65e4
// GLOBAL: MW2MATROX 0x100b1fcc
// GLOBAL: MW23DFX 0x1025117c
// GLOBAL: MW2ATI 0x1018548c
// GLOBAL: MW2S3 0x1006af84
// GLOBAL: MW2POWERVR 0x101b06f4
MechS32 g_gameplayInputEnabled = 1;

// GLOBAL: MW2 0x100b25a8
// GLOBAL: MW2GBL 0x100a58a8
// GLOBAL: MW2PREMERCS 0x100b65e8
// GLOBAL: MW2MATROX 0x100b1fd0
// GLOBAL: MW23DFX 0x10251180
// GLOBAL: MW2ATI 0x10185490
// GLOBAL: MW2S3 0x1006af88
// GLOBAL: MW2POWERVR 0x101b06f8
MechS32 g_keyboardDeviceIndex = 0;

// The line of INPUT.MAP being parsed.
// GLOBAL: MW2 0x100b25ac
// GLOBAL: MW2GBL 0x100a58ac
// GLOBAL: MW2PREMERCS 0x100b65ec
// GLOBAL: MW2MATROX 0x100b1fd4
// GLOBAL: MW23DFX 0x10251184
// GLOBAL: MW2ATI 0x10185494
// GLOBAL: MW2S3 0x1006af8c
// GLOBAL: MW2POWERVR 0x101b06fc
MechS32 g_inputMapLine = 0;

// GLOBAL: MW2 0x100b25b0
// GLOBAL: MW2GBL 0x100a58b0
// GLOBAL: MW2PREMERCS 0x100b65f0
// GLOBAL: MW2MATROX 0x100b1fd8
// GLOBAL: MW23DFX 0x10251188
// GLOBAL: MW2ATI 0x10185498
// GLOBAL: MW2S3 0x1006af90
// GLOBAL: MW2POWERVR 0x101b0700
AnalogBinding g_analogBindings[50] = {0};

// GLOBAL: MW2 0x100b3eb0
// GLOBAL: MW2GBL 0x100a71b0
// GLOBAL: MW2PREMERCS 0x100b7ef0
// GLOBAL: MW2MATROX 0x100b38d8
// GLOBAL: MW23DFX 0x10252a88
// GLOBAL: MW2ATI 0x10186d98
// GLOBAL: MW2S3 0x1006c890
// GLOBAL: MW2POWERVR 0x101b2000
InputAxis g_inputAxes[50] = {0};

// GLOBAL: MW2 0x100b4878
// GLOBAL: MW2GBL 0x100a7b78
// GLOBAL: MW2PREMERCS 0x100b88b8
// GLOBAL: MW2MATROX 0x100b42a0
// GLOBAL: MW23DFX 0x10253450
// GLOBAL: MW2ATI 0x10187760
// GLOBAL: MW2S3 0x1006d258
// GLOBAL: MW2POWERVR 0x101b29c8
DiscreteBinding g_discreteBindings[100] = {0};

// GLOBAL: MW2 0x100b7370
// GLOBAL: MW2GBL 0x100aa670
// GLOBAL: MW2PREMERCS 0x100bb3b0
// GLOBAL: MW2MATROX 0x100b6d98
// GLOBAL: MW23DFX 0x10255f48
// GLOBAL: MW2ATI 0x1018a258
// GLOBAL: MW2S3 0x1006fd50
// GLOBAL: MW2POWERVR 0x101b54c0
MechS32 g_inputDeviceCount = 0;

// The input driver classes, in the order RegisterInputDevice asks them for a device.
// GLOBAL: MW2 0x100b7378
// GLOBAL: MW2GBL 0x100aa678
// GLOBAL: MW2PREMERCS 0x100bb3b8
// GLOBAL: MW2MATROX 0x100b6da0
// GLOBAL: MW23DFX 0x10255f50
// GLOBAL: MW2ATI 0x1018a260
// GLOBAL: MW2S3 0x1006fd58
// GLOBAL: MW2POWERVR 0x101b54c8
InputDriverModule* g_inputDriverClasses[3] = {&g_keyboardDriver, &g_mouseDriver, &g_joystickDriver};

// The sinks INPUT.MAP can bind.
// GLOBAL: MW2 0x100b7388
// GLOBAL: MW2GBL 0x100aa688
// GLOBAL: MW2MATROX 0x100b6db0
// GLOBAL: MW23DFX 0x10256540
// GLOBAL: MW2ATI 0x1018a850
// GLOBAL: MW2S3 0x1006fd68
// GLOBAL: MW2POWERVR 0x101b5ab8
InputSink g_inputSinks[] = {
	{"torso_tilt", &g_localSteering.m_torsoTilt, 0, 0, -60, 60, 4, 0},
	{"torso_tilt_plus", &g_localSteering.m_torsoTiltPlus, 1, 0, 0, 0, 0, 0},
	{"torso_tilt_minus", &g_localSteering.m_torsoTiltMinus, 1, 0, 0, 0, 0, 0},
	{"torso_tilt_reset", &g_localSteering.m_torsoTiltReset, 1, 0, 0, 0, 0, 0},
#ifdef MW2_NARROW_TORSO_PAN
	// The 3Dfx edition has other ranges for the torso pan and the throttle.
	{"torso_pan", &g_localSteering.m_torsoPan, 0, 0, -90, 90, 3, 0},
#else
	{"torso_pan", &g_localSteering.m_torsoPan, 0, 0, -140, 140, 3, 0},
#endif
	{"torso_pan_plus", &g_localSteering.m_torsoPanPlus, 1, 0, 0, 0, 0, 0},
	{"torso_pan_minus", &g_localSteering.m_torsoPanMinus, 1, 0, 0, 0, 0, 0},
	{"torso_pan_set", &g_localSteering.m_torsoPanSet, 1, 0, 0, 0, 0, 0},
	{"torso_pan_reset", &g_localSteering.m_torsoPanReset, 1, 0, 0, 0, 0, 0},
#ifdef MW2_NO_REVERSE_THROTTLE
	{"throttle", &g_localSteering.m_throttle, 0, 0, 0, 1024, 6, 16},
#else
	{"throttle", &g_localSteering.m_throttle, 0, 0, -120, 1082, 6, 16},
#endif
	{"legs_pan_delta", &g_localSteering.m_legsPanDelta, 0, 0, -1024, 1024, 7, 0},
	{"jumpjet_enabled", &g_localSteering.m_jumpJetEnabled, 1, 0, 0, 0, 0, 0},
	{"jumpjet_fire_left", &g_localSteering.m_jumpJetFireLeft, 1, 0, 0, 0, 0, 0},
	{"jumpjet_fire_right", &g_localSteering.m_jumpJetFireRight, 1, 0, 0, 0, 0, 0},
	{"jumpjet_fire_forward", &g_localSteering.m_jumpJetFireForward, 1, 0, 0, 0, 0, 0},
	{"jumpjet_fire_backward", &g_localSteering.m_jumpJetFireBackward, 1, 0, 0, 0, 0, 0},
	{"throttle_plus", &g_localSteering.m_throttlePlus, 1, 0, 0, 0, 0, 0},
	{"throttle_minus", &g_localSteering.m_throttleMinus, 1, 0, 0, 0, 0, 0},
	{"throttle_set", &g_localSteering.m_throttleSet, 1, 0, 0, 0, 0, 0},
	{"weapon_fire", &g_localSteering.m_weaponFire, 1, 0, 0, 0, 0, 0},
	{"weapon_cycle", &g_localSteering.m_weaponCycle, 2, 0, 0, 0, 0, 0},
	{"weapon_fire_group", &g_localSteering.m_weaponFireGroup, 1, 0, 0, 0, 0, 0},
	{"weapon_fire_group_1", &g_localSteering.m_weaponFireGroup1, 1, 0, 0, 0, 0, 0},
	{"weapon_fire_group_2", &g_localSteering.m_weaponFireGroup2, 1, 0, 0, 0, 0, 0},
	{"weapon_fire_group_3", &g_localSteering.m_weaponFireGroup3, 1, 0, 0, 0, 0, 0},
	{"weapon_cycle_group", &g_localSteering.m_weaponCycleGroup, 2, 0, 0, 0, 0, 0},
	{"toggle_group_fire", &g_localSteering.m_toggleGroupFire, 2, 0, 0, 0, 0, 0},
	{"legs_pan_minus", &g_localSteering.m_legsPanMinus, 1, 0, 0, 0, 0, 0},
	{"legs_pan_plus", &g_localSteering.m_legsPanPlus, 1, 0, 0, 0, 0, 0},
	{"advance_nav", &g_localSteering.m_advanceNav, 2, 0, 0, 0, 0, 0},
	{"previous_nav", &g_localSteering.m_previousNav, 2, 0, 0, 0, 0, 0},
	{"reset_nav", &g_localSteering.m_resetNav, 2, 0, 0, 0, 0, 0},
	{"advance_target", &g_localSteering.m_advanceTarget, 2, 0, 0, 0, 0, 0},
	{"previous_target", &g_localSteering.m_previousTarget, 2, 0, 0, 0, 0, 0},
	{"reset_target", &g_localSteering.m_resetTarget, 2, 0, 0, 0, 0, 0},
	{"target_reticle", &g_localSteering.m_targetReticle, 2, 0, 0, 0, 0, 0},
	{"target_friendly", &g_localSteering.m_targetFriendly, 2, 0, 0, 0, 0, 0},
	{"nearest_enemy", &g_localSteering.m_nearestEnemy, 2, 0, 0, 0, 0, 0},
	{"target_last_shot", &g_localSteering.m_targetLastShot, 2, 0, 0, 0, 0, 0},
	{"inspect_target", &g_localSteering.m_inspectTarget, 2, 0, 0, 0, 0, 0},
	{"advance_gamepiece", &g_localSteering.m_advanceGamepiece, 2, 0, 0, 0, 0, 0},
	{"previous_gamepiece", &g_localSteering.m_previousGamepiece, 2, 0, 0, 0, 0, 0},
	{"reset_gamepiece", &g_localSteering.m_resetGamepiece, 2, 0, 0, 0, 0, 0},
	{"advance_gamething", &g_localSteering.m_advanceGamething, 2, 0, 0, 0, 0, 0},
	{"previous_gamething", &g_localSteering.m_previousGamething, 2, 0, 0, 0, 0, 0},
	{"reset_gamething", &g_localSteering.m_resetGamething, 2, 0, 0, 0, 0, 0},
	{"self_destruct", &g_localSteering.m_selfDestruct, 2, 0, 0, 0, 0, 0},
	{"autopilot", &g_localSteering.m_autopilot, 2, 0, 0, 0, 0, 0},
	{"pilot_tilt", &g_sinkPilotTilt, 0, 0, -60, 60, 5, 0},
	{"pilot_tilt_plus", &g_sinkPilotTiltPlus, 1, 0, 0, 0, 0, 0},
	{"pilot_tilt_minus", &g_sinkPilotTiltMinus, 1, 0, 0, 0, 0, 0},
	{"pilot_tilt_reset", &g_sinkPilotTiltReset, 1, 0, 0, 0, 0, 0},
	{"pilot_pan", &g_sinkPilotPan, 0, 0, -70, 70, 5, 0},
	{"pilot_pan_plus", &g_sinkPilotPanPlus, 1, 0, 0, 0, 0, 0},
	{"pilot_pan_minus", &g_sinkPilotPanMinus, 1, 0, 0, 0, 0, 0},
	{"pilot_pan_reset", &g_sinkPilotPanReset, 1, 0, 0, 0, 0, 0},
	{"glance_left", (void*) &g_sinkGlanceLeft, 1, 0, 0, 0, 0, 0},
	{"glance_right", (void*) &g_sinkGlanceRight, 1, 0, 0, 0, 0, 0},
	{"glance_up", (void*) &g_sinkGlanceUp, 1, 0, 0, 0, 0, 0},
	{"glance_down", (void*) &g_sinkGlanceDown, 1, 0, 0, 0, 0, 0},
	{"eyepoint_tilt", &g_sinkEyepointTilt, 0, 0, -16, 16, 4, 0},
	{"eyepoint_tilt_plus", &g_sinkEyepointTiltPlus, 1, 0, 0, 0, 0, 0},
	{"eyepoint_tilt_minus", &g_sinkEyepointTiltMinus, 1, 0, 0, 0, 0, 0},
	{"eyepoint_tilt_reset", &g_sinkEyepointTiltReset, 1, 0, 0, 0, 0, 0},
	{"eyepoint_pan_delta", &g_sinkEyepointPanDelta, 0, 0, -72, 72, 4, 0},
	{"eyepoint_pan_plus", &g_sinkEyepointPanPlus, 1, 0, 0, 0, 0, 0},
	{"eyepoint_pan_minus", &g_sinkEyepointPanMinus, 1, 0, 0, 0, 0, 0},
	{"eyepoint_pan_reset", &g_sinkEyepointPanReset, 1, 0, 0, 0, 0, 0},
	{"eyepoint_slide_delta", &g_sinkEyepointSlideDelta, 0, 0, -3600, 3600, 6, 0},
	{"eyepoint_slide_plus", &g_sinkEyepointSlidePlus, 1, 0, 0, 0, 0, 0},
	{"eyepoint_slide_minus", &g_sinkEyepointSlideMinus, 1, 0, 0, 0, 0, 0},
	{"track_distance_delta", &g_sinkTrackDistanceDelta, 0, 0, -3600, 3600, 6, 0},
	{"track_distance_plus", &g_sinkTrackDistancePlus, 1, 0, 0, 0, 0, 0},
	{"track_distance_minus", &g_sinkTrackDistanceMinus, 1, 0, 0, 0, 0, 0},
	{"track_height_delta", &g_sinkTrackHeightDelta, 0, 0, -3600, 3600, 6, 0},
	{"track_height_plus", &g_sinkTrackHeightPlus, 1, 0, 0, 0, 0, 0},
	{"track_height_minus", &g_sinkTrackHeightMinus, 1, 0, 0, 0, 0, 0},
	{"zoom_factor", &g_sinkZoomFactor, 0, 1, 1, 4, 6, 0},
	{"zoom_factor_plus", &g_sinkZoomFactorPlus, 1, 0, 0, 0, 0, 0},
	{"zoom_factor_minus", &g_sinkZoomFactorMinus, 1, 0, 0, 0, 0, 0},
	{"zoom_factor_reset", &g_sinkZoomFactorReset, 1, 0, 0, 0, 0, 0},
	{"menu_item", &g_sinkMenuItem, 0, 0, -1, 1, 4, 0},
	{"menu_item_reset", &g_sinkMenuItemReset, 1, 0, 0, 0, 0, 0},
	{"menu_value", &g_sinkMenuValue, 0, 0, -1, 1, 4, 0},
	{"menu_value_reset", &g_sinkMenuValueReset, 1, 0, 0, 0, 0, 0},
	{"menu_enter", &g_sinkMenuEnter, 2, 0, 0, 0, 0, 0},
	{"menu_abort", &g_sinkMenuAbort, 2, 0, 0, 0, 0, 0},
};

// The GAMEKEY.MAP action names.
// GLOBAL: MW2 0x100b7e68
// GLOBAL: MW2GBL 0x100ab168
// GLOBAL: MW2PREMERCS 0x100bbea8
// GLOBAL: MW2MATROX 0x100b7890
// GLOBAL: MW23DFX 0x10257bc0
// GLOBAL: MW2S3 0x10070848
// GLOBAL: MW2POWERVR 0x101b7138
GameKeyName g_gameKeyNames[] = {
	{"RESET_INPUTS", 1},
	{"MFD_CYCLE", 2},
#ifdef MW2_MFD_GAME_KEYS
	{"MFD_CYCLE_A", 3},
	{"MFD_CYCLE_B", 4},
	{"MFD_CYCLE_C", 5},
	{"MFD_CYCLE_D", 6},
	{"MFD_CYCLE_E", 7},
	{"MFD_CYCLE_F", 8},
	{"MFDS_RESET", 9},
	{"SAVE_HUD", 10},
	{"TOGGLE_COMPASS", 11},
	{"TOGGLE_WEAPONS", 12},
#endif
	{"TOGGLE_HTAL", GAME_KEY_ACTION(3)},
	{"TOGGLE_REAR_VIEW", GAME_KEY_ACTION(4)},
	{"TOGGLE_DOWN_VIEW", GAME_KEY_ACTION(5)},
	{"TOGGLE_WEAPON_DISPLAY", GAME_KEY_ACTION(6)},
	{"TOGGLE_DAMAGE_DISPLAY", GAME_KEY_ACTION(7)},
	{"TOGGLE_TARGET_DISPLAY", GAME_KEY_ACTION(8)},
	{"COCKPIT_VIEW", GAME_KEY_ACTION(9)},
	{"FREEEYE_VIEW", GAME_KEY_ACTION(10)},
	{"TRACK_VIEW", GAME_KEY_ACTION(11)},
	{"TRACK_NEXT", GAME_KEY_ACTION(12)},
	{"TRACK_PREV", GAME_KEY_ACTION(13)},
#ifdef MW2_MFD_GAME_KEYS
	{"STARMATE_SET_ALL", 24},
	{"STARMATE_SET_PT2", 25},
	{"STARMATE_SET_PT3", 26},
	{"STARMATE_ATTACK", 27},
	{"STARMATE_DEFEND", 28},
	{"STARMATE_JOIN", 29},
	{"STARMATE_ENGAGE", 30},
	{"STARMATE_SHUTDOWN", 31},
#endif
	{"ORDINANCE_VIEW", GAME_KEY_ACTION(14)},
	{"JUMP_DEBUGGER", GAME_KEY_ACTION(15)},
	{"REQUEST_ORDINANCE", GAME_KEY_ACTION(16)},
	{"FEET_TO_TORSO", GAME_KEY_ACTION(17)},
	{"INSPECT_TARGET", GAME_KEY_ACTION(18)},
	{"TOGGLE_HUD", GAME_KEY_ACTION(19)},
	{"JETTISON_AMMO", GAME_KEY_ACTION(20)},
	{"COCKPIT_ZOOM_IN", GAME_KEY_ACTION(21)},
	{"COCKPIT_ZOOM_OUT", GAME_KEY_ACTION(22)},
	{"COCKPIT_RESET_ZOOM", GAME_KEY_ACTION(23)},
	{"DECREASE_THROTTLE", GAME_KEY_ACTION(24)},
	{"INCREASE_THROTTLE", GAME_KEY_ACTION(25)},
	{"THROTTLE_STOP", GAME_KEY_ACTION(26)},
	{"THROTTLE_2", GAME_KEY_ACTION(27)},
	{"THROTTLE_3", GAME_KEY_ACTION(28)},
	{"THROTTLE_4", GAME_KEY_ACTION(29)},
	{"THROTTLE_5", GAME_KEY_ACTION(30)},
	{"THROTTLE_6", GAME_KEY_ACTION(31)},
	{"THROTTLE_7", GAME_KEY_ACTION(32)},
	{"THROTTLE_8", GAME_KEY_ACTION(33)},
	{"THROTTLE_9", GAME_KEY_ACTION(34)},
	{"THROTTLE_FULL", GAME_KEY_ACTION(35)},
	{"NEXT_GAMETHING", GAME_KEY_ACTION(36)},
	{"PREV_GAMETHING", GAME_KEY_ACTION(37)},
	{"RESET_GAMETHING", GAME_KEY_ACTION(38)},
	{"RESET_TARGETTING", GAME_KEY_ACTION(39)},
	{"NEXT_NAVPOINT", GAME_KEY_ACTION(40)},
	{"PREV_NAVPOINT", GAME_KEY_ACTION(41)},
	{"RESET_NAVPOINT", GAME_KEY_ACTION(42)},
	{"NEXT_GAMEPIECE", GAME_KEY_ACTION(43)},
	{"PREV_GAMEPIECE", GAME_KEY_ACTION(44)},
	{"RESET_GAMEPIECE", GAME_KEY_ACTION(45)},
	{"NEXT_RADAR_MODE", GAME_KEY_ACTION(46)},
	{"RADAR_ZOOM_IN", GAME_KEY_ACTION(47)},
	{"RADAR_ZOOM_OUT", GAME_KEY_ACTION(48)},
	{"RADAR_RESET_ZOOM", GAME_KEY_ACTION(49)},
	{"RADAR_MAP_TOGGLE", GAME_KEY_ACTION(50)},
	{"MAIN_MENU", GAME_KEY_ACTION(51)},
	{"USER_MENU", GAME_KEY_ACTION(52)},
	{"ALL_PT_MENU", GAME_KEY_ACTION(53)},
	{"PT_2_MENU", GAME_KEY_ACTION(54)},
	{"PT_3_MENU", GAME_KEY_ACTION(55)},
	{"HELP_MENU", GAME_KEY_ACTION(58)},
	{"EJECT", GAME_KEY_ACTION(59)},
	{"TOGGLE_AUTOEJECT", GAME_KEY_ACTION(60)},
	{"STARTUP_MECH", GAME_KEY_ACTION(61)},
	{"SHUTDOWN_MECH", GAME_KEY_ACTION(62)},
	{"REVERSE_DIRECTION", GAME_KEY_ACTION(63)},
	{"OVERRIDE_SHUTDOWN", GAME_KEY_ACTION(64)},
	{"INFRARED", GAME_KEY_ACTION(166)},
	{"ENHANCED_VISION", GAME_KEY_ACTION(167)},
#ifdef MW2_AMS_KEY
	{"TOGGLE_AMS", GAME_KEY_ACTION(168)},
#endif
	{"DISPLAY_OBJECTIVES", GAME_KEY_ACTION(65)},
#ifdef MW2_CHAT_KEY
	{"CHAT_MODE", GAME_KEY_ACTION(65) + 1},
#endif
	{"STARMATE_MENU", GAME_KEY_ACTION(66)},
	{"STARMATE_ONE_MENU", GAME_KEY_ACTION(67)},
	{"STARMATE_TWO_MENU", GAME_KEY_ACTION(68)},
	{"STARMATE_THREE_MENU", GAME_KEY_ACTION(69)},
	{"STARMATE_FOUR_MENU", GAME_KEY_ACTION(70)},
	{"STARMATE_FIVE_MENU", GAME_KEY_ACTION(71)},
	{"STARMATE_SIX_MENU", GAME_KEY_ACTION(72)},
	{"STARMATE_SEVEN_MENU", GAME_KEY_ACTION(73)},
	{"TOGGLE_AUTOPILOT", GAME_KEY_ACTION(74)},
	{"TOGGLE_GROUP_FIRE", GAME_KEY_ACTION(75)},
	{"NEXT_TARGET", GAME_KEY_ACTION(76)},
	{"PREV_TARGET", GAME_KEY_ACTION(77)},
	{"RESET_TARGET", GAME_KEY_ACTION(78)},
	{"TARGET_NEAREST_ENEMY", GAME_KEY_ACTION(80)},
	{"TARGET_FRIENDLY", GAME_KEY_ACTION(81)},
	{"TARGET_AT_RETICLE", GAME_KEY_ACTION(82)},
	{"TARGET_LAST_SHOT", GAME_KEY_ACTION(83)},
	{"NEXT_OBJECTIVE", GAME_KEY_ACTION(84)},
	{"PUNCH_IN_AUTO_HDG", GAME_KEY_ACTION(85)},
	{"TOGGLE_MASC", GAME_KEY_ACTION(86)},
	{"SELF_DESTRUCT", GAME_KEY_ACTION(87)},
	{"PAUSE_GAME", GAME_KEY_ACTION(88)},
	{"EXIT_SIM", GAME_KEY_ACTION(89)},
	{"ABOUT_BANNER", GAME_KEY_ACTION(90)},
	{"DUMP_GIF", GAME_KEY_ACTION(91)},
	{"DEBUG_CRITICAL_HITS_1", GAME_KEY_ACTION(92)},
	{"DEBUG_CRITICAL_HITS_2", GAME_KEY_ACTION(93)},
	{"DEBUG_CRITICAL_HITS_3", GAME_KEY_ACTION(94)},
	{"DEBUG_CRITICAL_HITS_4", GAME_KEY_ACTION(95)},
	{"DEBUG_CRITICAL_HITS_5", GAME_KEY_ACTION(96)},
	{"DEBUG_CRITICAL_HITS_6", GAME_KEY_ACTION(97)},
	{"DEBUG_CRITICAL_HITS_7", GAME_KEY_ACTION(98)},
	{"DEBUG_CRITICAL_HITS_8", GAME_KEY_ACTION(99)},
	{"DEBUG_CRITICAL_HITS_9", GAME_KEY_ACTION(100)},
	{"DEBUG_CRITICAL_HITS_0", GAME_KEY_ACTION(101)},
	{"SELECT_HEAD", GAME_KEY_ACTION(102)},
	{"SELECT_RIGHT_TORSO", GAME_KEY_ACTION(103)},
	{"SELECT_CENTER_TORSO", GAME_KEY_ACTION(104)},
	{"SELECT_LEFT_TORSO", GAME_KEY_ACTION(105)},
	{"SELECT_RIGHT_ARM", GAME_KEY_ACTION(106)},
	{"SELECT_LEFT_ARM", GAME_KEY_ACTION(107)},
	{"SELECT_RIGHT_LEG", GAME_KEY_ACTION(108)},
	{"SELECT_LEFT_LEG", GAME_KEY_ACTION(109)},
	{"AI_0_MONO_OBJECTIVES", GAME_KEY_ACTION(110)},
	{"AI_1_MONO_OBJECTIVES", GAME_KEY_ACTION(111)},
	{"AI_2_MONO_OBJECTIVES", GAME_KEY_ACTION(112)},
	{"AI_3_MONO_OBJECTIVES", GAME_KEY_ACTION(113)},
	{"AI_4_MONO_OBJECTIVES", GAME_KEY_ACTION(114)},
	{"AI_5_MONO_OBJECTIVES", GAME_KEY_ACTION(115)},
	{"AI_6_MONO_OBJECTIVES", GAME_KEY_ACTION(116)},
	{"AI_7_MONO_OBJECTIVES", GAME_KEY_ACTION(117)},
	{"AI_8_MONO_OBJECTIVES", GAME_KEY_ACTION(118)},
	{"AI_9_MONO_OBJECTIVES", GAME_KEY_ACTION(119)},
	{"OBJECTIVES_OFF", GAME_KEY_ACTION(120)},
	{"NEXT_MONO_OBJECTIVE", GAME_KEY_ACTION(121)},
	{"PREV_MONO_OBJECTIVE", GAME_KEY_ACTION(122)},
	{"FAIL_MONO_OBJECTIVE", GAME_KEY_ACTION(123)},
	{"SUCCEED_MONO_OBJECTIVE", GAME_KEY_ACTION(124)},
	{"TOGGLE_GP_TEXT_MAPS", GAME_KEY_ACTION(125)},
	{"TOGGLE_GT_TEXT_MAPS", GAME_KEY_ACTION(126)},
	{"TOGGLE_GS_TEXT_MAPS", GAME_KEY_ACTION(127)},
	{"TOGGLE_TERRAIN_TEXT_MAPS", GAME_KEY_ACTION(128)},
	{"GO_TURBO_CONTROL", GAME_KEY_ACTION(129)},
	{"TOGGLE_MEM_INFO", GAME_KEY_ACTION(130)},
	{"TOGGLE_CACHE_INFO", GAME_KEY_ACTION(131)},
	{"COLLISION_WIREFRAME", GAME_KEY_ACTION(132)},
	{"TOGGLE_FRAMERATE_MAIN", GAME_KEY_ACTION(133)},
	{"TOGGLE_SCENE_INFO_MAIN", GAME_KEY_ACTION(135)},
	{"TOGGLE_EYE_POSITION_MAIN", GAME_KEY_ACTION(136)},
	{"TOGGLE_VULNERABILITY", GAME_KEY_ACTION(137)},
	{"TOGGLE_AMMO_LIMIT", GAME_KEY_ACTION(138)},
	{"TOGGLE_SPLASH_DAMAGE", GAME_KEY_ACTION(139)},
	{"TOGGLE_COLLISION_DAMAGE", GAME_KEY_ACTION(140)},
	{"TOGGLE_HEAT_TRACKING", GAME_KEY_ACTION(141)},
	{"TOGGLE_LOD_QUALITY", GAME_KEY_ACTION(142)},
	{"DUMP_CACHE_ONCE", GAME_KEY_ACTION(143)},
	{"DUMP_CACHE_REPEAT", GAME_KEY_ACTION(144)},
	{"TOGGLE_RADAR_DAMAGE", GAME_KEY_ACTION(134)},
	{"TOGGLE_FRAMERATE", GAME_KEY_ACTION(145)},
	{"CRACK_MODE", GAME_KEY_ACTION(146)},
	{"TOGGLE_EYE_POSITION", GAME_KEY_ACTION(147)},
	{"TOGGLE_SCENE_INFO", GAME_KEY_ACTION(148)},
	{"TOGGLE_WIREFRAME", GAME_KEY_ACTION(149)},
	{"TOGGLE_PALETTE", GAME_KEY_ACTION(150)},
	{"ADD_WEAPON_TO_GROUP_1", GAME_KEY_ACTION(151)},
	{"ADD_WEAPON_TO_GROUP_2", GAME_KEY_ACTION(152)},
	{"ADD_WEAPON_TO_GROUP_3", GAME_KEY_ACTION(153)},
#ifdef MW2_MFD_GAME_KEYS
	{"ADD_WEAPON_TO_GROUP_4", 188},
	{"ADD_WEAPON_TO_GROUP_5", 190},
#endif
	{"NEXT_WEAPON_GROUP", GAME_KEY_ACTION(154)},
	{"FIRE_WEAPON_GROUP", GAME_KEY_ACTION(155)},
	{"CREATE_NAVPOINT_TEST", GAME_KEY_ACTION(159)},
	{"DELETE_NAVPOINT_TEST", GAME_KEY_ACTION(160)},
#ifdef MW2_MFD_GAME_KEYS
	{"FIRE_CYCLE_WEAPON_GROUP", 192},
#endif
	{"TOGGLE_SAVE_FRAMES", GAME_KEY_ACTION(161)},
	{"PLAY_CURRENT_MIDI", GAME_KEY_ACTION(162)},
	{"PLAY_NEXT_MIDI", GAME_KEY_ACTION(163)},
	{"HACK_JETS", GAME_KEY_ACTION(164)},
	{"KILL_TARGETTED_MECH", GAME_KEY_ACTION(165)},
};

// The game key action of each key code (0: none), loaded from GAMEKEY.MAP.
// GLOBAL: MW2 0x100b8190
// GLOBAL: MW2GBL 0x100ab498
// GLOBAL: MW2PREMERCS 0x100bc240
// GLOBAL: MW2MATROX 0x100b7bb8
// GLOBAL: MW23DFX 0x10257ee8
// GLOBAL: MW2ATI 0x1018c1f8
// GLOBAL: MW2S3 0x10070b70
// GLOBAL: MW2POWERVR 0x101b7460
MechU8 g_gameKeyByKeyCode[0x800] = {0};

// The key names GAMEKEY.MAP can use in a key sequence, and their key codes (or modifier bits).
// GLOBAL: MW2 0x100b8990
// GLOBAL: MW2GBL 0x100abc98
// GLOBAL: MW2PREMERCS 0x100bca40
// GLOBAL: MW2MATROX 0x100b83b8
// GLOBAL: MW23DFX 0x10258798
// GLOBAL: MW2ATI 0x1018caa8
// GLOBAL: MW2S3 0x10071370
// GLOBAL: MW2POWERVR 0x101b7d10
GameKeyModifier g_gameKeyModifiers[] = {
	{"ALT", 0x400},       {"CTRL", 0x100},     {"SHIFT", 0x200}, {"F1", 0xb1},         {"F2", 0xb2},
	{"F3", 0xb3},         {"F4", 0xb4},        {"F5", 0xb5},     {"F6", 0xb6},         {"F7", 0xb7},
	{"F8", 0xb8},         {"F9", 0xb9},        {"F10", 0xba},    {"F11", 0xbb},        {"F12", 0xbc},
	{"TAB", 0x9},         {"INSERT", 0xc0},    {"ESC", 0x1b},    {"BSP", 0x8},         {"ENTER", 0xd},
	{"PAUSE", 0x1ff},     {"POUND", 0x23},     {"PLUS", 0x2b},   {"GREY SLASH", 0xca}, {"GREY ASTERICK", 0xcb},
	{"GREY MINUS", 0xcc}, {"GREY PLUS", 0xcd},
};

// The text of ReportInputDeviceError's message box.
// GLOBAL: MW2 0x100bf1d0
// GLOBAL: MW2GBL 0x100b9448
// GLOBAL: MW2PREMERCS 0x100cb820
// GLOBAL: MW2MATROX 0x10124d40
// GLOBAL: MW23DFX 0x1007ce80
// GLOBAL: MW2ATI 0x10071e80
// GLOBAL: MW2S3 0x1007cf38
// GLOBAL: MW2POWERVR 0x1006fec8
MechChar g_inputErrorText[0x400];

// GLOBAL: MW2 0x100bf5d0
// GLOBAL: MW2GBL 0x100b9848
// GLOBAL: MW2PREMERCS 0x100cbc20
// GLOBAL: MW2MATROX 0x10124d28
// GLOBAL: MW23DFX 0x1007ce68
// GLOBAL: MW2ATI 0x10071e68
// GLOBAL: MW2S3 0x1007d338
// GLOBAL: MW2POWERVR 0x1006feb0
MechS32 g_inputDevicePresent[5];

// GLOBAL: MW2 0x100bf5e8
// GLOBAL: MW2GBL 0x100b9860
// GLOBAL: MW2PREMERCS 0x100cbc38
// GLOBAL: MW2MATROX 0x10124a00
// GLOBAL: MW23DFX 0x1007cb40
// GLOBAL: MW2ATI 0x10071b40
// GLOBAL: MW2S3 0x1007d350
// GLOBAL: MW2POWERVR 0x1006fb88
InputDeviceInfo g_inputDeviceInfos[5];

// What each device's last poll reported.
// GLOBAL: MW2 0x100bf830
// GLOBAL: MW2GBL 0x100b9aa8
// GLOBAL: MW2PREMERCS 0x100cbe80
// GLOBAL: MW2MATROX 0x10125140
// GLOBAL: MW23DFX 0x1007d280
// GLOBAL: MW2ATI 0x10072280
// GLOBAL: MW2S3 0x1007d598
// GLOBAL: MW2POWERVR 0x100702c8
InputDeviceState g_inputDeviceStates[5];

// The INPUT.MAP name of each device.
// GLOBAL: MW2 0x100bf9c0
// GLOBAL: MW2GBL 0x100b9c38
// GLOBAL: MW2PREMERCS 0x100cc010
// GLOBAL: MW2MATROX 0x10124c60
// GLOBAL: MW23DFX 0x1007cda0
// GLOBAL: MW2ATI 0x10071da0
// GLOBAL: MW2S3 0x1007d728
// GLOBAL: MW2POWERVR 0x1006fde8
MechChar g_inputDeviceNames[5][0x28];

// GLOBAL: MW2 0x100bfa88
// GLOBAL: MW2GBL 0x100b9d00
// GLOBAL: MW2PREMERCS 0x100cc0d8
// GLOBAL: MW2MATROX 0x10124c48
// GLOBAL: MW23DFX 0x1007cd88
// GLOBAL: MW2ATI 0x10071d88
// GLOBAL: MW2S3 0x1007d7f0
// GLOBAL: MW2POWERVR 0x1006fdd0
InputDriverModule* g_inputDrivers[5];

// Finds the device INPUT.MAP calls p_name among the drivers' devices and opens it. Returns its
// index, or -1 if it can't be opened; a missing device still takes an index.
// FUNCTION: MW2 0x10079340
// FUNCTION: MW2GBL 0x10031510
// FUNCTION: MW2PREMERCS 0x1006ee50
// FUNCTION: MW2MATROX 0x1007bb00
// FUNCTION: MW23DFX 0x10038c00
// FUNCTION: MW2ATI 0x100377c0
// FUNCTION: MW2S3 0x10030d70
// FUNCTION: MW2POWERVR 0x100324c0
MechS32 RegisterInputDevice(MechChar* p_name)
{
	MechS32 index;
	MechS32 driver;
	MechS32 i;
	MechS32 count;

	for (driver = 0; driver < 3; driver++) {
		if (g_inputDriverClasses[driver]) {
			count = g_inputDriverClasses[driver]->m_getDeviceCount();
			if (count) {
				for (i = 0; i < count; i++) {
					index = g_inputDeviceCount;
					if (g_inputDeviceCount >= 5) {
						Error(0x11, "Too many input devices specified", 0);
						return -1;
					}

					if (!g_inputDriverClasses[driver]->m_fillDeviceInfo(i, &g_inputDeviceInfos[index])) {
						if (!_strcmpi(g_inputDeviceInfos[index].m_shortName, p_name)) {
							strcpy(g_inputDeviceNames[index], p_name);
							g_inputDrivers[index] = g_inputDriverClasses[driver];
							g_inputDeviceCount++;
							if (g_inputDrivers[index]->m_openDevice(&g_inputDeviceInfos[index])) {
								ReportInputDeviceError(0x70, NULL, g_inputDeviceInfos[index].m_displayName);
								g_inputDevicePresent[index] = FALSE;
								return -1;
							}
							else {
								g_inputDevicePresent[index] = TRUE;
								return index;
							}
						}
						else {
							g_inputDriverClasses[driver]->m_closeDevice(&g_inputDeviceInfos[index]);
						}
					}
				}
			}
		}
	}

	index = g_inputDeviceCount;
	g_inputDeviceCount++;
	strcpy(g_inputDeviceNames[index], p_name);
	g_inputDevicePresent[index] = FALSE;
	ReportInputDeviceError(0x6e, NULL, p_name);
	return -1;
}

// Returns the index of the device INPUT.MAP calls p_name, registering it the first time, or -1
// if it isn't there.
// FUNCTION: MW2 0x1007959c
// FUNCTION: MW2GBL 0x10031742
// FUNCTION: MW2PREMERCS 0x1006f082
// FUNCTION: MW2MATROX 0x1007bd5c
// FUNCTION: MW23DFX 0x10038de0
// FUNCTION: MW2ATI 0x100379a0
// FUNCTION: MW2S3 0x10030f50
// FUNCTION: MW2POWERVR 0x100326a0
MechS32 FindInputDevice(MechChar* p_name)
{
	MechS32 index;
	MechS32 i;

	for (i = 0; i < g_inputDeviceCount; i++) {
		if (!_strcmpi(g_inputDeviceNames[i], p_name)) {
			return g_inputDevicePresent[i] ? i : -1;
		}
	}

	index = RegisterInputDevice(p_name);
	if (!_strcmpi(p_name, "keyboard")) {
		if (index != -1) {
			g_keyboardDeviceIndex = index;
		}
		else {
			Error(0x11, "\nKeyboard initialization error", 0);
		}
	}

	return index;
}

// Returns the axis of device p_device that INPUT.MAP names p_name, a number or a short name, or
// -1.
// FUNCTION: MW2 0x1007966a
// FUNCTION: MW2GBL 0x1003180f
// FUNCTION: MW2PREMERCS 0x1006f150
// FUNCTION: MW2MATROX 0x1007be29
// FUNCTION: MW23DFX 0x10038e70
// FUNCTION: MW2ATI 0x10037a30
// FUNCTION: MW2S3 0x10030fe0
// FUNCTION: MW2POWERVR 0x10032730
MechS32 FindInputAxis(MechS32 p_device, MechChar* p_name)
{
	MechS32 i;

	if (isdigit(*p_name)) {
		return atoi(p_name);
	}

	if (p_device == -1) {
		return -1;
	}

	if (!g_inputDeviceInfos[p_device].m_axisShortNames) {
		Error(0x11, "No input channel \"%s\". Line %d", p_name, g_inputMapLine, 0);
		return -1;
	}

	for (i = 0; i < g_inputDeviceInfos[p_device].m_axisCount; i++) {
		if (g_inputDeviceInfos[p_device].m_axisShortNames[i] &&
			!_strcmpi(g_inputDeviceInfos[p_device].m_axisShortNames[i], p_name)) {
			return i;
		}
	}

	ReportInputDeviceError(0x6f, p_name, g_inputDeviceInfos[p_device].m_displayName);
	return -1;
}

// Returns the button of device p_device that INPUT.MAP names p_name, a number or a short name,
// or -1.
// FUNCTION: MW2 0x100797de
// FUNCTION: MW2GBL 0x10031983
// FUNCTION: MW2PREMERCS 0x1006f2c4
// FUNCTION: MW2MATROX 0x1007bf9d
// FUNCTION: MW23DFX 0x10038f60
// FUNCTION: MW2ATI 0x10037b20
// FUNCTION: MW2S3 0x100310d0
// FUNCTION: MW2POWERVR 0x10032820
MechS32 FindInputButton(MechS32 p_device, MechChar* p_name)
{
	MechS32 i;

	if (isdigit(*p_name)) {
		return atoi(p_name);
	}

	if (p_device == -1) {
		return -1;
	}

	if (!g_inputDeviceInfos[p_device].m_buttonShortNames) {
		Error(0x11, "No input channel \"%s\". Line %d", p_name, g_inputMapLine, 0);
		return -1;
	}

	for (i = 0; i < g_inputDeviceInfos[p_device].m_buttonCount; i++) {
		if (g_inputDeviceInfos[p_device].m_buttonShortNames[i] &&
			!_strcmpi(g_inputDeviceInfos[p_device].m_buttonShortNames[i], p_name)) {
			return i;
		}
	}

	ReportInputDeviceError(0x6f, p_name, g_inputDeviceInfos[p_device].m_displayName);
	return -1;
}

// Returns the sink INPUT.MAP calls p_name, or NULL.
// FUNCTION: MW2 0x10079952
// FUNCTION: MW2GBL 0x10031af7
// FUNCTION: MW2PREMERCS 0x1006f438
// FUNCTION: MW2MATROX 0x1007c111
// FUNCTION: MW23DFX 0x10039050
// FUNCTION: MW2ATI 0x10037c10
// FUNCTION: MW2S3 0x100311c0
// FUNCTION: MW2POWERVR 0x10032910
InputSink* FindInputSink(MechChar* p_name)
{
	MechS32 i;

	for (i = 0; i < sizeof(g_inputSinks) / sizeof(g_inputSinks[0]); i++) {
		if (!_strcmpi(g_inputSinks[i].m_name, p_name)) {
			return &g_inputSinks[i];
		}
	}

	return NULL;
}

// Reads the next line of INPUT.MAP that isn't blank once its comment is cut off. Returns 0 at
// the end of the file.
// FUNCTION: MW2 0x100799b7
// FUNCTION: MW2GBL 0x10031b5c
// FUNCTION: MW2PREMERCS 0x1006f49d
// FUNCTION: MW2MATROX 0x1007c176
// FUNCTION: MW23DFX 0x10039090
// FUNCTION: MW2ATI 0x10037c50
// FUNCTION: MW2S3 0x10031200
// FUNCTION: MW2POWERVR 0x10032950
MechS32 ReadInputMapLine(MechChar* p_buffer, MechS32 p_size, FILE* p_file)
{
	MechChar* p;
	MechS32 found;

	found = FALSE;
	for (;;) {
		if (!fgets(p_buffer, p_size, p_file)) {
			return 0;
		}

		g_inputMapLine++;
		for (p = p_buffer; *p; p++) {
			if (*p == '\n' || *p == '#') {
				*p = '\0';
				break;
			}
			else if (!isspace(*p)) {
				found = TRUE;
			}
		}

		if (found) {
			return 1;
		}
	}
}

// Returns the axis state of the axis sink p_sink, setting one up the first time: its range and
// rest position, and the outputs of its _plus, _minus, _set and _reset sinks. A _delta sink's
// keys move its position directly.
// FUNCTION: MW2 0x10079aac
// FUNCTION: MW2GBL 0x10031c51
// FUNCTION: MW2PREMERCS 0x1006f592
// FUNCTION: MW2MATROX 0x1007c26b
// FUNCTION: MW23DFX 0x10039140
// FUNCTION: MW2ATI 0x10037d00
// FUNCTION: MW2S3 0x100312b0
// FUNCTION: MW2POWERVR 0x10032a00
InputAxis* GetOrCreateInputAxis(InputSink* p_sink)
{
	MechChar* suffix;
	InputSink* sink;
	MechChar name[40];
	MechS32 i;

	for (i = 0; i < g_inputAxisCount; i++) {
		if (g_inputAxes[i].m_output == p_sink->m_output) {
			return &g_inputAxes[i];
		}
	}

	i = g_inputAxisCount;
	g_inputAxisCount++;
	if (g_inputAxisCount > 50) {
		Error(0x11, "Too many analog sinks: line %d", g_inputMapLine, 0);
		return NULL;
	}

	if (strstr(p_sink->m_name, "_delta")) {
		g_inputAxes[i].m_rate = &g_inputAxes[i].m_position;
	}
	else {
		g_inputAxes[i].m_rate = &g_inputAxes[i].m_ownRate;
	}

	g_inputAxes[i].m_output = p_sink->m_output;
	g_inputAxes[i].m_range = p_sink->m_max - p_sink->m_min;
	g_inputAxes[i].m_minFixed = p_sink->m_min << 16;
	g_inputAxes[i].m_outputShift = p_sink->m_outputShift;
	g_inputAxes[i].m_rest = p_sink->m_rest << 16;
	g_inputAxes[i].m_rest -= g_inputAxes[i].m_minFixed;
	g_inputAxes[i].m_rest <<= 1;
	g_inputAxes[i].m_rest /= g_inputAxes[i].m_range;
	g_inputAxes[i].m_rest -= 0x10000;
	g_inputAxes[i].m_rampShift = p_sink->m_rampShift;
	g_inputAxes[i].m_position = g_inputAxes[i].m_rest;

	strcpy(name, p_sink->m_name);
	suffix = strstr(name, "_delta");
	if (suffix) {
		*suffix = '\0';
	}
	strcat(name, "_plus");
	sink = FindInputSink(name);
	if (sink) {
		g_inputAxes[i].m_plusHeld = sink->m_output;
	}
	else {
		g_inputAxes[i].m_plusHeld = NULL;
	}

	strcpy(name, p_sink->m_name);
	suffix = strstr(name, "_delta");
	if (suffix) {
		*suffix = '\0';
	}
	strcat(name, "_minus");
	sink = FindInputSink(name);
	if (sink) {
		g_inputAxes[i].m_minusHeld = sink->m_output;
	}
	else {
		g_inputAxes[i].m_minusHeld = NULL;
	}

	strcpy(name, p_sink->m_name);
	strcat(name, "_set");
	sink = FindInputSink(name);
	if (sink) {
		g_inputAxes[i].m_setHeld = sink->m_output;
	}
	else {
		g_inputAxes[i].m_setHeld = NULL;
	}

	strcpy(name, p_sink->m_name);
	strcat(name, "_reset");
	sink = FindInputSink(name);
	if (sink) {
		g_inputAxes[i].m_resetHeld = sink->m_output;
	}
	else {
		g_inputAxes[i].m_resetHeld = NULL;
	}

	return &g_inputAxes[i];
}

// Binds channel p_channel of device p_deviceName (NULL: none, for the axis's keys) to the axis
// sink p_sink. A binding of the axis without a channel takes the channel. Returns the binding's
// index, or -1.
// FUNCTION: MW2 0x10079f71
// FUNCTION: MW2GBL 0x10032083
// FUNCTION: MW2PREMERCS 0x1006f9c4
// FUNCTION: MW2MATROX 0x1007c72f
// FUNCTION: MW23DFX 0x10039440
// FUNCTION: MW2ATI 0x10038000
// FUNCTION: MW2S3 0x100315b0
// FUNCTION: MW2POWERVR 0x10032d00
MechS32 AddAnalogBinding(InputSink* p_sink, MechChar* p_deviceName, MechS32 p_channel)
{
	MechS32 device;
	MechS32 index;
	MechS32 i;
	MechS32 found;

	for (i = 0; i < g_analogBindingCount; i++) {
		if (g_analogBindings[i].m_axis && g_analogBindings[i].m_axis->m_output == p_sink->m_output) {
			if (!p_deviceName) {
				return i;
			}

			if (!g_analogBindings[i].m_channelValue) {
				found = FindInputDevice(p_deviceName);
				g_analogBindings[i].m_channelValue = &g_inputDeviceStates[found].m_axes[p_channel];
				g_analogBindings[i].m_device = found;
				g_analogBindings[i].m_channel = p_channel;
				return i;
			}
		}
	}

	index = g_analogBindingCount;
	g_analogBindingCount++;
	if (g_analogBindingCount > 50) {
		Error(0x11, "Too many analog controls: line %d", g_inputMapLine, 0);
		return -1;
	}

	if (p_deviceName) {
		device = FindInputDevice(p_deviceName);
	}
	else {
		device = -1;
	}

	g_analogBindings[index].m_device = device;
	g_analogBindings[index].m_channel = p_channel;
	g_analogBindings[index].m_axis = GetOrCreateInputAxis(p_sink);
	if (device >= 0) {
		g_analogBindings[index].m_channelValue = &g_inputDeviceStates[device].m_axes[p_channel];
	}
	else {
		g_analogBindings[index].m_channelValue = NULL;
	}

	g_analogBindings[index].m_menuOnly = !strncmp(p_sink->m_name, "menu_", 5);

	return index;
}

// Reads a binding's conditions, "+ device button" or "- device button" a line, up to the closing
// brace. Returns 0 on an error, or if the last button isn't there.
// FUNCTION: MW2 0x1007a168
// FUNCTION: MW2GBL 0x1003227a
// FUNCTION: MW2PREMERCS 0x1006fbbb
// FUNCTION: MW2MATROX 0x1007c926
// FUNCTION: MW23DFX 0x10039590
// FUNCTION: MW2ATI 0x10038150
// FUNCTION: MW2S3 0x10031700
// FUNCTION: MW2POWERVR 0x10032e50
MechS32 ParseInputConditions(FILE* p_file, MechS32* p_count, InputCondition* p_conditions)
{
	MechS32 device;
	MechChar line[256];
	MechChar buttonName[20];
	MechChar deviceName[40];
	MechChar sign[2];
	MechS32 button;
	MechChar name[40];

	button = 0;
	*p_count = 0;
	while (ReadInputMapLine(line, 0xff, p_file)) {
		if (line[0] == '}') {
			break;
		}

		if (sscanf(line, " %1[+-] %s %s", sign, deviceName, buttonName) != 3) {
			Error(0x11, "input mapping error: line %d, read error - \"%s\"", g_inputMapLine, name, 0);
			return FALSE;
		}

		if (*p_count >= 8) {
			Error(0x11, "input mapping error: line %d, chord too big - \"%s\"", g_inputMapLine, name, 0);
			return FALSE;
		}

		device = FindInputDevice(deviceName);
		button = FindInputButton(device, buttonName);
		if (button != -1) {
			p_conditions[*p_count].m_negate = sign[0] == '-';

			p_conditions[*p_count].m_word = &g_inputDeviceStates[device].m_buttons[button >> 5];
			p_conditions[*p_count].m_mask = 1 << (button & 0x1f);
			(*p_count)++;
		}
	}

	return button == -1 ? FALSE : TRUE;
}

// Returns whether all p_count conditions hold.
// FUNCTION: MW2 0x1007a330
// FUNCTION: MW2GBL 0x10032442
// FUNCTION: MW2PREMERCS 0x1006fd83
// FUNCTION: MW2MATROX 0x1007caca
// FUNCTION: MW23DFX 0x100396e0
// FUNCTION: MW2ATI 0x100382a0
// FUNCTION: MW2S3 0x10031850
// FUNCTION: MW2POWERVR 0x10032fa0
MechS32 CheckInputConditions(MechS32 p_count, InputCondition* p_conditions)
{
	MechS32 i;

	for (i = 0; i < p_count; i++) {
		if (p_conditions[i].m_negate == 1) {
			if (*p_conditions[i].m_word & p_conditions[i].m_mask) {
				return FALSE;
			}
		}
		else {
			if (!(*p_conditions[i].m_word & p_conditions[i].m_mask)) {
				return FALSE;
			}
		}
	}

	return TRUE;
}

// Parses an INPUT.MAP file: a block per sink, "name {" and its bindings up to "}". An axis sink
// reads "+ device channel" (- inverts it) and the binding's conditions; a button sink reads its
// conditions, and a _plus or _minus button also binds its axis (or the axis's _delta sink) so its
// keys move it.
// FUNCTION: MW2 0x1007a3d1
// FUNCTION: MW2GBL 0x100324e3
// FUNCTION: MW2PREMERCS 0x1006fe24
// FUNCTION: MW2MATROX 0x1007cb6b
// FUNCTION: MW23DFX 0x10039730
// FUNCTION: MW2ATI 0x100382f0
// FUNCTION: MW2S3 0x100318a0
// FUNCTION: MW2POWERVR 0x10032ff0
MechS32 ParseInputMap(FILE* p_file)
{
	MechChar line[256];
	InputSink* sink;
	MechS32 index;
	MechChar channelName[20];
	MechChar deviceName[40];
	MechS32 channel;
	MechChar name[40];
	MechChar sign[2];
	MechChar* suffix;

	while (ReadInputMapLine(line, 0xff, p_file)) {
		if (sscanf(line, "%s {", name) != 1) {
			Error(0x11, "input mapping error: line %d, read error", g_inputMapLine, 0);
			return FALSE;
		}

		sink = FindInputSink(name);
		if (!sink) {
			Error(0x11, "input mapping error: line %d, bad field \"%s\"", g_inputMapLine, name, 0);
			return FALSE;
		}

		if (sink->m_kind == c_inputSinkAxis) {
			if (!ReadInputMapLine(line, 0xff, p_file)) {
				Error(0x11, "input mapping error: line %d, unexpected EOF", g_inputMapLine, 0);
				return FALSE;
			}

			if (sscanf(line, " %1[+-] %s %s", sign, deviceName, channelName) != 3) {
				Error(0x11, "input mapping error: line %d, read error - \"%s\"", g_inputMapLine, name, 0);
				return FALSE;
			}

			channel = FindInputAxis(FindInputDevice(deviceName), channelName);
			if (channel != -1) {
				index = AddAnalogBinding(sink, deviceName, channel);
				if (index < 0) {
					return FALSE;
				}

				g_analogBindings[index].m_sign = sign[0] == '-' ? -1 : 1;

				if (!ParseInputConditions(
						p_file,
						&g_analogBindings[index].m_conditionCount,
						g_analogBindings[index].m_conditions
					)) {
					return FALSE;
				}
			}
			else {
				while (ReadInputMapLine(line, 0xff, p_file)) {
					if (line[0] == '}') {
						break;
					}
				}
			}
		}
		else if (sink->m_kind == c_inputSinkButton) {
			index = g_discreteBindingCount;
			if (ParseInputConditions(
					p_file,
					&g_discreteBindings[index].m_conditionCount,
					g_discreteBindings[index].m_conditions
				)) {
				g_discreteBindingCount++;
				if (g_discreteBindingCount > 100) {
					Error(0x11, "Too many discrete controls: line %d", g_inputMapLine, 0);
					return FALSE;
				}

				g_discreteBindings[index].m_output = sink->m_output;
				g_discreteBindings[index].m_edge = FALSE;
				g_discreteBindings[index].m_menuOnly = !strncmp(sink->m_name, "menu_", 5);

				suffix = strstr(name, "_plus");
				if (!suffix) {
					suffix = strstr(name, "_minus");
				}

				if (suffix) {
					*suffix = '\0';
					sink = FindInputSink(name);
					if (!sink) {
						strcat(name, "_delta");
						sink = FindInputSink(name);
					}

					if (sink && AddAnalogBinding(sink, NULL, 0) < 0) {
						return FALSE;
					}
				}
			}
		}
		else {
			index = g_discreteBindingCount;
			if (ParseInputConditions(
					p_file,
					&g_discreteBindings[index].m_conditionCount,
					g_discreteBindings[index].m_conditions
				)) {
				g_discreteBindingCount++;
				g_discreteBindings[index].m_output = sink->m_output;
				g_discreteBindings[index].m_edge = TRUE;
				g_discreteBindings[index].m_menuOnly = !strncmp(sink->m_name, "menu_", 5);
			}
		}
	}

	return TRUE;
}

// Loads INPUT.MAP, then each present device's own map: giddi\<device name without its trailing
// digits>.std.
// FUNCTION: MW2 0x1007a97a
// FUNCTION: MW2GBL 0x10032a79
// FUNCTION: MW2PREMERCS 0x100703ba
// FUNCTION: MW2MATROX 0x1007d0ba
// FUNCTION: MW23DFX 0x10039ac0
// FUNCTION: MW2ATI 0x10038680
// FUNCTION: MW2S3 0x10031c30
// FUNCTION: MW2POWERVR 0x10033380
MechS32 LoadInputMap(void)
{
	MechChar name[256];
	MechS32 i;
	FILE* file;

	file = fopen("input.map", "r");
	if (!file) {
		Error(0x11, "Can't open input mapping file", 0);
		return FALSE;
	}

	ParseInputMap(file);
	fclose(file);
	for (i = 0; i < g_inputDeviceCount; i++) {
		if (g_inputDevicePresent[i]) {
			strcpy(name, "giddi");
			strcat(name, "\\");
			strcat(name, g_inputDeviceNames[i]);
			while (isdigit(name[strlen(name) - 1])) {
				name[strlen(name) - 1] = '\0';
			}

			strcat(name, ".std");
			file = fopen(name, "r");
			if (file) {
				ParseInputMap(file);
				fclose(file);
			}
		}
	}

	return TRUE;
}

// Loads GAMEKEY.MAP: an action name and its key sequence ("CTRL+F1") a line.
// FUNCTION: MW2 0x1007aba9
// FUNCTION: MW2GBL 0x10032c6a
// FUNCTION: MW2PREMERCS 0x100705ac
// FUNCTION: MW2MATROX 0x1007d2c1
// FUNCTION: MW23DFX 0x10039c90
// FUNCTION: MW2ATI 0x10038850
// FUNCTION: MW2S3 0x10031e00
// FUNCTION: MW2POWERVR 0x10033550
MechS32 LoadGamekeyMap(void)
{
	MechChar line[256];
	MechChar* token;
	MechS16 keyCode;
	MechU32 i;
	MechChar keys[40];
	MechChar action[40];
	FILE* file;
	MechS32 code;

	g_gameKeyByKeyCode[0x20] = GAME_KEY_ACTION(0x4f);
	g_gameKeyByKeyCode[0x171] = GAME_KEY_ACTION(0x59);
	g_inputMapLine = 0;
	file = fopen("gamekey.map", "r");
	if (!file) {
		Error(0x11, "Can't open gamekey mapping file", 0);
		return FALSE;
	}

	while (ReadInputMapLine(line, 0xff, file)) {
		if (sscanf(line, "%s %s", action, keys) != 2) {
			Error(0x11, "gamekey mapping error: line %d, read error", g_inputMapLine, 0);
			return FALSE;
		}

		token = strtok(keys, "+");
		if (!token) {
			Error(0x11, "gamekey mapping error: line %d, bad sequence", g_inputMapLine, 0);
			return FALSE;
		}

		keyCode = 0;
		while (token) {
			code = 0;
			for (i = 0; i < sizeof(g_gameKeyModifiers) / sizeof(g_gameKeyModifiers[0]); i++) {
				if (!_strcmpi(g_gameKeyModifiers[i].m_name, token)) {
					code = g_gameKeyModifiers[i].m_code;
					break;
				}
			}

			if (code) {
				keyCode += code;
			}
			else {
				keyCode += tolower(*token);
			}

			token = strtok(NULL, "+");
		}

		for (i = 0; i < sizeof(g_gameKeyNames) / sizeof(g_gameKeyNames[0]); i++) {
			if (!_strcmpi(g_gameKeyNames[i].m_name, action)) {
				break;
			}
		}

		if (i >= sizeof(g_gameKeyNames) / sizeof(g_gameKeyNames[0])) {
			Error(0x11, "gamekey mapping error: line %d, no gamekey %s", g_inputMapLine, action);
			return FALSE;
		}

		if (g_gameKeyByKeyCode[keyCode] && g_gameKeyNames[i].m_action != g_gameKeyByKeyCode[keyCode]) {
			Error(0x11, "gamekey mapping error: line %d, multiple defines '%s' ", g_inputMapLine, line);
			return FALSE;
		}

		g_gameKeyByKeyCode[keyCode] = g_gameKeyNames[i].m_action;
	}

	fclose(file);
	return TRUE;
}

// Runs an analog binding's axis keys for the frame: _reset centers the axis (and the device's
// channel), _set works the position back from the output, and unless something already drove the
// axis, _plus and _minus ramp it. Returns whether the axis was driven.
// FUNCTION: MW2 0x1007aecc
// FUNCTION: MW2GBL 0x10032f8d
// FUNCTION: MW2PREMERCS 0x100708cf
// FUNCTION: MW2MATROX 0x1007d57b
// FUNCTION: MW23DFX 0x10039ee0
// FUNCTION: MW2ATI 0x10038aa0
// FUNCTION: MW2S3 0x10032050
// FUNCTION: MW2POWERVR 0x100337a0
MechS32 UpdateAxisFromKeys(AnalogBinding* p_binding)
{
	InputAxis* axis;
	MechS32 driven;
	MechS32 device;
	MechS32 channel;
	MechDouble position;

	axis = p_binding->m_axis;
	driven = axis->m_driven;
	if (axis->m_resetHeld && *axis->m_resetHeld) {
		if (p_binding->m_channelValue) {
			device = p_binding->m_device;
			channel = p_binding->m_channel;
			g_inputDrivers[device]->m_centerAxis(g_inputDeviceInfos[device].m_driverData, channel);
		}

		*axis->m_rate = 0;
		axis->m_position = axis->m_rest;
		driven = TRUE;
	}
	else if (axis->m_setHeld && *axis->m_setHeld) {
#ifdef MW2_FLOAT_SCALARS
		position = (MechS32) (*axis->m_output * 65536.0f + 0.5f) << axis->m_outputShift;
#else
		position = *axis->m_output << axis->m_outputShift;
#endif
		position -= axis->m_minFixed;
		position *= 2.0;
		position /= axis->m_range;
		position -= 65536.0;
		*axis->m_rate = 0;
		axis->m_position = position;
		driven = TRUE;
	}

	if (!axis->m_driven) {
		if (axis->m_plusHeld && *axis->m_plusHeld) {
			if (*axis->m_rate < 0) {
				*axis->m_rate = 0;
			}

			*axis->m_rate += g_deltaTime * 3 << axis->m_rampShift;
			if (*axis->m_rate > 0x8000) {
				*axis->m_rate = 0x8000;
			}

			axis->m_position += *axis->m_rate;
			if (axis->m_position > 0x10000) {
				axis->m_position = 0x10000;
			}

			driven = TRUE;
		}

		if (axis->m_minusHeld && *axis->m_minusHeld) {
			if (*axis->m_rate > 0) {
				*axis->m_rate = 0;
			}

			*axis->m_rate -= g_deltaTime * 3 << axis->m_rampShift;
			if (*axis->m_rate < -0x8000) {
				*axis->m_rate = -0x8000;
			}

			axis->m_position += *axis->m_rate;
			if (axis->m_position < -0x10000) {
				axis->m_position = -0x10000;
			}

			driven = TRUE;
		}

		if (!driven) {
			*axis->m_rate = 0;
		}
	}

	axis->m_driven = driven;
	return driven;
}

// Loads INPUT.MAP and GAMEKEY.MAP and flushes the keyboard's key codes.
// FUNCTION: MW2 0x1007b177
// FUNCTION: MW2GBL 0x10033238
// FUNCTION: MW2PREMERCS 0x10070b7a
// FUNCTION: MW2MATROX 0x1007d84b
// FUNCTION: MW23DFX 0x1003a080
// FUNCTION: MW2ATI 0x10038c40
// FUNCTION: MW2S3 0x100321e0
// FUNCTION: MW2POWERVR 0x10033940
void FirstInputs(void)
{
	LoadInputMap();
	LoadGamekeyMap();
	g_inputDrivers[g_keyboardDeviceIndex]->m_flushKeyCodes();
}

// Updates the sinks for the frame: runs each axis's _reset and _set keys, clears the button sinks,
// polls the devices (the keyboard only outside the chat), runs the bindings (only the menu_ ones
// while gameplay input is off) and reads the frame's key code. Alt+Enter toggles full screen
// instead.
// FUNCTION: MW2 0x1007b19b
// FUNCTION: MW2GBL 0x1003325c
// FUNCTION: MW2MATROX 0x1007d86f
// FUNCTION: MW23DFX 0x1003a0a0
// FUNCTION: MW2ATI 0x10038c60
// FUNCTION: MW2S3 0x10032200
// FUNCTION: MW2POWERVR 0x10033960
void UpdateInputs(void)
{
	MechS32 i;
	MechS16 keyCode;
	AnalogBinding* binding;
	InputAxis* axis;
	MechS32 device;
	MechS32 channel;
	MechDouble position;
	MechS32 pressed;
	MechS32 value;

	for (i = 0; i < g_analogBindingCount; i++) {
		binding = &g_analogBindings[i];
		axis = binding->m_axis;
		if (axis->m_resetHeld && *axis->m_resetHeld) {
			if (binding->m_channelValue) {
				device = binding->m_device;
				channel = binding->m_channel;
				if (g_inputDevicePresent[device]) {
					g_inputDrivers[device]->m_centerAxis(g_inputDeviceInfos[device].m_driverData, channel);
				}
			}

			axis->m_position = axis->m_rest;
			*axis->m_rate = 0;
			binding->m_lastValue = 0;
		}
		else if (axis->m_setHeld && *axis->m_setHeld) {
#ifdef MW2_FLOAT_SCALARS
			position = (MechS32) (*axis->m_output * 65536.0f + 0.5f) << axis->m_outputShift;
#else
			position = *axis->m_output << axis->m_outputShift;
#endif
			position -= axis->m_minFixed;
			position *= 2.0;
			position /= axis->m_range;
			position -= 65536.0;
			*axis->m_rate = 0;
			axis->m_position = position;
		}

		axis->m_driven = FALSE;
	}

	for (i = 0; i < sizeof(g_inputSinks) / sizeof(g_inputSinks[0]); i++) {
		if (g_inputSinks[i].m_kind != c_inputSinkAxis) {
			*(MechS8*) g_inputSinks[i].m_output = 0;
		}
	}

	for (i = 0; i < g_inputDeviceCount; i++) {
		if (g_inputDevicePresent[i] && (!g_chatRecipient || i != g_keyboardDeviceIndex)) {
			g_inputDrivers[i]->m_poll(
				g_inputDeviceInfos[i].m_driverData,
				g_inputDeviceStates[i].m_axes,
				g_inputDeviceStates[i].m_buttons
			);
		}
	}

	for (i = 0; i < g_discreteBindingCount; i++) {
		if (g_gameplayInputEnabled || g_discreteBindings[i].m_menuOnly) {
			pressed = CheckInputConditions(g_discreteBindings[i].m_conditionCount, g_discreteBindings[i].m_conditions);
			if (!g_discreteBindings[i].m_edge || !g_discreteBindings[i].m_previous) {
				*g_discreteBindings[i].m_output |= (MechS8) pressed;
			}

			g_discreteBindings[i].m_previous = pressed;
		}
	}

	for (i = 0; i < g_analogBindingCount; i++) {
		if (g_gameplayInputEnabled || g_analogBindings[i].m_menuOnly) {
			if (UpdateAxisFromKeys(&g_analogBindings[i])) {
			}
			else if (
				g_analogBindings[i].m_channelValue &&
				CheckInputConditions(g_analogBindings[i].m_conditionCount, g_analogBindings[i].m_conditions)
			) {
				value = *g_analogBindings[i].m_channelValue * g_analogBindings[i].m_sign;
				if (g_analogBindings[i].m_lastValue != value) {
					g_analogBindings[i].m_axis->m_position = value;
				}
#ifdef MW2_AXIS_RATE_POINTER_TEST
				else if (g_analogBindings[i].m_axis->m_rate == &g_analogBindings[i].m_axis->m_position) {
#else
				else if (*g_analogBindings[i].m_axis->m_rate == g_analogBindings[i].m_axis->m_position) {
#endif
					g_analogBindings[i].m_axis->m_position = value;
				}

				g_analogBindings[i].m_lastValue = value;
			}

			value = g_analogBindings[i].m_axis->m_position;
			if (value > 0x10000) {
				value = 0x10000;
			}

			if (value < -0x10000) {
				value = -0x10000;
			}

			value += 0x10000;
			value = g_analogBindings[i].m_axis->m_range * value;
			value /= 2;
			value += g_analogBindings[i].m_axis->m_minFixed;
#ifdef MW2_FLOAT_SCALARS
			value >>= g_analogBindings[i].m_axis->m_outputShift;
			*g_analogBindings[i].m_axis->m_output = value * (1.0f / 65536.0f);
#else
			*g_analogBindings[i].m_axis->m_output = value >> g_analogBindings[i].m_axis->m_outputShift;
#endif
		}
	}

	if (g_inputDevicePresent[g_keyboardDeviceIndex]) {
		g_inputDrivers[g_keyboardDeviceIndex]->m_readKeyCode(&keyCode);
	}

#ifdef MW2_NO_FULLSCREEN_KEY
	g_localSteering.m_keyCode = keyCode;
#else
	if (keyCode == 0x40d) {
		ToggleFullScreen();
	}
	else {
		g_localSteering.m_keyCode = keyCode;
	}
#endif
}

// FUNCTION: MW2 0x1007b704
// FUNCTION: MW2GBL 0x100337c6
// FUNCTION: MW2PREMERCS 0x10071108
// FUNCTION: MW2MATROX 0x1007ddf3
// FUNCTION: MW23DFX 0x1003a360
// FUNCTION: MW2ATI 0x10038f20
// FUNCTION: MW2S3 0x100324a0
// FUNCTION: MW2POWERVR 0x10033c20
void CloseInputDevices(void)
{
	MechS32 i;

	i = g_inputDeviceCount;
	while (i--) {
		if (g_inputDevicePresent[i]) {
			g_inputDrivers[i]->m_closeDevice(&g_inputDeviceInfos[i]);
		}
	}
}

// FUNCTION: MW2 0x1007b768
// FUNCTION: MW2GBL 0x1003382a
// FUNCTION: MW2PREMERCS 0x1007116c
// FUNCTION: MW2MATROX 0x1007de57
// FUNCTION: MW23DFX 0x1003a3c0
// FUNCTION: MW2ATI 0x10038f80
// FUNCTION: MW2S3 0x10032500
// FUNCTION: MW2POWERVR 0x10033c80
void DisableGameplayInput(void)
{
	g_gameplayInputEnabled = FALSE;
}

// FUNCTION: MW2 0x1007b77d
// FUNCTION: MW2GBL 0x1003383f
// FUNCTION: MW2PREMERCS 0x10071181
// FUNCTION: MW2MATROX 0x1007de6c
// FUNCTION: MW23DFX 0x1003a3d0
// FUNCTION: MW2ATI 0x10038f90
// FUNCTION: MW2S3 0x10032510
// FUNCTION: MW2POWERVR 0x10033c90
void EnableGameplayInput(void)
{
	g_gameplayInputEnabled = TRUE;
}

// The game key action of p_keyCode.
// FUNCTION: MW2 0x1007b792
// FUNCTION: MW2GBL 0x10033854
// FUNCTION: MW2PREMERCS 0x10071196
// FUNCTION: MW2MATROX 0x1007de81
// FUNCTION: MW23DFX 0x1003a3e0
// FUNCTION: MW2ATI 0x10038fa0
// FUNCTION: MW2S3 0x10032520
// FUNCTION: MW2POWERVR 0x10033ca0
MechS16 LookupGameKey(MechS16 p_keyCode)
{
	return g_gameKeyByKeyCode[p_keyCode];
}

// Reports an input device error p_code in a message box: 0x6e a missing device, 0x6f a missing
// channel, 0x70 a device that doesn't open. Cancel exits the game.
// FUNCTION: MW2 0x1007b7b1
// FUNCTION: MW2GBL 0x10033873
// FUNCTION: MW2PREMERCS 0x100711b5
// FUNCTION: MW2MATROX 0x1007dea0
// FUNCTION: MW23DFX 0x1003a3f0
// FUNCTION: MW2ATI 0x10038fb0
// FUNCTION: MW2S3 0x10032530
// FUNCTION: MW2POWERVR 0x10033cb0
void ReportInputDeviceError(MechS32 p_code, MechChar* p_channel, MechChar* p_device)
{
	MechS32 hidden;

	hidden = FALSE;
#ifdef MW2_NO_MOUSE_ERRORS
	if (!strcmp("Mouse", p_device)) {
		return;
	}

#endif
	switch (p_code) {
	case 0x6e:
		sprintf(
			g_inputErrorText,
			"The input device \"%s\" does not exist or is not configured properly.\n\nTo eliminate the problem, "
			"you should check the Windows Joystick Control Panel settings, or reconfigure your Cockpit Controls "
			"in the Clan Hall without using \"%s\".\n\nPush OK to disable this device and continue, or push "
			"CANCEL to abort the mission.",
			p_device,
			p_device
		);
		break;
	case 0x6f:
		sprintf(
			g_inputErrorText,
			"The input channel \"%s\" on device \"%s\" does not exist.\n\nIf you have changed your joystick "
			"configuration, you should also reconfigure your Cockpit Controls in the Clan Hall.\n\nPush OK to "
			"disable this control and continue, or push CANCEL to abort the mission.",
			p_channel,
			p_device
		);
		break;
	case 0x70:
		sprintf(
			g_inputErrorText,
			"The input device \"%s\" is not connected properly.\n\nTo eliminate the problem, you should check "
			"that your joystick is plugged in correctly and check the Windows Control Panel settings. "
			"Alternatively, you could reconfigure your Cockpit Controls in the Clan Hall without using "
			"\"%s\".\n\nPush OK to disable this device and continue, or push CANCEL to abort the mission.",
			p_device,
			p_device
		);
		break;
	default:
		sprintf(
			g_inputErrorText,
			"An input device has caused an undefined error. Sorry, no other information is available.\nPush OK "
			"to ignore this error and continue, or push CANCEL to abort the mission."
		);
		break;
	}

	WriteToMw2Log(g_inputErrorText);
	if (ShowCursor(TRUE) <= 0) {
		hidden = TRUE;
		while (ShowCursor(TRUE) < 0) {
		}
	}

	if (g_windowMode == 1) {
		ShowWindow(g_gameWindow, SW_SHOWMINNOACTIVE);
	}

	if (MessageBox(g_gameWindow, g_inputErrorText, "MechWarrior2 Message", MB_OKCANCEL | MB_ICONASTERISK) == IDCANCEL) {
		ShutdownOnError();
		exit(p_code);
	}

	if (g_windowMode == 1) {
		ShowWindow(g_gameWindow, SW_RESTORE);
	}

	if (hidden) {
		while (ShowCursor(FALSE) >= 0) {
		}
	}
}
