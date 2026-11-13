#ifndef DDRAW_H
#define DDRAW_H

// DDRAW.DLL, DirectDraw as DirectX 2 shipped it, which the shell and the simulator draw their
// full-screen modes through (ddraw.def): our own declarations of what the game uses, not the SDK's
// header. Names, layouts, vtable slot order and values are DirectDraw's. The interfaces the game
// calls declare every slot, so that each method sits at its offset; the C units call them through
// the IDirectDraw*_Method macros, which only C gets. Structures and interfaces the game never
// touches but that appear in a method's signature are left incomplete. clang-format is off for
// the interfaces: RemoveSemicolon takes a DECLARE_INTERFACE_ body for a function's and drops the
// semicolon C++ needs.
#include <objbase.h>
#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct IDirectDraw* LPDIRECTDRAW;
	typedef struct IDirectDrawSurface* LPDIRECTDRAWSURFACE;
	typedef struct IDirectDrawPalette* LPDIRECTDRAWPALETTE;
	typedef struct IDirectDrawClipper* LPDIRECTDRAWCLIPPER;

	typedef struct _DDSURFACEDESC* LPDDSURFACEDESC;
	typedef struct _DDOVERLAYFX* LPDDOVERLAYFX;
	typedef struct _DDBLTBATCH* LPDDBLTBATCH;

	// DD_OK, or an error of facility 0x876 (MAKE_HRESULT(1, 0x876, code)).
#define DD_OK 0
#define DDERR_SURFACELOST ((HRESULT) 0x887601c2L) // code 450

	HRESULT WINAPI DirectDrawCreate(GUID* p_guid, LPDIRECTDRAW* p_directDraw, IUnknown* p_unkOuter);

	typedef HRESULT(PASCAL* LPDDENUMMODESCALLBACK)(LPDDSURFACEDESC p_desc, LPVOID p_context);
	typedef HRESULT(PASCAL* LPDDENUMSURFACESCALLBACK)(
		LPDIRECTDRAWSURFACE p_surface,
		LPDDSURFACEDESC p_desc,
		LPVOID p_context
	);

	// A range of color values treated as transparent (both bounds inclusive). 0x08 bytes.
	typedef struct _DDCOLORKEY {
		DWORD dwColorSpaceLowValue;  // 0x00
		DWORD dwColorSpaceHighValue; // 0x04
	} DDCOLORKEY;
	typedef DDCOLORKEY* LPDDCOLORKEY;

	// The optional parameters of IDirectDrawSurface::Blt; the game fills with dwFillColor.
	// 0x64 bytes on Win32.
	typedef struct _DDBLTFX {
		DWORD dwSize;               // 0x00
		DWORD dwDDFX;               // 0x04
		DWORD dwROP;                // 0x08
		DWORD dwDDROP;              // 0x0c
		DWORD dwRotationAngle;      // 0x10
		DWORD dwZBufferOpCode;      // 0x14
		DWORD dwZBufferLow;         // 0x18
		DWORD dwZBufferHigh;        // 0x1c
		DWORD dwZBufferBaseDest;    // 0x20
		DWORD dwZDestConstBitDepth; // 0x24
		union {
			DWORD dwZDestConst;
			LPDIRECTDRAWSURFACE lpDDSZBufferDest;
		}; // 0x28
		DWORD dwZSrcConstBitDepth; // 0x2c
		union {
			DWORD dwZSrcConst;
			LPDIRECTDRAWSURFACE lpDDSZBufferSrc;
		}; // 0x30
		DWORD dwAlphaEdgeBlendBitDepth; // 0x34
		DWORD dwAlphaEdgeBlend;         // 0x38
		DWORD dwReserved;               // 0x3c
		DWORD dwAlphaDestConstBitDepth; // 0x40
		union {
			DWORD dwAlphaDestConst;
			LPDIRECTDRAWSURFACE lpDDSAlphaDest;
		}; // 0x44
		DWORD dwAlphaSrcConstBitDepth; // 0x48
		union {
			DWORD dwAlphaSrcConst;
			LPDIRECTDRAWSURFACE lpDDSAlphaSrc;
		}; // 0x4c
		union {
			DWORD dwFillColor;
			DWORD dwFillDepth;
			LPDIRECTDRAWSURFACE lpDDSPattern;
		}; // 0x50
		DDCOLORKEY ddckDestColorkey; // 0x54
		DDCOLORKEY ddckSrcColorkey;  // 0x5c
	} DDBLTFX;
	typedef DDBLTFX* LPDDBLTFX;

	// A surface's capabilities (DDSCAPS_*). 0x04 bytes.
	typedef struct _DDSCAPS {
		DWORD dwCaps; // 0x00
	} DDSCAPS;
	typedef DDSCAPS* LPDDSCAPS;

	// The capabilities IDirectDraw::GetCaps reports for the driver and for the emulation layer,
	// in DirectX 2's size (0x13c bytes). The ROP arrays hold one bit per raster operation.
	typedef struct _DDCAPS {
		DWORD dwSize;                         // 0x00
		DWORD dwCaps;                         // 0x04: DDCAPS_*
		DWORD dwCaps2;                        // 0x08
		DWORD dwCKeyCaps;                     // 0x0c
		DWORD dwFXCaps;                       // 0x10
		DWORD dwFXAlphaCaps;                  // 0x14
		DWORD dwPalCaps;                      // 0x18
		DWORD dwSVCaps;                       // 0x1c
		DWORD dwAlphaBltConstBitDepths;       // 0x20
		DWORD dwAlphaBltPixelBitDepths;       // 0x24
		DWORD dwAlphaBltSurfaceBitDepths;     // 0x28
		DWORD dwAlphaOverlayConstBitDepths;   // 0x2c
		DWORD dwAlphaOverlayPixelBitDepths;   // 0x30
		DWORD dwAlphaOverlaySurfaceBitDepths; // 0x34
		DWORD dwZBufferBitDepths;             // 0x38
		DWORD dwVidMemTotal;                  // 0x3c
		DWORD dwVidMemFree;                   // 0x40
		DWORD dwMaxVisibleOverlays;           // 0x44
		DWORD dwCurrVisibleOverlays;          // 0x48
		DWORD dwNumFourCCCodes;               // 0x4c
		DWORD dwAlignBoundarySrc;             // 0x50
		DWORD dwAlignSizeSrc;                 // 0x54
		DWORD dwAlignBoundaryDest;            // 0x58
		DWORD dwAlignSizeDest;                // 0x5c
		DWORD dwAlignStrideAlign;             // 0x60
		DWORD dwRops[8];                      // 0x64
		DDSCAPS ddsCaps;                      // 0x84
		DWORD dwMinOverlayStretch;            // 0x88
		DWORD dwMaxOverlayStretch;            // 0x8c
		DWORD dwMinLiveVideoStretch;          // 0x90
		DWORD dwMaxLiveVideoStretch;          // 0x94
		DWORD dwMinHwCodecStretch;            // 0x98
		DWORD dwMaxHwCodecStretch;            // 0x9c
		DWORD dwReserved1;                    // 0xa0
		DWORD dwReserved2;                    // 0xa4
		DWORD dwReserved3;                    // 0xa8
		DWORD dwSVBCaps;                      // 0xac: system to video memory blits
		DWORD dwSVBCKeyCaps;                  // 0xb0
		DWORD dwSVBFXCaps;                    // 0xb4
		DWORD dwSVBRops[8];                   // 0xb8
		DWORD dwVSBCaps;                      // 0xd8: video to system memory blits
		DWORD dwVSBCKeyCaps;                  // 0xdc
		DWORD dwVSBFXCaps;                    // 0xe0
		DWORD dwVSBRops[8];                   // 0xe4
		DWORD dwSSBCaps;                      // 0x104: system to system memory blits
		DWORD dwSSBCKeyCaps;                  // 0x108
		DWORD dwSSBFXCaps;                    // 0x10c
		DWORD dwSSBRops[8];                   // 0x110
		DWORD dwReserved4;                    // 0x130
		DWORD dwReserved5;                    // 0x134
		DWORD dwReserved6;                    // 0x138
	} DDCAPS;
	typedef DDCAPS* LPDDCAPS;

#define DDCAPS_BANKSWITCHED 0x08000000L

	// A surface's pixel format; the unions name each field for RGB, YUV, Z and alpha formats.
	// 0x20 bytes.
	typedef struct _DDPIXELFORMAT {
		DWORD dwSize;   // 0x00
		DWORD dwFlags;  // 0x04
		DWORD dwFourCC; // 0x08
		union {
			DWORD dwRGBBitCount;
			DWORD dwYUVBitCount;
			DWORD dwZBufferBitDepth;
			DWORD dwAlphaBitDepth;
		}; // 0x0c
		union {
			DWORD dwRBitMask;
			DWORD dwYBitMask;
		}; // 0x10
		union {
			DWORD dwGBitMask;
			DWORD dwUBitMask;
		}; // 0x14
		union {
			DWORD dwBBitMask;
			DWORD dwVBitMask;
		}; // 0x18
		union {
			DWORD dwRGBAlphaBitMask;
			DWORD dwYUVAlphaBitMask;
		}; // 0x1c
	} DDPIXELFORMAT;
	typedef DDPIXELFORMAT* LPDDPIXELFORMAT;

	// A surface's description: what to create (dwFlags says which fields are set), or what
	// GetSurfaceDesc and Lock report. 0x6c bytes on Win32.
	typedef struct _DDSURFACEDESC {
		DWORD dwSize;            // 0x00
		DWORD dwFlags;           // 0x04: DDSD_*
		DWORD dwHeight;          // 0x08
		DWORD dwWidth;           // 0x0c
		LONG lPitch;             // 0x10
		DWORD dwBackBufferCount; // 0x14
		union {
			DWORD dwMipMapCount;
			DWORD dwZBufferBitDepth;
			DWORD dwRefreshRate;
		}; // 0x18
		DWORD dwAlphaBitDepth;         // 0x1c
		DWORD dwReserved;              // 0x20
		LPVOID lpSurface;              // 0x24
		DDCOLORKEY ddckCKDestOverlay;  // 0x28
		DDCOLORKEY ddckCKDestBlt;      // 0x30
		DDCOLORKEY ddckCKSrcOverlay;   // 0x38
		DDCOLORKEY ddckCKSrcBlt;       // 0x40
		DDPIXELFORMAT ddpfPixelFormat; // 0x48
		DDSCAPS ddsCaps;               // 0x68
	} DDSURFACEDESC;

#define DDSD_CAPS 0x00000001L
#define DDSD_HEIGHT 0x00000002L
#define DDSD_WIDTH 0x00000004L
#define DDSD_BACKBUFFERCOUNT 0x00000020L

#define DDSCAPS_BACKBUFFER 0x00000004L
#define DDSCAPS_COMPLEX 0x00000008L
#define DDSCAPS_FLIP 0x00000010L
#define DDSCAPS_OFFSCREENPLAIN 0x00000040L
#define DDSCAPS_PRIMARYSURFACE 0x00000200L
#define DDSCAPS_SYSTEMMEMORY 0x00000800L
#define DDSCAPS_VIDEOMEMORY 0x00004000L

	// IDirectDraw::CreatePalette's flags.
#define DDPCAPS_8BIT 0x00000004L
#define DDPCAPS_ALLOW256 0x00000040L

	// IDirectDraw::SetCooperativeLevel's flags.
#define DDSCL_FULLSCREEN 0x00000001L
#define DDSCL_NORMAL 0x00000008L
#define DDSCL_EXCLUSIVE 0x00000010L

	// IDirectDrawSurface::Blt's, BltFast's, Flip's and Lock's flags.
#define DDBLT_COLORFILL 0x00000400L
#define DDBLT_WAIT 0x01000000L
#define DDBLTFAST_WAIT 0x00000010
#define DDFLIP_WAIT 0x00000001L
#define DDLOCK_WAIT 0x00000001L

	// The DirectDraw object: display modes, cooperation with GDI, and the factory of surfaces
	// and palettes.
	// clang-format off
#undef INTERFACE
#define INTERFACE IDirectDraw
	DECLARE_INTERFACE_(IDirectDraw, IUnknown)
	{
		STDMETHOD(QueryInterface)(THIS_ REFIID p_iid, LPVOID * p_object) PURE; // vtable+0x00
		STDMETHOD_(ULONG, AddRef)(THIS) PURE;                                  // vtable+0x04
		STDMETHOD_(ULONG, Release)(THIS) PURE;                                 // vtable+0x08
		STDMETHOD(Compact)(THIS) PURE;                                         // vtable+0x0c
		STDMETHOD(CreateClipper)(THIS_ DWORD p_flags, LPDIRECTDRAWCLIPPER * p_clipper, IUnknown * p_unkOuter)
			PURE; // vtable+0x10
		STDMETHOD(CreatePalette)(
			THIS_ DWORD p_flags,
			LPPALETTEENTRY p_entries,
			LPDIRECTDRAWPALETTE * p_palette,
			IUnknown * p_unkOuter
		) PURE; // vtable+0x14
		STDMETHOD(CreateSurface)(THIS_ LPDDSURFACEDESC p_desc, LPDIRECTDRAWSURFACE * p_surface, IUnknown * p_unkOuter)
			PURE; // vtable+0x18
		STDMETHOD(DuplicateSurface)(THIS_ LPDIRECTDRAWSURFACE p_surface, LPDIRECTDRAWSURFACE * p_duplicate)
			PURE; // vtable+0x1c
		STDMETHOD(EnumDisplayModes)(
			THIS_ DWORD p_flags,
			LPDDSURFACEDESC p_desc,
			LPVOID p_context,
			LPDDENUMMODESCALLBACK p_callback
		) PURE; // vtable+0x20
		STDMETHOD(EnumSurfaces)(
			THIS_ DWORD p_flags,
			LPDDSURFACEDESC p_desc,
			LPVOID p_context,
			LPDDENUMSURFACESCALLBACK p_callback
		) PURE;                                                                           // vtable+0x24
		STDMETHOD(FlipToGDISurface)(THIS) PURE;                                           // vtable+0x28
		STDMETHOD(GetCaps)(THIS_ LPDDCAPS p_driverCaps, LPDDCAPS p_emulationCaps) PURE;   // vtable+0x2c
		STDMETHOD(GetDisplayMode)(THIS_ LPDDSURFACEDESC p_desc) PURE;                     // vtable+0x30
		STDMETHOD(GetFourCCCodes)(THIS_ LPDWORD p_count, LPDWORD p_codes) PURE;           // vtable+0x34
		STDMETHOD(GetGDISurface)(THIS_ LPDIRECTDRAWSURFACE * p_surface) PURE;             // vtable+0x38
		STDMETHOD(GetMonitorFrequency)(THIS_ LPDWORD p_frequency) PURE;                   // vtable+0x3c
		STDMETHOD(GetScanLine)(THIS_ LPDWORD p_line) PURE;                                // vtable+0x40
		STDMETHOD(GetVerticalBlankStatus)(THIS_ LPBOOL p_inVerticalBlank) PURE;           // vtable+0x44
		STDMETHOD(Initialize)(THIS_ GUID * p_guid) PURE;                                  // vtable+0x48
		STDMETHOD(RestoreDisplayMode)(THIS) PURE;                                         // vtable+0x4c
		STDMETHOD(SetCooperativeLevel)(THIS_ HWND p_hWnd, DWORD p_flags) PURE;            // vtable+0x50
		STDMETHOD(SetDisplayMode)(THIS_ DWORD p_width, DWORD p_height, DWORD p_bpp) PURE; // vtable+0x54
		STDMETHOD(WaitForVerticalBlank)(THIS_ DWORD p_flags, HANDLE p_event) PURE;        // vtable+0x58
	};
	// clang-format on

	// A palette of a DirectDraw surface.
	// clang-format off
#undef INTERFACE
#define INTERFACE IDirectDrawPalette
	DECLARE_INTERFACE_(IDirectDrawPalette, IUnknown)
	{
		STDMETHOD(QueryInterface)(THIS_ REFIID p_iid, LPVOID * p_object) PURE; // vtable+0x00
		STDMETHOD_(ULONG, AddRef)(THIS) PURE;                                  // vtable+0x04
		STDMETHOD_(ULONG, Release)(THIS) PURE;                                 // vtable+0x08
		STDMETHOD(GetCaps)(THIS_ LPDWORD p_caps) PURE;                         // vtable+0x0c
		STDMETHOD(GetEntries)(THIS_ DWORD p_flags, DWORD p_base, DWORD p_count, LPPALETTEENTRY p_entries)
			PURE; // vtable+0x10
		STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW p_directDraw, DWORD p_flags, LPPALETTEENTRY p_entries)
			PURE; // vtable+0x14
		STDMETHOD(SetEntries)(THIS_ DWORD p_flags, DWORD p_base, DWORD p_count, LPPALETTEENTRY p_entries)
			PURE; // vtable+0x18
	};
	// clang-format on

	// A DirectDraw surface: the primary surface, its flipping chain, and offscreen buffers.
	// clang-format off
#undef INTERFACE
#define INTERFACE IDirectDrawSurface
	DECLARE_INTERFACE_(IDirectDrawSurface, IUnknown)
	{
		STDMETHOD(QueryInterface)(THIS_ REFIID p_iid, LPVOID * p_object) PURE;   // vtable+0x00
		STDMETHOD_(ULONG, AddRef)(THIS) PURE;                                    // vtable+0x04
		STDMETHOD_(ULONG, Release)(THIS) PURE;                                   // vtable+0x08
		STDMETHOD(AddAttachedSurface)(THIS_ LPDIRECTDRAWSURFACE p_surface) PURE; // vtable+0x0c
		STDMETHOD(AddOverlayDirtyRect)(THIS_ LPRECT p_rect) PURE;                // vtable+0x10
		STDMETHOD(Blt)(
			THIS_ LPRECT p_destRect,
			LPDIRECTDRAWSURFACE p_source,
			LPRECT p_sourceRect,
			DWORD p_flags,
			LPDDBLTFX p_fx
		) PURE;                                                                             // vtable+0x14
		STDMETHOD(BltBatch)(THIS_ LPDDBLTBATCH p_batch, DWORD p_count, DWORD p_flags) PURE; // vtable+0x18
		STDMETHOD(BltFast)(
			THIS_ DWORD p_x,
			DWORD p_y,
			LPDIRECTDRAWSURFACE p_source,
			LPRECT p_sourceRect,
			DWORD p_flags
		) PURE;                                                                                    // vtable+0x1c
		STDMETHOD(DeleteAttachedSurface)(THIS_ DWORD p_flags, LPDIRECTDRAWSURFACE p_surface) PURE; // vtable+0x20
		STDMETHOD(EnumAttachedSurfaces)(THIS_ LPVOID p_context, LPDDENUMSURFACESCALLBACK p_callback)
			PURE; // vtable+0x24
		STDMETHOD(EnumOverlayZOrders)(THIS_ DWORD p_flags, LPVOID p_context, LPDDENUMSURFACESCALLBACK p_callback)
			PURE;                                                                                         // vtable+0x28
		STDMETHOD(Flip)(THIS_ LPDIRECTDRAWSURFACE p_target, DWORD p_flags) PURE;                          // vtable+0x2c
		STDMETHOD(GetAttachedSurface)(THIS_ LPDDSCAPS p_caps, LPDIRECTDRAWSURFACE * p_surface) PURE;      // vtable+0x30
		STDMETHOD(GetBltStatus)(THIS_ DWORD p_flags) PURE;                                                // vtable+0x34
		STDMETHOD(GetCaps)(THIS_ LPDDSCAPS p_caps) PURE;                                                  // vtable+0x38
		STDMETHOD(GetClipper)(THIS_ LPDIRECTDRAWCLIPPER * p_clipper) PURE;                                // vtable+0x3c
		STDMETHOD(GetColorKey)(THIS_ DWORD p_flags, LPDDCOLORKEY p_colorKey) PURE;                        // vtable+0x40
		STDMETHOD(GetDC)(THIS_ HDC * p_hdc) PURE;                                                         // vtable+0x44
		STDMETHOD(GetFlipStatus)(THIS_ DWORD p_flags) PURE;                                               // vtable+0x48
		STDMETHOD(GetOverlayPosition)(THIS_ LPLONG p_x, LPLONG p_y) PURE;                                 // vtable+0x4c
		STDMETHOD(GetPalette)(THIS_ LPDIRECTDRAWPALETTE * p_palette) PURE;                                // vtable+0x50
		STDMETHOD(GetPixelFormat)(THIS_ LPDDPIXELFORMAT p_format) PURE;                                   // vtable+0x54
		STDMETHOD(GetSurfaceDesc)(THIS_ LPDDSURFACEDESC p_desc) PURE;                                     // vtable+0x58
		STDMETHOD(Initialize)(THIS_ LPDIRECTDRAW p_directDraw, LPDDSURFACEDESC p_desc) PURE;              // vtable+0x5c
		STDMETHOD(IsLost)(THIS) PURE;                                                                     // vtable+0x60
		STDMETHOD(Lock)(THIS_ LPRECT p_rect, LPDDSURFACEDESC p_desc, DWORD p_flags, HANDLE p_event) PURE; // vtable+0x64
		STDMETHOD(ReleaseDC)(THIS_ HDC p_hdc) PURE;                                                       // vtable+0x68
		STDMETHOD(Restore)(THIS) PURE;                                                                    // vtable+0x6c
		STDMETHOD(SetClipper)(THIS_ LPDIRECTDRAWCLIPPER p_clipper) PURE;                                  // vtable+0x70
		STDMETHOD(SetColorKey)(THIS_ DWORD p_flags, LPDDCOLORKEY p_colorKey) PURE;                        // vtable+0x74
		STDMETHOD(SetOverlayPosition)(THIS_ LONG p_x, LONG p_y) PURE;                                     // vtable+0x78
		STDMETHOD(SetPalette)(THIS_ LPDIRECTDRAWPALETTE p_palette) PURE;                                  // vtable+0x7c
		STDMETHOD(Unlock)(THIS_ LPVOID p_surface) PURE;                                                   // vtable+0x80
		STDMETHOD(UpdateOverlay)(
			THIS_ LPRECT p_sourceRect,
			LPDIRECTDRAWSURFACE p_dest,
			LPRECT p_destRect,
			DWORD p_flags,
			LPDDOVERLAYFX p_fx
		) PURE;                                                                                    // vtable+0x84
		STDMETHOD(UpdateOverlayDisplay)(THIS_ DWORD p_flags) PURE;                                 // vtable+0x88
		STDMETHOD(UpdateOverlayZOrder)(THIS_ DWORD p_flags, LPDIRECTDRAWSURFACE p_reference) PURE; // vtable+0x8c
	};
	// clang-format on

#if !defined(__cplusplus) || defined(CINTERFACE)
	// The C units' method calls.
#define IDirectDraw_Release(p) (p)->lpVtbl->Release(p)
#define IDirectDraw_CreatePalette(p, a, b, c, d) (p)->lpVtbl->CreatePalette(p, a, b, c, d)
#define IDirectDraw_CreateSurface(p, a, b, c) (p)->lpVtbl->CreateSurface(p, a, b, c)
#define IDirectDraw_GetCaps(p, a, b) (p)->lpVtbl->GetCaps(p, a, b)
#define IDirectDraw_RestoreDisplayMode(p) (p)->lpVtbl->RestoreDisplayMode(p)
#define IDirectDraw_SetCooperativeLevel(p, a, b) (p)->lpVtbl->SetCooperativeLevel(p, a, b)
#define IDirectDraw_SetDisplayMode(p, a, b, c) (p)->lpVtbl->SetDisplayMode(p, a, b, c)

#define IDirectDrawPalette_Release(p) (p)->lpVtbl->Release(p)
#define IDirectDrawPalette_SetEntries(p, a, b, c, d) (p)->lpVtbl->SetEntries(p, a, b, c, d)

#define IDirectDrawSurface_Release(p) (p)->lpVtbl->Release(p)
#define IDirectDrawSurface_Blt(p, a, b, c, d, e) (p)->lpVtbl->Blt(p, a, b, c, d, e)
#define IDirectDrawSurface_BltFast(p, a, b, c, d, e) (p)->lpVtbl->BltFast(p, a, b, c, d, e)
#define IDirectDrawSurface_Flip(p, a, b) (p)->lpVtbl->Flip(p, a, b)
#define IDirectDrawSurface_GetAttachedSurface(p, a, b) (p)->lpVtbl->GetAttachedSurface(p, a, b)
#define IDirectDrawSurface_GetSurfaceDesc(p, a) (p)->lpVtbl->GetSurfaceDesc(p, a)
#define IDirectDrawSurface_IsLost(p) (p)->lpVtbl->IsLost(p)
#define IDirectDrawSurface_Lock(p, a, b, c, d) (p)->lpVtbl->Lock(p, a, b, c, d)
#define IDirectDrawSurface_Restore(p) (p)->lpVtbl->Restore(p)
#define IDirectDrawSurface_SetPalette(p, a) (p)->lpVtbl->SetPalette(p, a)
#define IDirectDrawSurface_Unlock(p, a) (p)->lpVtbl->Unlock(p, a)
#endif

#ifdef __cplusplus
}
#endif

#endif // DDRAW_H
