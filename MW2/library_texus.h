#ifdef 0
// 3dfx's Texus texture library in MW2_3DFX.DLL (3rdparty/texus, built by vc22/ through
// 3rdparty/texus/texus.cmake): 0x10068270-0x1006e8b0, between the game's code and the import
// thunks. 3dfx built it as a static library with VC++ 2.2 at /Ox /G5 /QIfdiv- (__cdecl), and the
// comparison build compiles it the same way with /Z7, so reccmp sees the C names of its CodeView
// records: the functions, statics included, are named by them. The objects follow the DLL's order,
// the order LINK pulls them from the library in.

// texuslib.obj

// LIBRARY: MW23DFX 0x10068270
// _txLengthToLOD

// LIBRARY: MW23DFX 0x100683f0
// _txDimensionsToAspectRatio

// LIBRARY: MW23DFX 0x100684a0
// _txSmallestLOD

// LIBRARY: MW23DFX 0x100684f0
// txPalToNcc

// LIBRARY: MW23DFX 0x100685f0
// txInit3dfInfo

// LIBRARY: MW23DFX 0x10068710
// txConvert

// util.obj

// LIBRARY: MW23DFX 0x10068980
// txLog2

// LIBRARY: MW23DFX 0x10068b10
// txFloorPow2

// LIBRARY: MW23DFX 0x10068b30
// txCeilPow2

// LIBRARY: MW23DFX 0x10068b50
// txPanic

// LIBRARY: MW23DFX 0x10068b80
// txError

// LIBRARY: MW23DFX 0x10068bb0
// txGCD

// LIBRARY: MW23DFX 0x10068be0
// txYABtoPal256

// LIBRARY: MW23DFX 0x10068cc0
// txNearestColor

// LIBRARY: MW23DFX 0x10068de0
// txAspectRatio

// LIBRARY: MW23DFX 0x10068ee0
// txRectCopy

// LIBRARY: MW23DFX 0x10068f30
// txMemRequired

// LIBRARY: MW23DFX 0x10068f90
// txMipAlloc

// LIBRARY: MW23DFX 0x10069020
// txBasename

// LIBRARY: MW23DFX 0x10069090
// txPathAndBasename

// LIBRARY: MW23DFX 0x100690f0
// txExtension

// LIBRARY: MW23DFX 0x10069140
// txMipFree

// LIBRARY: MW23DFX 0x10069170
// txMalloc

// LIBRARY: MW23DFX 0x10069180
// txFree

// LIBRARY: MW23DFX 0x10069190
// txRealloc

// quantize.obj

// LIBRARY: MW23DFX 0x100691b0
// _txPixQuantize_RGB332

// LIBRARY: MW23DFX 0x100691e0
// _txPixQuantize_RGB332_D4x4

// LIBRARY: MW23DFX 0x100692c0
// _txPixQuantize_RGB332_DErr

// LIBRARY: MW23DFX 0x100695a0
// _txPixQuantize_A8

// LIBRARY: MW23DFX 0x100695b0
// _txPixQuantize_I8

// LIBRARY: MW23DFX 0x10069630
// _txPixQuantize_AI44

// LIBRARY: MW23DFX 0x100696b0
// _txPixQuantize_AI44_D4x4

// LIBRARY: MW23DFX 0x10069770
// _txPixQuantize_AI44_DErr

// LIBRARY: MW23DFX 0x100698c0
// _txPixQuantize_ARGB8332

// LIBRARY: MW23DFX 0x100698f0
// _txPixQuantize_ARGB8332_D4x4

// LIBRARY: MW23DFX 0x100699e0
// _txPixQuantize_ARGB8332_DErr

// LIBRARY: MW23DFX 0x10069a10
// _txPixQuantize_RGB565

// LIBRARY: MW23DFX 0x10069a40
// _txPixQuantize_RGB565_D4x4

// LIBRARY: MW23DFX 0x10069b30
// _txPixQuantize_RGB565_DErr

// LIBRARY: MW23DFX 0x10069e10
// _txPixQuantize_ARGB1555

// LIBRARY: MW23DFX 0x10069e60
// _txPixQuantize_ARGB1555_D4x4

// LIBRARY: MW23DFX 0x10069f60
// _txPixQuantize_ARGB1555_DErr

// LIBRARY: MW23DFX 0x1006a270
// _txPixQuantize_ARGB4444

// LIBRARY: MW23DFX 0x1006a2b0
// _txPixQuantize_ARGB4444_D4x4

// LIBRARY: MW23DFX 0x1006a3a0
// _txPixQuantize_ARGB4444_DErr

// LIBRARY: MW23DFX 0x1006a690
// _txPixQuantize_AI88

// LIBRARY: MW23DFX 0x1006a710
// _txImgQuantize

// LIBRARY: MW23DFX 0x1006aa50
// txMipQuantize

// mipmap.obj

// LIBRARY: MW23DFX 0x1006ac60
// _txImgHalve

// LIBRARY: MW23DFX 0x1006aec0
// txMipMipmap

// clamp.obj

// LIBRARY: MW23DFX 0x1006af80
// _txImgClamp

// LIBRARY: MW23DFX 0x1006b030
// txMipClamp

// resample.obj

// LIBRARY: MW23DFX 0x1006b1c0
// _txResampleX

// LIBRARY: MW23DFX 0x1006b3c0
// _txImgResample

// LIBRARY: MW23DFX 0x1006b6c0
// txMipResample

// dequant.obj

// LIBRARY: MW23DFX 0x1006b880
// _txPixRgb332to8888

// LIBRARY: MW23DFX 0x1006b8d0
// _txPixA8to8888

// LIBRARY: MW23DFX 0x1006b8e0
// _txPixI8to8888

// LIBRARY: MW23DFX 0x1006b900
// _txPixAi44to8888

// LIBRARY: MW23DFX 0x1006b940
// _txPixArgb8332to8888

// LIBRARY: MW23DFX 0x1006b9b0
// _txPixRgb565to8888

// LIBRARY: MW23DFX 0x1006ba10
// _txPixArgb1555to8888

// LIBRARY: MW23DFX 0x1006ba80
// _txPixRgba4444to8888

// LIBRARY: MW23DFX 0x1006baf0
// _txPixAi88to8888

// LIBRARY: MW23DFX 0x1006bb20
// _txImgDequantizeRGB332

// LIBRARY: MW23DFX 0x1006bb60
// _txImgDequantizeYIQ422

// LIBRARY: MW23DFX 0x1006bbd0
// _txImgDequantizeA8

// LIBRARY: MW23DFX 0x1006bc10
// _txImgDequantizeI8

// LIBRARY: MW23DFX 0x1006bc50
// _txImgDequantizeAI44

// LIBRARY: MW23DFX 0x1006bc90
// _txImgDequantizeP8

// LIBRARY: MW23DFX 0x1006bce0
// _txImgDequantizeARGB8332

// LIBRARY: MW23DFX 0x1006bd30
// _txImgDequantizeAYIQ8422

// LIBRARY: MW23DFX 0x1006bdb0
// _txImgDequantizeRGB565

// LIBRARY: MW23DFX 0x1006be00
// _txImgDequantizeARGB1555

// LIBRARY: MW23DFX 0x1006be50
// _txImgDequantizeARGB4444

// LIBRARY: MW23DFX 0x1006bea0
// _txImgDequantizeAI88

// LIBRARY: MW23DFX 0x1006bef0
// _txImgDequantizeAP88

// LIBRARY: MW23DFX 0x1006bf50
// _txImgDequantizeARGB8888

// LIBRARY: MW23DFX 0x1006bf90
// txMipDequantize

// pal256.obj

// LIBRARY: MW23DFX 0x1006c1c0
// txMipPal256

// LIBRARY: MW23DFX 0x1006c590
// QuantHistogram

// LIBRARY: MW23DFX 0x1006c610
// CutBoxes

// LIBRARY: MW23DFX 0x1006c6b0
// GreatestVariance

// LIBRARY: MW23DFX 0x1006c700
// BoxStats

// LIBRARY: MW23DFX 0x1006c810
// CutBox

// LIBRARY: MW23DFX 0x1006c970
// FindCutpoint

// LIBRARY: MW23DFX 0x1006cb60
// UpdateFrequencies

// LIBRARY: MW23DFX 0x1006ccf0
// ComputeRGBMap

// LIBRARY: MW23DFX 0x1006cd30
// SetRGBmap

// ncc.obj

// LIBRARY: MW23DFX 0x1006cdd0
// _txPixQuantize_YIQ422

// LIBRARY: MW23DFX 0x1006cff0
// _txPixQuantize_YIQ422_D4x4

// LIBRARY: MW23DFX 0x1006d1f0
// _txImgNcc

// LIBRARY: MW23DFX 0x1006d2a0
// _txMipNccStatTable

// LIBRARY: MW23DFX 0x1006d580
// txMipNcc

// diffuse.obj

// LIBRARY: MW23DFX 0x1006d8e0
// _txToDiffuseIndex

// LIBRARY: MW23DFX 0x1006dc30
// txDiffuseIndex

// nccnnet.obj

// LIBRARY: MW23DFX 0x1006dcf0
// _nn_modifyNeurons

// LIBRARY: MW23DFX 0x1006e1d0
// _nn_initTables

// LIBRARY: MW23DFX 0x1006e390
// _nn_randomOrder

// LIBRARY: MW23DFX 0x1006e3b0
// txMapPal256toYAB

// LIBRARY: MW23DFX 0x1006e670
// txMipNccNNet

// Data

// GLOBAL: MW23DFX 0x10266650
// txVerbose

// GLOBAL: MW23DFX 0x102666e0
// Format_Name

// GLOBAL: MW23DFX 0x10266728
// _explode3

// GLOBAL: MW23DFX 0x10266f24
// explode3

// GLOBAL: MW23DFX 0x102672b0
// P1_8

// GLOBAL: MW23DFX 0x102672b4
// P2_8

// GLOBAL: MW23DFX 0x102672b8
// P3_8

// GLOBAL: MW23DFX 0x102672c0
// P4_8

// GLOBAL: MW23DFX 0x102672d0
// P5_8

// GLOBAL: MW23DFX 0x102672f0
// P6_8

#endif
