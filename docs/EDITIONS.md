# The editions of the simulator

MechWarrior 2's simulator (`MW2.DLL`) shipped in several editions built from one source: the
Windows 95 release this project started from, one build per 3D card, the expansion Ghost
Bear's Legacy's, and an unused early build of Mercenaries' simulator, left behind on the Mercenaries discs. This document records
what is known about each one and what differs between them. Its second half is generated from the
sources, so that it can't drift from them.

Eight editions are targets today; the table lists the editions after 1.1 in the order they were
built. The S3 and PowerVR editions joined after the others: every one of their functions is
annotated, but most of what they changed from the ATI edition is still undiagnosed. Ghost Bear's
Legacy joined next: every function of its game code is byte-identical or proven by a witness. The
early Mercenaries build joined last, built from GBL's source and features until its own are
found.

| | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx |
|---|---|---|---|---|---|---|---|---|
| reccmp target | `MW2` | `MW2ATI` | `MW2PREMERCS` | `MW2S3` | `MW2MATROX` | `MW2POWERVR` | `MW2GBL` | `MW23DFX` |
| Binary | `MW2.DLL` of the freely downloadable 1.1 patch | `MW2_ATI.DLL`: the ATI 3D Rage edition's `MW2.DLL` | `MW2_PREMERCS.DLL`: an unused early build of Mercenaries' simulator, between GBL and Mercenaries, left as `/MERCS/MW2.DLL` on the Mercenaries discs | `MW2_S3.DLL`: the S3 ViRGE edition's `MW2.DLL` | `MW2_MATROX.DLL`: the Matrox Mystique edition's `MW2.DLL` | `MW2_POWERVR.DLL`: the PowerVR edition's `MW2.DLL` | `MW2_GBL.DLL`: the `MW2.DLL` of Ghost Bear's Legacy's freely downloadable 1.1 patch (`GBLPATCH.EXE`) | `MW2_3DFX.DLL`: the Diamond Monster disc's `MW2.DLL` |
| Linked | 1996-09-20 | 1996-04-11 | 1996-07-08 | 1996-07-12 | 1996-08-22 | 1996-09-06 | 1996-10-18 | 1996-10-21 |
| Compiler | VC++ 4.1 (`cl` 10.10.6038) | VC++ 2.2 (LINK 2.55) | VC++ 4.1 (LINK 3.10) | VC++ 4.1 (LINK 3.10) | VC++ 4.0 (`cl` 10.00, LINK 3.00) | VC++ 2.2 (LINK 2.55) | VC++ 4.1 (LINK 3.10) | VC++ 2.2 (`cl` 9.10) |
| Flags | `/Od /Oi /G5` | `/Ox /Ow /Oi /G5 /Gr /Gy`; `network.c`, `shots.c` and `mechreload.c` `/Od`; `KeyboardPoll` without `/Ow` | `/Od /G5`; `abs` and `fabs` expanded | `/Ox /Ow /Oi /G5 /Gr /Gy`; `network.c`, `shots.c` and `mechreload.c` `/Od`; `QueueFace` unoptimized; `KeyboardPoll` without `/Ow` | `/Od /Oi /G5`; the renderer `/Ox /G5 /Op`, the depth sort `/Ox /Oa /Op` | `/Ox /Ow /Oi /G5 /Gr /Gy`; `network.c`, `shots.c` and `mechreload.c` `/Od`; `KeyboardPoll` without `/Ow` | `/Od /G5`; `abs` and `fabs` expanded | `/Ox /Ow /Oi /G5 /Gr /Gy`; `network.c`, `shots.c` and `mechreload.c` `/Od`; `QueueFace` unoptimized; `KeyboardPoll` without `/Ow` |
| C runtime | `LIBCMTD` (`/MTd`) | `LIBC` (`/ML`) | `LIBCMTD` (`/MTd`) | `LIBC` (`/ML`) | `LIBCMT` (`/MT`) | `LIBC` (`/ML`) | `LIBCMTD` (`/MTd`) | `LIBC` (`/ML`) |
| Draws through | DirectDraw and VFX | ATI's 3D CIF (`ati3dcif.dll`), on DirectDraw | DirectDraw and VFX | S3's toolkit (`S3DTKW.DLL`), on DirectDraw; no VFX3D or VFXREND | Matrox's `MSI95.DLL` | VideoLogic's SGL (`sgl.dll`), PowerVR's scene graph library | DirectDraw and VFX | Glide 1.x (`GLIDE.DLL`), with 3dfx's Texus linked in |
| Identity macros | none | `MW2_ACCEL`, `MW2_ATI` | `MW2_PREMERCS` | `MW2_ACCEL`, `MW2_S3` | `MW2_ACCEL`, `MW2_MATROX` | `MW2_ACCEL`, `MW2_POWERVR` | `MW2_GBL` | `MW2_ACCEL`, `MW2_3DFX` |
| Game-code functions | 1537 | 1427 | 1640 | 1396 | 1597 | 1423 | 1546 | 1574 |

## How the source tells them apart

The build gives each target its identity macros and nothing else. `util/types.h` maps an identity
to **features**, one `#define` each, named for what the feature does and true of every edition
that has it; the comment above each `#define` is its documentation, and the table below is made
from it. The code tests features. It tests an identity only where naming a feature would say
nothing more:

- an edition's renderer back end (MSI95, Glide) and what the shared code does for it;
- an edition's own definition of a function, where its version differs in its control flow or in
  more than a few statements ("Functions with a definition per edition" below; the comment above
  each definition says what that edition's does);
- a computation one edition's source writes another way with the same behaviour (a source form);
- the artefacts of how an edition's source was laid out rather than what its game does: fragments
  included at another object's position, globals in `.data` in one edition and `.bss` in another,
  functions only 1.1's object has, declaration orders, and the pragmas that reproduce a unit's
  flags. `MW2_INLINE_HELPERS` is the one artefact with a name of its own.

Units only the accelerated editions build are in `MW2/src/accel` and `MW2/include/accel`; an
edition's own are in `MW2/src/matrox`, `MW2/src/ati`, `MW2/src/s3`, `MW2/src/powervr` and `MW2/src/3dfx`.

## What a line of this document rests on

A function that compiles to the original's bytes proves its code, its types, the data layout and
the constants, for the editions that compile it. It proves nothing about names or descriptions,
which are readings of that code: so is every sentence here.

- **Which feature a site belongs to** is evidence only where the editions differ. The Matrox and
  3Dfx editions share every feature of `MW2_ACCEL`, so no binary separates the float conversion
  from the A3D interface or the 16-bit drawing: those sites are attributed by what their code
  does, and a combination no edition has (A3D without float scalars) is code nothing has compiled.
- **Whether an edition really has a site's code** is proven only by a function of that edition
  that is byte-identical or has a witness and compiles the site. Most features of both accelerated
  editions are both editions' because nothing shows otherwise yet. The proof of every site for
  every edition, with line numbers, is published with the progress reports on each push to `main`
  ([`EDITIONSITES.md`](https://anpage.github.io/demech2/EDITIONSITES.md), also in the
  [continuous release](https://github.com/anpage/demech2/releases/tag/continuous)); this document
  leaves both out, so that it changes only when the editions' source does.

## What the accelerated editions changed

The 3D-card editions were built from the game's source as it stood when each was made, which is
not 1.1's: the ATI edition was linked in April 1996 and the Matrox edition in August, both before
1.1 (September), and the 3Dfx edition in October. They share the accelerated code below, which the
ATI edition, the oldest, already has. The Matrox edition is the one built like 1.1 (`/Od`), so most
of what follows was found in it; the ATI and 3Dfx editions are the same C built by another
compiler with optimization.

- **Fixed point became float.** The Matrox edition converted the simulation's 16.16 fixed-point values to `float`: struct members at the same offsets (vectors, object positions, velocities, normals, distances, ranges, heat, throttle, bounding boxes, the eyepoint) and about 180 globals, read with `fld` where 1.1 shifts (`x >> 16` becomes a conversion through `__ftol`). This, not changed layouts, is why most of its changed functions differ; the conversions are uniform enough to suggest the original source switched types and conversion macros with a build flag.
- **The A3D layer.** The Matrox edition's renderer (`0x1005a5f0`–`0x10067500`, every caller of `MSI95.DLL` but `SimMain`) is one object of the project, compiled by the same VC++ 4.0 with `/Ox /G5 /Op` where the other units have `/Od`: it is linked between `damagepanel.c` and `keyboard.c`, with one run of `.rdata`, `.data` and `.bss` of its own, and it calls the game's functions (the resource cache, the debug log). Its names (`A3D_map_polygon`, `A3D_polygon_clip_XY_and_render`, `A3D_TEXTURE_fill`) mirror VFX's polygon API, and the ATI and S3 editions carry their own `A3D` layers with the same API: the game's own accelerated replacement for VFX, one back end per card. What looks hand-tuned is `/G5`'s Pentium scheduling: a load and `test` where `/Od` writes `cmp [g], 0`, interleaved prologues, `nop`s (`npad 1`) before a `push`, registers renamed through `ebx`. `/Ox` rather than `/O2`: functions pad to 16 with `npad` (no `/Gy` COMDATs) and identical strings stay separate (no `/Gf`); `/Op` gives its float stores through a stack temporary. 4.0 specifically: one function (`0x1005f790`) compiles differently under 2.2, 4.1 and 4.2. The Matrox edition's depth-sort object (`depthsort.c`'s functions, `0x100232f0`–`0x1002390b`) is optimized too, with `/Ox /Oa /Op`: `/Oa` keeps globals in registers across a store through a pointer. The build compiles `MW2/src/matrox/a3d.c` and `depthsort.c` with those flags.
- **The 16-bit 2D primitives.** The game code doesn't draw through VFXA in the Matrox edition: its calls to `VFX_line_draw`, `VFX_shape_draw`, `VFX_shape_visible_rectangle`, `VFX_pane_wipe`, `VFX_ellipse_draw` and `VFX_string_draw` go to a 16-bit port of VFXA (`0x100563dc`–`0x100591a0`, after `loadres.c`: ML frames, VFXA's routines in VFXA's order, storing 16-bit pixels), which the ATI and S3 editions carry too. The 3Dfx edition carries it too. `MW2/include/accel/vfx16.h` declares them and, under `MW2_VFX16`, redirects the VFXA names to them: a drawing unit includes it after `vfxa.h`, behind `#ifdef MW2_VFX16`. VFXA stays linked for its other routines (`supanim.c`'s offscreen panes still draw through it). The object is `MW2/src/accel/vfx16.asm`, a MASM unit generated by `tools/asm2masm.py --target MW2MATROX`, with VFXA's names plus a `16` suffix (VFXA's own publics are linked alongside) and VFXA's data layout; only the accelerated editions' ML builds assemble it, so it has no portable C.
- **VFX.** The three VFX objects are laid out the same in every edition, but for the ATI edition's
  first two VFX3D routines (`VFX_flat_polygon`, `VFX_Gouraud_polygon`), an older version of
  their code than the release's, which the later editions have. The accelerated editions
  link VFX 1.15 as released where 1.1's was modified (`MW2_VFX_STOCK`), and only
  `VFX_polygon_render` and `VFX_polygon_clip_XY_and_render` differ between the two: the Matrox
  edition doesn't set `ES` and reads `poly_vectors` through `CS` (`MW2_VFXREND_CS_SEGMENT`), the
  3Dfx edition keeps 1.1's code.
- **Object order.** `vc40/targets.cmake` lists MW2's units in the Matrox edition's object order, found from the functions and data they share. 1.1's units the edition has nothing of (the fixed-point helpers the float conversion replaced, and `directdraw.c` and `texpoly.c`, which its renderer replaced) aren't linked: nothing the edition compiles calls them (`DrawBoundingSpheres`, which called `ProjectRadius`, is 1.1's alone, `MW2_NO_BOUNDING_SPHERES`), and an `/Od` object without `/Gy` would be linked whole, `DirectDrawCreate` import included. `audiomenu.c`, data only, sits between `statuspanels.c`'s data and `depthsort.c`'s (`0x100a5318`, 1.1's layout 0x90 further on). `tools/check_units.py --target MW2MATROX` passes with no known findings.

## The 3Dfx edition

- **Flags.** `/Ox /Ow /G5 /Gr /Gy` reproduce the original byte for byte (`LoadSndCfg`, `LoadDifficultyCfg`, `KnockMechOver` and about 1000 more from today's sources); `network.c`, `shots.c` and `mechreload.c` are `/Od` (still `/Gr`). `/Ow` (no aliasing except across function calls), not `/Oa`: the original reloads what a pointer reaches after every call (`KnockMechOver`'s `p_mech->m_player`, `TestPointUnderFloor`'s model, `CycleTarget`'s target id after each `Target*` call), which `/Oa` keeps in a register, and keeps the stores to a local whose address a call takes as an integer (`GetCdAudioPosition`'s `MCI_STATUS_PARMS`), which `/Oa` drops; without either, it reloads after stores through pointers too. Moving every unit from `/Oa` to `/Ow` raised about 100 functions and lowered only `QueueFace`, by 0.1%. `vc22/targets.cmake` sets `DEMECH2_OPTIMIZATION` on every other source, which the `vc22/` sub-project honours like the 4.x builds (`/Od` is a compile option there, not one of `CMAKE_<LANG>_FLAGS`). Its CRT is 2.2's single-threaded `LIBC` (`/ML`): the original imports no TLS or critical section functions, and its `fclose` takes no lock.
- **Texus.** Texus (`3rdparty/texus`) is 3dfx's texture library, which the renderer converts the textures with: `swlibs/texus/lib` of Debian's `glide` 2002.04.10 (the 1999 Glide source release), the 11 files the DLL links, under its own licence (`glide_license.txt`, the 3DFX GLIDE Source Code General Public License, not the repository's LGPL; see its `README`). Changes to the source sit between dated `demech2: begin change` / `end change` comments, like `3rdparty/vfx`'s. 3dfx built it, `__cdecl`, with VC++ 2.2 at `/Ox /G5 /QIfdiv-` (no `/Gy`: its functions pad with `npad`; no FDIV check), as a static library (`3rdparty/texus/texus.cmake`) the target links first. `MW2/library_texus.h` annotates it (`LIBRARY`, outside the game-code `--total`): it starts at `0x10068270`, where the calling convention changes, and ends at the import thunks (`0x1006e8b0`). About half of its 101 functions match; the rest is mostly register allocation, which depends on the symbols a file declares (its headers stay verbatim).
- **The renderer and the display back end.** `MW2/src/3dfx/a3d.c` (`0x10066570`-`0x100680b2`, after `accel/graphicsmenu.c`) draws through Glide: its vertex is Glide's `GrVertex` (`A3DVertex` under `MW2_3DFX`, the Matrox vertex's members keeping their names), its textures VFX `WINDOW`s of 16-bit texels, converted through Texus and cached, or read converted from `MW2.MIP`, which the game writes on its first run; it writes the 2D panels straight into Glide's frame buffer. `MW2/src/3dfx/glidedraw.c` (`0x100059a0`-`0x10005b65`, after `gdi.c`) is the display back end and refresh modes, `directdraw.c`'s functions one for one, in DirectDraw's slot of `g_displayBackends`/`g_refreshModes`. The edition has no key color fills (nor the `0xfffe` key-colour wipes of the panels, `ClipRectToPane` and the visible-rectangle bounds of `DrawHudShape`: those are `MW2_KEY_COLOR`, the Matrox edition's), and wipes the 3D view's panes with VFX's 16-bit `VFX_pane_wipe`. Its pixels are RGB555 where the Matrox edition's are RGB565 (`PIXEL_RGB16`, the palette in `ApplyPaletteResource`), and it reads no `LOD.PAR`: the detail levels are 1.1's constants. The renderer's `.bss` variables are `static` (their order is the object's), and 2.2 lays statics out in an order of its own: a pointer just past an array names a different neighbour on either side.
- **Data.** What the 3Dfx edition changed (found by datacmp): the map colors (`targeting.c`) are other palette entries in both accelerated editions; the torso pan and throttle ranges of `g_inputSinks` (`inputmap.c`) are the 3Dfx edition's own; its graphics page lists a fourth item, a "Resolution 800x600" setting in the texture filter's place (`accel/graphicsmenu.c`; the "Res800x600" registry value, which `SimMain`'s settings pass to the renderer for `grSstOpen`). At 800x600 the menus keep their 640x480 size, scaling their points by eight tenths of the frame (`ScaleMenuPointToFrame`), and `DrawAnim2d` tiles animation 0 across its pane (`TileShapeFrame`). The resource type tags and extensions are one `char*` global each where 1.1 has two arrays: 2.2 lays an array's strings out ahead of it, and the original alternates strings and pointers.

## The ATI edition

- **What it is.** `MW2_ATI.DLL`, the ATI 3D Rage edition's simulator, was linked on 1996-04-11 by LINK 2.55: the oldest edition decompiled, five months before 1.1 and six before the 3Dfx edition. It keeps its settings under 1.0's registry key (`Software\Activision\MechWarrior 2\1.0`). Its C already has what the later accelerated editions share (the float conversion, the 16-bit VFXA port, the A3D interface), and it was built the way the 3Dfx edition was six months later: VC++ 2.2, `__fastcall` the default, `int3` between functions, 2.2's single-threaded `LIBC` (without the `UnhandledExceptionFilter` import the 3Dfx edition has) and no Texus. 1186 of the 3Dfx edition's 1574 game functions are byte-identical to the ATI edition's, relocations aside, so the target starts from the 3Dfx edition's flags and unit order: `tools/check_units.py` places every annotated function and global with them.
- **Its back ends.** It draws its polygons through ATI's 3D CIF (`ati3dcif.dll`: `ATI3DCIF_Init`, contexts, registered textures, primitive lists and strips) in DirectDraw surfaces, and its display back end and refresh modes are DirectDraw's, `directdraw.c`'s functions one for one in the slot after `gdi.c` that the 3Dfx edition later fills with Glide's. The renderer (`MW2/src/ati/a3d.c`, `0x10063af0` to the import thunks at `0x10065066`) caches each context state it sets and draws a polygon as a list of triangles fanned out of its first vertex, in the CIF's `C3D_VTCF` vertices (ten floats); its textures, up to 400, are copied into DirectDraw surfaces of 16 to 2048 texels square (128 wide with their mip levels under them, each averaged from the one above) and registered with the CIF, looked up by their texels, the least recently used one of a size taken when there is no other, and copied again after a palette change. Its `DrawAnimatedPolygon` draws a vertex at a time, its texture coordinates fractions of the texture; the 3Dfx edition's, which has the same steps, adds MW2.MIP and Glide's 0-255 coordinates. Its direct texture drawer (`bandpoly.c`'s `FUN_1002a480`) hands the renderer a table of rows. Its `SetGameResolution` asks DirectDraw whether it has 512x384 in 16 bits for `VESA480.DLL` (`MW2_RES_512X384`: an `EnumDisplayModes` query in `directdraw.c`, which the later editions no longer have), and its `QueueFace` is compiled without optimization, as the 3Dfx edition's still is.
- **What the later editions kept.** The 3Dfx edition keeps much of the ATI edition's code: its RGB555 pixels (`ApplyPaletteResource`), VFX's pane wipe for the 3D view (`DrawMapView`), the cockpit panels at the end of `targeting.c`'s object, the inline helpers, the message backgrounds, the torso pan and throttle ranges, the map layouts' gauge functions, the unterminated BWD version, `ProjectPolygon`'s own near-plane clip and the sky and ground drawers' own clip, none of which the Matrox edition has. The Matrox edition keeps its texture filter setting, but no longer lists it on its graphics page, where the 3Dfx edition lists 800x600 instead. `SimMain` and `SimWindowProc` are one definition for the ATI and 3Dfx editions: the ATI edition's `SimMain` reads the sky and ground and the render settings right after the command line, tests the palette through DCs it doesn't release (as 1.1 still does), fills through DirectDraw and shuts its renderer down with `A3D_shutdown`; the 3Dfx edition reads the settings before `FirstClock` and releases the DCs. Every site that tests `MW2_MATROX` or `MW2_3DFX`, or a feature no proven ATI function compiles, was checked against it: each is in a proven function, one the audit classifies, or one whose residue `tools/known_diffs.txt` explains as codegen alone, after four sites took the edition's own code (its blue, green, opaque alpha and red stores in `FUN_100751a9` and `FUN_10068b57`, the camera distance `SpawnEffectEx` computes with `cameraZ` assigned inside its sum, and `RunMenuStatus` without the empty cases 2 and 3, as every optimized edition).
- **Flags.** The 3Dfx edition's, settled on the ATI and PowerVR editions alike: their `network.c`, `mechreload.c` and `shots.c` are byte-identical or witnessed at `/Od` (all but `UpdateShot`, which the audit classifies), and `/Ow` gives `KnockMechOver`'s 34 instructions, which the witness proves in both, where `/Oa` gives 39 (`p_mech->m_player` kept in a register across the calls) and neither flag 35. `KeyboardPoll`, without `/Ow`, is byte-identical in both. The optimized editions link without their 1.1 units nothing calls (the fixed-point helpers, `texpoly.c`): the ATI, S3 and PowerVR editions' `/OPT:REF` dropped them anyway, the 3Dfx edition's link, without it, kept them.
- **Annotations** were ported from the 3Dfx original by bytes (`tools/port_annotations.py --from MW23DFX MW2_3DFX.DLL --to MW2ATI MW2_ATI.DLL`). Relocations are masked, so a function or global placed by bytes or pointers can be another one that differs only in what it addresses (the ATI edition's texture filter is at the place of the 3Dfx edition's 800x600 setting, its DirectDraw back end at that of the 3Dfx edition's Glide one): datacmp and `tools/check_imports.py` find them.

## The GBL edition

- **What it is.** `MW2_GBL.DLL` is Ghost Bear's Legacy's simulator: the `MW2.DLL` of the expansion's 1.1 patch (`GBLPATCH.EXE`, on the Internet Archive as `GBLPATCH`: like 1.1's `MW2PATCH.EXE`, an ARJ archive at `0x1853`), the same file as the GBL disc's `/MW2.DLL`. LINK 3.10 linked it on 1996-10-18, four weeks after 1.1, and it is 1.1's code: 1164 of 1.1's 1560 annotated functions are in it byte for byte, relocations aside.
- **Build.** VC++ 4.1 at `/Od /G5` with `LIBCMTD`, as 1.1, but without `/Oi`: there is no inline `strlen` in its game code (1.1 has 85), it calls `memset`, `memcpy`, `strcmp` and `memcmp` where 1.1 expands them (`FindSession`, `FindIpxProviderCallback`), and `_sqrt`, `_sin` and `_atan` where 1.1 calls their `__CI` entries. It expands `abs` and `fabs` all the same, as 1.1 does; how its build did that isn't known (a `#pragma intrinsic` or other headers), and `util/types.h` reproduces it with a `#pragma intrinsic(abs, fabs)` under `MW2_GBL`. It was linked without `/DEBUG` (the DLL has no debug directory), so with LINK's default `/OPT:REF`: the functions nothing calls are gone, with the imports of the C runtime they bring in (1.1's `HeapWalk`, locale and console imports), which is most of why its code is 20 KB smaller than 1.1's.
- **Placement.** Its object order is its own (`MW2/sources_gbl.cmake`: it shares no long run of units with 1.1's, the Matrox edition's or the 2.2 editions'), found from the functions and data it shares with 1.1, and `tools/check_units.py` passes with it. Three of its objects are parts of 1.1's: the cockpit panels (`cockpitpanels.c`, after `timedoverlays.c`), the faces' projection (`faceprojection.c`, the end of 1.1's `objectanim.c`, after `mechcollision.c`, where the Matrox edition has its float versions) and `clock.c`'s tables with the rotation's normalization (`clockmath.c`, at the end of its code); it builds those fragments as units. It defines `g_shouldToggleFullscreen`, `g_desktopWidth` and `g_desktopHeight` among `refreshmode.c`'s data, not `simmain.c`'s, and places `RunAIRules` after `FirstAI` (annotated by name in `ai.h`).
- **Annotations** were ported by bytes from 1.1's original, then from the rebuild (`tools/port_annotations.py --rebuild`: the functions that differ from 1.1's only by the intrinsics); the globals datacmp then reported were left out until the edition's data was written. Every function of its game code is annotated now, and every global 1.1 has that GBL compiles, but two 1.1 placeholders nothing references: the last functions by their neighbours and sizes, the last globals from the rebuild and by the offset between known neighbours in each unit, VFX's labels in `.text` by their bytes. Every function is byte-identical (999) or proven by a witness (566), and `tools/known_diffs.txt` has no GBL line.
- **Structures.** Its `Mech` (0x114 bytes) has `m_externalHeat` at 0x98, the heat the radius damage adds and `CalculateHeat` takes (`MW2_EXTERNAL_HEAT`), and a second word of flags at its end (`MW2_MECH_SECOND_FLAGS`); a mech has 14 weapon slots, not 10 (`MW2_FOURTEEN_WEAPON_SLOTS`, `WEAPON_SLOT_COUNT`). Its `Player` (0x1b8 bytes) has an `int` for its AI flags, where 1.1 has a short (`MW2_AI_FLAGS_INT`), and a flag after `m_nextFireTime` and two values at its end that its AI setup takes from a table of pilots for a gunnery above 6 (`MW2_PILOT_TABLE`). Its 14 weapon slots have 14 weapon panels, seven in each column, so its panels from the target's on are 4 further (`c_panelTarget` 17, `c_panelCount` 30) and its panel tables have 30 entries.
- **VFX.** GBL links VFXA as released (`MW2_VFX_STOCK`, the accelerated editions' feature) with `VFX_lookaside_write` (`MW2_VFX_LOOKASIDE_WRITE`), and keeps 1.1's aligned fill in VFX3D's `VFX_flat_polygon` (`MW2_VFX3D_ALIGNED_FILL`, as the ATI and PowerVR editions do); its VFXREND is 1.1's. The target passes ML its features (`demech2_edition_masm_defines`).
- **Data.** Two game keys of its own: `CHAT_MODE`, action 66, so its actions from 66 on are one higher (`MW2_CHAT_KEY`, `GAME_KEY_ACTION`), and `TOGGLE_AMS`, 169 (`MW2_AMS_KEY`). Its local star starts at 0, as the accelerated editions' does (`MW2_LOCAL_STAR_FROM_ZERO`). Its message boxes and refresh mode errors name Ghost Bear's Legacy, typos included (`MW2_GHOST_BEAR_TITLES`), and it finds its CD by `GBL_HERC.DRV` (`MW2_GBL_CD_DRIVER`). `GetEngagementAIFlags` returns an `int` (`MW2_ENGAGEMENT_FLAGS_INT`), and a recentering torso snaps to the legs within 1 degree, not 4 (`MW2_TORSO_SNAP_ONE_DEGREE`).
- **Weapons, shots and effects** (`MW2_WEAPON_FUNCTIONS`). A weapon type (`WeaponDef`, 0x68 bytes) starts with a function its slots call each frame, its launch function (`SpawnShot`, which now takes the definition, for all but the AMS), flags and the first of eight animations; the table has 35 weapons, Inf SRM, Torpedo, InfSSRM and ArrowIV the new ones. The flags say when the launch sound plays (a volley's first shot or each), whether the weapon selection stops at it (not the AMS), whether its guided shot eases its turns (the torpedo), whether its shot starts the animations (the flamer) or leaves from the hardpoint itself, and whether it marks the mech it hits (the NARC: a guided weapon then locks on that mech at once). A shot (0x58 bytes, 255 of them) carries its weapon's flags; an effect (0x2c, 255) the shape it is attached to; an effect type (0x2c bytes, 36 of them) its own damage and heat per tick and flags (attached to the hit mech's object, uncounted), where 1.1 damages by 0x100 a tick whatever the type. `UpdateEffects` grows the radius of the nuke's and effect 0x23's damage as they burn, and the radius damage heats mechs from outside. `PlayPositionalSound` takes the sample's flags and plays nothing past a cutoff distance (`MW2_POSITIONAL_SOUND_FLAGS`); a destroyed game thing counts for the team whoever destroyed it (`MW2_THING_KILLS_FORGET_SHOOTER`), and a shot destroys a building at once (`MW2_BUILDINGS_FALL_AT_ONCE`). The AMS (`MW2/src/gbl/ams.c`, an object of its own before `refreshmode.c`'s) fires when an enemy missile within 50000 units closes on its mech, one to five rounds a launch, and gives each missile it reaches, once, a range from the mech at which it detonates: three times in four a random one beyond 2500 units. Its game key turns it off (a slot state of 4) and on.
- **Damage, kills and the AI.** A destroyed mech blows the critical slot of its first ammunition bin, and a pilot ejects when auto-eject is on or the mech isn't the player's (`MW2_DESTROY_BLOWS_FIRST_BIN`); kills and hits count for the player whose shot hit last, and in a network game only the local mech's damage and death are played out (`MW2_KILLS_BY_LAST_SHOOTER`); an overheated mech dies after 1810 ticks. Its maneuver tables are data, where 1.1 fills them at run time: every type but the mech has the stupid maneuver, and four more tables serve the pilot table, whose entries (for a gunnery above 6, `InitializeAI`) give a pilot its skills, its maneuver table and the range at which it breaks off (`MW2_STATIC_MANEUVER_TABLES`). An AI pilot whose pilot-table entry has the flag fires its weapons as one group (`GroupAIWeapons`, `DecideAIFire`, `RunAIWeapons`), a player's AI flags are an `int` (`MW2_AI_FLAGS_INT`), only the local player inspects targets (`MW2_LOCAL_INSPECTION`), and its source copies a nav point and a ray with `memcpy` (`MW2_STRUCT_COPY_MEMCPY`).
- **Keys and cheats.** Its cheat codes are a table with the message each shows, codes and messages stored XORed with 0xa and the messages decoded at the first key typed (`MW2_CHEAT_TABLE`: `g_cheatCodes`, `DecodeCheatMessages`, `HandleCheatInput`), with codes of its own. `RunGameKey` and `HandleDebugKey` number their keys with `GAME_KEY_ACTION` and take the chat and AMS keys, and the PT menu keys do what the first three starmate menu keys do (`MW2_PT_MENU_KEYS`). `GetIniValue` cuts a value at a `;` and trims it unless asked for the raw value (`MW2_INI_COMMENTS`), and the mouse reports no input errors (`MW2_NO_MOUSE_ERRORS`).
- **Missions.** Its objectives are 1.1's with the Matrox edition's leader test (`MW2_LEADER_NEAR_TARGET`) and the 3D-card editions' network stars (`MW2_NETWORK_STAR_PLAYERS`, `MW2_RESTART_KEEPS_LISTED`), and the local star's mission fails once its mech is lost (`MW2_LOST_MECH_FAILS_STAR`). In the world stream a scrounge record whose id names no class takes a static object's shape (`MW2_SCROUNGE_STATIC_OBJECTS`), and the players it creates for other machines or the AI get an AI mode by the kind of game (`MW2_REMOTE_PLAYER_AI_MODE`). Heat sinks cool five times as well at -100 degrees or colder (`MW2_DEEP_COLD_COOLING`).
- **Network and display.** `SimMain` updates the players before a network launch too (`MW2_PLAYERS_BEFORE_LAUNCH`), and `UpdateMech` leaves another machine's mech where it is (a truck at its object's height) (`MW2_REMOTE_MECHS_STAY`). Its DirectDraw back end is 1.1's built with DirectX 1's headers (`MW2_DDRAW_CAPS_DX1`), whose `DdrawStop` leaves the cooperative level and the display mode as they are (`MW2_DDRAW_STOP_KEEPS_MODE`); `StartSupAnim` keeps the backdrop's first 16 colours (`MW2_SUPANIM_KEEPS_PALETTE`).
- **How its source was written.** The artefacts (the table's *Artefact.* lines) are mostly calls its build makes where 1.1 expands them: `memcpy` for structure copies (`MW2_STRUCT_COPY_MEMCPY`), the C runtime's `abs`, `labs` and `strcmp` in some units (`MW2_ABS_CALLS`, `MW2_BEARING_LABS`, `MW2_NAME_STRCMP`), `MemCopy` and `MemSet` in C (`MW2_MEM_HELPERS_CALL_CRT`). Three functions need a declaration order of their own (`#ifdef MW2_GBL` in `MoveMechWithCollisions`, `SelectNextWeaponInGroup` and `BuildQuadtreeChild`), where the order GBL's code needs would cost another edition its proof.

## The early Mercenaries build

- **What it is.** `MW2_PREMERCS.DLL` is an unused, early transitional build between Ghost Bear's Legacy and Mercenaries, almost certainly an early build of Mercenaries' simulator that was included by accident: the Mercenaries disc carries it as `MW2.DLL` in its `/MERCS` folder, beside Mercenaries' own simulator (`MERCSW.DLL`, 1996-09-16), which is the one the game runs, and nothing loads it. Mercenaries 1.05, the Enhanced Direct 3D disc, and the BattlePack, GBL BattlePack and Titanium releases carry the same file, unused there too. LINK 3.10 linked it on 1996-07-08, three months before GBL's 1.1 patch and two after the GBL Pentium disc's simulator (1996-06-20), from a tree its debug directory names `\mercs\mw2.dll` (its PDB `C:\mw2win\Mw2___Wi\mw2.pdb`). It is Ghost Bear's Legacy's code: its message boxes are titled "Ghost Bear's Legacy", it finds its CD by `GBL_HERC.DRV`, and it has GBL's chat and AMS keys and three of its four new weapons (not the Torpedo); and it reads Mercenaries' first files (`mercs.hud`, `MW2CAR.CFG`). Its code is 43 KB larger than 1.1's and its `.bss` 0.5 MB larger.
- **Build.** VC++ 4.1 at `/Od /G5` with `LIBCMTD`, without `/Oi`, as GBL: four `repne scasb` in its game code where 1.1 has 89, and `abs` and `fabs` expanded as many times as in GBL's. It was linked with `/DEBUG` (it has a debug directory), so without `/OPT:REF`: its C runtime is 1.1's set of objects (90 `KERNEL32` imports, as 1.1), less `mbscat.obj`, since its `strcpy` and `strcat` come from `strcat.obj`. Its game code ends at the import thunks (`0x1008aa62`), which precede the C runtime.
- **Its starting point.** By bytes, 1134 of GBL's functions are in it (1032 of 1.1's), and it expands what GBL expands, so it starts as GBL: `MW2_PREMERCS` has every feature of `MW2_GBL` (each stays until its code shows otherwise), and it builds GBL's units in GBL's order, with GBL's fragments as units. One feature already separates it from GBL: its `JoystickOpenDevice` probes with `joyGetPosEx` alone, on an uncleared `JOYINFOEX` (`MW2_JOYSTICK_OPEN_POSEX`, as the Matrox edition's), where GBL's October build probes as 1.1 does (the original imports no `joyGetPos`). Its own object order is neither GBL's nor 1.1's (the functions it shares with each keep their order in runs of about a quarter of them).
- **Placement.** `MW2/sources_premercs.cmake` lists GBL's units in its object order, and `tools/check_units.py` passes with it. The functions it shares with GBL place most of them; the eight whose code changed (`mechviewpanel.c`, `damagepanel.c`, `weaponpanel.c`, `targetpanel.c`, `emptyfns.c`, `mekfile.c`, `gbl/ams.c` and `settings.c`) were placed by their functions' sizes and calls, and the units that hold only data (`cockpitlayouts.c`, `bwdkeywords.c`, `weapondata.c` and the menus) by their data. Its VFX objects come first, right after the two MASM units, where GBL has VFXA after `mw2prj.c` and VFX3D and VFXREND after `eyepoint.c`; the AMS (`gbl/ams.c`) follows `weapons.c`; and like GBL it has the cockpit panels, the faces' projection and `clock.c`'s tables as objects of their own, and `g_shouldToggleFullscreen`, `g_desktopWidth` and `g_desktopHeight` among `refreshmode.c`'s data. Two objects after `shots.c` (`0x10087ee0`, 19 functions, and `0x10089830`, 12) have no counterpart in GBL: they create, move and draw objects through its `UpdateObj` (below).
- **Its players** (`MW2_PLAYER_UNK_MEMBERS`) are 0x1fe bytes, where GBL's are 0x1b8 (`AllocPlayer`'s size): 0x28 bytes more between the target information and the name, 2 after `m_piloting` and 0x1c after `m_nextDetectCheck`, found from the offsets the AI's and the maneuvers' functions read (the names' terminators that `BwdExecuteStream` writes moved with them). Nothing decompiled reads them yet. Its mechs are larger too (`MW2_MECH_UNK_MEMBERS`): `GetMechAllocSize` adds 0x1a3 bytes for the mech, 0x74 per weapon slot and 0x1c per ammunition bin, where GBL's are 0x114, 0x68 and 0x14. `InitMechArrays` sets up the slots' and bins' members at GBL's offsets, and the rebuild matches the most functions with the extra bytes at each structure's end (0x8f, 0xc and 8).
- **The shape grid** (`MW2_SHAPE_GRID`). Its scene files the shapes in a grid of 12 by 12 cells of 25000 units, the first cell's corner at -150000 in x and z (`g_shapeGrid`, in `shapelists.c`), with a list for the shapes outside it: each cell is a list head that points at itself, and a shape keeps its cell in two members inserted before `m_collisionData`, so its shapes are 0x54 bytes, not 0x4c. The functions that touch the shapes' later members differ from GBL's by those 8 bytes alone. `UpdateObj` (`0x10033604`) updates the tree (1.1's `UpdateObj`, here `UpdateObjTree`) and then, for a root while the grid is on, files the root's shape and its children's in the cell it has moved to (`UpdateShapeGridCell`), or in the off-grid list. `UnlinkShape` resets a shape's cell, and the lists' other functions file a shape in its cell, or in the off-grid list, where 1.1's take the scene's list. Three functions list cells for the renderer and the collisions to go through (`g_shapeGridLists`, the scene's list alone while the grid is off): every cell (`ListAllShapeGridCells`), the cells around the eyepoint's, the farthest first, in rings whose start follows its heading (`ListShapeGridCellsAhead`, which the scene's drawing calls), and a point's cell with three of its neighbours (`ListShapeGridCellsAround`, for the collisions); that one picks the quadrant from the position modulo 12, not modulo the cell, so the neighbours it takes don't follow the point's place in its cell, and its third quadrant takes a neighbour on the other side. But nothing turns the grid on: the two functions that set and clear `g_shapeGridEnabled` (`EnableShapeGrid`, `DisableShapeGrid`) have no caller or reference, so the build draws and collides as GBL does. `shapelists.c`'s grid code is decompiled; its callers, which go through the lists, are not yet.
- **Annotations** were ported by bytes from GBL's original, then from 1.1's for the functions GBL lacks (`tools/port_annotations.py`), GBL's address taken where the two disagreed (`g_effectInfo`), and again from the rebuild once the shapes had their grid members; the C runtime's `strcpy` and `strcat` are `strcat.obj`'s. The functions that stayed unproven after the witness search were left out, as the globals datacmp then reported (most of them tables of pointers to functions not annotated yet).

## Adding an edition

What a new 3D-card edition of the simulator needs (ATI, S3, PowerVR), in the order that worked
for the Matrox and 3Dfx editions. Each step names the files that list the editions today: none of
them finds a new one by itself. A later revision of the game is another question, to settle before
this list applies: if most of its functions pair with 1.1's it joins the tree the same way, with
features named for what it changed (Ghost Bear's Legacy, whose functions are mostly 1.1's); if
not, it gets a tree of its own.

### 1. The binary and its toolchain

- **Identify the build before writing source.** The link date and linker version are in the PE
  header; the compiler is the one whose C runtime the binary's matches as it is (4.0's `LIBCMT` for
  the Matrox edition, 2.2's `LIBC` for the 3Dfx edition); the calling convention and the frames
  show the rest (`/Gr`: arguments in `ecx`/`edx`; `/Gy`: `int3` between functions inside an
  object; `/Ox`: no `/Od` frame). Don't assume one set of flags for every unit: both editions have
  units compiled differently from the rest, and one function with a pragma of its own.
- **The compiler.** VC++ 2.2 and 4.0 are set up (`vc22/`, `vc40/`: one nested build per compiler,
  since a CMake project has one compiler per language). Another compiler takes another sub-project
  like them: the `ExternalProject` and its environment file in the top-level `CMakeLists.txt`, a
  `DEMECH2_MSVC<nn>_ROOT` option, the compiler's checkout in `docker/Dockerfile` and
  `docker/entrypoint.sh` and in both workflows, and a line in the README's requirements. If
  reccmp's cvdump can't read its linker's PDB, compile with `/Z7` and link with 4.1's LINK against
  the compiler's own libraries, as the 2.2 targets do.
- **Settle the flags by probing**, one unit against the original: the flags are right when
  functions come out byte-identical from the shared source, not when they look close. Record what
  decided each flag in the edition's section of this document (the 3Dfx edition's `/Ow` against
  `/Oa`).

### 2. The target and its identity

- **Names.** A reccmp target (`MW2<EDITION>`), the original's file name (`MW2_<EDITION>.DLL`) and
  an identity macro (`MW2_<EDITION>`), defined by the build together with `MW2_ACCEL`.
- **The target:** `reccmp-project.yml` (hash, `source-root` `MW2` and `common`, its place in the
  target order, which is also the order of annotations on a shared function), the target's
  definition in its sub-project's `targets.cmake` (its own list of MW2's and `MW2/src/accel`'s sources,
  in the closest edition's order to start with; the identity macros among its compile options and through
  `demech2_edition_masm_defines` for ML), `reccmp_add_external_target` in the top-level
  `CMakeLists.txt`, the original's name in `.gitignore`, and the README's `reccmp-user.yml` example.
- **The identity in the table.** `util/types.h`: the two `#error` tests of impossible identities,
  and an `#ifdef MW2_<EDITION>` block for the edition's own features, empty at first. The edition
  starts with every feature of `MW2_ACCEL` and none of another edition's.
- **The lists of identities and simulators in the tools:**
  `tools/unitbuild.py` (`EDITION_MACROS`, and `TARGETS`: the target's CMake directory and
  environment file), `cmake/editions.cmake` (the `#ifdef` it recognizes), `tools/funccmp.py`
  (`EDITION_DEFINES`), `tools/compare_objects.py` and `tools/edition_sites.py` (`SIMULATORS`; the
  latter's `EDITION_NAMES` and `SHORT` too), `tools/check_block_sizes.py` (`EDITION_DEFINES`, by
  folder), `tools/coverage.py` (`COVERERS`: which `/Od` target's proofs cover an optimized one's
  functions), `tools/states_page.py` (the target's title) and `tools/vfx_names.py` (where the VFX
  objects' data starts).
- **CI.** `.github/workflows/build.yml`: the `retail` list of the `targets` job, the nested
  build's outputs in the two artifact lists, the certify job's loop, and the decomplint job's mapping of
  the target to the `MW2` folder. `.github/workflows/verify-retail.yml`: `RETAIL_ENTRIES` (with
  the `--total`, below), the original's hash, the coverage `case`, and the `--states` of the
  "Report the edition sites" step. The original itself goes into the private repository the retail
  workflow checks out.

At this point the target builds from the Matrox edition's C without a renderer, and the byte gate
(`python tools/compare_objects.py main .`) must show every object of the other targets unchanged.
Keep it that way: an edition's code behind its `#ifdef` costs the others nothing, but a declaration
added to a shared header can move every unit that includes it.

### 3. The back end

- **Its folder.** `MW2/src/<edition>/` and `MW2/include/<edition>/`: `a3d.c`, the A3D back end
  over the card's library, and the display back end (the `directdraw.c` functions one for one, in
  DirectDraw's slot of `g_displayBackends` and `g_refreshModes`). Code two editions share never
  goes in one edition's folder: it belongs in `MW2/src/accel`.
- **The card's library** in `3rdparty/`: a `.def` for the import library and our own declarations
  of what the game uses, with `__cdecl` spelled out if the edition is built with `/Gr`. A library
  linked in statically takes its own source, licence and `LIBRARY` annotations (`3rdparty/texus`,
  `MW2/library_texus.h`).
- **The interface.** `MW2/include/accel/a3d.h` declares what every renderer implements under
  shared names, and each edition's own entry points under its identity. What the shared code
  does for one back end tests that edition's identity, since a back end's name would only restate
  the edition; when a second edition shares the behaviour, it becomes a feature.

### 4. Pairing and annotations

- **An edition built at `/Od`** pairs with 1.1's bytes: `tools/port_annotations.py --from MW2
  MW2.DLL --to <TARGET> <FILE>` writes the annotations of the functions that match up to
  relocations, slots and comparison order, maps the globals through their relocations and matches
  the C runtime. `tools/edition.py` keeps the pairing for the rest (`report`, `cmp`, `auto`,
  `audit`, `crt`); it is written for `MW2MATROX` (the binaries' names, the `vc40` build, the end
  of the game code), so the new target is a parameter to add there.
- **An optimized edition** can't be paired by bytes. Its annotations come from the rebuild, which
  is byte-identical wherever the source is right (`port_annotations.py --rebuild`, with
  `--located-differing` once the pointer tables are mapped), and from references:
  `tools/edition_3dfx.py` pairs a source function with the original's function whose calls,
  imports, globals and strings are most like its rebuilt version's (`state`, `report`, `auto`,
  `names`, `unpaired`). It is the 3Dfx edition's (its binary, its build folder, the end of its
  code): generalize it rather than copy it.
- **The total.** The game-code function count is the target's `total` in `targets.toml` (and the
  `--total` in `verify-retail.yml`, which CI checks against it): Ghidra's count below the C runtime for an `/Od` build; for a `/Gy` build
  `tools/edition.py`'s `functions` command, which traces the code (Ghidra missed 261 of the 3Dfx
  edition's 1574).
- **The rest by name:** the C runtime in `MW2/library_msvc.h`, the VFX objects' static data in
  `reccmp/<target>-vfx.csv` (`tools/vfx_names.py`; the three objects' data are laid out the same
  in every edition so far), and by-name markers in the unit's header for a function the edition
  places out of its file's order.
- **Placement.** The object order goes into the target's source list, found from the functions
  and data the edition shares with the others. Code it places in another object than 1.1 takes a
  fragment, included at each edition's position under that edition's macro. Data it places in
  another object moves for every target: globals at the edge of a 1.1 unit that the edition puts
  next to another unit belong to that neighbour, and a block whose neighbours differ between
  editions is a data-only unit of its own (`cockpitlayouts.c`, `commandpoint2menu.c`). Until the units are
  placed, the target skips `tools/check_units.py` in `verify-retail.yml`. A `/Gy` build has no
  object boundaries to read (every function is padded to 16): `<TARGET> comdat` in
  `tools/check_units_expected.txt` makes the check place its units by the order of their runs of
  code and data alone, as for `MW23DFX`.

### 5. Which features it has

A new edition is the first evidence about every feature that has only been attributed. Go through
the tables below in this order:

1. **The features one edition has and the other doesn't** (the `MW2_MATROX` and `MW2_3DFX`
   tables). The new edition has each one, lacks it, or does a third thing; a feature it shares
   moves to a block both editions' identities select, and a feature can't stay true of an edition
   that only partly has it: split it.
2. **The features with a value per edition** (`MW2_PIXEL_FORMAT`): the new edition's value, read
   from the code that uses it.
3. **The features of `MW2_ACCEL`.** Each was both editions' because nothing showed otherwise. Once
   the target has annotations and states, the published `EDITIONSITES.md` gives, per feature and
   target, whether a byte-identical or witnessed function compiles each site: an unverified site
   in a function that doesn't match is the first suspect, before the source's shape. A feature the
   new edition lacks leaves `MW2_ACCEL`'s block for the blocks of the editions that have it, and
   what that separates (the float conversion from A3D, say) is then evidence where it was a
   reading: say so in "What a line of this document rests on".
4. **The functions with a definition per edition** (the list below): which version the edition
   compiles, or its own.
5. **What isn't a feature:** the data it changed (datacmp finds it), globals in `.data` in one
   edition and `.bss` in another, fragments, declaration orders, and, in an optimized build, the
   helpers it expands inline (`MW2_INLINE_HELPERS`), the float expressions its folding
   distinguishes and the declarations that need `__cdecl`. These test the identity, as artefacts
   and source forms.

A single difference seen in two editions may be one later revision of the source rather than two
features: the third and fourth editions are what can tell. Describe what editions share in the order they were built
(the identity table's link dates): an older edition's code is what a later one kept or changed,
not the other way round. The ATI edition, the oldest, has no "3Dfx edition's" code: the 3Dfx
edition has the ATI edition's. When a site's difference has no name
yet, add the feature to the table with its comment before using it, and run `--doc`.

### 6. Proofs and documents

- **Proofs:** `tools/witness.py --write` on the target (and, after any change to a shared header,
  on the other simulators' units that include it), `tools/certify.py`, `tools/audit.py --write`
  for its lines in `tools/known_diffs.txt`, and the expected findings of `tools/check_imports.py`,
  `tools/check_units.py` and `tools/check_block_layout.py`. No comment says how well a function
  matches; the records and the known differences do.
- **This document:** a column in the identity table, a section for the edition (how its flags
  were found, its renderer and display back end, its library, the data it changed), the comments
  above its features and above its own definitions of functions, then `python
  tools/edition_sites.py --doc`.
- **Elsewhere:** its entry in `targets.toml`, then `python tools/targets.py --doc` (the README's
  status table and `docs/TARGETS.md`) and `--check`, which names the lists still kept by hand
  (`reccmp-project.yml`, `verify-retail.yml`); and the working rules its compiler or its build
  adds, in `docs/EDITION_NOTES.md`.

## Keeping this document

Everything below the marker is written by `python tools/edition_sites.py --doc` (from the
repository root, with a configured build tree: which target compiles which branch comes from the
targets' own compile commands). CI runs `--check`, which fails when the document is stale, when a
feature has no comment above its `#define`, when nothing tests a feature, when a conditional
tests an `MW2_` macro the table doesn't define, and when no target compiles a branch.

<!-- generated by tools/edition_sites.py --doc: begin -->

## Features

From the table in `util/types.h` (211 features) and the 1435 conditional sites of the simulators' sources (`MW2/`, `common/`, `3rdparty/vfx`). A feature's cell is its value where it has one.

### The ATI, S3, Matrox, PowerVR and 3Dfx editions (`MW2_ACCEL`)

Every edition that has one of these has them all, so no binary separates one from another: which of them a site belongs to is a reading of what its code does.

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_FLOAT_SCALARS` |  | yes |  | yes | yes | yes |  | yes | 348 | *Representation.* The simulation's 16.16 fixed-point values are floats (MechScalar, FIXED_*) |
| `MW2_A3D` |  | yes |  | yes | yes | yes |  | yes | 67 | The A3D renderer interface and its records: queued polygons, projected vertices, depth lists |
| `MW2_VERTEX_SHADING` |  | yes |  | yes | yes | yes |  | yes | 13 | Per-vertex shading for the renderer: WTB normals and colours, shaded projected vertices |
| `MW2_VFX16` |  | yes |  | yes | yes | yes |  | yes | 35 | The 2D drawing goes to the 16-bit port of VFXA |
| `MW2_LARGER_POLYGON_BUFFERS` |  | yes |  | yes | yes | yes |  | yes | 1 | A larger draw buffer and polygon limit |
| `MW2_NO_DISSOLVE` |  | yes |  | yes | yes | yes |  | yes | 1 | StartPalettes blends through the display back end, without 1.1's pixel dissolve |
| `MW2_DISPLAYDIB_DISABLED` |  | yes |  | yes | yes | yes |  | yes | 1 | DispDibBegin always fails |
| `MW2_REFRESH_MODE_RETRY` |  | yes |  | yes | yes | yes |  | yes | 2 | InitRefreshMode tries a mode without checking it's available, and marks it |
| `MW2_RAW_SCREENSHOTS` |  | yes |  | yes | yes | yes |  | yes | 3 | Screenshots are the screen's raw 24-bit pixels (MW2NNNN.888), not GIFs |
| `MW2_GRAPHICS_MENU` |  | yes |  | yes | yes | yes |  | yes | 6 | The graphics page, in place of 1.1's monitor brightness page |
| `MW2_REGISTRY_SETTINGS` |  | yes |  | yes | yes | yes |  | yes | 2 | The sky, the ground and the renderer's settings are read from the registry |
| `MW2_DEFAULT_DRAW_MODE` |  | yes |  | yes | yes | yes |  | yes | 1 | The draw mode and its parameter start at 0 (1.1: -1 and 1) |
| `MW2_HUD_ART` |  | yes |  | yes | yes | yes |  | yes | 10 | The HUD art: its resolution chosen before the 512x384 override, and the shapes' numbering |
| `MW2_HUD_SHAPE_IDS` |  | yes |  | yes | yes | yes |  | yes | 6 | HUD shape resource ids 3 higher (the pause banner, the target panel, the nav icons) |
| `MW2_SLIDER_SHAPES` |  | yes |  | yes | yes | yes |  | yes | 1 | The menu sliders' shapes are 3 higher |
| `MW2_MENU_BACKGROUND_0X118` |  | yes |  | yes | yes | yes |  | yes | 1 | The main menu's background is shape 0x118, not 0x112 |
| `MW2_PRJ_CEL_IDS` |  | yes |  | yes | yes | yes |  | yes | 1 | The resource archive numbers the preloaded cels differently |
| `MW2_BRIGHT_MAP_COLORS` |  | yes |  | yes | yes | yes |  | yes | 3 | The maps' colours are other (brighter) palette entries |
| `MW2_COCKPIT_POINT_ONLY` |  | yes |  | yes | yes | yes |  | yes | 3 | The cockpit layout keeps only its point, and SetRes scales no gauge pane or point |
| `MW2_UNRESOLVED_GAUGE_INDICES` |  | yes |  | yes | yes | yes |  | yes | 2 | LoadCockpitLayout doesn't map the gauge indices to functions |
| `MW2_DEBUG_OVERLAY_COLOR` |  | yes |  | yes | yes | yes |  | yes | 1 | The debug overlays' text is colour 0x52, not 0x4f |
| `MW2_NO_EYE_POSITION_OVERLAY` |  | yes |  | yes | yes | yes |  | yes | 1 | No eye position overlay |
| `MW2_NO_BOUNDING_SPHERES` |  | yes |  | yes | yes | yes |  | yes | 2 | No debug drawing of the shapes' bounding spheres |
| `MW2_INFRARED_OFF_FADE` |  | yes |  | yes | yes | yes |  | yes | 1 | Turning infrared off fades back to the base palette |
| `MW2_FADE_BACK_DURATION` |  | yes |  | yes | yes | yes |  | yes | 1 | The palette's fade back takes the duration as its step count |
| `MW2_TIME_OF_DAY_FADE_ONCE` |  | yes |  | yes | yes | yes |  | yes | 1 | FadeToTimeOfDayPhase fades only while no phase is set |
| `MW2_FACE_KIND_BIT15` |  | yes |  | yes | yes | yes |  | yes | 3 | Bit 15 of a face's id is part of its kind, not a flag for MapFaceId's remap |
| `MW2_WTB_NEGATED_NAMES` |  | yes |  | yes | yes | yes |  | yes | 1 | LoadShapeRecord decodes negated shape names |
| `MW2_MODEL_MIN_ONE_FACE` |  | yes |  | yes | yes | yes |  | yes | 1 | AddModel allocates at least one face |
| `MW2_GRID_CELL_XZ` |  | yes |  | yes | yes | yes |  | yes | 5 | The grid object's anchor and cell keep only x and z |
| `MW2_NO_PROJECTION_OFFSET` |  | yes |  | yes | yes | yes |  | yes | 3 | The eyepoint has no projection offset |
| `MW2_FRUSTUM_HEIGHT_SIGN_FROM_SIDE` |  | yes |  | yes | yes | yes |  | yes | 1 | The vertical frustum limit takes its sign from the side distance |
| `MW2_NO_RAY_BOX_ENTRY_BEHIND` |  | yes |  | yes | yes | yes |  | yes | 1 | TestRayBox doesn't record whether the ray entered behind |
| `MW2_AIMED_SHAPE_RESET` |  | yes |  | yes | yes | yes |  | yes | 2 | DestroyObjShape clears the aimed shape when it destroys it |
| `MW2_BUILDING_BOUNCE` |  | yes |  | yes | yes | yes |  | yes | 1 | The bounce off buildings isn't negated, and is halved either way |
| `MW2_COLLISION_DAMAGE_X2` |  | yes |  | yes | yes | yes |  | yes | 1 | Damage from below scales by twice the mass ratio, not four times |
| `MW2_RADIUS_HEAD_DAMAGE_HALF` |  | yes |  | yes | yes | yes |  | yes | 1 | Radius (fire) damage to the head is half, not a quarter |
| `MW2_NO_ENGINE_NOTE` |  | yes |  | yes | yes | yes |  | yes | 1 | StartEngineNote returns at once |
| `MW2_JOYINFOEX_UNCLEARED` |  | yes |  | yes | yes | yes |  | yes | 1 | JoystickPoll neither clears its JOYINFOEX nor sets its size |
| `MW2_NO_FULLSCREEN_KEY` |  | yes |  | yes | yes | yes |  | yes | 1 | Key code 0x40d no longer toggles full screen |
| `MW2_LEMMY_CHEAT` |  | yes |  | yes | yes | yes |  | yes | 1 | The "lemmy" cheat |
| `MW2_NO_DORCS_CHEAT` |  | yes |  | yes | yes | yes |  | yes | 1 | 1.1's "dorcs" cheat is gone |
| `MW2_EXTRA_DEBUG_PRINTS` |  | yes |  | yes | yes | yes |  | yes | 5 | Debug output both accelerated editions add |

### The ATI, Pre-Mercs, S3, Matrox, PowerVR, GBL and 3Dfx editions (`MW2_ACCEL`, `MW2_GBL`, `MW2_PREMERCS`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_VFX_STOCK` |  | yes | yes | yes | yes | yes | yes | yes | 7 | VFX as released (unaligned fills and runs, word-wise character copies), where 1.1 modified it; the assembly gets it from cmake/editions.cmake |
| `MW2_VFX_LOOKASIDE_WRITE` |  | yes | yes | yes | yes | yes | yes | yes | 2 | VFXA's VFX_lookaside_write, which sets one lookaside entry (cmake/editions.cmake) |
| `MW2_LEADER_NEAR_TARGET` |  | yes | yes | yes | yes | yes | yes | yes | 3 | IsTeamNearTarget tests only the team's leader |
| `MW2_LOCAL_STAR_FROM_ZERO` |  | yes | yes | yes | yes | yes | yes | yes | 2 | The local star starts at 0, and the player's formation goes to it |
| `MW2_NETWORK_STAR_PLAYERS` |  | yes | yes | yes | yes | yes | yes | yes | 2 | UpdateObjective takes star N as player N in network games |
| `MW2_RESTART_KEEPS_LISTED` |  | yes | yes | yes | yes | yes | yes | yes | 1 | Restarting a star mission keeps its objectives' targets' listed flags |
| `MW2_AXIS_RATE_POINTER_TEST` |  | yes | yes | yes | yes | yes | yes | yes | 1 | UpdateInputs compares an axis' rate pointer with its position's address |

### The ATI, Pre-Mercs, Matrox and 3Dfx editions (`MW2_MATROX`, `MW2_3DFX`, `MW2_ATI`, `MW2_PREMERCS`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_JOYSTICK_OPEN_POSEX` |  | yes | yes |  | yes |  |  | yes | 2 | JoystickOpenDevice probes with joyGetPosEx alone, on an uncleared JOYINFOEX (Matrox); JoystickOpenDevice probes with joyGetPosEx alone, on an uncleared JOYINFOEX (3Dfx); JoystickOpenDevice probes with joyGetPosEx alone, on an uncleared JOYINFOEX (ATI); JoystickOpenDevice probes with joyGetPosEx alone, on an uncleared JOYINFOEX (GBL's later build probes as 1.1 does) (Pre-Mercs) |

### A value per edition: the ATI, S3, Matrox, PowerVR and 3Dfx editions (`MW2_MATROX`, `MW2_3DFX`, `MW2_ATI`, `MW2_S3`, `MW2_POWERVR`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_PIXEL_FORMAT` |  | 555 |  | 555 | 565 | 555 |  | 555 | 2 | The 16-bit pixels and the palette: RGB565 (Matrox); The 16-bit pixels and the palette: RGB555 (3Dfx); The 16-bit pixels and the palette: RGB555 (ATI); The 16-bit pixels and the palette: RGB555 (S3); The 16-bit pixels and the palette: RGB555 (PowerVR) |

### The Matrox edition (`MW2_MATROX`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_KEY_COLOR` |  |  |  |  | yes |  |  |  | 48 | Key colour fills and 0xfffe wipes for the renderer's overlay, ClipRectToPane |
| `MW2_VFXREND_CS_SEGMENT` |  |  |  |  | yes |  |  |  | 3 | VFXREND's polygon renderer reads its table through CS and leaves ES as it is (cmake/editions.cmake) |
| `MW2_LOD_PAR` |  |  |  |  | yes |  |  |  | 11 | LOD.PAR's LOD_SCALE scales the detail scale |
| `MW2_DEPTH_ENTRY_TAG` |  |  |  |  | yes |  |  |  | 2 | A depth entry is 0xc bytes, a dword ahead of the polygon |
| `MW2_RES_640X400` |  |  |  |  | yes |  |  |  | 1 | 640x400: SimMain sets it instead of SetGameResolution, and the art follows |
| `MW2_DETAIL_LEVEL_IN_LOADER` |  |  |  |  | yes |  |  |  | 2 | LoadClassLevel records the player's detail level itself |
| `MW2_SKIP_LOADED_LEVEL` |  |  |  |  | yes |  |  |  | 1 | LoadClassEntryShape returns early when the entry is loaded at the level |
| `MW2_SHOT_DAMAGE_AFTER_RECORD` |  |  |  |  | yes |  |  |  | 2 | A shot's heat and damage are applied after RecordAttack |
| `MW2_POWER_STATE_7_KEYS` |  |  |  |  | yes |  |  |  | 3 | Both power keys do nothing in power state 7, tested first (1.1: key 0x3e only) |
| `MW2_JOYSTICK_AXIS_SUMS` |  |  |  |  | yes |  |  |  | 2 | JoystickPoll sums the axes between polls |
| `MW2_FULLSCREEN_SAVES_MODE` |  |  |  |  | yes |  |  |  | 1 | ToggleFullScreen records the current mode as the fastest |
| `MW2_MOUSE_PEEK_RETURNS` |  |  |  |  | yes |  |  |  | 1 | HandleMessages returns when its mouse-skipping PeekMessage finds nothing |
| `MW2_STDCALL_TIMER_CALLBACK` |  |  |  |  | yes |  |  |  | 2 | SupAnimTimerCallback is __stdcall and takes the timer's argument |
| `MW2_DEBUG_LOG_TEXT` |  |  |  |  | yes |  |  |  | 3 | The debug log's messages end in newlines, with prints of its own |

### The ATI, S3 and Matrox editions (`MW2_MATROX`, `MW2_ATI`, `MW2_S3`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_TEXTURE_FILTER` |  | yes |  | yes | yes |  |  |  | 7 | The "Filter" registry setting, passed to the renderer |

### The ATI, Matrox and PowerVR editions (`MW2_MATROX`, `MW2_ATI`, `MW2_POWERVR`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_ART_RESOLUTION_512X384` |  | yes |  |  | yes | yes |  |  | 1 | ChooseArtResolution takes art resolution 1 for a 512x384 window |

### The ATI, S3 and 3Dfx editions (`MW2_3DFX`, `MW2_ATI`, `MW2_S3`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_QUEUEFACE_REUSED_LOCALS` |  | yes |  | yes |  |  |  | yes | 5 | QueueFace copies the face's normal into locals for its backface test, summing the z term first, keeps the z in depth, sums the average depth in dot, counts the depth loops down and or-s the vertices' clip codes into a signed byte |
| `MW2_PROJECT_Y_AFTER_X` |  | yes |  | yes |  |  |  | yes | 2 | ProjectVertex projects y after it tests x against the view's sides |
| `MW2_HORIZON_FLOAT_Y` |  | yes |  | yes |  |  |  | yes | 2 | HorizonYAtX keeps the horizon's y a float, converting it to an integer only for the return |
| `MW2_OUTLINE_CLEARS_FLAGS` |  | yes |  | yes |  |  |  | yes | 1 | The wireframe outline clears flag bits 12, 14 and 15 |
| `MW2_GROUND_HEIGHT_IN_TEST` |  | yes |  | yes |  |  |  | yes | 1 | *Artefact.* UpdateShot assigns the ground's height inside its comparison |

### The ATI, S3, PowerVR and 3Dfx editions (`MW2_3DFX`, `MW2_ATI`, `MW2_S3`, `MW2_POWERVR`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_SLOPE_ONCE` |  | yes |  | yes |  | yes |  | yes | 2 | InitSlopeTables computes each slope once, for its cosine and its sine |
| `MW2_MESSAGE_BACKGROUND_SHAPES` |  | yes |  | yes |  | yes |  | yes | 1 | Message boxes' backgrounds are shape 0x4f, not 0x4c |
| `MW2_NARROW_TORSO_PAN` |  | yes |  | yes |  | yes |  | yes | 1 | The torso pan's range is -90 to 90 |
| `MW2_NO_REVERSE_THROTTLE` |  | yes |  | yes |  | yes |  | yes | 1 | The throttle's range is 0 to 1024 |
| `MW2_HIT_FEEDBACK_FLAG_CHECK` |  | yes |  | yes |  | yes |  | yes | 1 | No hit feedback for the local player while its flag 2 is set |
| `MW2_NET_RETURN_ZERO` |  | yes |  | yes |  | yes |  | yes | 2 | FindSession and StopExternalIO return 0 where the others fall off the end |
| `MW2_NO_NET_DEBUG_PRINTS` |  | yes |  | yes |  | yes |  | yes | 3 | NetSend and NetReceive log nothing |
| `MW2_LAYOUT_GAUGE_FUNCTIONS` |  | yes |  | yes |  | yes |  | yes | 5 | The map layouts hold their gauge functions from the start, not indices in g_cockpitGauges |
| `MW2_INLINE_HELPERS` |  | yes |  | yes |  | yes |  | yes | 12 | *Artefact.* The edition's headers define FixedMul16, FixedDiv16, MulDiv64, HashName and DebugBreakpoint __inline (its /Ox expands them in every caller), and MemCopy and MemSet are C |
| `MW2_BLOCK_SCOPED_LOCALS` |  | yes |  | yes |  | yes |  | yes | 20 | *Artefact.* The edition's source declares some of UpdateMech's, LateUpdateMech's, DodgeShot's and the HUD brackets' locals in the blocks that use them, where VC++ 2.2 and 4.1 let them share the frame (1.1's and the Matrox edition's declare them at the top) |
| `MW2_DODGE_SIDE_FIRST` |  | yes |  | yes |  | yes |  | yes | 3 | *Artefact.* DodgeShot picks the side before it divides the heading |
| `MW2_BOUNCE_DOT_STATEMENTS` |  | yes |  | yes |  | yes |  | yes | 1 | *Artefact.* MoveMechWithCollisions sums the bounce's dot product in three statements and negates the speed before it quarters it (2.2 and 4.1 then keep each term's doubling and fold the sign) |
| `MW2_OBJECTIVE_TARGET_BYTES` |  | yes |  | yes |  | yes |  | yes | 1 | *Artefact.* The edition's source reads an AI target id's two bytes themselves (missionobjective.h's OBJECTIVE_TARGET_KIND), where 1.1's and the Matrox edition's shift the word |
| `MW2_PROJECT_NEAR_CLIP` |  | yes |  | yes |  | yes |  | yes | 1 | ProjectPolygon clips to the near plane itself, as 1.1's QueueFace does, not through the renderer |
| `MW2_SKY_GROUND_CLIPPED_HELPERS` |  | yes |  | yes |  | yes |  | yes | 4 | The sky and ground drawers clip the polygon to the pane themselves: their callers draw it even when ProjectPolygon leaves no points |
| `MW2_CLIP_TO_PANE_EDGES` |  | yes |  | yes |  | yes |  | yes | 4 | The polygon drawers clip to the pane's edges themselves (FUN_10067b90), and the outline is closed |
| `MW2_FRAME_TEXELS` |  | yes |  | yes |  | yes |  | yes | 2 | The textures are the animations' frames as they are, 16-bit texels (FUN_10051dfe), and their average colors are computed from them |
| `MW2_BRIGHTNESS_STORE_CHAINED` |  | yes |  | yes |  | yes |  | yes | 1 | *Artefact.* RestoreBrightness passes the brightness to PreviewBrightness through its store |
| `MW2_CONDITIONAL_EXPRESSIONS` |  | yes |  | yes |  | yes |  | yes | 10 | *Artefact.* The edition's source writes some flags and choices as expressions (`m_up = angle <= 180`, `x = c ? a : b`) where 1.1's and the Matrox edition's /Od code shows if and else (GetRectNeedleAt, HandleDebugKey, InitializeManeuvers' skill flags, ReelMotionTask's jump flag) |
| `MW2_HIDE_CACHE_INFO_NO_ARGS` |  | yes |  | yes |  | yes |  | yes | 1 | HideCacheInfo blanks its line without sprintf's two stray arguments |
| `MW2_ONE_RESOURCE_MESSAGE` |  | yes |  | yes |  | yes |  | yes | 2 | *Artefact.* LoadCachedResource formats both of its messages in one buffer |
| `MW2_TILT_NEGATIVE_FACTOR` |  | yes |  | yes |  | yes |  | yes | 1 | *Artefact.* RunAIWeapons scales the torso tilt by -0xf00, where the others negate the product |
| `MW2_MEK_LOG_FLOATS` |  | yes |  | yes |  | yes |  | yes | 2 | LoadMechConfig logs the armor and the jump thrust as %f |

### The S3 and 3Dfx editions (`MW2_3DFX`, `MW2_S3`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_SKY_SETTINGS_BEFORE_CLOCK` |  |  |  | yes |  |  |  | yes | 1 | SimMain reads the sky and ground settings (LoadSkyGroundSettings) just before FirstClock |
| `MW2_SUPANIM_DC_RELEASED` |  |  |  | yes |  |  |  | yes | 2 | SimMain releases the DC it tests for StartSupAnim |
| `MW2_PALETTE_DC_RELEASED` |  |  |  | yes |  |  |  | yes | 3 | SimMain releases the DC it tests the palette through |

### The 3Dfx edition (`MW2_3DFX`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_RENDER_SETTINGS_BEFORE_CLOCK` |  |  |  |  |  |  |  | yes | 1 | SimMain reads the render settings (FUN_1008fad0) just before FirstClock |
| `MW2_RES_800X600` |  |  |  |  |  |  |  | yes | 16 | The "Res800x600" setting: 800x600, the menus scaled, animation 0 tiled |
| `MW2_ART_RESOLUTION_NEAREST` |  |  |  |  |  |  |  | yes | 1 | ChooseArtResolution takes the nearest of the three sizes |
| `MW2_SKY_TILE_DEFAULTS` |  |  |  |  |  |  |  | yes | 2 | The sky and ground textures' tiling defaults: 4 x 0.8, and 20000 |
| `MW2_SKY_DEFAULTS_AT_END` |  |  |  |  |  |  |  | yes | 1 | LoadSkyGroundSettings sets the defaults when skygnd.par has no line for the mission too |

### The ATI and 3Dfx editions (`MW2_3DFX`, `MW2_ATI`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_SKY_GROUND_COLORS` |  | yes |  |  |  |  |  | yes | 1 | DrawSkyAndGround sets the sky and ground palette entries |
| `MW2_SKY_GROUND_ALL_CASES` |  | yes |  |  |  |  |  | yes | 1 | DrawSkyAndGround goes through the horizon's cases when it draws neither the sky nor the ground |
| `MW2_RECENTER_NO_RETEST` |  | yes |  |  |  |  |  | yes | 1 | *Artefact.* LateUpdateMech's torso recentring records the heading without testing isLocal again (it holds there) |
| `MW2_DEBUG_LOG_FORMATS` |  | yes |  |  |  |  |  | yes | 1 | The debug log prints the sky's and the ground's colours on one line |

### The ATI, Pre-Mercs, GBL and 3Dfx editions (`MW2_3DFX`, `MW2_ATI`, `MW2_GBL`, `MW2_PREMERCS`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_NO_STATE_7_POWER_TEST` |  | yes | yes |  |  |  | yes | yes | 1 | Key 0x3e asks for power down in power state 7 too, where 1.1 ignores it |

### The S3, PowerVR and 3Dfx editions (`MW2_3DFX`, `MW2_S3`, `MW2_POWERVR`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_DRAG_DIVIDED_ONCE` |  |  |  | yes |  | yes |  | yes | 1 | UpdateMech's air drag divides by the length once, then scales both axes by it |

### The ATI, Pre-Mercs, PowerVR and GBL editions (`MW2_ATI`, `MW2_POWERVR`, `MW2_GBL`, `MW2_PREMERCS`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_VFX3D_ALIGNED_FILL` |  | yes | yes |  |  | yes | yes |  | 1 | VFX3D's VFX_flat_polygon fills its spans with the destination aligned, as 1.1's modified VFX does, where its other routines are VFX as released (the assembly gets it from cmake/editions.cmake) |

### The ATI and S3 editions (`MW2_ATI`, `MW2_S3`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_TEXTURE_FILTER_ITEM` |  | yes |  | yes |  |  |  |  | 1 | The graphics page lists the texture filter, third |
| `MW2_FADE_STEPS_ONLY` |  | yes |  | yes |  |  |  |  | 1 | DdrawPaletteFade computes the fade's steps into g_paletteColors without setting DirectDraw's palette |
| `MW2_DDRAW_MODE_16BIT` |  | yes |  | yes |  |  |  |  | 1 | DdrawInit sets a 16-bit display mode |
| `MW2_DDRAW_FILL_SIZE` |  | yes |  | yes |  |  |  |  | 1 | DdrawInit clears the frame with DdrawFill(0, 0, width, height), not to width - 1 and height - 1 |

### The ATI edition (`MW2_ATI`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_JUMP_JET_THRUST_FIRST` |  | yes |  |  |  |  |  |  | 4 | UpdateMech's jump jets add their thrust to accelX before the drag (the later editions add the drag and the thrust first, as for accelZ) |
| `MW2_RES_512X384` |  | yes |  |  |  |  |  |  | 4 | VESA480.DLL is 512x384 when DirectDraw has the mode in 16 bits (FUN_100059b0), 640x480 otherwise |
| `MW2_DDRAW_FALLBACK_SURFACES_ONLY` |  | yes |  |  |  |  |  |  | 1 | DdrawCreateSurfaces makes the primary surface of the larger modes only the second way: with one back buffer when it flips, without video memory in refresh mode 3, and not at all in mode 2 |

### The ATI, S3 and PowerVR editions (`MW2_ATI`, `MW2_S3`, `MW2_POWERVR`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_DDRAW_PALETTE_UNSET` |  | yes |  | yes |  | yes |  |  | 2 | The DirectDraw back end's palette writes record the colors and compute the entries, but set no DirectDraw palette (the display is 16-bit) |

### The ATI, Pre-Mercs and GBL editions (`MW2_ATI`, `MW2_GBL`, `MW2_PREMERCS`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_DDRAW_CAPS_DX1` |  | yes | yes |  |  |  | yes |  | 2 | *Artefact.* DdrawInit's DDCAPS is DirectX 1's, 0xac bytes (the edition was built with DirectX 1's headers) |
| `MW2_DDRAW_NO_CAPS_FLAG` |  | yes | yes |  |  |  | yes |  | 3 | DdrawCreateSurfaces sets no DDSD_CAPS in its surface descriptions |

### The S3 edition (`MW2_S3`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_QUEUEFACE_TEXTURE_RANGE` |  |  |  | yes |  |  |  |  | 4 | QueueFace takes the range of the face's texture coordinates, and the polygon's flags and outcodes from it |
| `MW2_PROJECT_ALL_NEAR_EMPTY` |  |  |  | yes |  |  |  |  | 4 | ProjectPolygon leaves no points, at once, when every vertex is nearer than the near plane |
| `MW2_JOYSTICK_PROBE_SIZE_ONLY` |  |  |  | yes |  |  |  |  | 1 | JoystickOpenDevice's probe sets its JOYINFOEX's size without clearing it |
| `MW2_RENDER_SETTINGS_AFTER_MODES` |  |  |  | yes |  |  |  |  | 1 | SimMain reads the render settings (FUN_1008fad0) once the refresh mode is set |
| `MW2_SHAPES_PALETTE_POLYGON` |  |  |  | yes |  |  |  |  | 4 | DrawTriangle, DrawQuad and DrawPentagon draw through the palette polygon (FUN_1000d960), clipped to the pane's four edges, not g_renderSettings' drawer |
| `MW2_SKIP_LOADED_LEVEL_WITH_RELEASED` |  |  |  | yes |  |  |  |  | 1 | LoadClassEntryShape returns early when the entry is loaded at the level, in the test of a released entry (\|\|) |
| `MW2_COLLISION_DAMAGE_UNGROUPED` |  |  |  | yes |  |  |  |  | 1 | *Artefact.* ApplyCollisionDamage's damage times the masses' ratio isn't grouped (the edition divides the masses first) |
| `MW2_GRID_OBJECT_HIDDEN` |  |  |  | yes |  |  |  |  | 2 | The grid object is never shown: SetGridObject clears its set and shown flags once it has hidden it, and ShowGridObject only hides it |
| `MW2_NO_VFXREND` |  |  |  | yes |  |  |  |  | 1 | VFX3D and VFXREND, VFX's polygon fillers and renderer, aren't linked: the A3D renderer draws every polygon, and FirstRender has no code block to make writable |
| `MW2_ART_RESOLUTION_FIT` |  |  |  | yes |  |  |  |  | 1 | ChooseArtResolution takes the first size whose width reaches the window's, and records the last before it that fits inside the window |
| `MW2_BLIT_NO_WAIT` |  |  |  | yes |  |  |  |  | 1 | DdrawBlit's BltFast doesn't wait for the blitter (DDBLTFAST_NOCOLORKEY, not DDBLTFAST_WAIT) |
| `MW2_DARKER_AVERAGE_COLORS` |  |  |  | yes |  |  |  |  | 1 | An animation frame's average color is its texels' mean components times 8, less 12 (FUN_100519b0) |
| `MW2_SATELLITE_CLIP_ALL_EDGES` |  |  |  | yes |  |  |  |  | 2 | The satellite view clips its polygons and bands to all four pane edges (SatelliteDrawPolygon) |
| `MW2_OUTLINE_FLUSH` |  |  |  | yes |  |  |  |  | 2 | DrawQueuedPolygon flushes the renderer (FUN_10069591) between a wireframe polygon's fill and its outline |
| `MW2_QUEUED_POLYGON_EDGES_0XFF` |  |  |  | yes |  |  |  |  | 1 | DrawQueuedPolygon passes 0xff for a filled polygon's edges, not 0xf |
| `MW2_LIT_POLYGON_SHADE_BIT3` |  |  |  | yes |  |  |  |  | 1 | DrawScenePolygon's lit polygons (0x1000) set only bit 3 of their color's shade (\| 8), not all four (\| 0xf): the middle of the color's ramp, which the light then scales |
| `MW2_CHEAT_RECYCLE_HELD` |  |  |  | yes |  |  |  |  | 1 | A held weapon is released at once while bit 1 of the unlimited ammo setting is set |
| `MW2_TIME_OF_DAY_PHASES_1_TO_3` |  |  |  | yes |  |  |  |  | 1 | FirstEnvironment starts the day at a phase's time for phases 1 to 3 only (0 keeps the time it has) |
| `MW2_LIT_GROUND_FILL` |  |  |  | yes |  |  |  |  | 2 | SimMain fills the frame with the ground's colour lit by the renderer's light (g_unk0x10184700) once the go state has bit 2, textured ground or not |
| `MW2_NO_SIMMAIN_A3D_SHUTDOWN` |  |  |  | yes |  |  |  |  | 1 | SimMain doesn't call A3D_shutdown after its last Blit |
| `MW2_FLAG_CHEAT` |  |  |  | yes |  |  |  |  | 1 | The "flag" cheat ("Flag's Invincible!"): invulnerable, unlimited ammo, no heat, jets and their fuel |

### The S3 and PowerVR editions (`MW2_S3`, `MW2_POWERVR`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_RESET_TICKS_SAVE_ORDER` |  |  |  | yes |  | yes |  |  | 1 | ResetTicks restores ebx and ecx in the order it saved them (ticks.asm), where the others swap them |
| `MW2_CHEAT_RECYCLE` |  |  |  | yes |  | yes |  |  | 1 | A weapon recycles at once while bit 1 of the unlimited ammo setting is set (the "flag" cheat's 3) |
| `MW2_POWER_KEYS_ONE_CASE` |  |  |  | yes |  | yes |  |  | 1 | Key 0x3d is key 0x3e's case: neither asks for power down in power state 7 |

### The PowerVR edition (`MW2_POWERVR`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_NO_INITIAL_DRAW_MODE` |  |  |  |  |  | yes |  |  | 1 | The game starts with no draw mode (-1, as in 1.1), though without 1.1's fallback to the others |
| `MW2_TIME_OF_DAY_LOOKUP` |  |  |  |  |  | yes |  |  | 4 | The time of day's phase is a function of the time (FUN_10006f10), which also gives SimMain the palette of the current one (FUN_10006f30) |
| `MW2_FADE_LIGHT_COLOUR` |  |  |  |  |  | yes |  |  | 2 | A palette fade sets the renderer's light to the fade's tint over 32 (FUN_100607d0), white when it has none |
| `MW2_PRJ_PATH_FREED` |  |  |  |  |  | yes |  |  | 4 | FirstResource frees the MW2.PRJ path it duplicated, on the game's heap |
| `MW2_HUD_MARKER_LAG` |  |  |  |  |  | yes |  |  | 5 | The HUD's target marker keeps where it drew (statics): each frame draws the one kept from the frame before, then moves it, drawing the new one only when none was kept |
| `MW2_TARGET_RANGE_AT_BOTTOM` |  |  |  |  |  | yes |  |  | 5 | The target panel writes the target's range on its bottom line, not on the line after its name |
| `MW2_MATRIX_PITCH_ASIN_ONLY` |  |  |  |  |  | yes |  |  | 1 | GetMatrixAngles' test for a steep pitch is inverted (the arcsine unless the pitch's sine is both above 0.1 and below -0.1), so the pitch always comes from the arcsine |
| `MW2_CHRIS_FLAG_CHEATS` |  |  |  |  |  | yes |  |  | 1 | The "chris" and "flag" cheats |
| `MW2_TUNING_CHEATS` |  |  |  |  |  | yes |  |  | 2 | Cheat codes that tune the renderer: the fog's density, the ground's and the sky's tiles and the sky's scrolling speed |
| `MW2_COLLISION_TRANSFORMS_MODELS` |  |  |  |  |  | yes |  |  | 4 | The face tests of the shape collision (TestPointUnderFloor, TestPointAboveFaces, TestRayFaces) transform the shape's model themselves when it is older than the shape's transform: the renderer (SGL) doesn't transform the models the depth sort did |
| `MW2_NO_SCREENSHOT_KEY` |  |  |  |  |  | yes |  |  | 1 | Key 0x5b takes no screenshot |
| `MW2_NO_WIREFRAME_KEY` |  |  |  |  |  | yes |  |  | 1 | Key 0xa7 goes to RunGameKey's default case, where the others switch the wireframe view on and off with it |
| `MW2_RENDERER_TARGET_VIEW` |  |  |  |  |  | yes |  |  | 2 | The target panel renders no view of its target (nor loads its detail level): it only wipes the pane, unless the cockpit's power state is 2, and outlines it |
| `MW2_UNROTATED_EFFECTS` |  |  |  |  |  | yes |  |  | 2 | SpawnEffectEx leaves an effect's object unrotated (it takes the rotation all the same), and moves one that hit a thing toward the eye by its shape's radius |
| `MW2_LOOKASIDE_RED_SHIFT_10` |  |  |  |  |  | yes |  |  | 1 | ApplyPaletteResource writes the lookaside table's pixels with red shifted by 10, as for RGB565 |
| `MW2_A3D_NO_PALETTE` |  |  |  |  |  | yes |  |  | 1 | The renderer takes no palette: ApplyPaletteResource doesn't pass it to A3D_SetPalette |
| `MW2_HASH_NAME_ROL32` |  |  |  |  |  | yes |  |  | 1 | HashName rotates the whole 32-bit hash after each character, not its low word |
| `MW2_NO_MAP_DAMAGE_ANIM` |  |  |  |  |  | yes |  |  | 1 | The damaged map view flickers its gauge without drawing the view's damage animation (DrawDamagedMapView) |
| `MW2_MOUSE_PEEK_BREAKS` |  |  |  |  |  | yes |  |  | 1 | HandleMessages stops skipping mouse messages when its PeekMessage finds nothing, and handles the last one it had |

### The Pre-Mercs and GBL editions (`MW2_GBL`, `MW2_PREMERCS`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_EXTERNAL_HEAT` |  |  | yes |  |  |  | yes |  | 3 | Mechs take heat from outside (Mech::m_externalHeat, which the radius damage adds to and CalculateHeat adds to the frame's heat) |
| `MW2_MECH_SECOND_FLAGS` |  |  | yes |  |  |  | yes |  | 2 | Mechs have a second word of flags (Mech::m_unk0x112, whose bit 0 UpdateShot sets for a NARC hit) |
| `MW2_FOURTEEN_WEAPON_SLOTS` |  |  | yes |  |  |  | yes |  | 7 | Mechs have 14 weapon slots, not 10 (WEAPON_SLOT_COUNT) |
| `MW2_PILOT_TABLE` |  |  | yes |  |  |  | yes |  | 12 | An AI player whose gunnery is above 6 takes its skills from a table of pilots, with two more values and a flag the AI's fire decisions test (Player::m_unk0x160, m_unk0x1b0, m_unk0x1b4) |
| `MW2_STATIC_MANEUVER_TABLES` |  |  | yes |  |  |  | yes |  | 7 | The maneuver tables are data, five of them the pilot table's, and each player has its table and a range (Player::m_maneuverTable, m_unk0x1b0: InitializeManeuvers) |
| `MW2_AI_FLAGS_INT` |  |  | yes |  |  |  | yes |  | 4 | *Representation.* A player's AI flags are an int, not a short (PlayerAi::m_flags) |
| `MW2_CHAT_KEY` |  |  | yes |  |  |  | yes |  | 2 | The CHAT_MODE game key, action 66: the later actions are one higher (GAME_KEY_ACTION) |
| `MW2_AMS_KEY` |  |  | yes |  |  |  | yes |  | 3 | The TOGGLE_AMS game key, action 169 |
| `MW2_GHOST_BEAR_TITLES` |  |  | yes |  |  |  | yes |  | 4 | The message boxes and the refresh mode errors name Ghost Bear's Legacy |
| `MW2_GBL_CD_DRIVER` |  |  | yes |  |  |  | yes |  | 1 | The game's CD is the drive with GBL_HERC.DRV |
| `MW2_ENGAGEMENT_FLAGS_INT` |  |  | yes |  |  |  | yes |  | 2 | *Representation.* GetEngagementAIFlags returns an int, not a short |
| `MW2_TORSO_SNAP_ONE_DEGREE` |  |  | yes |  |  |  | yes |  | 1 | A recentering torso snaps to the legs within 1 degree, not 4 (TORSO_SNAP_RANGE) |
| `MW2_WEAPON_FUNCTIONS` |  |  | yes |  |  |  | yes |  | 59 | Weapons work through their definitions: each has a launch function (SpawnShot for all but the AMS), a function called each frame, flags (the launch sounds, an animation) and an animation, and there are 35 of them; shots and effects carry the weapon's flags and a word more, and there are 255 of each and 36 effect types, with four more words each (WeaponDef, Shot, Effect, EffectInfo) |
| `MW2_POSITIONAL_SOUND_FLAGS` |  |  | yes |  |  |  | yes |  | 2 | PlayPositionalSound takes the sample's flags (PlayPositionalSoundEx; the old entry point passes 0x32), and plays nothing beyond a cutoff distance, g_unk0x100ba608 hundred units, when that is set |
| `MW2_THING_KILLS_FORGET_SHOOTER` |  |  | yes |  |  |  | yes |  | 2 | A game thing's destruction counts for the local team whoever's shot destroyed it, and forgets the shooter (KillGameThing) |
| `MW2_ABS_CALLS` |  |  | yes |  |  |  | yes |  | 7 | *Artefact.* Weapons.c, ai.c, maneuvers.c and gridobject.c call the C runtime's abs, which GBL's other units expand (MW2_GBL's pragma above), and bandpoly.c, AssignStarTarget and ChooseArtResolution its labs |
| `MW2_BEARING_LABS` |  |  | yes |  |  |  | yes |  | 1 | *Artefact.* GetBearingAndRange compares with the C runtime's labs |
| `MW2_BUILDINGS_FALL_AT_ONCE` |  |  | yes |  |  |  | yes |  | 3 | A shot destroys a game thing whose shape is of kind 0xb0 at once, and every destroyed thing leaves effect 0x20b (DamageGameThing) |
| `MW2_DESTROY_BLOWS_FIRST_BIN` |  |  | yes |  |  |  | yes |  | 3 | A destroyed mech blows the critical slot of its first ammunition bin, a bin blows whatever its weapon's ammunition, and its pilot ejects when auto-eject is on or the mech isn't the local player's (DestroyMech, DestroyCriticalSlot) |
| `MW2_SELF_DESTRUCT_MESSAGE` |  |  | yes |  |  |  | yes |  | 1 | EjectPlayer tells the local player with cockpit message 4 when the mech goes without ejecting |
| `MW2_KILLS_BY_LAST_SHOOTER` |  |  | yes |  |  |  | yes |  | 5 | A mech's kill counts for the player whose shot hit it last, the local player's outcome is 4 unless they ejected, and in a network game only the local mech's death is played out (KillMech, which DestroySection and DestroyCriticalSlot call whatever the game) |
| `MW2_MORE_CRITICAL_HITS` |  |  | yes |  |  |  | yes |  | 2 | A critical hit on a flamer and on the AMS has a message of its own (DestroyCriticalSlot) |
| `MW2_AI_STOPS_GROUP_FIRE` |  |  | yes |  |  |  | yes |  | 2 | The AI's attack and target states stop the group fire as well as the fire (AiStateAttack, AiStateMove) |
| `MW2_STRUCT_COPY_MEMCPY` |  |  | yes |  |  |  | yes |  | 43 | *Artefact.* GBL's source copies some structures with memcpy, which its build calls, where the optimized editions show a structure assignment: nav points, rays, players, matrices, weapon slots, render settings, sample headers and session descriptions (RemoveNavPoint, CopyRay, LogPlayerStatusLines, the object matrices, LayoutWeaponPanels, the mech view panel, StartSample, FindSession) |
| `MW2_LOCAL_INSPECTION` |  |  | yes |  |  |  | yes |  | 1 | Only the local player inspects its target, which counts as inspected once anyone has, and the inspect request stays set (UpdateTarget) |
| `MW2_CHEAT_TABLE` |  |  | yes |  |  |  | yes |  | 4 | The cheat codes are a table with the messages each shows, all stored XORed with 0xa, the messages decoded at the first key typed; GBL has codes of its own, and six of them do nothing (g_cheatCodes, HandleCheatInput) |
| `MW2_INI_COMMENTS` |  |  | yes |  |  |  | yes |  | 4 | GetIniValue cuts a value at a ';' and trims it, unless its second argument asks for the raw value (which error.c's call doesn't pass) |
| `MW2_NO_MOUSE_ERRORS` |  |  | yes |  |  |  | yes |  | 1 | ReportInputDeviceError reports nothing about the mouse |
| `MW2_DEPTH_QUEUE_INDEX` |  |  | yes |  |  |  | yes |  | 2 | *Artefact.* DrawDepthQueue takes the next entry's index into a local before it copies the entry |
| `MW2_LENGTH_SUM_FROM_Z` |  |  | yes |  |  |  | yes |  | 3 | *Artefact.* The vector lengths sum the squares from z down (FloatVectorLength, ScaleVectorToLength, BuildMatrixFromDirection) |
| `MW2_SKIP_SHAPE_TYPE_20` |  |  | yes |  |  |  | yes |  | 1 | QueueObjTree draws no shape of type 0x20 |
| `MW2_PT_MENU_KEYS` |  |  | yes |  |  |  | yes |  | 3 | The ALL_PT_MENU, PT_2_MENU and PT_3_MENU game keys (actions 53 to 55) do what the first three starmate menu keys do (RunGameKey) |
| `MW2_SCREENSHOT_ACQUIRE_IF` |  |  | yes |  |  |  | yes |  | 1 | *Artefact.* RunGameKey's screenshot acquires the frame buffer in an if with an empty else, where 1.1 has a conditional expression |
| `MW2_LOST_MECH_FAILS_STAR` |  |  | yes |  |  |  | yes |  | 2 | The local star's mission fails once the local mech is lost (UpdateObjectives, g_localMechLost) |
| `MW2_NAME_STRCMP` |  |  | yes |  |  |  | yes |  | 2 | *Artefact.* FindIpxProviderCallback and FillJoystickDeviceInfo compare names with strcmp, not memcmp |
| `MW2_MEM_HELPERS_CALL_CRT` |  |  | yes |  |  |  | yes |  | 4 | MemCopy and MemSet return the C runtime's memcpy and memset, where 1.1's are assembly |
| `MW2_SCROUNGE_STATIC_OBJECTS` |  |  | yes |  |  |  | yes |  | 4 | A scrounge record whose id names no class takes a static object's shape, loading the object (BwdExecuteStream) |
| `MW2_REMOTE_PLAYER_AI_MODE` |  |  | yes |  |  |  | yes |  | 1 | A player the stream creates for another machine or the AI gets AI mode 1 in a network game and 2 otherwise (BwdExecuteStream) |
| `MW2_DEEP_COLD_COOLING` |  |  | yes |  |  |  | yes |  | 1 | A mech's heat sinks cool five times as well at -100 degrees or colder, on top of the cold bonus (LoadMechConfig) |
| `MW2_PLAYERS_BEFORE_LAUNCH` |  |  | yes |  |  |  | yes |  | 1 | SimMain updates the players every frame, before a network launch too |
| `MW2_SUPANIM_KEEPS_PALETTE` |  |  | yes |  |  |  | yes |  | 1 | StartSupAnim leaves the palette's first 16 entries as the backdrop set them |
| `MW2_SOUND_INFO_FILL_MINUS_ONE` |  |  | yes |  |  |  | yes |  | 1 | *Artefact.* InitSoundInfo fills the table with -1, not 0xff (which GBL's memset call shows) |
| `MW2_DDRAW_STOP_KEEPS_MODE` |  |  | yes |  |  |  | yes |  | 1 | DdrawStop releases DirectDraw without returning the window to normal cooperation or restoring the display mode |
| `MW2_REMOTE_MECHS_STAY` |  |  | yes |  |  |  | yes |  | 1 | In a network game, UpdateMech leaves a player with flag 1 (another machine's) where it is unless it is the local one, and holds a truck at its object's height |

### The Pre-Mercs edition (`MW2_PREMERCS`)

| Feature | 1.1 | ATI | Pre-Mercs | S3 | Matrox | PowerVR | GBL | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|--:|---|
| `MW2_SHAPE_GRID` |  |  | yes |  |  |  |  |  | 26 | The scene's shapes are filed in a grid of 12 by 12 cells, 25000 units each, around the origin, each shape keeping its cell (Shape::m_gridX, m_gridZ; shapelists.c) |
| `MW2_PLAYER_UNK_MEMBERS` |  |  | yes |  |  |  |  |  | 4 | Players have 0x46 bytes more than GBL's, in three places nothing decompiled reads yet (Player::m_unk0xe8, m_unk0x184, m_unk0x1a0) |
| `MW2_MECH_UNK_MEMBERS` |  |  | yes |  |  |  |  |  | 4 | Mechs, their weapon slots and their ammunition bins have 0x8f, 0xc and 8 bytes more than GBL's, at their ends (Mech::m_unk0x114, WeaponSlot::m_unk0x68, AmmoBin::m_unk0x14) |

## Where each feature is tested

- `MW2_FLOAT_SCALARS`: `MW2/include/ammobin.h`: `AmmoBin`; `MW2/include/bandpoly.h`: file scope (2); `MW2/include/classentry.h`: file scope; `MW2/include/clock.h`: file scope (2); `MW2/include/cockpit.h`: file scope; `MW2/include/eyepoint.h`: `Eyepoint`; `MW2/include/filevector3.h`: file scope; `MW2/include/filexform.h`: file scope; `MW2/include/fixedfloat.h`: file scope; `MW2/include/fixedtrig.h`: file scope (2); `MW2/include/inradius.h`: the definition of `IsWithinRadius`; `MW2/include/integrate.h`: file scope; `MW2/include/maneuvers.h`: file scope; `MW2/include/mapview.h`: file scope; `MW2/include/projectedvertex.h`: file scope; `MW2/include/queuedpolygon.h`: `QueuedPolygon`; `MW2/include/ramp.h`: file scope (3); `MW2/include/screenscale.h`: file scope; `MW2/include/shape.h`: file scope; `MW2/include/targeting.h`: file scope; `MW2/include/vertex.h`: `Vertex` (2); `MW2/include/view.h`: file scope; `MW2/include/weaponslot.h`: `WeaponBin`, file scope (2); `MW2/src/ai.c`: `AddClamped`, `AiStateMove` (2), `FindNearestTarget`, `GetApproachThrottle`, `SteerToTarget`, `SweepTorso` (2); `MW2/src/aiweapons.c`: `DecideAIFire`; `MW2/src/bandpoly.c`: file scope; `MW2/src/bargauges.c`: `DrawHeatBar`, `DrawHeatRateBar`, `DrawThrottleGauge`; `MW2/src/brightnessmenu.c`: `GetBrightnessFraction`; `MW2/src/classtable.c`: `ChoosePlayerDetailLevels`; `MW2/src/clockmath.c`: the definition of `Hypot2D`, the definitions of `InitSinAtanTables`, `InitSlopeTables`; `MW2/src/cockpit.c`: `DrawMapContents` (3), `DrawMapFieldOfView` (3), `DrawMapNavPoints` (3), `DrawMapTarget` (3), `DrawMapUnits` (3), `DrawMapViewText` (5), `GetMapHeightShade`, `ZoomMapView` (3); `MW2/src/cockpitpanels.c`: `UpdateCockpit`; `MW2/src/collision.c`: `IntersectRayFace`, `IsBelowFacePlane`, `IsPointInFace`, `IsPointInFaceXY` (2), `IsPointInFaceXZ` (2), `IsPointInFaceYZ` (2), `TestSegmentCollision`; `MW2/src/config.c`: `LoadReels` (2); `MW2/src/damagepanel.c`: `DrawArmorBars` (4), `DrawDamageOutline` (2), file scope; `MW2/src/debris.c`: `BlowOffChunk`, `DamageChunksInRadius`, `PushDebrisPiece`, `ThrowDebrisPiece`, `UpdateDebrisPiece` (4); `MW2/src/depthsort.c`: `QueueShapeLod`; `MW2/src/door.c`: `LateUpdateDoor` (2), `UpdateDoor`; `MW2/src/environment.c`: `FirstEnvironment`, `QuadrupleMechCooling`, `QuarterMechCooling`; `MW2/src/eyepoint.c`: `GetCameraFloor`, `RestoreView`, `SaveView`, `UpdateEyepoint` (2), `UpdateFreeEyeView`, `UpdateTrackView`; `MW2/src/fadepal.c`: `EmitWreckSmoke`, `FlashZappedPaletteLevel`, `PlayPlayerHitFeedback`; `MW2/src/fixedsqrt.c`: the definition of `NormalizeVectorGuarded`; `MW2/src/gamekeys.c`: `RunGameKey`; `MW2/src/geocache.c`: `ApplyBlockXform`, `BeginBlock`, `LoadQuadtreeBoxes`, `ToggleBlockBoxes`; `MW2/src/gpanim.c`: `UpdateMotion`; `MW2/src/horizon.c`: `HorizonXAtY`, `HorizonYAtX`, `IsAboveHorizon`; `MW2/src/hud.c`: `DrawAltimeter` (6), `DrawCompass`, `DrawCompassMarkers`, `DrawObjectBrackets`, `DrawPlayerBrackets`, `InitHudGauges` (2); `MW2/src/inputmap.c`: `UpdateAxisFromKeys`, `UpdateInputs` (2); `MW2/src/inradius.c`: the definition of `IsWithinRadius`; `MW2/src/integrate.c`: the definition of `IntegrateMidpoint`; `MW2/src/maneuvers.c`: `AvoidObstacles`, `BuildProbeRay`, `CanJump`, `ChooseFlankPlace`, `DodgeShot` (2), `GetAvoidSide`, `GetHeadingTo`, `GetOffsetPoint` (4), `IsSharpTurn`, `IsStandableShape`, `JumpToTurn`, `ManeuverAchick` (2), `ManeuverAdfa` (2), `ManeuverBehind`, `ManeuverStupid` (2), `PlaceOffsetNav`, `RunManeuver`, `StartManeuver`, file scope; `MW2/src/mapicon.c`: `DrawMapIcon` (4); `MW2/src/mapview.c`: `BeginMapView` (2), `CullMapViewShape` (3), `ProjectMapPoint` (2), `ProjectMapViewVertex`, the definition of `ProjectMapCoordinate`; `MW2/src/mechclass.c`: `FirstMech`, `LateUpdateMech` (7), the definition of `UpdateMech`; `MW2/src/mechcollision.c`: `ApplyCollisionDamage`, `CollideWithBuildings` (2), `CollideWithMechs` (6), `DamageMechHittingShape`, `DamageMechsInCollision`, `MoveMechWithCollisions` (3); `MW2/src/mechdamage.c`: `ApplyDamageToMech` (3), `CalculateHeat` (2), `DestroyCriticalSlot`, `PunchInAutoHeading`, `RunAutopilot` (2); `MW2/src/mekfile.c`: `LoadMechConfig` (5); `MW2/src/network.c`: `ReceiveCollisionMsg`, `ReceiveStateMsg`, file scope (2); `MW2/src/normalizerotation.c`: `NormalizeRotation`, `ScaleVectorToLength`; `MW2/src/objectanim.c`: `OrbitTask` (4), `PathTask` (2), `SpinTask` (2); `MW2/src/objective.c`: `DoFirstObjtv`; `MW2/src/palette.c`: `StartPaletteFade`; `MW2/src/quadtree.c`: `TestQuadtreeRay`; `MW2/src/ramp.c`: `SetWrappedRampTarget`, `UpdateWrappedRamp` (3), file scope, the definitions of `StartFloatRamp`, `UpdateFloatRamp`; `MW2/src/ray.c`: `AdvanceRayStart`, `BuildRayFixed`, `BuildRayFloat`, `BuildRayFromDirection`, `ClipRaySlab`, `ClipRayToGround`, `SetRayLength`; `MW2/src/recordstacks.c`: `InitializeDrawBuffer`; `MW2/src/resource.c`: `CreateObjectNode`, `PathRecord`, file scope; `MW2/src/screenscale.c`: `DrawGaugeEllipse`, `GetEllipseEdgeAtAngle` (2), `GetEllipseNeedleAt` (2), `GetEllipseNeedleToward`, `GetLineSlope` (2), `GetRectEdgeAtSlope`, `GetRectNeedleAt` (2), `IsInsideGaugeEllipse` (3), `ScaleRectToLowRes`, file scope; `MW2/src/setres.c`: `SetPixelAspect`; `MW2/src/shape.c`: `AddShapeVertex`, `ComputeFaceNormal` (2); `MW2/src/shapebounds.c`: `ComputeShapeBounds` (2); `MW2/src/shapecollision.c`: `TestPointAboveFaces`; `MW2/src/shapegeom.c`: `ApproximateShapeDistance`, `RayShapeDistance`, `TransformModel`, `TransformShapeCenter`, the definition of `ComputeTriangleNormal`, the definition of `DivDifference17`, the definition of `SolvePlaneY`; `MW2/src/shots.c`: `DamageMechsInRadius`, `DamageThingsInRadius`, `GuideMissileToTarget` (2), `HeatMechsNearFires`, `SwayShot`, `UpdateShot` (3); `MW2/src/soundfx.c`: `CalculateSamplePan`, `PlayPositionalSound` (3), `UpdateAmbientSound` (2); `MW2/src/statuspanels.c`: `DrawSpeedPanel`; `MW2/src/targeting.c`: `UpdateTarget`, file scope; `MW2/src/targetpanel.c`: `DrawTargetPanel` (2), `DrawTargetPanelText`; `MW2/src/transform.c`: `BuildMatrixEx`, `GetMatrixAngles`, `MultiplyRotations`, `OrthogonalizeMatrixColumn`, `RotatePoint`, `SetIdentityMatrix`, `TransformPoint`; `MW2/src/view.c`: `CullSceneShape` (6), `CullShapeToFrustum` (5), `ProjectWorldPoint` (2), `SelectEyepoint`, `SetFarPlane`, `SetNearPlane`, `UpdateProjection`, file scope; `MW2/src/weapons.c`: `GetEyeAimDirection` (2), `SpawnLaunchFx` (3), `SpawnShot`, `UpdateAimDistance`, `UpdateWeaponLock`; `MW2/src/world.c`: `BwdExecuteStream` (6); `MW2/src/wtbshapes.c`: `LoadShapeRecord` (3), `SetShapeScale`
- `MW2_A3D`: `MW2/include/animation.h`: file scope (3); `MW2/include/animframe.h`: `AnimFrame`; `MW2/include/bandpoly.h`: file scope (2); `MW2/include/cockpit.h`: file scope; `MW2/include/objectanim.h`: file scope; `MW2/include/palette.h`: file scope; `MW2/include/polydraw.h`: file scope (4); `MW2/include/queuedpolygon.h`: `QueuedPolygon`; `MW2/include/rendersettings.h`: `RenderSettings` (2); `MW2/include/shape.h`: file scope; `MW2/include/vertex.h`: `Vertex`; `MW2/src/animation.c`: `DrawAnimatedPolygon`, file scope (4), the definition of `RotateTexturePoint`, the definitions of `CalcAverageBitmapColors`, `FUN_1001e300`, `FUN_100519b0`, `FUN_10051ba1`, `FUN_10051c07`, `FUN_10051dfe`, `FUN_10051ea2`; `MW2/src/bandpoly.c`: `DrawBand` (2), the definition of `DrawBandPolygon`, the definition of `DrawBandWithFlash`; `MW2/src/cockpit.c`: `DrawMapView`, `SatelliteDrawPolygon` (2), file scope, the definition of `FUN_100751a9`; `MW2/src/depthsort.c`: file scope (2), the definition of `DrawQueuedPolygon`, the definitions of `ClipEdgeToColumn`, `ClipEdgeToRow`, `ClipPolygonToScreen`, `DrawQueuedPolygon`, `GetEdgeCrossingDepth`; `MW2/src/faceshade.c`: `GetFaceColor`; `MW2/src/fadepal.c`: `RenderViewToPane` (2); `MW2/src/objectanim.c`: file scope; `MW2/src/palette.c`: `ApplyPaletteResource`, `StartPaletteFade`; `MW2/src/polydraw.c`: `DrawPentagon`, `DrawQuad`, `DrawScenePolygon` (2), `DrawTriangle`, `g_renderSettings`, file scope (3), the definition of `DrawSkyAndGround`, the definition of `FUN_10067b90`, the definitions of `DrawLineTo`, `DrawPolygonOrLine`, the definitions of `FUN_10068856`, `FUN_100688fe`, `FUN_100689f5`, `FUN_10068b57`, `FUN_10068e33`, `FUN_10069172`, `FUN_100692db`, `LoadSkyGroundSettings`, `ReadTextLine`; `MW2/src/recordstacks.c`: `AllocProjectedVertex`, `AllocQueuedPolygon`, `InitializeDrawBuffer` (2); `MW2/src/render.c`: file scope (2); `MW2/src/shape.c`: `AddShapeVertex` (2); `MW2/src/simmain.c`: file scope; `MW2/src/targetpanel.c`: `DrawTargetPanel`
- `MW2_VERTEX_SHADING`: `MW2/include/projectedvertex.h`: `ProjectedVertex`; `MW2/include/wtbface.h`: file scope; `MW2/include/wtbvertex.h`: file scope; `MW2/src/shapegeom.c`: `TransformModel`; `MW2/src/wtbshapes.c`: `LoadShapeRecord` (8), file scope
- `MW2_VFX16`: `MW2/include/accel/vfx16.h`: file scope; `MW2/include/menu.h`: file scope; `MW2/include/palette.h`: file scope (2); `MW2/src/anim2d.c`: file scope; `MW2/src/bargauges.c`: file scope; `MW2/src/cockpit.c`: file scope; `MW2/src/damagepanel.c`: `DrawDamageOutline`, `InitDamagePanel`, file scope; `MW2/src/depthsort.c`: file scope; `MW2/src/hud.c`: file scope; `MW2/src/mapicon.c`: file scope; `MW2/src/mechviewpanel.c`: file scope; `MW2/src/menu.c`: file scope (2); `MW2/src/menucontrols.c`: file scope; `MW2/src/overlay.c`: file scope; `MW2/src/palette.c`: `ApplyPaletteResource` (2), file scope; `MW2/src/palidentity.c`: `ResetTextColors`; `MW2/src/pausebanner.c`: file scope; `MW2/src/render.c`: file scope; `MW2/src/screenscale.c`: `DrawGaugeEllipse`, file scope; `MW2/src/simmain.c`: file scope; `MW2/src/statuspanels.c`: file scope; `MW2/src/targetpanel.c`: `DrawTargetPanel`, `DrawTargetPanelText`, file scope; `MW2/src/timedoverlays.c`: file scope; `MW2/src/weaponpanel.c`: `DrawWeaponPanel` (2), file scope
- `MW2_VFX_STOCK`: `3rdparty/vfx/VFX.INC`: `RMOVSB32`, `RSTOSB32`, `RXLAT32`; `3rdparty/vfx/VFX3D.ASM`: file scope; `3rdparty/vfx/VFXA.ASM`: `VFX_character_draw`, `VFX_pane_wipe` (2)
- `MW2_VFX_LOOKASIDE_WRITE`: `3rdparty/vfx/VFXA.ASM`: file scope (2)
- `MW2_LARGER_POLYGON_BUFFERS`: `MW2/src/render.c`: `FirstRender`
- `MW2_NO_DISSOLVE`: `MW2/src/palette.c`: `StartPalettes`
- `MW2_DISPLAYDIB_DISABLED`: `MW2/src/dispdib.c`: `DispDibBegin`
- `MW2_REFRESH_MODE_RETRY`: `MW2/src/refreshmode.c`: `InitRefreshMode` (2)
- `MW2_RAW_SCREENSHOTS`: `MW2/src/config.c`: `SaveScreenshot` (2); `MW2/src/gamekeys.c`: `RunGameKey`
- `MW2_GRAPHICS_MENU`: `MW2/src/mainmenu.c`: `g_mainMenuPage`, file scope (5)
- `MW2_REGISTRY_SETTINGS`: `MW2/src/simmain.c`: file scope, the definitions of `FUN_10007db0`, `FUN_1008fad0`
- `MW2_DEFAULT_DRAW_MODE`: `MW2/src/render.c`: file scope
- `MW2_HUD_ART`: `MW2/include/setres.h`: file scope (2); `MW2/src/hud.c`: `DrawAltimeter`, file scope (2); `MW2/src/mechviewpanel.c`: file scope; `MW2/src/render.c`: `ShowBanner` (2); `MW2/src/setres.c`: `ChooseArtResolution`, file scope
- `MW2_HUD_SHAPE_IDS`: `MW2/src/pausebanner.c`: `DrawPausedBanner`; `MW2/src/targetpanel.c`: `DrawTargetPanel` (5)
- `MW2_SLIDER_SHAPES`: `MW2/src/mainmenu.c`: `g_sliderShapes`
- `MW2_MENU_BACKGROUND_0X118`: `MW2/src/mainmenu.c`: `g_mainMenu`
- `MW2_PRJ_CEL_IDS`: `MW2/src/animation.c`: `g_preloadCels`
- `MW2_BRIGHT_MAP_COLORS`: `MW2/src/cockpitlayouts.c`: file scope (3)
- `MW2_COCKPIT_POINT_ONLY`: `MW2/include/cockpit.h`: file scope; `MW2/src/cockpit.c`: file scope; `MW2/src/setres.c`: `SetRes`
- `MW2_UNRESOLVED_GAUGE_INDICES`: `MW2/src/cockpit.c`: `LoadCockpitLayout` (2)
- `MW2_DEBUG_OVERLAY_COLOR`: `MW2/src/overlay.c`: file scope
- `MW2_NO_EYE_POSITION_OVERLAY`: `MW2/src/overlay.c`: `DrawDebugOverlays`
- `MW2_NO_BOUNDING_SPHERES`: `MW2/src/render.c`: `DrawScene`, the definition of `DrawBoundingSpheres`
- `MW2_INFRARED_OFF_FADE`: `MW2/src/environment.c`: `SetInfrared`
- `MW2_FADE_BACK_DURATION`: `MW2/src/palette.c`: `StartPaletteFade`
- `MW2_TIME_OF_DAY_FADE_ONCE`: `MW2/src/environment.c`: `FadeToTimeOfDayPhase`
- `MW2_FACE_KIND_BIT15`: `MW2/src/cockpit.c`: file scope; `MW2/src/wtbshapes.c`: `MapFaceId` (2)
- `MW2_WTB_NEGATED_NAMES`: `MW2/src/wtbshapes.c`: `LoadShapeRecord`
- `MW2_MODEL_MIN_ONE_FACE`: `MW2/src/addmodel.c`: `AddModel`
- `MW2_GRID_CELL_XZ`: `MW2/src/gridobject.c`: `SetGridObject`, `UpdateGridObject` (3), file scope
- `MW2_NO_PROJECTION_OFFSET`: `MW2/src/palette.c`: `GetViewCenter`, `SelectPane`; `MW2/src/view.c`: `ResetEyepointView`
- `MW2_FRUSTUM_HEIGHT_SIGN_FROM_SIDE`: `MW2/src/view.c`: `CullShapeToFrustum`
- `MW2_NO_RAY_BOX_ENTRY_BEHIND`: `MW2/src/shapecollision.c`: `TestRayBox`
- `MW2_AIMED_SHAPE_RESET`: `MW2/src/shape.c`: `DestroyObjShape`, file scope
- `MW2_BUILDING_BOUNCE`: `MW2/src/mechcollision.c`: `CollideWithBuildings`
- `MW2_COLLISION_DAMAGE_X2`: `MW2/src/mechcollision.c`: `ApplyCollisionDamage`
- `MW2_RADIUS_HEAD_DAMAGE_HALF`: `MW2/src/shots.c`: `DamageMechsInRadius`
- `MW2_NO_ENGINE_NOTE`: `MW2/src/midi.c`: `StartEngineNote`
- `MW2_LEADER_NEAR_TARGET`: `MW2/src/objective.c`: `IsTeamNearTarget` (3)
- `MW2_LOCAL_STAR_FROM_ZERO`: `MW2/src/resource.c`: `LoadStarTable`; `MW2/src/team.c`: file scope
- `MW2_NETWORK_STAR_PLAYERS`: `MW2/src/objective.c`: `UpdateObjective` (2)
- `MW2_RESTART_KEEPS_LISTED`: `MW2/src/objective.c`: `RestartStarMission`
- `MW2_JOYINFOEX_UNCLEARED`: `MW2/src/joystick.c`: `JoystickPoll`
- `MW2_AXIS_RATE_POINTER_TEST`: `MW2/src/inputmap.c`: `UpdateInputs`
- `MW2_NO_FULLSCREEN_KEY`: `MW2/src/inputmap.c`: `UpdateInputs`
- `MW2_LEMMY_CHEAT`: `MW2/src/gamekeys.c`: `HandleCheatInput`
- `MW2_NO_DORCS_CHEAT`: `MW2/src/gamekeys.c`: `HandleCheatInput`
- `MW2_EXTRA_DEBUG_PRINTS`: `MW2/src/logwindow.c`: file scope, the definition of `MonoPrintHex`; `MW2/src/mechdamage.c`: `DestroyMech`, file scope; `MW2/src/recordstacks.c`: `InitializeDrawBuffer`
- `MW2_JOYSTICK_OPEN_POSEX`: `MW2/src/joystick.c`: `JoystickOpenDevice` (2)
- `MW2_PIXEL_FORMAT`: `MW2/src/palette.c`: `ApplyPaletteResource`; `MW2/src/polydraw.c`: file scope
- `MW2_KEY_COLOR`: `MW2/include/hud.h`: file scope; `MW2/include/palette.h`: file scope; `MW2/src/accel/vfx16.asm`: file scope; `MW2/src/cockpit.c`: `DrawMapView`; `MW2/src/cockpitpanels.c`: `DrawPanelAnim`; `MW2/src/damagepanel.c`: `DrawDamageOutline`, file scope; `MW2/src/hud.c`: `DrawAltimeter` (2), `DrawCompass`, `DrawHudShape` (2), file scope, the definitions of `ClipRectToPane`, `FUN_1001e01a`; `MW2/src/mechviewpanel.c`: `DrawMechViewPanel` (7), file scope; `MW2/src/menu.c`: `DrawAndRunMenu` (2); `MW2/src/pausebanner.c`: `DrawPausedBanner`; `MW2/src/statuspanels.c`: `DrawAutopilotPanel`, `DrawHeatPanel`, `DrawHeatRatePanel`, `DrawJetsPanel`, `DrawKillsPanel`, `DrawMascPanel`, `DrawNetworkPanel` (3), `DrawObjectivesPanel`, `DrawSpeedPanel`; `MW2/src/targetpanel.c`: `DrawTargetPanel` (7), `DrawTargetPanelText`; `MW2/src/timedoverlays.c`: `DrawTextBox`, `DrawTimedOverlays`; `MW2/src/weaponpanel.c`: `DrawWeaponPanel`, `DrawWeaponPanelStartup`
- `MW2_VFXREND_CS_SEGMENT`: `3rdparty/vfx/VFXREND.ASM`: `VFX_polygon_clip_XY_and_render`, `VFX_polygon_render` (2)
- `MW2_LOD_PAR`: `MW2/src/classtable.c`: `ChoosePlayerDetailLevels` (2), file scope; `MW2/src/targetpanel.c`: `DrawTargetPanel` (2), file scope (2), the definition of `IsPlayerInView`; `MW2/src/view.c`: `UpdateProjection` (2), file scope
- `MW2_TEXTURE_FILTER`: `MW2/src/accel/graphicsmenu.c`: file scope (4), the definitions of `GetFilterTextures`, `SetFilterTextures`; `MW2/src/simmain.c`: `FUN_1008fad0` (2)
- `MW2_DEPTH_ENTRY_TAG`: `MW2/include/depthsort.h`: `DepthEntry`; `MW2/src/depthsort.c`: file scope
- `MW2_RES_640X400`: `MW2/src/setres.c`: `ChooseArtResolution`
- `MW2_ART_RESOLUTION_512X384`: `MW2/src/setres.c`: `ChooseArtResolution`
- `MW2_DETAIL_LEVEL_IN_LOADER`: `MW2/src/classtable.c`: `ChoosePlayerDetailLevels`, `LoadClassLevel`
- `MW2_SKIP_LOADED_LEVEL`: `MW2/src/classtable.c`: `LoadClassEntryShape`
- `MW2_SHOT_DAMAGE_AFTER_RECORD`: `MW2/src/shots.c`: `UpdateShot` (2)
- `MW2_POWER_STATE_7_KEYS`: `MW2/src/gamekeys.c`: `RunGameKey` (3)
- `MW2_JOYSTICK_AXIS_SUMS`: `MW2/src/joystick.c`: file scope, the definition of `JoystickPoll`
- `MW2_FULLSCREEN_SAVES_MODE`: `MW2/src/refreshmode.c`: `ToggleFullScreen`
- `MW2_MOUSE_PEEK_RETURNS`: `MW2/src/simmain.c`: `HandleMessages`
- `MW2_STDCALL_TIMER_CALLBACK`: `MW2/src/supanim.c`: `SupAnimTimerCallback`, file scope
- `MW2_DEBUG_LOG_TEXT`: `MW2/src/refreshmode.c`: file scope; `MW2/src/simmain.c`: `UpdatePauseState` (2)
- `MW2_QUEUEFACE_REUSED_LOCALS`: `MW2/src/accel/queueface.c`: `QueueFace` (5)
- `MW2_PROJECT_Y_AFTER_X`: `MW2/src/accel/queueface.c`: `ProjectVertex` (2)
- `MW2_SLOPE_ONCE`: `MW2/src/accel/floatmath.c`: `InitSlopeTables` (2)
- `MW2_HORIZON_FLOAT_Y`: `MW2/src/horizon.c`: `HorizonYAtX` (2)
- `MW2_SKY_SETTINGS_BEFORE_CLOCK`: `MW2/src/simmain.c`: `SimMain`
- `MW2_RENDER_SETTINGS_BEFORE_CLOCK`: `MW2/src/simmain.c`: `SimMain`
- `MW2_SUPANIM_DC_RELEASED`: `MW2/src/simmain.c`: `SimMain` (2)
- `MW2_PALETTE_DC_RELEASED`: `MW2/src/simmain.c`: `SimMain` (3)
- `MW2_RES_800X600`: `MW2/src/accel/graphicsmenu.c`: `g_graphicsPage`, file scope (4), the definitions of `GetRes800x600`, `SetRes800x600`; `MW2/src/anim2d.c`: `DrawAnim2d`, file scope, the definition of `TileShapeFrame`; `MW2/src/menu.c`: `LayoutMenu` (2), file scope, the definition of `ScaleMenuPointToFrame`; `MW2/src/simmain.c`: `FUN_1008fad0` (2), the definition of `SetGameResolution`
- `MW2_ART_RESOLUTION_NEAREST`: `MW2/src/setres.c`: the definition of `ChooseArtResolution`
- `MW2_SKY_TILE_DEFAULTS`: `MW2/src/polydraw.c`: `LoadSkyGroundSettings`, file scope
- `MW2_SKY_DEFAULTS_AT_END`: `MW2/src/polydraw.c`: `LoadSkyGroundSettings`
- `MW2_SKY_GROUND_COLORS`: `MW2/src/polydraw.c`: `DrawSkyAndGround`
- `MW2_SKY_GROUND_ALL_CASES`: `MW2/src/polydraw.c`: `DrawSkyAndGround`
- `MW2_MESSAGE_BACKGROUND_SHAPES`: `MW2/src/timedoverlays.c`: file scope
- `MW2_OUTLINE_CLEARS_FLAGS`: `MW2/src/depthsort.c`: `DrawQueuedPolygon`
- `MW2_NARROW_TORSO_PAN`: `MW2/src/inputmap.c`: `g_inputSinks`
- `MW2_NO_REVERSE_THROTTLE`: `MW2/src/inputmap.c`: `g_inputSinks`
- `MW2_HIT_FEEDBACK_FLAG_CHECK`: `MW2/src/shots.c`: `UpdateShot`
- `MW2_NET_RETURN_ZERO`: `MW2/src/network.c`: `FindSession`, `StopExternalIO`
- `MW2_NO_NET_DEBUG_PRINTS`: `MW2/src/netio.c`: `NetReceive`, `NetSend` (2)
- `MW2_NO_STATE_7_POWER_TEST`: `MW2/src/gamekeys.c`: `RunGameKey`
- `MW2_LAYOUT_GAUGE_FUNCTIONS`: `MW2/src/cockpitlayouts.c`: `g_largeMapLayout`, `g_satelliteLayout`, `g_smallMapLayout`, file scope; `MW2/src/targeting.c`: file scope
- `MW2_INLINE_HELPERS`: `MW2/include/debugbreak.h`: the definition of `DebugBreakpoint`; `MW2/include/fixeddiv.h`: the definition of `FixedDiv16`; `MW2/include/fixedmul.h`: the definition of `FixedMul16`; `MW2/include/muldiv.h`: the definition of `MulDiv64`; `MW2/include/namehash.h`: the definition of `HashName`; `MW2/src/debugbreak.c`: the definition of `DebugBreakpoint`; `MW2/src/fixeddiv.c`: the definition of `FixedDiv16`; `MW2/src/fixedmul.c`: the definition of `FixedMul16`; `MW2/src/loadres.c`: `MemCopy`, `MemSet`; `MW2/src/muldiv.c`: the definition of `MulDiv64`; `MW2/src/namehash.c`: the definition of `HashName`
- `MW2_BLOCK_SCOPED_LOCALS`: `MW2/src/hud.c`: `DrawObjectBrackets` (4), `DrawPlayerBrackets` (3); `MW2/src/maneuvers.c`: `DodgeShot` (5); `MW2/src/mechclass.c`: `LateUpdateMech` (2), `UpdateMech` (6)
- `MW2_DRAG_DIVIDED_ONCE`: `MW2/src/mechclass.c`: `UpdateMech`
- `MW2_RECENTER_NO_RETEST`: `MW2/src/mechclass.c`: `LateUpdateMech`
- `MW2_DODGE_SIDE_FIRST`: `MW2/src/maneuvers.c`: `DodgeShot` (3)
- `MW2_BOUNCE_DOT_STATEMENTS`: `MW2/src/mechcollision.c`: `MoveMechWithCollisions`
- `MW2_OBJECTIVE_TARGET_BYTES`: `MW2/include/missionobjective.h`: file scope
- `MW2_DEBUG_LOG_FORMATS`: `MW2/src/polydraw.c`: `DrawSkyAndGround`
- `MW2_PROJECT_NEAR_CLIP`: `MW2/src/accel/queueface.c`: `ProjectPolygon`
- `MW2_SKY_GROUND_CLIPPED_HELPERS`: `MW2/src/animation.c`: the definitions of `FUN_10051c07`, `FUN_10051ea2`; `MW2/src/polydraw.c`: `FUN_10068e33`, `FUN_10069172`, `FUN_100692db`
- `MW2_CLIP_TO_PANE_EDGES`: `MW2/src/polydraw.c`: `FUN_10068856`, `FUN_100688fe`, `FUN_100689f5`, `FUN_10068b57`
- `MW2_FRAME_TEXELS`: `MW2/src/animation.c`: `FUN_100519b0`, the definition of `FUN_10051dfe`
- `MW2_BRIGHTNESS_STORE_CHAINED`: `MW2/src/brightnessmenu.c`: `RestoreBrightness`
- `MW2_CONDITIONAL_EXPRESSIONS`: `MW2/src/maneuvers.c`: `InitializeManeuvers` (5); `MW2/src/objectanim.c`: `ReelMotionTask`; `MW2/src/pausebanner.c`: `HandleDebugKey`; `MW2/src/screenscale.c`: `GetEllipseNeedleAt` (2), `GetRectNeedleAt`
- `MW2_HIDE_CACHE_INFO_NO_ARGS`: `MW2/src/overlay.c`: `HideCacheInfo`
- `MW2_ONE_RESOURCE_MESSAGE`: `MW2/src/loadres.c`: `LoadCachedResource` (2)
- `MW2_TILT_NEGATIVE_FACTOR`: `MW2/src/aiweapons.c`: `RunAIWeapons`
- `MW2_MEK_LOG_FLOATS`: `MW2/src/mekfile.c`: `LoadMechConfig` (2)
- `MW2_GROUND_HEIGHT_IN_TEST`: `MW2/src/shots.c`: `UpdateShot`
- `MW2_VFX3D_ALIGNED_FILL`: `3rdparty/vfx/VFX3D.ASM`: file scope
- `MW2_TEXTURE_FILTER_ITEM`: `MW2/src/accel/graphicsmenu.c`: `g_graphicsPage`
- `MW2_JUMP_JET_THRUST_FIRST`: `MW2/src/mechclass.c`: `UpdateMech` (4)
- `MW2_RES_512X384`: `MW2/include/directdraw.h`: file scope; `MW2/src/directdraw.c`: file scope, the definitions of `FUN_10005950`, `FUN_100059b0`; `MW2/src/simmain.c`: `SetGameResolution`
- `MW2_FADE_STEPS_ONLY`: `MW2/src/directdraw.c`: the definition of `DdrawPaletteFade`
- `MW2_DDRAW_PALETTE_UNSET`: `MW2/src/directdraw.c`: the definition of `DdrawWritePaletteEntries`, the definition of `DdrawWritePaletteGamma`
- `MW2_DDRAW_MODE_16BIT`: `MW2/src/directdraw.c`: `DdrawInit`
- `MW2_DDRAW_FILL_SIZE`: `MW2/src/directdraw.c`: `DdrawInit`
- `MW2_DDRAW_CAPS_DX1`: `MW2/src/directdraw.c`: `DdrawInit` (2)
- `MW2_DDRAW_NO_CAPS_FLAG`: `MW2/src/directdraw.c`: `DdrawCreateSurfaces` (3)
- `MW2_DDRAW_FALLBACK_SURFACES_ONLY`: `MW2/src/directdraw.c`: `DdrawCreateSurfaces`
- `MW2_QUEUEFACE_TEXTURE_RANGE`: `MW2/src/accel/queueface.c`: `QueueFace` (4)
- `MW2_PROJECT_ALL_NEAR_EMPTY`: `MW2/src/accel/queueface.c`: `ProjectPolygon` (4)
- `MW2_JOYSTICK_PROBE_SIZE_ONLY`: `MW2/src/joystick.c`: `JoystickOpenDevice`
- `MW2_RENDER_SETTINGS_AFTER_MODES`: `MW2/src/simmain.c`: `SimMain`
- `MW2_SHAPES_PALETTE_POLYGON`: `MW2/src/polydraw.c`: `DrawPentagon`, `DrawQuad`, `DrawTriangle`, file scope
- `MW2_SKIP_LOADED_LEVEL_WITH_RELEASED`: `MW2/src/classtable.c`: `LoadClassEntryShape`
- `MW2_COLLISION_DAMAGE_UNGROUPED`: `MW2/src/mechcollision.c`: `ApplyCollisionDamage`
- `MW2_GRID_OBJECT_HIDDEN`: `MW2/src/gridobject.c`: `SetGridObject`, the definition of `ShowGridObject`
- `MW2_NO_VFXREND`: `MW2/src/render.c`: `FirstRender`
- `MW2_ART_RESOLUTION_FIT`: `MW2/src/setres.c`: the definition of `ChooseArtResolution`
- `MW2_BLIT_NO_WAIT`: `MW2/src/directdraw.c`: `DdrawBlit`
- `MW2_RESET_TICKS_SAVE_ORDER`: `MW2/src/ticks.asm`: `ResetTicks`
- `MW2_DARKER_AVERAGE_COLORS`: `MW2/src/animation.c`: `FUN_100519b0`
- `MW2_SATELLITE_CLIP_ALL_EDGES`: `MW2/src/cockpit.c`: `SatelliteDrawPolygon`, file scope
- `MW2_OUTLINE_FLUSH`: `MW2/src/depthsort.c`: `DrawQueuedPolygon`, file scope
- `MW2_QUEUED_POLYGON_EDGES_0XFF`: `MW2/src/depthsort.c`: `DrawQueuedPolygon`
- `MW2_LIT_POLYGON_SHADE_BIT3`: `MW2/src/polydraw.c`: `DrawScenePolygon`
- `MW2_CHEAT_RECYCLE`: `MW2/src/weapons.c`: `UpdateWeaponFireState`
- `MW2_CHEAT_RECYCLE_HELD`: `MW2/src/weapons.c`: `UpdateWeaponFireState`
- `MW2_TIME_OF_DAY_PHASES_1_TO_3`: `MW2/src/environment.c`: `FirstEnvironment`
- `MW2_LIT_GROUND_FILL`: `MW2/src/simmain.c`: `SimMain`, file scope
- `MW2_NO_SIMMAIN_A3D_SHUTDOWN`: `MW2/src/simmain.c`: `SimMain`
- `MW2_FLAG_CHEAT`: `MW2/src/gamekeys.c`: `HandleCheatInput`
- `MW2_POWER_KEYS_ONE_CASE`: `MW2/src/gamekeys.c`: `RunGameKey`
- `MW2_NO_INITIAL_DRAW_MODE`: `MW2/src/render.c`: file scope
- `MW2_TIME_OF_DAY_LOOKUP`: `MW2/src/environment.c`: `UpdateTimeOfDay` (3), the definitions of `FUN_10006f10`, `FUN_10006f30`
- `MW2_FADE_LIGHT_COLOUR`: `MW2/src/palette.c`: `StartPaletteFade` (2)
- `MW2_PRJ_PATH_FREED`: `MW2/src/mw2prj.c`: `FirstResource` (4)
- `MW2_HUD_MARKER_LAG`: `MW2/src/hud.c`: `DrawHud`, file scope (2), the definition of `DrawTargetMarker`, the definitions of `DrawBrackets`, `DrawObjectBrackets`, `DrawPlayerBrackets`
- `MW2_TARGET_RANGE_AT_BOTTOM`: `MW2/src/targetpanel.c`: `DrawTargetPanelText` (5)
- `MW2_MATRIX_PITCH_ASIN_ONLY`: `MW2/src/transform.c`: `GetMatrixAngles`
- `MW2_CHRIS_FLAG_CHEATS`: `MW2/src/gamekeys.c`: `HandleCheatInput`
- `MW2_TUNING_CHEATS`: `MW2/src/gamekeys.c`: `HandleCheatInput`, file scope
- `MW2_COLLISION_TRANSFORMS_MODELS`: `MW2/src/shapecollision.c`: `TestPointAboveFaces`, `TestPointUnderFloor`, `TestRayFaces`, file scope
- `MW2_NO_SCREENSHOT_KEY`: `MW2/src/gamekeys.c`: `RunGameKey`
- `MW2_NO_WIREFRAME_KEY`: `MW2/src/gamekeys.c`: `RunGameKey`
- `MW2_RENDERER_TARGET_VIEW`: `MW2/src/targetpanel.c`: `DrawTargetPanel` (2)
- `MW2_UNROTATED_EFFECTS`: `MW2/src/shots.c`: `SpawnEffectEx` (2)
- `MW2_LOOKASIDE_RED_SHIFT_10`: `MW2/src/palette.c`: `ApplyPaletteResource`
- `MW2_A3D_NO_PALETTE`: `MW2/src/palette.c`: `ApplyPaletteResource`
- `MW2_HASH_NAME_ROL32`: `MW2/include/namehash.h`: `HashName`
- `MW2_NO_MAP_DAMAGE_ANIM`: `MW2/src/cockpit.c`: `DrawDamagedMapView`
- `MW2_MOUSE_PEEK_BREAKS`: `MW2/src/simmain.c`: `HandleMessages`
- `MW2_EXTERNAL_HEAT`: `MW2/include/mech.h`: `Mech`; `MW2/src/mechclass.c`: file scope; `MW2/src/mechdamage.c`: the definition of `CalculateHeat`
- `MW2_MECH_SECOND_FLAGS`: `MW2/include/mech.h`: `Mech`; `MW2/src/mechclass.c`: file scope
- `MW2_FOURTEEN_WEAPON_SLOTS`: `MW2/include/cockpitpanel.h`: `enum`; `MW2/include/weaponslot.h`: file scope; `MW2/src/cockpitpanels.c`: file scope (4); `MW2/src/config.c`: `LoadCptFile`
- `MW2_PILOT_TABLE`: `MW2/include/aiweapons.h`: file scope; `MW2/include/players.h`: `Player` (2); `MW2/src/ai.c`: `InitializeAI` (3), file scope (2); `MW2/src/aiweapons.c`: `RunAIWeapons`, the definition of `DecideAIFire`, the definition of `GroupAIWeapons`; `MW2/src/poolsizes.c`: `BuildStaticMemoryTable`
- `MW2_STATIC_MANEUVER_TABLES`: `MW2/include/maneuvers.h`: file scope; `MW2/src/maneuvers.c`: `ChooseManeuver`, `ManeuverStupid`, file scope (3), the definition of `InitializeManeuvers`
- `MW2_AI_FLAGS_INT`: `MW2/include/playerai.h`: `PlayerAi`; `MW2/src/ai.c`: `AssignStarObjective`, `LeadStar`, file scope
- `MW2_CHAT_KEY`: `MW2/src/gamekeys.c`: `RunGameKey`; `MW2/src/inputmap.c`: `g_gameKeyNames`
- `MW2_AMS_KEY`: `MW2/src/gamekeys.c`: `RunGameKey`, file scope; `MW2/src/inputmap.c`: `g_gameKeyNames`
- `MW2_GHOST_BEAR_TITLES`: `MW2/src/debugprint.c`: `ShowMessage`; `MW2/src/refreshmode.c`: `ToggleFullScreen` (2); `MW2/src/simmain.c`: `SimMain`
- `MW2_GBL_CD_DRIVER`: `MW2/src/gamecd.c`: `FindGameCdDrive`
- `MW2_ENGAGEMENT_FLAGS_INT`: `MW2/include/ai.h`: file scope; `MW2/src/ai.c`: `GetEngagementAIFlags`
- `MW2_TORSO_SNAP_ONE_DEGREE`: `MW2/src/mechclass.c`: file scope
- `MW2_WEAPON_FUNCTIONS`: `MW2/include/effect.h`: `Effect`, file scope (2); `MW2/include/effectinfo.h`: `EffectInfo`, file scope (2); `MW2/include/shots.h`: `Shot` (2), file scope (4); `MW2/include/weapondata.h`: file scope; `MW2/include/weapondef.h`: `WeaponDef`, file scope; `MW2/include/weapons.h`: file scope (3); `MW2/src/shots.c`: `DamageMechsInRadius` (4), `DetonateShot` (2), `FirstShots`, `GuideMissileToTarget` (2), `ResetShotSlot`, `SpawnEffect`, `SpawnLaunchEffect`, `SpawnRotatedEffect`, `UpdateNuke`, `UpdateShot` (3), file scope, the definition of `SpawnEffectEx`, the definition of `UpdateEffects`; `MW2/src/weapondata.c`: file scope (3); `MW2/src/weaponpanel.c`: `DrawWeaponPanel`; `MW2/src/weapons.c`: `LoadWeaponSounds`, `SelectNextWeapon` (2), `SelectNextWeaponGroup` (2), `SelectNextWeaponInGroup` (2), `SelectWeaponGroup` (3), `SpawnLaunchFx` (2), `UpdateWeaponLock`, file scope, the definition of `FireWeaponGroup`, the definition of `SpawnShot`, the definition of `UpdateWeaponFireState`
- `MW2_POSITIONAL_SOUND_FLAGS`: `MW2/include/soundfx.h`: file scope; `MW2/src/soundfx.c`: the definitions of `PlayPositionalSound`, `PlayPositionalSoundEx`
- `MW2_THING_KILLS_FORGET_SHOOTER`: `MW2/src/shots.c`: `KillGameThing` (2)
- `MW2_ABS_CALLS`: `MW2/src/ai.c`: `AssignStarTarget`, file scope; `MW2/src/bandpoly.c`: file scope; `MW2/src/gridobject.c`: file scope; `MW2/src/maneuvers.c`: file scope; `MW2/src/setres.c`: `ChooseArtResolution`; `MW2/src/weapons.c`: file scope
- `MW2_BEARING_LABS`: `MW2/src/targeting.c`: file scope
- `MW2_BUILDINGS_FALL_AT_ONCE`: `MW2/src/shots.c`: `DamageGameThing` (3)
- `MW2_DESTROY_BLOWS_FIRST_BIN`: `MW2/src/mechdamage.c`: `DestroyCriticalSlot` (2), the definition of `DestroyMech`
- `MW2_SELF_DESTRUCT_MESSAGE`: `MW2/src/mechdamage.c`: `EjectPlayer`
- `MW2_KILLS_BY_LAST_SHOOTER`: `MW2/src/mechdamage.c`: `DestroyCriticalSlot` (2), `DestroySection`, the definition of `ApplyDamageToMech`, the definition of `KillMech`
- `MW2_MORE_CRITICAL_HITS`: `MW2/src/mechdamage.c`: `DestroyCriticalSlot` (2)
- `MW2_AI_STOPS_GROUP_FIRE`: `MW2/src/ai.c`: `AiStateAttack`, `AiStateMove`
- `MW2_STRUCT_COPY_MEMCPY`: `MW2/src/ai.c`: `LogPlayerStatusLines` (2), file scope; `MW2/src/cockpitpanels.c`: `LayoutWeaponPanels` (2); `MW2/src/depthsort.c`: `ClipPolygonToScreen` (17); `MW2/src/geocache.c`: `DestroyStaticObject`; `MW2/src/mechviewpanel.c`: `DrawMechViewPanel` (3), `SetMechViewRenderSettings`, file scope; `MW2/src/network.c`: `FindSession`, `FindSessionCallback`; `MW2/src/object.c`: `CreateObj`, `InitObj`, `SetObjLocalMatrix`, `SetObjTransform`, `SetObjWorldMatrix`, `UpdateObjWorld`; `MW2/src/objective.c`: `EndTheMission2`; `MW2/src/ray.c`: `CopyRay`, file scope; `MW2/src/soundfx.c`: `StartSample`; `MW2/src/targeting.c`: `RemoveNavPoint`, file scope; `MW2/src/targetpanel.c`: `DrawTargetPanel`
- `MW2_LOCAL_INSPECTION`: `MW2/src/targeting.c`: the definition of `UpdateTarget`
- `MW2_CHEAT_TABLE`: `MW2/src/gamekeys.c`: `TypedCodeMatches`, file scope (2), the definitions of `DecodeCheatMessages`, `HandleCheatInput`
- `MW2_INI_COMMENTS`: `MW2/include/inifile.h`: file scope; `MW2/src/inifile.c`: `GetIniValue` (3)
- `MW2_NO_MOUSE_ERRORS`: `MW2/src/inputmap.c`: `ReportInputDeviceError`
- `MW2_DEPTH_QUEUE_INDEX`: `MW2/src/depthsort.c`: `DrawDepthQueue` (2)
- `MW2_LENGTH_SUM_FROM_Z`: `MW2/src/clockmath.c`: `BuildMatrixFromDirection`; `MW2/src/fixedsqrt.c`: `FloatVectorLength`; `MW2/src/normalizerotation.c`: `ScaleVectorToLength`
- `MW2_SKIP_SHAPE_TYPE_20`: `MW2/src/depthsort.c`: `QueueObjTree`
- `MW2_PT_MENU_KEYS`: `MW2/src/gamekeys.c`: `RunGameKey` (3)
- `MW2_SCREENSHOT_ACQUIRE_IF`: `MW2/src/gamekeys.c`: `RunGameKey`
- `MW2_LOST_MECH_FAILS_STAR`: `MW2/src/objective.c`: `UpdateObjectives`, file scope
- `MW2_NAME_STRCMP`: `MW2/src/joystick.c`: `FillJoystickDeviceInfo`; `MW2/src/network.c`: `FindIpxProviderCallback`
- `MW2_MEM_HELPERS_CALL_CRT`: `MW2/src/loadres.c`: `MemCopy` (2), `MemSet` (2)
- `MW2_SCROUNGE_STATIC_OBJECTS`: `MW2/include/geocache.h`: file scope (2); `MW2/src/world.c`: `BwdExecuteStream` (2)
- `MW2_REMOTE_PLAYER_AI_MODE`: `MW2/src/world.c`: `BwdExecuteStream`
- `MW2_DEEP_COLD_COOLING`: `MW2/src/mekfile.c`: `LoadMechConfig`
- `MW2_PLAYERS_BEFORE_LAUNCH`: `MW2/src/simmain.c`: `SimMain`
- `MW2_SUPANIM_KEEPS_PALETTE`: `MW2/src/supanim.c`: `StartSupAnim`
- `MW2_SOUND_INFO_FILL_MINUS_ONE`: `MW2/src/soundlimits.c`: `InitSoundInfo`
- `MW2_DDRAW_STOP_KEEPS_MODE`: `MW2/src/directdraw.c`: `DdrawStop`
- `MW2_REMOTE_MECHS_STAY`: `MW2/src/mechclass.c`: `UpdateMech`
- `MW2_SHAPE_GRID`: `MW2/include/object.h`: file scope; `MW2/include/shape.h`: `Shape`; `MW2/include/shapelists.h`: file scope (4); `MW2/src/object.c`: `UpdateObjTree` (2), the definition of `UpdateObj`; `MW2/src/shapelists.c`: `AddSceneShape` (3), `EnableShapeCollision`, `FreeSceneShapes`, `InitShapeLists` (2), `ShowDensityShapes` (2), `ShowShape` (2), `UnlinkShape`, file scope (3), the definitions of `DisableShapeGrid`, `EnableShapeGrid`, `InitShapeGridNeighbours`, `ListAllShapeGridCells`, `ListShapeGridCellsAhead`, `ListShapeGridCellsAround`, the definitions of `GetShapeGridHead`, `GetShapeGridList`, `MoveObjTreeOffGrid`, `MoveObjTreeToCell`, `UpdateShapeGridCell`
- `MW2_PLAYER_UNK_MEMBERS`: `MW2/include/players.h`: `Player` (3); `MW2/src/ai.c`: file scope
- `MW2_MECH_UNK_MEMBERS`: `MW2/include/ammobin.h`: `AmmoBin`; `MW2/include/mech.h`: `Mech`; `MW2/include/weaponslot.h`: `WeaponSlot`; `MW2/src/mechclass.c`: file scope

## Functions with a definition per edition

Where an edition's version of a function differs in its control flow or in more than a few statements, each version is a definition of its own; the comment above each says what that edition's does.

| Function | File | Selected by | Definitions |
|---|---|---|---|
| `AddShapeFace` | `MW2/src/shape.c` | `defined(MW2_POWERVR)` | PowerVR; 1.1, ATI, Pre-Mercs, S3, Matrox, GBL and 3Dfx |
| `ApplyDamageToMech` | `MW2/src/mechdamage.c` | `defined(MW2_KILLS_BY_LAST_SHOOTER)` | Pre-Mercs and GBL; 1.1, ATI, S3, Matrox, PowerVR and 3Dfx |
| `CalculateHeat` | `MW2/src/mechdamage.c` | `defined(MW2_EXTERNAL_HEAT)` | Pre-Mercs and GBL; 1.1, ATI, S3, Matrox, PowerVR and 3Dfx |
| `ChooseArtResolution` | `MW2/src/setres.c` | `defined(MW2_ART_RESOLUTION_NEAREST)` | 3Dfx; S3; 1.1, ATI, Pre-Mercs, Matrox, PowerVR and GBL |
| `ChoosePlayerDetailLevels` | `MW2/src/classtable.c` | `defined(MW2_POWERVR)` | PowerVR; 1.1, ATI, Pre-Mercs, S3, Matrox, GBL and 3Dfx |
| `DdrawCreateSurfaces` | `MW2/src/directdraw.c` | `defined(MW2_S3)` | S3; 1.1, ATI, Pre-Mercs and GBL |
| `DdrawPaletteFade` | `MW2/src/directdraw.c` | `defined(MW2_FADE_STEPS_ONLY)` | ATI and S3; 1.1, Pre-Mercs, PowerVR and GBL |
| `DdrawWritePaletteEntries` | `MW2/src/directdraw.c` | `defined(MW2_DDRAW_PALETTE_UNSET)` | ATI, S3 and PowerVR; 1.1, Pre-Mercs and GBL |
| `DdrawWritePaletteGamma` | `MW2/src/directdraw.c` | `defined(MW2_DDRAW_PALETTE_UNSET)` | ATI, S3 and PowerVR; 1.1, Pre-Mercs and GBL |
| `DecideAIFire` | `MW2/src/aiweapons.c` | `defined(MW2_PILOT_TABLE)` | Pre-Mercs and GBL; 1.1, ATI, S3, Matrox, PowerVR and 3Dfx |
| `DestroyMech` | `MW2/src/mechdamage.c` | `defined(MW2_DESTROY_BLOWS_FIRST_BIN)` | Pre-Mercs and GBL; 1.1, ATI, S3, Matrox, PowerVR and 3Dfx |
| `DrawMapView` | `MW2/src/cockpit.c` | `defined(MW2_POWERVR)` | PowerVR; 1.1, ATI, Pre-Mercs, S3, Matrox, GBL and 3Dfx |
| `DrawMechViewPanel` | `MW2/src/mechviewpanel.c` | `defined(MW2_POWERVR)` | PowerVR; 1.1, ATI, Pre-Mercs, S3, Matrox, GBL and 3Dfx |
| `DrawObjectBrackets` | `MW2/src/hud.c` | `!defined(MW2_HUD_MARKER_LAG)` | 1.1, ATI, Pre-Mercs, S3, Matrox, GBL and 3Dfx; PowerVR |
| `DrawPlayerBrackets` | `MW2/src/hud.c` | `!defined(MW2_HUD_MARKER_LAG)` | 1.1, ATI, Pre-Mercs, S3, Matrox, GBL and 3Dfx; PowerVR |
| `DrawScene` | `MW2/src/render.c` | `defined(MW2_POWERVR)` | PowerVR; 1.1, ATI, Pre-Mercs, S3, Matrox, GBL and 3Dfx |
| `DrawSkyAndGround` | `MW2/src/polydraw.c` | `defined(MW2_A3D)` | ATI, S3, Matrox, PowerVR and 3Dfx; 1.1, Pre-Mercs and GBL |
| `DrawSkyAndGround` | `MW2/src/polydraw.c` | `defined(MW2_S3)` | S3; ATI, Matrox, PowerVR and 3Dfx |
| `DrawTargetMarker` | `MW2/src/hud.c` | `!defined(MW2_HUD_MARKER_LAG)` | 1.1, ATI, Pre-Mercs, S3, Matrox, GBL and 3Dfx; PowerVR |
| `FireWeaponGroup` | `MW2/src/weapons.c` | `defined(MW2_WEAPON_FUNCTIONS)` | Pre-Mercs and GBL; 1.1, ATI, S3, Matrox, PowerVR and 3Dfx |
| `FirstRender` | `MW2/src/render.c` | `defined(MW2_POWERVR)` | PowerVR; 1.1, ATI, Pre-Mercs, S3, Matrox, GBL and 3Dfx |
| `FUN_100519b0` | `MW2/src/animation.c` | `defined(MW2_POWERVR)` | PowerVR; ATI, S3, Matrox and 3Dfx |
| `FUN_10051c07` | `MW2/src/animation.c` | `defined(MW2_S3)` | S3; ATI, PowerVR and 3Dfx |
| `FUN_10051ea2` | `MW2/src/animation.c` | `defined(MW2_S3)` | S3; ATI, PowerVR and 3Dfx |
| `FUN_10067b90` | `MW2/src/polydraw.c` | `defined(MW2_S3)` | S3; ATI, Matrox, PowerVR and 3Dfx |
| `FUN_10068856` | `MW2/src/polydraw.c` | `defined(MW2_S3)` | S3; ATI, Matrox, PowerVR and 3Dfx |
| `FUN_100688fe` | `MW2/src/polydraw.c` | `defined(MW2_S3)` | S3; ATI, Matrox, PowerVR and 3Dfx |
| `FUN_100689f5` | `MW2/src/polydraw.c` | `defined(MW2_S3)` | S3; ATI, Matrox, PowerVR and 3Dfx |
| `FUN_10068b57` | `MW2/src/polydraw.c` | `defined(MW2_S3)` | S3; ATI, Matrox, PowerVR and 3Dfx |
| `FUN_10069172` | `MW2/src/polydraw.c` | `defined(MW2_S3)` | S3; ATI, Matrox, PowerVR and 3Dfx |
| `FUN_100692db` | `MW2/src/polydraw.c` | `defined(MW2_S3)` | S3; ATI, Matrox, PowerVR and 3Dfx |
| `FUN_100751a9` | `MW2/src/cockpit.c` | `defined(MW2_S3)` | S3; ATI, Matrox, PowerVR and 3Dfx |
| `FUN_1008fad0` | `MW2/src/simmain.c` | `defined(MW2_POWERVR)` | PowerVR; ATI, S3, Matrox and 3Dfx |
| `HandleCheatInput` | `MW2/src/gamekeys.c` | `defined(MW2_CHEAT_TABLE)` | Pre-Mercs and GBL; 1.1, ATI, S3, Matrox, PowerVR and 3Dfx |
| `InitializeManeuvers` | `MW2/src/maneuvers.c` | `defined(MW2_STATIC_MANEUVER_TABLES)` | Pre-Mercs and GBL; 1.1, ATI, S3, Matrox, PowerVR and 3Dfx |
| `JoystickPoll` | `MW2/src/joystick.c` | `defined(MW2_JOYSTICK_AXIS_SUMS)` | Matrox; PowerVR; 1.1, ATI, Pre-Mercs, S3, GBL and 3Dfx |
| `KillMech` | `MW2/src/mechdamage.c` | `defined(MW2_KILLS_BY_LAST_SHOOTER)` | Pre-Mercs and GBL; 1.1, ATI, S3, Matrox, PowerVR and 3Dfx |
| `LoadSkyGroundSettings` | `MW2/src/polydraw.c` | `defined(MW2_S3)` | S3; ATI, Matrox, PowerVR and 3Dfx |
| `PlayPositionalSound` | `MW2/src/soundfx.c` | `defined(MW2_POSITIONAL_SOUND_FLAGS)` | Pre-Mercs and GBL; 1.1, ATI, S3, Matrox, PowerVR and 3Dfx |
| `RotateTexturePoint` | `MW2/src/animation.c` | `defined(MW2_S3)` | S3; ATI, Matrox, PowerVR and 3Dfx |
| `SetGameResolution` | `MW2/src/simmain.c` | `defined(MW2_RES_800X600)` | 3Dfx; S3; PowerVR; 1.1, ATI, Pre-Mercs, Matrox and GBL |
| `ShowGridObject` | `MW2/src/gridobject.c` | `defined(MW2_GRID_OBJECT_HIDDEN)` | S3; 1.1, ATI, Pre-Mercs, Matrox, PowerVR, GBL and 3Dfx |
| `SimMain` | `MW2/src/simmain.c` | `defined(MW2_MATROX)` | Matrox; PowerVR; ATI, S3 and 3Dfx; 1.1, Pre-Mercs and GBL |
| `SimWindowProc` | `MW2/src/simmain.c` | `defined(MW2_MATROX)` | Matrox; ATI, S3, PowerVR and 3Dfx; 1.1, Pre-Mercs and GBL |
| `SpawnEffectEx` | `MW2/src/shots.c` | `defined(MW2_WEAPON_FUNCTIONS)` | Pre-Mercs and GBL; 1.1, ATI, S3, Matrox, PowerVR and 3Dfx |
| `SpawnShot` | `MW2/src/weapons.c` | `defined(MW2_WEAPON_FUNCTIONS)` | Pre-Mercs and GBL; 1.1, ATI, S3, Matrox, PowerVR and 3Dfx |
| `SwitchCockpitView` | `MW2/src/cockpit.c` | `defined(MW2_POWERVR)` | PowerVR; 1.1, ATI, Pre-Mercs, S3, Matrox, GBL and 3Dfx |
| `UpdateEffects` | `MW2/src/shots.c` | `defined(MW2_WEAPON_FUNCTIONS)` | Pre-Mercs and GBL; 1.1, ATI, S3, Matrox, PowerVR and 3Dfx |
| `UpdateMech` | `MW2/src/mechclass.c` | `defined(MW2_FLOAT_SCALARS)` | ATI, S3, Matrox, PowerVR and 3Dfx; 1.1, Pre-Mercs and GBL |
| `UpdateTarget` | `MW2/src/targeting.c` | `defined(MW2_LOCAL_INSPECTION)` | Pre-Mercs and GBL; 1.1, ATI, S3, Matrox, PowerVR and 3Dfx |
| `UpdateWeaponFireState` | `MW2/src/weapons.c` | `defined(MW2_WEAPON_FUNCTIONS)` | Pre-Mercs and GBL; 1.1, ATI, S3, Matrox, PowerVR and 3Dfx |

## Sites that test an edition

What tests `MW2_ACCEL`, `MW2_MATROX` or `MW2_3DFX` itself rather than a feature, sorted by what the site holds (the tool's reading of its content).

### `MW2_ACCEL` (85 sites)

- In a function: the edition's back end, what the shared code does for it, or the same computation written another way (9): `MW2/src/aiweapons.c`: `DecideAIFire`; `MW2/src/cockpit.c`: `SatelliteFaceColor`; `MW2/src/mekfile.c`: `LoadMechConfig`; `MW2/src/menucontrols.c`: `RunMenuStatus` (2); `MW2/src/ramp.c`: `SetWrappedRampTarget`, `UpdateWrappedRamp`; `MW2/src/world.c`: `BwdExecuteStream`; `MW2/src/wtbshapes.c`: `LoadShapeRecord`
- An edition's own definition of a function, or functions only some editions' objects have (7): `MW2/src/accel/queueface.c`: the definitions of `ClipEdgeToNearPlane`, `GetFaceShade`, `GetViewVertex`, `ProjectVertex`, `QueueFace`; `MW2/src/shape.c`: the definition of `GetShapeLoadFlags`, the definition of `SelectModelByKey`, the definition of `SelectNextModel`, the definitions of `FUN_1003a859`, `FUN_1003a889`, `GetModelKey`, the definitions of `GetNewShapeFlags`, `SelectFirstModel`, `SetNewShapeFlags`, the definitions of `GetShapeFace`, `GetTransformedVertex`, `GetVertexPosition`
- A fragment included at the position of the object that holds it (21): `MW2/src/ai.c`: file scope (7); `MW2/src/clockmath.c`: file scope; `MW2/src/cockpit.c`: file scope (3); `MW2/src/config.c`: file scope; `MW2/src/depthsort.c`: file scope (2); `MW2/src/joystick.c`: file scope (2); `MW2/src/shape.c`: file scope (4); `MW2/src/transform.c`: file scope
- The same definition at another address or in another section (34): `MW2/src/cockpit.c`: file scope; `MW2/src/inputmap.c`: file scope (32); `MW2/src/recordstacks.c`: file scope
- Data and declarations that differ (10): `MW2/include/simmain.h`: file scope; `MW2/src/gdi.c`: file scope; `MW2/src/inputmap.c`: file scope (8)
- Includes (4): `MW2/include/depthsort.h`: file scope; `MW2/src/classtable.c`: file scope; `MW2/src/simmain.c`: file scope; `MW2/src/view.c`: file scope

### `MW2_ATI` (34 sites)

- In a function: the edition's back end, what the shared code does for it, or the same computation written another way (14): `MW2/src/animation.c`: `DrawAnimatedPolygon`, `FUN_10069591`; `MW2/src/cockpit.c`: `FUN_100751a9`; `MW2/src/directdraw.c`: `DdrawLockBuffer`; `MW2/src/mapview.c`: `BeginMapView`; `MW2/src/mechdamage.c`: `ApplyDamageToMech`; `MW2/src/polydraw.c`: `DrawSkyAndGround`, `FUN_100688fe`, `FUN_10068b57` (2); `MW2/src/shots.c`: `SpawnEffectEx`; `MW2/src/simmain.c`: `SimMain`; `MW2/src/view.c`: `UpdateProjection` (2)
- An edition's own definition of a function, or functions only some editions' objects have (3): `MW2/src/bandpoly.c`: the definition of `FUN_1002a480`; `MW2/src/simmain.c`: the definition of `SimMain`, the definition of `SimWindowProc`
- A fragment included at the position of the object that holds it (1): `MW2/src/targeting.c`: file scope
- Data and declarations that differ (8): `MW2/include/accel/a3d.h`: file scope; `MW2/include/accel/a3dvertex.h`: file scope; `MW2/include/animation.h`: file scope; `MW2/include/bandpoly.h`: file scope; `MW2/include/directdraw.h`: file scope; `MW2/src/bwdkeywords.c`: file scope; `MW2/src/damagepanel.c`: file scope; `MW2/src/directdraw.c`: file scope
- Includes (4): `MW2/include/bandpoly.h`: file scope; `MW2/include/directdraw.h`: file scope; `MW2/src/animation.c`: file scope; `MW2/src/bandpoly.c`: file scope
- Pragmas that reproduce a unit's flags (4): `MW2/src/accel/queueface.c`: file scope (2); `MW2/src/keyboard.c`: file scope (2)

### `MW2_PREMERCS` (5 sites)

- A fragment included at the position of the object that holds it (3): `MW2/src/clock.c`: file scope; `MW2/src/config.c`: file scope; `MW2/src/objectanim.c`: file scope
- Data and declarations that differ (2): `MW2/src/refreshmode.c`: file scope; `MW2/src/simmain.c`: file scope

### `MW2_S3` (64 sites)

- In a function: the edition's back end, what the shared code does for it, or the same computation written another way (14): `MW2/src/animation.c`: `DrawAnimatedPolygon`, `FUN_10069591`; `MW2/src/directdraw.c`: `DdrawInit` (2), `DdrawLockBuffer`, `DdrawStop` (2), `DdrawUnlock`; `MW2/src/mapview.c`: `BeginMapView`; `MW2/src/mechdamage.c`: `ApplyDamageToMech`; `MW2/src/polydraw.c`: `FUN_10068b57` (2); `MW2/src/view.c`: `UpdateProjection` (2)
- An edition's own definition of a function, or functions only some editions' objects have (19): `MW2/src/animation.c`: the definition of `FUN_1001e300`, the definition of `RotateTexturePoint`, the definitions of `FUN_10051c07`, `FUN_10051ea2`; `MW2/src/cockpit.c`: the definition of `FUN_100751a9`; `MW2/src/directdraw.c`: the definition of `DdrawCreateSurfaces`, the definitions of `FUN_10005950`, `FUN_100059b0`; `MW2/src/polydraw.c`: the definition of `DrawSkyAndGround`, the definition of `FUN_1000bcc0`, the definition of `FUN_1000d960`, the definition of `FUN_10067b90`, the definition of `FUN_10068856`, the definition of `FUN_100688fe`, the definition of `FUN_100689f5`, the definition of `FUN_10068b57`, the definition of `LoadSkyGroundSettings`, the definitions of `FUN_10069172`, `FUN_100692db`; `MW2/src/simmain.c`: the definition of `SetGameResolution`, the definition of `SimMain`, the definition of `SimWindowProc`
- A fragment included at the position of the object that holds it (2): `MW2/src/eyepoint.c`: file scope; `MW2/src/palette.c`: file scope
- The same definition at another address or in another section (1): `MW2/src/menu.c`: file scope
- Data and declarations that differ (18): `MW2/include/accel/a3d.h`: file scope; `MW2/include/accel/a3dvertex.h`: file scope; `MW2/include/animation.h`: file scope (2); `MW2/include/bandpoly.h`: file scope; `MW2/include/directdraw.h`: file scope (2); `MW2/include/polydraw.h`: file scope; `MW2/src/animation.c`: file scope; `MW2/src/bwdkeywords.c`: file scope; `MW2/src/damagepanel.c`: file scope; `MW2/src/directdraw.c`: file scope; `MW2/src/palette.c`: file scope; `MW2/src/polydraw.c`: file scope (4); `MW2/src/render.c`: file scope
- Includes (6): `MW2/include/bandpoly.h`: file scope; `MW2/include/directdraw.h`: file scope; `MW2/src/animation.c`: file scope; `MW2/src/bandpoly.c`: file scope; `MW2/src/directdraw.c`: file scope; `MW2/src/simmain.c`: file scope
- Pragmas that reproduce a unit's flags (4): `MW2/src/accel/queueface.c`: file scope (2); `MW2/src/keyboard.c`: file scope (2)

### `MW2_MATROX` (17 sites)

- In a function: the edition's back end, what the shared code does for it, or the same computation written another way (5): `MW2/src/animation.c`: `CalcAverageBitmapColors`, `DrawAnimatedPolygon`; `MW2/src/menucontrols.c`: `RunMenuStatus` (2); `MW2/src/shots.c`: `DamageThingsInRadius`
- An edition's own definition of a function, or functions only some editions' objects have (3): `MW2/src/animation.c`: the definitions of `FUN_10051c07`, `FUN_10051dfe`, `FUN_10051ea2`; `MW2/src/simmain.c`: the definition of `SimMain`, the definition of `SimWindowProc`
- A fragment included at the position of the object that holds it (1): `MW2/src/staticmem.c`: file scope
- Data and declarations that differ (5): `MW2/include/accel/a3d.h`: file scope; `MW2/src/refreshmode.c`: `g_displayBackends`, `g_refreshModes`; `MW2/src/simmain.c`: file scope (2)
- Includes (3): `MW2/src/refreshmode.c`: file scope; `MW2/src/simmain.c`: file scope (2)

### `MW2_POWERVR` (209 sites)

- In a function: the edition's back end, what the shared code does for it, or the same computation written another way (78): `MW2/src/addmodel.c`: `AddModel` (2); `MW2/src/animation.c`: `AddAnimFrame` (2), `AdvanceAnimations` (2), `DrawAnimatedPolygon`, `FUN_10051ba1`, `InitAnimations` (2), `SetAnimMode`, `StartAnimation` (2); `MW2/src/classtable.c`: `LoadClassEntryShape` (5), `ReleaseClassEntryShape`, `ReleaseObjShape` (2), `ResetClassTable`; `MW2/src/cockpit.c`: `ToggleSatelliteView`; `MW2/src/cockpitpanel.c`: `InitCockpitPanel` (2); `MW2/src/cockpitpanels.c`: `InitCockpitPanels` (2); `MW2/src/directdraw.c`: `DdrawBlit`, `DdrawBlitFlip`, `DdrawBlitRect`, `DdrawFlip`, `DdrawLockBuffer`, `DdrawPaletteFade` (5), `DdrawStretchBlit`; `MW2/src/eyepoint.c`: `ApplyCameraFov`; `MW2/src/geocache.c`: `LoadStaticObject`; `MW2/src/joystick.c`: `JoystickOpenDevice` (2); `MW2/src/mapview.c`: `BeginMapView`, `DrawMapViewScene`; `MW2/src/mechdamage.c`: `ApplyDamageToMech`; `MW2/src/mechreload.c`: `RestoreMechSegments`; `MW2/src/object.c`: `CreateObj`, `DestroyObjTree`, `DetachObj`, `InitObj`, `MoveObj`, `RaisePartDamageLevel`, `RotateObj`, `RotateObjMatrix` (2), `SetObjLocalMatrix` (2), `SetObjPosition`, `SetObjRotation`, `SetObjRotationMatrix` (2), `SetObjShape`, `SetObjTransform` (2); `MW2/src/polydraw.c`: `DrawSkyAndGround`, `FUN_100688fe`, `FUN_10068b57` (2), `LoadSkyGroundSettings`; `MW2/src/render.c`: `ShutdownRender`; `MW2/src/shape.c`: `CreateShape`, `FreeShape`; `MW2/src/shapelists.c`: `HideShape`, `ShowShape`; `MW2/src/view.c`: `UpdateProjection` (2); `MW2/src/wtbshapes.c`: `LoadShapeRecord` (2)
- An edition's own definition of a function, or functions only some editions' objects have (25): `MW2/src/accel/graphicsmenu.c`: the definitions of `GetFilterTextures`, `GetFog`, `GetSmoothShading`, `GetTexturedGround`, `GetTexturedSky`, `SetFilterTextures`, `SetFog`, `SetSmoothShading`, `SetTexturedGround`, `SetTexturedSky`; `MW2/src/animation.c`: the definition of `FUN_100519b0`, the definition of `FUN_10069586`, the definitions of `FUN_10027350`, `FUN_100273e0`; `MW2/src/bandpoly.c`: the definition of `FUN_1002a480`; `MW2/src/classtable.c`: the definition of `ChoosePlayerDetailLevels`, the definition of `FUN_100259c0`; `MW2/src/cockpit.c`: the definition of `DrawMapView`, the definition of `FUN_10003310`, the definition of `SwitchCockpitView`; `MW2/src/directdraw.c`: the definition of `DdrawFill`, the definition of `DdrawStretchBlit320`, the definitions of `DdrawCreateSurfaces`, `DdrawInit`; `MW2/src/eyepoint.c`: the definition of `FUN_10028ac0`; `MW2/src/joystick.c`: the definition of `JoystickPoll`; `MW2/src/mechviewpanel.c`: the definitions of `DrawMechViewPanel`, `FUN_10010b10`, `FUN_10010d50`; `MW2/src/render.c`: the definition of `DrawScene`, the definition of `FirstRender`; `MW2/src/shape.c`: the definition of `SetShapeDamageLevel`, the definitions of `AddShapeFace`, `AddShapeFaceIndex`; `MW2/src/simmain.c`: the definition of `SetGameResolution`, the definition of `SimMain`, the definition of `SimWindowProc`, the definitions of `FUN_10007db0`, `FUN_1008fad0`; `MW2/src/targetpanel.c`: the definitions of `FUN_10020450`, `FUN_100206e0`
- A fragment included at the position of the object that holds it (1): `MW2/src/targeting.c`: file scope
- The same definition at another address or in another section (35): `MW2/src/accel/graphicsmenu.c`: file scope; `MW2/src/inputmap.c`: file scope (32); `MW2/src/perf.c`: file scope; `MW2/src/settings.c`: file scope
- Data and declarations that differ (50): `MW2/include/accel/a3d.h`: file scope; `MW2/include/accel/a3dvertex.h`: file scope; `MW2/include/animation.h`: `Animation`, file scope (2); `MW2/include/animframe.h`: `AnimFrame`; `MW2/include/bandpoly.h`: file scope; `MW2/include/classentry.h`: `ClassEntry`; `MW2/include/cockpit.h`: file scope; `MW2/include/cockpitpanel.h`: `CockpitPanel` (2); `MW2/include/mechviewpanel.h`: file scope; `MW2/include/model.h`: `Model`; `MW2/include/object.h`: `SceneObject`; `MW2/include/shape.h`: `Shape`, file scope (4); `MW2/include/targetpanel.h`: file scope; `MW2/src/accel/graphicsmenu.c`: file scope (4); `MW2/src/animation.c`: file scope (3); `MW2/src/bwdkeywords.c`: file scope; `MW2/src/classtable.c`: file scope; `MW2/src/cockpit.c`: file scope; `MW2/src/cockpitlayouts.c`: file scope; `MW2/src/cockpitpanel.c`: file scope; `MW2/src/cockpitpanels.c`: `g_cockpitPanelPanes`; `MW2/src/damagepanel.c`: file scope; `MW2/src/directdraw.c`: file scope (3); `MW2/src/geocache.c`: file scope; `MW2/src/inputmap.c`: file scope (8); `MW2/src/joystick.c`: file scope; `MW2/src/object.c`: file scope; `MW2/src/simmain.c`: file scope; `MW2/src/targetpanel.c`: file scope
- Includes (16): `MW2/include/bandpoly.h`: file scope; `MW2/src/animation.c`: file scope (2); `MW2/src/bandpoly.c`: file scope; `MW2/src/classtable.c`: file scope; `MW2/src/cockpit.c`: file scope; `MW2/src/directdraw.c`: file scope; `MW2/src/eyepoint.c`: file scope; `MW2/src/mapview.c`: file scope; `MW2/src/mechreload.c`: file scope; `MW2/src/mechviewpanel.c`: file scope; `MW2/src/object.c`: file scope; `MW2/src/shape.c`: file scope; `MW2/src/shapelists.c`: file scope; `MW2/src/simmain.c`: file scope; `MW2/src/targetpanel.c`: file scope
- Pragmas that reproduce a unit's flags (4): `MW2/src/accel/queueface.c`: file scope (2); `MW2/src/keyboard.c`: file scope (2)

### `MW2_GBL` (8 sites)

- In a function: the edition's back end, what the shared code does for it, or the same computation written another way (3): `MW2/src/mechcollision.c`: `MoveMechWithCollisions`; `MW2/src/quadtree.c`: `BuildQuadtreeChild`; `MW2/src/weapons.c`: `SelectNextWeaponInGroup`
- A fragment included at the position of the object that holds it (3): `MW2/src/clock.c`: file scope; `MW2/src/config.c`: file scope; `MW2/src/objectanim.c`: file scope
- Data and declarations that differ (2): `MW2/src/refreshmode.c`: file scope; `MW2/src/simmain.c`: file scope

### `MW2_3DFX` (39 sites)

- In a function: the edition's back end, what the shared code does for it, or the same computation written another way (19): `MW2/src/animation.c`: `DrawAnimatedPolygon`, `FUN_10051c07` (3), `FUN_10051ea2` (2), `FUN_10069591`; `MW2/src/cockpit.c`: `FUN_100751a9`; `MW2/src/mapview.c`: `BeginMapView`; `MW2/src/mechdamage.c`: `ApplyDamageToMech`; `MW2/src/mw2prj.c`: `CachePreloads`; `MW2/src/polydraw.c`: `DrawSkyAndGround`, `FUN_100688fe`, `FUN_10068b57` (2); `MW2/src/shots.c`: `SpawnEffectEx`; `MW2/src/simmain.c`: `SimMain`; `MW2/src/view.c`: `UpdateProjection` (2)
- An edition's own definition of a function, or functions only some editions' objects have (4): `MW2/src/animation.c`: the definition of `FUN_1002cfd0`, the definition of `FUN_1002d760`; `MW2/src/simmain.c`: the definition of `SimMain`, the definition of `SimWindowProc`
- A fragment included at the position of the object that holds it (1): `MW2/src/targeting.c`: file scope
- Data and declarations that differ (9): `MW2/include/accel/a3d.h`: file scope; `MW2/include/accel/a3dvertex.h`: file scope; `MW2/include/animation.h`: file scope; `MW2/src/animation.c`: file scope; `MW2/src/bwdkeywords.c`: file scope; `MW2/src/damagepanel.c`: file scope; `MW2/src/refreshmode.c`: `g_displayBackends`, `g_refreshModes`; `MW2/src/simmain.c`: file scope
- Includes (2): `MW2/src/refreshmode.c`: file scope; `MW2/src/simmain.c`: file scope
- Pragmas that reproduce a unit's flags (4): `MW2/src/accel/queueface.c`: file scope (2); `MW2/src/keyboard.c`: file scope (2)

<!-- generated: end -->
