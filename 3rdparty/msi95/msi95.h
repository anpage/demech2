#ifndef MSI95_H
#define MSI95_H

// MSI95.DLL (Matrox Simple Interface, the Mystique's 3D API), which the Matrox edition draws
// through: our own declarations of the functions the game imports (msi95.def) and of the types they
// take, not Matrox's header. Names, types and values follow the SDK's msi95.h, which declares no
// msiSync: the edition's library is another version of the interface.
#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

	// What msiInit returns: the frame buffer's format and memory, the Z buffer's and the texture
	// cache's.
	// SIZE 0x54
	typedef struct _T_msiInfo {
		SHORT VersionMajor; // 0x00
		SHORT VersionMinor; // 0x02

		struct {
			LONG BytesPerPixel; // 0x04
			LONG rDepth;        // 0x08
			LONG gDepth;        // 0x0c
			LONG bDepth;        // 0x10
			LONG aDepth;        // 0x14
			LONG rBitShift;     // 0x18
			LONG gBitShift;     // 0x1c
			LONG bBitShift;     // 0x20
			LONG aBitShift;     // 0x24
			LONG Pitch;         // 0x28 — in bytes
			LPBYTE pMem;        // 0x2c
			LONG MemSize;       // 0x30 — in bytes
		} msiColor;

		struct {
			LONG BytesPerPixel; // 0x34
			LONG zDepth;        // 0x38
			LONG zBitShift;     // 0x3c
			LONG Pitch;         // 0x40 — in bytes
			LPBYTE pMem;        // 0x44
			LONG MemSize;       // 0x48 — in bytes
		} msiDepth;

		struct {
			LPBYTE pMem;  // 0x4c — the texture cache
			LONG MemSize; // 0x50
		} msiTexture;
	} T_msiInfo;

	// The rendering parameters (msiSetParameters): the texture, its look-up table, how it is applied
	// and keyed, the depth compare and the color processing.
	// SIZE 0x58
	typedef struct _T_msiParameters {
		struct {
			BOOL Enable;      // 0x00
			long Width;       // 0x04
			long Height;      // 0x08
			long Planes;      // 0x0c
			LPBYTE pMem;      // 0x10
			LPBYTE pHeap;     // 0x14
			long CacheOffset; // 0x18

			struct {
				LPBYTE pMem;      // 0x1c
				LPBYTE pHeap;     // 0x20
				long CacheOffset; // 0x24
			} msiLUT;

			BOOL Clamp_u;         // 0x28
			BOOL Clamp_v;         // 0x2c
			BOOL Modulate;        // 0x30
			BOOL Decal;           // 0x34
			BOOL Transparency;    // 0x38
			WORD KeyingColor;     // 0x3c
			WORD KeyingColorMask; // 0x3e
			WORD KeyingAlpha;     // 0x40
			WORD KeyingAlphaMask; // 0x42 — 1 bit
		} msiTexture;

		struct {
			BOOL Enable;   // 0x44
			DWORD Compare; // 0x48 — an msiCMP_ code
			BOOL Protect;  // 0x4c
		} msiDepth;

		struct {
			BOOL Dither;  // 0x50
			BOOL Protect; // 0x54
		} msiColor;
	} T_msiParameters;

	// The depth compares (T_msiParameters.msiDepth.Compare).
#define msiCMP_NEVER 1

	// Waits for the card (SimMain calls it once a frame). Not in the SDK's header.
	void __cdecl msiSync(void);

	BOOL __cdecl msiSetParameters(T_msiParameters* pParameters);
	BOOL __cdecl msiStartFrame(BOOL ClearRGB, float r, float g, float b, BOOL ClearZ, float z);
	BOOL __cdecl msiEndFrame(BOOL Dump, LONG Frame, BOOL Wait);
	BOOL __cdecl msiFreeTextureHeap(LPBYTE pTextureHeap);
	LPBYTE __cdecl msiAllocTextureHeap(LONG Pages);
	// pMemStatus is the SDK's T_msiMemoryStatus, which the game passes as the source.
	BOOL __cdecl msiBlitRect(
		LPBYTE Pheap,
		LPBYTE Pmemory,
		void* pMemStatus,
		WORD SourcePitch,
		WORD SourcePlanes,
		WORD BlitWidth,
		WORD BlitHeight,
		DWORD DestinationOff,
		DWORD KeyingColor,
		DWORD KeyingMask
	);
	BOOL __cdecl msiDrawSingleLine(DWORD color, DWORD XYStart, DWORD XYEnd, DWORD LineStyle);
	// The vertices are the SDK's T_msiVertex, the Matrox edition's A3DVertex.
	BOOL __cdecl msiRenderTriangle(void* pV0, void* pV1, void* pV2, BYTE Opacity);
	T_msiInfo* __cdecl msiInit(LONG Width, LONG Height, LONG Planes, BOOL ZBuffer, BOOL Debug, WNDPROC lpWindowProc);
	BOOL __cdecl msiExit(void);

#ifdef __cplusplus
}
#endif

#endif // MSI95_H
