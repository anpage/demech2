#ifndef __3DFX_H__
#define __3DFX_H__

// 3Dfx Interactive's basic types, as Glide and Texus (3rdparty/texus) use them: our own
// declarations of what they need, not the SDK's header.

typedef unsigned char FxU8;
typedef signed char FxI8;
typedef unsigned short FxU16;
typedef signed short FxI16;
typedef signed long FxI32;
typedef unsigned long FxU32;
typedef int FxBool;
typedef float FxFloat;
typedef double FxDouble;

#define FXTRUE 1
#define FXFALSE 0

#define FXBIT(i) (1L << (i))

// Glide's entry points are __stdcall (GLIDE.DLL exports _grGlideInit@0 ...).
#define FX_CALL __stdcall
#define FX_ENTRY

#endif // __3DFX_H__
