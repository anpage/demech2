#ifndef MATROX_COLORKEY_H
#define MATROX_COLORKEY_H

#include "pane.h"
#include "types.h"

// The Matrox edition draws the 2D panels on top of the 3D view, and marks the screen areas the
// panels cover with the key color 0xfffe. The functions that fill them, called from the panels' and
// the HUD's drawing; the Matrox edition's are in matrox/msidraw.c (its MSI back end).
#ifdef __cplusplus
extern "C"
{
#endif

	// Fills a rectangle of the screen (right and bottom exclusive) with the color 0xfffe.
	void FUN_10088246(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
	// Fills a pane's rectangle of the screen with the color 0xfffe.
	void FUN_10088280(PANE* p_pane);

#ifdef __cplusplus
}
#endif

#endif // MATROX_COLORKEY_H
