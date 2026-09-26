/* Smacker (SMACKW32.DLL) declarations: only what the game calls, with the types its matched
   callers prove. There is no SDK in the tree; add functions here as callers are decompiled.
   The import library is generated from smackw32.def; the original imports the functions by
   ordinal through the __cdecl thunks at 0x100492a2-0x100492de. */
#ifndef SMACKW32_H
#define SMACKW32_H

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct SMACK_TAG Smack;

	void SmackGoto(Smack* p_smack, int p_frame);
	void SmackDoFrame(Smack* p_smack);
	void SmackNextFrame(Smack* p_smack);
	void SmackBufferNewPalette(Smack* p_smack);

#ifdef __cplusplus
}
#endif

#endif /* SMACKW32_H */
