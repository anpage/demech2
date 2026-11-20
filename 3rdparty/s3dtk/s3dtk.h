#ifndef S3DTK_H
#define S3DTK_H

// S3DTKW.DLL, S3's ViRGE toolkit, as the S3 edition uses it (s3dtk.def): our own declarations of
// the functions it calls and of the types and constants they take, not S3's header. The library
// exports five functions; everything else goes through the renderer's function list, whose first
// two entries set and get a state and whose third draws a set of triangles. The states' numbers,
// their values and the layouts are the ones the game passes; the names say what the game does with
// them and follow the toolkit's own as far as we know them, which isn't checked against S3's header.
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
		ULONG initFlags;  // 0x0 — 2
		ULONG initUserID; // 0x4
		ULONG initAppID;  // 0x8
	} S3DTK_RENDERER_INITSTRUCT;

	// A surface the toolkit draws on or textures from: its offset in video memory (from the start of
	// the frame buffer), its size and its format.
	// SIZE 0x24
	typedef struct S3DTK_SURFACE {
		ULONG sfOffset;     // 0x00
		ULONG sfWidth;      // 0x04
		ULONG sfHeight;     // 0x08
		ULONG sfFormat;     // 0x0c
		ULONG m_unk0x10[5]; // 0x10
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
		);                  // 0x08
		void* m_unk0x0c[4]; // 0x0c
		// The error of the last call.
		ULONG(__cdecl* m_unk0x1c)(S3DTK_LPFUNCTIONLIST p_renderer); // 0x1c
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

	// The rendering types (S3DTK_RENDERINGTYPE): shaded, textured and lit, textured, and textured
	// and lit with perspective.
	enum {
		S3DTK_GOURAUD = 0,
		S3DTK_LITTEXTURE = 0x8000000,
		S3DTK_UNLITTEXTURE = 0x10000000,
		S3DTK_LITTEXTUREPERSPECT = 0x28000000
	};

	// The texture filters (S3DTK_TEXFILTERINGMODE): mip-mapped (M), then not, by the texels read
	// per pixel.
	enum {
		S3DTK_TEXM1TPP = 0,
		S3DTK_TEXM2TPP = 0x1000,
		S3DTK_TEXM4TPP = 0x2000,
		S3DTK_TEXM8TPP = 0x3000,
		S3DTK_TEX1TPP = 0x4000,
		S3DTK_TEX2TPP = 0x5000,
		S3DTK_TEX4TPP = 0x6000
	};

	// The texture blending (S3DTK_TEXBLENDINGMODE) and the alpha blending (S3DTK_ALPHABLENDING).
	enum {
		S3DTK_TEXMODULATE = 0x8000,
		S3DTK_TEXDECAL = 0x10000,
		S3DTK_ALPHAOFF = 0,
		S3DTK_ALPHATEXTURE = 0x80000
	};

	// The surface formats: the frame buffer's RGB555 (with 0x80000000 when it isn't drawn on
	// directly), and a texture's with or without its alpha bit.
	enum {
		S3DTK_VIDEORGB15 = 4,
		S3DTK_TEXARGB1555 = 0x20,
		S3DTK_TEXRGB555 = 0x40
	};

	// The triangle sets (S3DTK_TriangleSet): a fan.
#define S3DTK_TRIFAN 2

	ULONG __cdecl S3DTK_InitLib(ULONG p_flags);
	ULONG __cdecl S3DTK_ExitLib(void);
	ULONG __cdecl S3DTK_CreateRenderer(S3DTK_RENDERER_INITSTRUCT* p_init, S3DTK_LPFUNCTIONLIST* p_renderer);
	ULONG __cdecl S3DTK_DestroyRenderer(S3DTK_LPFUNCTIONLIST* p_renderer);
	ULONG __cdecl S3DTK_LinearToPhysical(ULONG p_linear);

#ifdef __cplusplus
}
#endif

#endif // S3DTK_H
