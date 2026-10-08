#ifndef MSI95_H
#define MSI95_H

// MSI95.DLL (Matrox Simple Interface, the Mystique's 3D API), which the Matrox edition draws
// through: only what the decompiled code calls (msi95.def). The A3D renderer calls the rest
// (msiInit, msiRenderTriangle, msiDrawSingleLine and two by ordinal).
#ifdef __cplusplus
extern "C"
{
#endif

	// Waits for the card (SimMain calls it once a frame).
	void msiSync(void);

	// The A3D renderer's (matrox/a3d.c). Parameter types are those its calls prove.
	void msiSetParameters(int p_parameters);
	void msiStartFrame(int p_clear, float p_red, float p_green, float p_blue, int p_unk0x10, int p_unk0x14);
	void msiEndFrame(int p_unk0x00, int p_unk0x04, int p_unk0x08);
	void msiFreeTextureHeap(void* p_heap);
	void* msiAllocTextureHeap(unsigned int p_pages);
	void msiBlitRect(
		void* p_heap,
		void* p_destination,
		void* p_source,
		int p_pitch,
		int p_bits,
		int p_width,
		int p_height,
		int p_offset,
		int p_color,
		int p_mask
	);
	void msiDrawSingleLine(int p_color, int p_from, int p_to, int p_unk0x0c);
	void msiRenderTriangle(void* p_a, void* p_b, void* p_c, int p_unk0x0c);
	void* msiInit(int p_width, int p_height, int p_bits, int p_unk0x0c, int p_unk0x10, void* p_windowProc);
	void msiExit(void);

#ifdef __cplusplus
}
#endif

#endif // MSI95_H
