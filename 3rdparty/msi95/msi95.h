#ifndef MSI95_H
#define MSI95_H

// MSI95.DLL (Matrox Simple Interface, the Mystique's 3D API), which the Matrox edition draws
// through: only what the decompiled code calls (msi95.def). The A3D renderer calls
// the rest (msiInit, msiExit, msiStartFrame, msiEndFrame, msiRenderTriangle, msiDrawSingleLine,
// msiBlitRect, msiSetParameters, msiAllocTextureHeap, msiFreeTextureHeap and two by ordinal).
#ifdef __cplusplus
extern "C"
{
#endif

	// Waits for the card (SimMain calls it once a frame).
	void msiSync(void);

#ifdef __cplusplus
}
#endif

#endif // MSI95_H
