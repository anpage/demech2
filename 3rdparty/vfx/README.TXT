[demech2 note]

These files come from John Miles's open-source release of the Miles Design
VFX Graphics System (the vfx119 archive, mirrored at
https://github.com/dah4k/VFX119); his original README follows below this note.

MechWarrior 2 for Windows 95 links a lightly modified version of this code into
both MW2.DLL and MW2SHELL.DLL: VFXA.ASM (2D primitives), VFX3D.ASM (polygon
fillers) and VFXREND.ASM (release 1.06, the polygon renderer), with VFX.INC and
RENDOPTS.INC. The build assembles them with MASM 6.11 (ML /coff /Cx /DMASM
/DFLAT_MODEL) into objects identical to the game's, code, data and relocations.
VFX.H and VFXREND.H aren't built: they document the C API.

Every change to the release is between "; demech2: begin change." and "; demech2:
end change" comments, which say why: MW2's own changes (aligned fills and copies,
its set of polygon primitives, GetCodeBlock) and a few for the tools (MASM 6.11's
handling of ORG, the debug information reccmp reads, publics for the tests). The
files are otherwise as released, but for their line endings and encoding (LF
and UTF-8 here; the release has CRLF and code page 437) and this note.

-------------------------------------------------------------------------------

This package contains the current Miles Design VFX Graphics System releases
for 32-bit protected-mode DOS (VFX V1.15) and Win32 (WinVFX V1.18).  VFX
and WinVFX have been used in dozens of bestselling PC games over the past
several years, ranging from SSI's AD&D titles to Digital Anvil's latest
release, Starlancer.

VFX simplifies a number of real-time graphics tasks, such as font rendering
and access to 2D pixel, line, and sprite primitives.  WinVFX and its
companion W32SAL.DLL make it easy to build a Windows graphics application
in minutes, hiding various Win32 subtleties such as fullscreen/windowed
mode switching, display mode control, and DirectDraw/CreateDIBSection()
abstraction from your application.

VFX is a fairly large API package with many features.  New users should begin
by running and studying the example code in the WINVFX16\DEMO subdirectory,
along with the API and tools manual (WINDOC\VFX_WW97.DOC, in Microsoft Word 97
format).

As of July 5, 2000, all source code and compiled object code in this package
may be considered "open-source freeware," usable by anyone for any purpose,
commercial or otherwise, without restriction or limitation.  I strongly
encourage, but do not require, anyone who adds any further value to this
package to release their modifications and improvements under the same terms.

I expressly disclaim any liability for bugs, defects, or intellectual-property
violations that may arise as a consequence of using the contents of this
package for any purpose, commercial or otherwise.  Use at your own risk!

John Miles (jmiles@pop.net)
Miles Design, Inc.
July 5, 2000

-----------------------
V1.17 of 11-May-2003
-----------------------

Several enhancements/fixes to SAL
Fixed one-pixel-width/high panes in VFX_pane_construct()

-----------------------
V1.18 of 2-Jan-2005
-----------------------

Added SAL_QUIET preference

-----------------------
V1.19 of 6-Jan-2006
-----------------------

Added SAL_USE_PARAMON preference
