/* Symbol source for the GLIDE import library (see glide.def). Only the __stdcall decoration,
   i.e. the argument byte count, matters: LIB reads this object to resolve the .def names, and it
   is never linked into a game DLL. The real prototypes are in glide.h. */

#define IMPORT0(name) \
	void __stdcall name(void) \
	{ \
	}
#define IMPORT1(name) \
	void __stdcall name(int p_0) \
	{ \
	}
#define IMPORT2(name) \
	void __stdcall name(int p_0, int p_1) \
	{ \
	}
#define IMPORT3(name) \
	void __stdcall name(int p_0, int p_1, int p_2) \
	{ \
	}
#define IMPORT4(name) \
	void __stdcall name(int p_0, int p_1, int p_2, int p_3) \
	{ \
	}
#define IMPORT5(name) \
	void __stdcall name(int p_0, int p_1, int p_2, int p_3, int p_4) \
	{ \
	}
#define IMPORT6(name) \
	void __stdcall name(int p_0, int p_1, int p_2, int p_3, int p_4, int p_5) \
	{ \
	}
#define IMPORT15(name) \
	void __stdcall name(int p_0, int p_1, int p_2, int p_3, int p_4, int p_5, int p_6, int p_7, int p_8, int p_9, int p_10, int p_11, int p_12, int p_13, int p_14) \
	{ \
	}

IMPORT4(grAlphaBlendFunction)
IMPORT5(grAlphaCombine)
IMPORT1(grAlphaTestFunction)
IMPORT3(grBufferClear)
IMPORT0(grBufferNumPending)
IMPORT1(grBufferSwap)
IMPORT1(grChromakeyMode)
IMPORT2(grDrawLine)
IMPORT2(grDrawPolygonVertexList)
IMPORT3(grDrawTriangle)
IMPORT1(grGlideGetState)
IMPORT0(grGlideInit)
IMPORT1(grGlideSetState)
IMPORT0(grGlideShutdown)
IMPORT0(grLfbBegin)
IMPORT1(grLfbBypassMode)
IMPORT0(grLfbEnd)
IMPORT1(grLfbGetWritePtr)
IMPORT1(grLfbWriteMode)
IMPORT0(grSstIsBusy)
IMPORT6(grSstOpen)
IMPORT1(grSstQueryHardware)
IMPORT1(grSstSelect)
IMPORT4(grTexCalcMemRequired)
IMPORT3(grTexClampMode)
IMPORT2(grTexCombineFunction)
IMPORT1(guAlphaSource)
IMPORT1(guColorCombineFunction)
IMPORT15(guTexAllocateMemory)
IMPORT3(guTexDownloadMipMap)
IMPORT0(guTexMemReset)
IMPORT1(guTexSource)
