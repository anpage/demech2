/* Symbol source for the ATI3DCIF import library (see ati3dcif.def). Only the __stdcall decoration,
   i.e. the argument byte count, matters: LIB reads this object to resolve the .def names, and it
   is never linked into a game DLL. The real prototypes are in ati3dcif.h. */

#define IMPORT0(name)                                                                                                  \
	void __stdcall name(void)                                                                                          \
	{                                                                                                                  \
	}
#define IMPORT1(name)                                                                                                  \
	void __stdcall name(int p_0)                                                                                       \
	{                                                                                                                  \
	}
#define IMPORT2(name)                                                                                                  \
	void __stdcall name(int p_0, int p_1)                                                                              \
	{                                                                                                                  \
	}
#define IMPORT3(name)                                                                                                  \
	void __stdcall name(int p_0, int p_1, int p_2)                                                                     \
	{                                                                                                                  \
	}

IMPORT0(ATI3DCIF_ContextCreate)
IMPORT1(ATI3DCIF_ContextDestroy)
IMPORT3(ATI3DCIF_ContextSetState)
IMPORT0(ATI3DCIF_Init)
IMPORT1(ATI3DCIF_RenderBegin)
IMPORT0(ATI3DCIF_RenderEnd)
IMPORT2(ATI3DCIF_RenderPrimList)
IMPORT2(ATI3DCIF_RenderPrimStrip)
IMPORT0(ATI3DCIF_Term)
IMPORT2(ATI3DCIF_TextureReg)
IMPORT1(ATI3DCIF_TextureUnreg)
