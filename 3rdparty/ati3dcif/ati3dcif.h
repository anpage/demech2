#ifndef ATI3DCIF_H
#define ATI3DCIF_H

// ATI3DCIF.DLL, ATI's 3D Rage Common Interface (the 3D CIF), in the version the ATI edition imports
// (ati3dcif.def): our own declarations of the functions it calls and of the types and constants they
// take, not the SDK's header. Names, types and values follow the SDK's ATI3DCIF.H; only what the
// game uses is declared. The SDK's later header has a larger C3D_TMAP (a palette, clamping, alpha
// blending and tiling after the chroma key): the game registers the 0x44-byte one.
#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef unsigned int C3D_BOOL;
	typedef int C3D_INT32;
	typedef unsigned int C3D_UINT32;
	typedef unsigned char C3D_UINT8;
	typedef float C3D_FLOAT32;
	typedef void* C3D_PVOID;

	// The SDK forces its enumerations to 32 bits.
#define C3D_FORCE_SIZE 0x7fffffff

	// An error code.
	typedef enum {
		C3D_EC_OK = 0,
		C3D_EC_FORCE_U32 = C3D_FORCE_SIZE
	} C3D_EC;

	// A rectangle, the viewport's.
	// SIZE 0x10
	typedef struct {
		C3D_INT32 top;    // 0x0
		C3D_INT32 left;   // 0x4
		C3D_INT32 bottom; // 0x8
		C3D_INT32 right;  // 0xc
	} C3D_RECT;

	// A color, a byte per component.
	// SIZE 0x4
	typedef union {
		struct {
			unsigned r : 8; // 0x0
			unsigned g : 8; // 0x1
			unsigned b : 8; // 0x2
			unsigned a : 8; // 0x3
		};
		C3D_UINT32 u32All; // 0x0
	} C3D_COLOR;

	// The vertex types (C3D_ERS_VERTEX_TYPE).
	typedef enum {
		C3D_EV_VTCF = 3,
		C3D_EV_FORCE_U32 = C3D_FORCE_SIZE
	} C3D_EVERTEX;

	// A vertex with a texture coordinate and a color: the position, the texture coordinates over w,
	// 1 / w, and the color (0-255 per component).
	// SIZE 0x28
	typedef struct {
		C3D_FLOAT32 x, y, z;    // 0x00
		C3D_FLOAT32 s, t, w;    // 0x0c
		C3D_FLOAT32 r, g, b, a; // 0x18
	} C3D_VTCF;

	// A strip of vertices, and a list of pointers to vertices.
	typedef void* C3D_VSTRIP;
	typedef void** C3D_VLIST;

	// The primitives (C3D_ERS_PRIM_TYPE).
	typedef enum {
		C3D_EPRIM_LINE = 0,
		C3D_EPRIM_TRI = 1,
		C3D_EPRIM_FORCE_U32 = C3D_FORCE_SIZE
	} C3D_EPRIM;

	// The shading (C3D_ERS_SHADE_MODE): with the solid color, the last vertex's, or interpolated.
	typedef enum {
		C3D_ESH_SOLID = 1,
		C3D_ESH_FLAT = 2,
		C3D_ESH_SMOOTH = 3,
		C3D_ESH_FORCE_U32 = C3D_FORCE_SIZE
	} C3D_ESHADE;

	// The blend factors of the source and the destination (C3D_ERS_ALPHA_SRC, C3D_ERS_ALPHA_DST).
	typedef enum {
		C3D_EASRC_ONE = 1,
		C3D_EASRC_FORCE_U32 = C3D_FORCE_SIZE
	} C3D_EASRC;

	typedef enum {
		C3D_EADST_ZERO = 0,
		C3D_EADST_FORCE_U32 = C3D_FORCE_SIZE
	} C3D_EADST;

	// A registered texture.
	typedef void* C3D_HTX;
	typedef C3D_HTX* C3D_PHTX;

	// The texture lighting (C3D_ERS_TMAP_LIGHT): the texel color, or times the interpolated one.
	typedef enum {
		C3D_ETL_NONE = 0,
		C3D_ETL_MODULATE = 1,
		C3D_ETL_FORCE_U32 = C3D_FORCE_SIZE
	} C3D_ETLIGHT;

	// The perspective correction (C3D_ERS_TMAP_PERSP_COR), from none to full (9).
	typedef enum {
		C3D_ETPC_NONE = 0,
		C3D_ETPC_THREE = 3,
		C3D_ETPC_FORCE_U32 = C3D_FORCE_SIZE
	} C3D_ETPERSPCOR;

	// The texture filters (C3D_ERS_TMAP_FILTER), minified then magnified: the nearest texel, a 2x2
	// blend, or a blend between mip levels.
	typedef enum {
		C3D_ETFILT_MINPNT_MAGPNT = 0,
		C3D_ETFILT_MINPNT_MAG2BY2 = 1,
		C3D_ETFILT_MIN2BY2_MAG2BY2 = 2,
		C3D_ETFILT_MIPLIN_MAG2BY2 = 4,
		C3D_ETFILT_FORCE_U32 = C3D_FORCE_SIZE
	} C3D_ETEXFILTER;

	// The texel operations (C3D_ERS_TMAP_TEXOP): the alpha mask draws no texel whose alpha bit is clear.
	typedef enum {
		C3D_ETEXOP_NONE = 0,
		C3D_ETEXOP_ALPHA_MASK = 3,
		C3D_ETEXOP_FORCE_U32 = C3D_FORCE_SIZE
	} C3D_ETEXOP;

	// The pixel and texel formats.
	typedef enum {
		C3D_EPF_RGB1555 = 3,
		C3D_EPF_FORCE_U32 = C3D_FORCE_SIZE
	} C3D_EPIXFMT;

	typedef enum {
		C3D_ETF_RGB1555 = 3,
		C3D_ETF_FORCE_U32 = C3D_FORCE_SIZE
	} C3D_ETEXFMT;

#define cu32MAX_TMAP_LEV 11

	// A texture map to register: its levels, largest first, and their size.
	// SIZE 0x44
	typedef struct {
		C3D_UINT32 u32Size;                    // 0x00
		C3D_BOOL bMipMap;                      // 0x04
		C3D_PVOID apvLevels[cu32MAX_TMAP_LEV]; // 0x08
		C3D_UINT32 u32MaxMapXSizeLg2;          // 0x34
		C3D_UINT32 u32MaxMapYSizeLg2;          // 0x38
		C3D_ETEXFMT eTexFormat;                // 0x3c
		C3D_COLOR clrTexChromaKey;             // 0x40
	} C3D_TMAP, *C3D_PTMAP;

	// A rendering context.
	typedef void* C3D_HRC;
	typedef void* C3D_PRSDATA;

	// The render states (ATI3DCIF_ContextSetState), each with the type its data points to.
	typedef enum {
		C3D_ERS_VERTEX_TYPE = 1,      // C3D_EVERTEX
		C3D_ERS_PRIM_TYPE = 2,        // C3D_EPRIM
		C3D_ERS_SOLID_CLR = 3,        // C3D_COLOR
		C3D_ERS_SHADE_MODE = 4,       // C3D_ESHADE
		C3D_ERS_TMAP_EN = 5,          // C3D_BOOL
		C3D_ERS_TMAP_SELECT = 6,      // C3D_HTX
		C3D_ERS_TMAP_LIGHT = 7,       // C3D_ETLIGHT
		C3D_ERS_TMAP_PERSP_COR = 8,   // C3D_ETPERSPCOR
		C3D_ERS_TMAP_FILTER = 9,      // C3D_ETEXFILTER
		C3D_ERS_TMAP_TEXOP = 10,      // C3D_ETEXOP
		C3D_ERS_ALPHA_SRC = 11,       // C3D_EASRC
		C3D_ERS_ALPHA_DST = 12,       // C3D_EADST
		C3D_ERS_SURF_DRAW_PTR = 13,   // C3D_PVOID: the surface drawn to
		C3D_ERS_SURF_DRAW_PITCH = 14, // C3D_UINT32: its pitch, in pixels
		C3D_ERS_SURF_DRAW_PF = 15,    // C3D_EPIXFMT: its pixel format
		C3D_ERS_SURF_VPORT = 16,      // C3D_RECT: the viewport
		C3D_ERS_FORCE_U32 = C3D_FORCE_SIZE
	} C3D_ERSID;

	C3D_EC WINAPI ATI3DCIF_Init(void);
	C3D_EC WINAPI ATI3DCIF_Term(void);
	C3D_EC WINAPI ATI3DCIF_TextureReg(C3D_PTMAP p_tmap, C3D_PHTX p_htx);
	C3D_EC WINAPI ATI3DCIF_TextureUnreg(C3D_HTX p_htx);
	C3D_HRC WINAPI ATI3DCIF_ContextCreate(void);
	C3D_EC WINAPI ATI3DCIF_ContextDestroy(C3D_HRC p_hrc);
	C3D_EC WINAPI ATI3DCIF_ContextSetState(C3D_HRC p_hrc, C3D_ERSID p_state, C3D_PRSDATA p_data);
	C3D_EC WINAPI ATI3DCIF_RenderBegin(C3D_HRC p_hrc);
	C3D_EC WINAPI ATI3DCIF_RenderEnd(void);
	C3D_EC WINAPI ATI3DCIF_RenderPrimStrip(C3D_VSTRIP p_vertices, C3D_UINT32 p_count);
	C3D_EC WINAPI ATI3DCIF_RenderPrimList(C3D_VLIST p_vertices, C3D_UINT32 p_count);

#ifdef __cplusplus
}
#endif

#endif // ATI3DCIF_H
