#ifndef S3DTK_H
#define S3DTK_H

// S3DTKW.DLL, S3's ViRGE toolkit (the S3D ToolKit), as the S3 edition uses it (s3dtk.def): our own
// declarations of the functions it calls and of the types and constants they take, not S3's header.
// Names, types and values follow the toolkit's S3DTK.H (1995-1997, cleaned up in September 1997,
// after the edition); only what the game uses is declared. The library exports the functions below;
// everything else goes through the renderer's function list.
#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

	// What the functions return.
#define S3DTK_OK 1

	// The renderer's initialization (S3DTK_CreateRenderer).
	// SIZE 0xc
	typedef struct S3DTK_RENDERER_INITSTRUCT {
		ULONG initFlags;  // 0x0
		ULONG initUserID; // 0x4
		ULONG initAppID;  // 0x8
	} S3DTK_RENDERER_INITSTRUCT;

	// The renderer's flags (initFlags): check the texture coordinates' range, which only
	// perspective-corrected texturing needs.
#define S3DTK_VERIFY_UVRANGE 2

	// A surface the toolkit draws on or textures from: its offset in video memory (from the start of
	// the frame buffer), its size and its format.
	// SIZE 0x24
	typedef struct S3DTK_SURFACE {
		ULONG sfOffset;    // 0x00
		ULONG sfWidth;     // 0x04
		ULONG sfHeight;    // 0x08
		ULONG sfFormat;    // 0x0c
		ULONG reserved[5]; // 0x10
	} S3DTK_SURFACE;

	// A vertex: the screen position, the depth, its w, the color (a byte per component, blue
	// first), the fog and the texture coordinates.
	// SIZE 0x20
	typedef struct S3DTK_VERTEX_TEX {
		float X; // 0x00
		float Y; // 0x04
		float Z; // 0x08
		float W; // 0x0c
		BYTE B;  // 0x10
		BYTE G;  // 0x11
		BYTE R;  // 0x12
		BYTE A;  // 0x13
		float D; // 0x14
		float U; // 0x18
		float V; // 0x1c
	} S3DTK_VERTEX_TEX;

	// The renderer: a list of functions, each taking the list.
	typedef struct S3DTK_FUNCTIONLIST S3DTK_FUNCTIONLIST;
	typedef S3DTK_FUNCTIONLIST* S3DTK_LPFUNCTIONLIST;

	// SIZE 0x20
	struct S3DTK_FUNCTIONLIST {
		ULONG(__cdecl* S3DTK_SetState)(S3DTK_LPFUNCTIONLIST p_renderer, ULONG p_state, ULONG p_value); // 0x00
		ULONG(__cdecl* S3DTK_GetState)(S3DTK_LPFUNCTIONLIST p_renderer, ULONG p_state, ULONG p_out);   // 0x04
		// Draws p_count vertices, a list of pointers to them, as triangles of the kind p_type.
		ULONG(__cdecl* S3DTK_TriangleSet)(
			S3DTK_LPFUNCTIONLIST p_renderer,
			ULONG* p_vertices,
			ULONG p_count,
			ULONG p_type
		); // 0x08
		// S3DTK_TriangleSetEx, S3DTK_BitBlt, S3DTK_BitBltTransparent and S3DTK_RectFill, which the game
		// doesn't call.
		void* m_unk0x0c[4]; // 0x0c
		// The error of the last call (a negative S3DTK error code).
		int(__cdecl* S3DTK_GetLastError)(S3DTK_LPFUNCTIONLIST p_renderer); // 0x1c
	};

	// The states (S3DTK_SetState, S3DTK_GetState).
	enum {
		S3DTK_VIDEOMEMORYADDRESS = 2,   // the frame buffer's linear address (got)
		S3DTK_DRAWSURFACE = 4,          // the S3DTK_SURFACE drawn on
		S3DTK_GRAPHICS_ENGINE_IDLE = 7, // the engine is idle (got)
		S3DTK_RENDERINGTYPE = 0xa,
		S3DTK_ZBUFFERENABLE = 0xd,
		S3DTK_TEXTUREACTIVE = 0xf, // the S3DTK_SURFACE textured from
		S3DTK_TEXFILTERINGMODE = 0x10,
		S3DTK_TEXBLENDINGMODE = 0x11,
		S3DTK_ALPHABLENDING = 0x13,
		S3DTK_FOGCOLOR = 0x14
	};

	// The fog color that turns the fog off (S3DTK_FOGCOLOR).
#define S3DTK_FOGOFF 0xffffffff

	// The rendering types (S3DTK_RENDERINGTYPE): shaded, textured and lit, textured, and textured
	// and lit with perspective.
	enum {
		S3DTK_GOURAUD = 0,
		S3DTK_LITTEXTURE = 0x8000000,
		S3DTK_UNLITTEXTURE = 0x10000000,
		S3DTK_LITTEXTUREPERSPECT = 0x28000000
	};

	// The texture filters (S3DTK_TEXFILTERINGMODE): mip-mapped (M), then not, by the texels read
	// per pixel (S3DTK_TEXV2TPP, the header says, was thought to be for video but is a linear filter).
	enum {
		S3DTK_TEXM1TPP = 0,
		S3DTK_TEXM2TPP = 0x1000,
		S3DTK_TEXM4TPP = 0x2000,
		S3DTK_TEXM8TPP = 0x3000,
		S3DTK_TEX1TPP = 0x4000,
		S3DTK_TEXV2TPP = 0x5000,
		S3DTK_TEX4TPP = 0x6000
	};

	// The texture blending (S3DTK_TEXBLENDINGMODE) and the alpha blending (S3DTK_ALPHABLENDING).
	enum {
		S3DTK_TEXMODULATE = 0x8000,
		S3DTK_TEXDECAL = 0x10000,
		S3DTK_ALPHAOFF = 0,
		S3DTK_ALPHATEXTURE = 0x80000
	};

	// The surface formats: the frame buffer's RGB555, in system memory rather than video memory with
	// S3DTK_SYSTEM, and two of the texture formats. The game registers its transparent textures as
	// S3DTK_TEXARGB4444 and its opaque ones as S3DTK_TEXARGB1555, though it fills both with the same
	// 1555 texels: the header is a year younger than the edition, whose toolkit may have numbered the
	// texture formats otherwise.
	enum {
		S3DTK_VIDEORGB15 = 4,
		S3DTK_TEXARGB4444 = 0x20,
		S3DTK_TEXARGB1555 = 0x40
	};
#define S3DTK_SYSTEM 0x80000000

	// The triangle sets (S3DTK_TriangleSet): a fan.
#define S3DTK_TRIFAN 2

	// p_init is the header's S3DTK_LIB_INIT (its flags and two addresses), which the game doesn't pass.
	ULONG __cdecl S3DTK_InitLib(void* p_init);
	ULONG __cdecl S3DTK_ExitLib(void);
	ULONG __cdecl S3DTK_CreateRenderer(S3DTK_RENDERER_INITSTRUCT* p_init, S3DTK_LPFUNCTIONLIST* p_renderer);
	ULONG __cdecl S3DTK_DestroyRenderer(S3DTK_LPFUNCTIONLIST* p_renderer);
	ULONG __cdecl S3DTK_LinearToPhysical(ULONG p_linear);

#ifdef __cplusplus
}
#endif

#endif // S3DTK_H
