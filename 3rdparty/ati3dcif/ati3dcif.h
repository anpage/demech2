#ifndef ATI3DCIF_H
#define ATI3DCIF_H

// ATI3DCIF.DLL, ATI's 3D Rage Common Interface (the 3D CIF), in the version the ATI edition imports
// (ati3dcif.def): our own declarations of the functions it calls and of the types and constants they
// take, not the SDK's header. Names follow the SDK's (ATI3DCIF.H); the values are the ones the game
// passes, and only those are declared.
#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

	typedef unsigned char C3D_UINT8;
	typedef long C3D_INT32;
	typedef unsigned long C3D_UINT32;
	typedef float C3D_FLOAT32;
	typedef C3D_UINT32 C3D_BOOL;
	typedef void* C3D_PVOID;

	// An error code: 0 on success.
	typedef C3D_UINT32 C3D_EC;
#define C3D_EC_OK 0

	// A rendering context and a registered texture.
	typedef C3D_UINT32 C3D_HRC;
	typedef C3D_UINT32 C3D_HTX;
	typedef C3D_HTX* C3D_PHTX;

	// A color, a byte per component.
	// SIZE 0x4
	typedef struct C3D_COLOR {
		C3D_UINT32 r : 8; // 0x0
		C3D_UINT32 g : 8; // 0x1
		C3D_UINT32 b : 8; // 0x2
		C3D_UINT32 a : 8; // 0x3
	} C3D_COLOR;

	// A rectangle, the viewport's.
	// SIZE 0x10
	typedef struct C3D_RECT {
		C3D_INT32 top;    // 0x0
		C3D_INT32 left;   // 0x4
		C3D_INT32 bottom; // 0x8
		C3D_INT32 right;  // 0xc
	} C3D_RECT;

	// A vertex with a texture coordinate and a color: the position, the texture coordinates over w,
	// 1 / w, and the color (0-255 per component).
	// SIZE 0x28
	typedef struct C3D_VTCF {
		C3D_FLOAT32 x; // 0x00
		C3D_FLOAT32 y; // 0x04
		C3D_FLOAT32 z; // 0x08
		C3D_FLOAT32 s; // 0x0c
		C3D_FLOAT32 t; // 0x10
		C3D_FLOAT32 w; // 0x14
		C3D_FLOAT32 r; // 0x18
		C3D_FLOAT32 g; // 0x1c
		C3D_FLOAT32 b; // 0x20
		C3D_FLOAT32 a; // 0x24
	} C3D_VTCF;

	// A strip of vertices, and a list of pointers to vertices.
	typedef C3D_PVOID C3D_VSTRIP;
	typedef C3D_PVOID* C3D_VLIST;

#define cu32MAX_TMAP_LEV 11

	// A texture map to register: its levels, largest first, and their size.
	// SIZE 0x44
	typedef struct C3D_TMAP {
		C3D_UINT32 u32Size;                    // 0x00
		C3D_BOOL bMipMap;                      // 0x04
		C3D_PVOID apvLevels[cu32MAX_TMAP_LEV]; // 0x08
		C3D_UINT32 u32MaxMapXSizeLg2;          // 0x34
		C3D_UINT32 u32MaxMapYSizeLg2;          // 0x38
		C3D_UINT32 eTexFormat;                 // 0x3c
		C3D_COLOR clrTexChromaKey;             // 0x40
	} C3D_TMAP;
	typedef C3D_TMAP* C3D_PTMAP;

	// The render states (ATI3DCIF_ContextSetState).
	typedef enum C3D_ERSID {
		C3D_ERS_VERTEX = 1,           // the vertex type (C3D_EV_VTCF)
		C3D_ERS_PRIM_TYPE = 2,        // the primitives (C3D_EPRIM_)
		C3D_ERS_SOLID_CLR = 3,        // the solid color (C3D_COLOR)
		C3D_ERS_SHADE_MODE = 4,       // C3D_ESH_
		C3D_ERS_TMAP_EN = 5,          // texture mapping on or off
		C3D_ERS_TMAP_SELECT = 6,      // the texture (C3D_HTX)
		C3D_ERS_TMAP_LIGHT = 7,       // C3D_ETL_
		C3D_ERS_TMAP_PERSP_COR = 8,   // C3D_ETPC_
		C3D_ERS_TMAP_FILTER = 9,      // C3D_ETFILT_
		C3D_ERS_TMAP_TEXOP = 10,      // C3D_ETEXOP_
		C3D_ERS_ALPHA_SRC = 11,       // C3D_EASRC_
		C3D_ERS_ALPHA_DST = 12,       // C3D_EADST_
		C3D_ERS_SURF_DRAW_PTR = 13,   // the surface drawn to
		C3D_ERS_SURF_DRAW_PITCH = 14, // its pitch, in pixels
		C3D_ERS_SURF_DRAW_PF = 15,    // its pixel format (C3D_EPF_)
		C3D_ERS_SURF_VPORT = 16       // the viewport (C3D_RECT)
	} C3D_ERSID;

#define C3D_EV_VTCF 3

#define C3D_EPRIM_LINE 0
#define C3D_EPRIM_TRI 1

#define C3D_ESH_SOLID 1
#define C3D_ESH_FLAT 2
#define C3D_ESH_SMOOTH 3

#define C3D_ETL_NONE 0
#define C3D_ETL_MODULATE 1

#define C3D_ETPC_NONE 0
#define C3D_ETPC_THREE 3

#define C3D_ETFILT_MINPNT_MAGPNT 0
#define C3D_ETFILT_MINPNT_MAG2BY2 1
#define C3D_ETFILT_MIN2BY2_MAG2BY2 2
#define C3D_ETFILT_MIPLIN_MAG2BY2 4

#define C3D_ETEXOP_NONE 0
#define C3D_ETEXOP_ALPHA_MASK 3

#define C3D_EASRC_ONE 1
#define C3D_EADST_ZERO 0

#define C3D_EPF_RGB1555 3
#define C3D_ETF_RGB1555 3

	C3D_EC WINAPI ATI3DCIF_Init(void);
	C3D_EC WINAPI ATI3DCIF_Term(void);
	C3D_HRC WINAPI ATI3DCIF_ContextCreate(void);
	C3D_EC WINAPI ATI3DCIF_ContextDestroy(C3D_HRC p_hrc);
	C3D_EC WINAPI ATI3DCIF_ContextSetState(C3D_HRC p_hrc, C3D_ERSID p_state, C3D_PVOID p_data);
	C3D_EC WINAPI ATI3DCIF_TextureReg(C3D_PTMAP p_tmap, C3D_PHTX p_htx);
	C3D_EC WINAPI ATI3DCIF_TextureUnreg(C3D_HTX p_htx);
	C3D_EC WINAPI ATI3DCIF_RenderBegin(C3D_HRC p_hrc);
	C3D_EC WINAPI ATI3DCIF_RenderEnd(void);
	C3D_EC WINAPI ATI3DCIF_RenderPrimStrip(C3D_VSTRIP p_vertices, C3D_UINT32 p_count);
	C3D_EC WINAPI ATI3DCIF_RenderPrimList(C3D_VLIST p_vertices, C3D_UINT32 p_count);

#ifdef __cplusplus
}
#endif

#endif // ATI3DCIF_H
