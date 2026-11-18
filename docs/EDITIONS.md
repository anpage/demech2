# The editions of the simulator

MechWarrior 2's simulator (`MW2.DLL`) shipped in several editions built from one source: the
Windows 95 release this project started from, and one build per 3D card. This document records
what is known about each one and what differs between them. Its second half is generated from the
sources, so that it can't drift from them.

Three editions are decompiled today. The ATI, S3 and PowerVR editions (optimized builds with
`__fastcall` and no frame pointers, VC++ 2.x, S3's 4.x) and Ghost Bear's Legacy aren't targets yet.

| | 1.1 | Matrox | 3Dfx |
|---|---|---|---|
| reccmp target | `MW2` | `MW2MATROX` | `MW23DFX` |
| Binary | `MW2.DLL` of the freely downloadable 1.1 patch | `MW2_MATROX.DLL`: the Matrox Mystique edition's `MW2.DLL` | `MW2_3DFX.DLL`: the Diamond Monster disc's `MW2.DLL` |
| Linked | | 1996-08-22 | 1996-10-21 |
| Compiler | VC++ 4.1 (`cl` 10.10.6038) | VC++ 4.0 (`cl` 10.00, LINK 3.00) | VC++ 2.2 (`cl` 9.10) |
| Flags | `/Od /Oi /G5` | `/Od /Oi /G5`; the renderer `/Ox /G5 /Op`, the depth sort `/Ox /Oa /Op` | `/Ox /Ow /Oi /G5 /Gr /Gy`; `network.c`, `shots.c` and `mechreload.c` `/Od`; `QueueFace` unoptimized |
| C runtime | `LIBCMTD` (`/MTd`) | `LIBCMT` (`/MT`) | `LIBC` (`/ML`) |
| Draws through | DirectDraw and VFX | Matrox's `MSI95.DLL` | Glide 1.x (`GLIDE.DLL`), with 3dfx's Texus linked in |
| Identity macros | none | `MW2_ACCEL`, `MW2_MATROX` | `MW2_ACCEL`, `MW2_3DFX` |
| Game-code functions | 1537 | 1597 | 1574 |

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
edition's own are in `MW2/src/matrox` and `MW2/src/3dfx`.

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

The 3D-card editions were built from 1.1's source. The Matrox edition is the one built like 1.1
(`/Od`), and the 3Dfx edition is its C built by another compiler with optimization, so most of
what follows was found in the Matrox edition and holds for both.

- **Fixed point became float.** The Matrox edition converted the simulation's 16.16 fixed-point values to `float`: struct members at the same offsets (vectors, object positions, velocities, normals, distances, ranges, heat, throttle, bounding boxes, the eyepoint) and about 180 globals, read with `fld` where 1.1 shifts (`x >> 16` becomes a conversion through `__ftol`). This, not changed layouts, is why most of its changed functions differ; the conversions are uniform enough to suggest the original source switched types and conversion macros with a build flag.
- **The A3D layer.** The Matrox edition's renderer (`0x1005a5f0`–`0x10067500`, every caller of `MSI95.DLL` but `SimMain`) is one object of the project, compiled by the same VC++ 4.0 with `/Ox /G5 /Op` where the other units have `/Od`: it is linked between `damagepanel.c` and `keyboard.c`, with one run of `.rdata`, `.data` and `.bss` of its own, and it calls the game's functions (the resource cache, the debug log). Its names (`A3D_map_polygon`, `A3D_polygon_clip_XY_and_render`, `A3D_TEXTURE_fill`) mirror VFX's polygon API, and the ATI and S3 editions carry their own `A3D` layers with the same API: the game's own accelerated replacement for VFX, one back end per card. What looks hand-tuned is `/G5`'s Pentium scheduling: a load and `test` where `/Od` writes `cmp [g], 0`, interleaved prologues, `nop`s (`npad 1`) before a `push`, registers renamed through `ebx`. `/Ox` rather than `/O2`: functions pad to 16 with `npad` (no `/Gy` COMDATs) and identical strings stay separate (no `/Gf`); `/Op` gives its float stores through a stack temporary. 4.0 specifically: one function (`0x1005f790`) compiles differently under 2.2, 4.1 and 4.2. The Matrox edition's depth-sort object (`depthsort.c`'s functions, `0x100232f0`–`0x1002390b`) is optimized too, with `/Ox /Oa /Op`: `/Oa` keeps globals in registers across a store through a pointer. The build compiles `MW2/src/matrox/a3d.c` and `depthsort.c` with those flags.
- **The 16-bit 2D primitives.** The game code doesn't draw through VFXA in the Matrox edition: its calls to `VFX_line_draw`, `VFX_shape_draw`, `VFX_shape_visible_rectangle`, `VFX_pane_wipe`, `VFX_ellipse_draw` and `VFX_string_draw` go to a 16-bit port of VFXA (`0x100563dc`–`0x100591a0`, after `loadres.c`: ML frames, VFXA's routines in VFXA's order, storing 16-bit pixels), which the ATI and S3 editions carry too. The 3Dfx edition carries it too. `MW2/include/accel/vfx16.h` declares them and, under `MW2_VFX16`, redirects the VFXA names to them: a drawing unit includes it after `vfxa.h`, behind `#ifdef MW2_VFX16`. VFXA stays linked for its other routines (`supanim.c`'s offscreen panes still draw through it). The object is `MW2/src/accel/vfx16.asm`, a MASM unit generated by `tools/asm2masm.py --target MW2MATROX`, with VFXA's names plus a `16` suffix (VFXA's own publics are linked alongside) and VFXA's data layout; only the accelerated editions' ML builds assemble it, so it has no portable C.
- **VFX.** The three VFX objects are laid out the same in every edition. The accelerated editions
  link VFX 1.15 as released where 1.1's was modified (`MW2_VFX_STOCK`), and only
  `VFX_polygon_render` and `VFX_polygon_clip_XY_and_render` differ between the two: the Matrox
  edition doesn't set `ES` and reads `poly_vectors` through `CS` (`MW2_VFXREND_CS_SEGMENT`), the
  3Dfx edition keeps 1.1's code.
- **Object order.** `vc40/targets.cmake` lists MW2's units in the Matrox edition's object order, found from the functions and data they share. The units with nothing found there (fixed-point helpers, and the units the renderer replaced: `directdraw.c`, `texpoly.c`, `horizon.c`…) are linked after the others until their callers' Matrox versions are written. `tools/check_units.py --target MW2MATROX` passes, with the Matrox edition's mixed objects not split yet listed in `tools/check_units_expected.txt`.

## The 3Dfx edition

- **Flags.** `/Ox /Ow /G5 /Gr /Gy` reproduce the original byte for byte (`LoadSndCfg`, `LoadDifficultyCfg`, `KnockMechOver` and about 1000 more from today's sources); `network.c`, `shots.c` and `mechreload.c` are `/Od` (still `/Gr`). `/Ow` (no aliasing except across function calls), not `/Oa`: the original reloads what a pointer reaches after every call (`KnockMechOver`'s `p_mech->m_player`, `TestPointUnderFloor`'s model, `CycleTarget`'s target id after each `Target*` call), which `/Oa` keeps in a register, and keeps the stores to a local whose address a call takes as an integer (`GetCdAudioPosition`'s `MCI_STATUS_PARMS`), which `/Oa` drops; without either, it reloads after stores through pointers too. Moving every unit from `/Oa` to `/Ow` raised about 100 functions and lowered only `QueueFace`, by 0.1%. `vc22/targets.cmake` sets `DEMECH2_OPTIMIZATION` on every other source, which the `vc22/` sub-project honours like the 4.x builds (`/Od` is a compile option there, not one of `CMAKE_<LANG>_FLAGS`). Its CRT is 2.2's single-threaded `LIBC` (`/ML`): the original imports no TLS or critical section functions, and its `fclose` takes no lock.
- **Texus.** Texus (`3rdparty/texus`) is 3dfx's texture library, which the renderer converts the textures with: `swlibs/texus/lib` of Debian's `glide` 2002.04.10 (the 1999 Glide source release), the 11 files the DLL links, under its own licence (`glide_license.txt`, the 3DFX GLIDE Source Code General Public License, not the repository's LGPL; see its `README`). Changes to the source sit between dated `demech2: begin change` / `end change` comments, like `3rdparty/vfx`'s. 3dfx built it, `__cdecl`, with VC++ 2.2 at `/Ox /G5 /QIfdiv-` (no `/Gy`: its functions pad with `npad`; no FDIV check), as a static library (`3rdparty/texus/texus.cmake`) the target links first. `MW2/library_texus.h` annotates it (`LIBRARY`, outside the game-code `--total`): it starts at `0x10068270`, where the calling convention changes, and ends at the import thunks (`0x1006e8b0`). About half of its 101 functions match; the rest is mostly register allocation, which depends on the symbols a file declares (its headers stay verbatim).
- **The renderer and the display back end.** `MW2/src/3dfx/a3d.c` (`0x10066570`-`0x100680b2`, after `accel/graphicsmenu.c`) draws through Glide: its vertex is Glide's `GrVertex` (`A3DVertex` under `MW2_3DFX`, the Matrox vertex's members keeping their names), its textures VFX `WINDOW`s of 16-bit texels, converted through Texus and cached, or read converted from `MW2.MIP`, which the game writes on its first run; it writes the 2D panels straight into Glide's frame buffer. `MW2/src/3dfx/glidedraw.c` (`0x100059a0`-`0x10005b65`, after `gdi.c`) is the display back end and refresh modes, `directdraw.c`'s functions one for one, in DirectDraw's slot of `g_displayBackends`/`g_refreshModes`. The edition has no key color fills (nor the `0xfffe` key-colour wipes of the panels, `ClipRectToPane` and the visible-rectangle bounds of `DrawHudShape`: those are `MW2_KEY_COLOR`, the Matrox edition's), and wipes the 3D view's panes with VFX's 16-bit `VFX_pane_wipe`. Its pixels are RGB555 where the Matrox edition's are RGB565 (`PIXEL_RGB16`, the palette in `ApplyPaletteResource`), and it reads no `LOD.PAR`: the detail levels are 1.1's constants. The renderer's `.bss` variables are `static` (their order is the object's), and 2.2 lays statics out in an order of its own: a pointer just past an array names a different neighbour on either side.
- **Data.** What the 3Dfx edition changed (found by datacmp): the map colors (`targeting.c`) are other palette entries in both accelerated editions; the torso pan and throttle ranges of `g_inputSinks` (`inputmap.c`) are the 3Dfx edition's own; its graphics page lists a fourth item, a "Resolution 800x600" setting in the texture filter's place (`accel/graphicsmenu.c`; the "Res800x600" registry value, which `SimMain`'s settings pass to the renderer for `grSstOpen`). At 800x600 the menus keep their 640x480 size, scaling their points by eight tenths of the frame (`ScaleMenuPointToFrame`), and `DrawAnim2d` tiles animation 0 across its pane (`TileShapeFrame`). The resource type tags and extensions are one `char*` global each where 1.1 has two arrays: 2.2 lays an array's strings out ahead of it, and the original alternates strings and pointers.

## Adding an edition

What a new 3D-card edition of the simulator needs (ATI, S3, PowerVR), in the order that worked
for the Matrox and 3Dfx editions. Each step names the files that list the editions today: none of
them finds a new one by itself. A later revision of the game (Ghost Bear's Legacy, 1.0) is another
question, to settle before this list applies: if most of its functions pair with 1.1's it joins
the tree the same way, with features named for what it changed; if not, it gets a tree of its own.

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
- **The total.** The game-code function count is the `--total` in `verify-retail.yml`, the README
  and `CLAUDE.md`: Ghidra's count below the C runtime for an `/Od` build; for a `/Gy` build
  `tools/edition.py`'s `functions` command, which traces the code (Ghidra missed 261 of the 3Dfx
  edition's 1574).
- **The rest by name:** the C runtime in `MW2/library_msvc.h`, the VFX objects' static data in
  `reccmp/<target>-vfx.csv` (`tools/vfx_names.py`; the three objects are laid out the same in
  every edition so far), and by-name markers in the unit's header for a function the edition
  places out of its file's order.
- **Placement.** The object order goes into the target's source list, found from the functions
  and data the edition shares with the others. Code it places in another object than 1.1 takes a
  fragment, included at each edition's position under that edition's macro. Until the units are
  placed, the target skips `tools/check_units.py` in `verify-retail.yml`, as `MW23DFX` does.

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
features: the third and fourth editions are what can tell. When a site's difference has no name
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
- **Elsewhere:** `CLAUDE.md` (the target list, the build table, the reccmp commands and totals,
  and the working rules its compiler adds), the README (the progress badge, the requirements, the
  build table) and the progress page's title in `tools/states_page.py`.

## Keeping this document

Everything below the marker is written by `python tools/edition_sites.py --doc` (from the
repository root, with a configured build tree: which target compiles which branch comes from the
targets' own compile commands). CI runs `--check`, which fails when the document is stale, when a
feature has no comment above its `#define`, when nothing tests a feature, when a conditional
tests an `MW2_` macro the table doesn't define, and when no target compiles a branch.

<!-- generated by tools/edition_sites.py --doc: begin -->

## Features

From the table in `util/types.h` (83 features) and the 859 conditional sites of the simulators' sources (`MW2/`, `common/`, `3rdparty/vfx`). A feature's cell is its value where it has one.

### The Matrox and 3Dfx editions (`MW2_ACCEL`)

Every edition that has one of these has them all, so no binary separates one from another: which of them a site belongs to is a reading of what its code does.

| Feature | 1.1 | Matrox | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|--:|---|
| `MW2_FLOAT_SCALARS` |  | yes | yes | 349 | *Representation.* The simulation's 16.16 fixed-point values are floats (MechScalar, FIXED_*) |
| `MW2_A3D` |  | yes | yes | 67 | The A3D renderer interface and its records: queued polygons, projected vertices, depth lists |
| `MW2_VERTEX_SHADING` |  | yes | yes | 13 | Per-vertex shading for the renderer: WTB normals and colours, shaded projected vertices |
| `MW2_VFX16` |  | yes | yes | 35 | The 2D drawing goes to the 16-bit port of VFXA |
| `MW2_VFX_STOCK` |  | yes | yes | 8 | VFX as released (unaligned fills and runs, word-wise character copies), where 1.1 modified it; the assembly gets it from cmake/editions.cmake |
| `MW2_VFX_LOOKASIDE_WRITE` |  | yes | yes | 2 | VFXA's VFX_lookaside_write, which sets one lookaside entry (cmake/editions.cmake) |
| `MW2_LARGER_POLYGON_BUFFERS` |  | yes | yes | 1 | A larger draw buffer and polygon limit |
| `MW2_NO_DISSOLVE` |  | yes | yes | 1 | StartPalettes blends through the display back end, without 1.1's pixel dissolve |
| `MW2_DISPLAYDIB_DISABLED` |  | yes | yes | 1 | DispDibBegin always fails |
| `MW2_REFRESH_MODE_RETRY` |  | yes | yes | 2 | InitRefreshMode tries a mode without checking it's available, and marks it |
| `MW2_RAW_SCREENSHOTS` |  | yes | yes | 3 | Screenshots are the screen's raw 24-bit pixels (MW2NNNN.888), not GIFs |
| `MW2_GRAPHICS_MENU` |  | yes | yes | 6 | The graphics page, in place of 1.1's monitor brightness page |
| `MW2_REGISTRY_SETTINGS` |  | yes | yes | 2 | The sky, the ground and the renderer's settings are read from the registry |
| `MW2_DEFAULT_DRAW_MODE` |  | yes | yes | 1 | The draw mode and its parameter start at 0 (1.1: -1 and 1) |
| `MW2_HUD_ART` |  | yes | yes | 10 | The HUD art: its resolution chosen before the 512x384 override, and the shapes' numbering |
| `MW2_HUD_SHAPE_IDS` |  | yes | yes | 6 | HUD shape resource ids 3 higher (the pause banner, the target panel, the nav icons) |
| `MW2_SLIDER_SHAPES` |  | yes | yes | 1 | The menu sliders' shapes are 3 higher |
| `MW2_MENU_BACKGROUND_0X118` |  | yes | yes | 1 | The main menu's background is shape 0x118, not 0x112 |
| `MW2_PRJ_CEL_IDS` |  | yes | yes | 1 | The resource archive numbers the preloaded cels differently |
| `MW2_BRIGHT_MAP_COLORS` |  | yes | yes | 3 | The maps' colours are other (brighter) palette entries |
| `MW2_COCKPIT_POINT_ONLY` |  | yes | yes | 3 | The cockpit layout keeps only its point, and SetRes scales no gauge pane or point |
| `MW2_UNRESOLVED_GAUGE_INDICES` |  | yes | yes | 2 | LoadCockpitLayout doesn't map the gauge indices to functions |
| `MW2_DEBUG_OVERLAY_COLOR` |  | yes | yes | 1 | The debug overlays' text is colour 0x52, not 0x4f |
| `MW2_NO_EYE_POSITION_OVERLAY` |  | yes | yes | 1 | No eye position overlay |
| `MW2_NO_BOUNDING_SPHERES` |  | yes | yes | 1 | No debug drawing of the shapes' bounding spheres |
| `MW2_INFRARED_OFF_FADE` |  | yes | yes | 1 | Turning infrared off fades back to the base palette |
| `MW2_FADE_BACK_DURATION` |  | yes | yes | 1 | The palette's fade back takes the duration as its step count |
| `MW2_TIME_OF_DAY_FADE_ONCE` |  | yes | yes | 1 | FadeToTimeOfDayPhase fades only while no phase is set |
| `MW2_FACE_KIND_BIT15` |  | yes | yes | 3 | Bit 15 of a face's id is part of its kind, not a flag for MapFaceId's remap |
| `MW2_WTB_NEGATED_NAMES` |  | yes | yes | 1 | LoadShapeRecord decodes negated shape names |
| `MW2_MODEL_MIN_ONE_FACE` |  | yes | yes | 1 | AddModel allocates at least one face |
| `MW2_GRID_CELL_XZ` |  | yes | yes | 5 | The grid object's anchor and cell keep only x and z |
| `MW2_NO_PROJECTION_OFFSET` |  | yes | yes | 3 | The eyepoint has no projection offset |
| `MW2_FRUSTUM_HEIGHT_SIGN_FROM_SIDE` |  | yes | yes | 1 | The vertical frustum limit takes its sign from the side distance |
| `MW2_NO_RAY_BOX_ENTRY_BEHIND` |  | yes | yes | 1 | TestRayBox doesn't record whether the ray entered behind |
| `MW2_AIMED_SHAPE_RESET` |  | yes | yes | 2 | DestroyObjShape clears the aimed shape when it destroys it |
| `MW2_BUILDING_BOUNCE` |  | yes | yes | 1 | The bounce off buildings isn't negated, and is halved either way |
| `MW2_COLLISION_DAMAGE_X2` |  | yes | yes | 1 | Damage from below scales by twice the mass ratio, not four times |
| `MW2_RADIUS_HEAD_DAMAGE_HALF` |  | yes | yes | 1 | Radius (fire) damage to the head is half, not a quarter |
| `MW2_NO_ENGINE_NOTE` |  | yes | yes | 1 | StartEngineNote returns at once |
| `MW2_LEADER_NEAR_TARGET` |  | yes | yes | 3 | IsTeamNearTarget tests only the team's leader |
| `MW2_LOCAL_STAR_FROM_ZERO` |  | yes | yes | 2 | The local star starts at 0, and the player's formation goes to it |
| `MW2_NETWORK_STAR_PLAYERS` |  | yes | yes | 2 | UpdateObjective takes star N as player N in network games |
| `MW2_RESTART_KEEPS_LISTED` |  | yes | yes | 1 | Restarting a star mission keeps its objectives' targets' listed flags |
| `MW2_JOYSTICK_OPEN_POSEX` |  | yes | yes | 2 | JoystickOpenDevice probes with joyGetPosEx alone, on an uncleared JOYINFOEX |
| `MW2_JOYINFOEX_UNCLEARED` |  | yes | yes | 1 | JoystickPoll neither clears its JOYINFOEX nor sets its size |
| `MW2_AXIS_RATE_POINTER_TEST` |  | yes | yes | 1 | UpdateInputs compares an axis' rate pointer with its position's address |
| `MW2_NO_FULLSCREEN_KEY` |  | yes | yes | 1 | Key code 0x40d no longer toggles full screen |
| `MW2_LEMMY_CHEAT` |  | yes | yes | 1 | The "lemmy" cheat |
| `MW2_NO_DORCS_CHEAT` |  | yes | yes | 1 | 1.1's "dorcs" cheat is gone |
| `MW2_EXTRA_DEBUG_PRINTS` |  | yes | yes | 5 | Debug output both accelerated editions add |

### A value per edition: the Matrox and 3Dfx editions (`MW2_MATROX`, `MW2_3DFX`)

| Feature | 1.1 | Matrox | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|--:|---|
| `MW2_PIXEL_FORMAT` |  | 565 | 555 | 2 | The 16-bit pixels and the palette: RGB565 (Matrox); The 16-bit pixels and the palette: RGB555 (3Dfx) |

### The Matrox edition (`MW2_MATROX`)

| Feature | 1.1 | Matrox | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|--:|---|
| `MW2_KEY_COLOR` |  | yes |  | 47 | Key colour fills and 0xfffe wipes for the renderer's overlay, ClipRectToPane |
| `MW2_VFXREND_CS_SEGMENT` |  | yes |  | 3 | VFXREND's polygon renderer reads its table through CS and leaves ES as it is (cmake/editions.cmake) |
| `MW2_LOD_PAR` |  | yes |  | 11 | LOD.PAR's LOD_SCALE scales the detail scale |
| `MW2_TEXTURE_FILTER` |  | yes |  | 7 | The "Filter" registry setting, passed to the renderer |
| `MW2_DEPTH_ENTRY_TAG` |  | yes |  | 2 | A depth entry is 0xc bytes, a dword ahead of the polygon |
| `MW2_RES_640X400` |  | yes |  | 1 | 640x400: SimMain sets it instead of SetGameResolution, and the art follows |
| `MW2_DETAIL_LEVEL_IN_LOADER` |  | yes |  | 2 | LoadClassLevel records the player's detail level itself |
| `MW2_SKIP_LOADED_LEVEL` |  | yes |  | 1 | LoadClassEntryShape returns early when the entry is loaded at the level |
| `MW2_SHOT_DAMAGE_AFTER_RECORD` |  | yes |  | 2 | A shot's heat and damage are applied after RecordAttack |
| `MW2_POWER_STATE_7_KEYS` |  | yes |  | 3 | Both power keys do nothing in power state 7, tested first (1.1: key 0x3e only) |
| `MW2_JOYSTICK_AXIS_SUMS` |  | yes |  | 2 | JoystickPoll sums the axes between polls |
| `MW2_FULLSCREEN_SAVES_MODE` |  | yes |  | 1 | ToggleFullScreen records the current mode as the fastest |
| `MW2_MOUSE_PEEK_RETURNS` |  | yes |  | 1 | HandleMessages returns when its mouse-skipping PeekMessage finds nothing |
| `MW2_STDCALL_TIMER_CALLBACK` |  | yes |  | 2 | SupAnimTimerCallback is __stdcall and takes the timer's argument |
| `MW2_DEBUG_LOG_TEXT` |  | yes |  | 3 | The debug log's messages end in newlines, with prints of its own |

### The 3Dfx edition (`MW2_3DFX`)

| Feature | 1.1 | Matrox | 3Dfx | Sites | What it does |
|---|:-:|:-:|:-:|--:|---|
| `MW2_RES_800X600` |  |  | yes | 16 | The "Res800x600" setting: 800x600, the menus scaled, animation 0 tiled |
| `MW2_ART_RESOLUTION_NEAREST` |  |  | yes | 1 | ChooseArtResolution takes the nearest of the three sizes |
| `MW2_SKY_TILE_DEFAULTS` |  |  | yes | 2 | The sky and ground textures' tiling defaults: 4 x 0.8, and 20000 |
| `MW2_SKY_GROUND_COLORS` |  |  | yes | 1 | DrawSkyAndGround sets the sky and ground palette entries |
| `MW2_MESSAGE_BACKGROUND_SHAPES` |  |  | yes | 1 | Message boxes' backgrounds are shape 0x4f, not 0x4c |
| `MW2_OUTLINE_CLEARS_FLAGS` |  |  | yes | 1 | The wireframe outline clears flag bits 12, 14 and 15 |
| `MW2_NARROW_TORSO_PAN` |  |  | yes | 1 | The torso pan's range is -90 to 90 |
| `MW2_NO_REVERSE_THROTTLE` |  |  | yes | 1 | The throttle's range is 0 to 1024 |
| `MW2_AI_TILT_DEGREES` |  |  | yes | 2 | The AI's torso raises for firing convert from 16.16 degrees |
| `MW2_HIT_FEEDBACK_FLAG_CHECK` |  |  | yes | 1 | No hit feedback for the local player while its flag 2 is set |
| `MW2_NET_RETURN_ZERO` |  |  | yes | 2 | FindSession and StopExternalIO return 0 where the others fall off the end |
| `MW2_NO_NET_DEBUG_PRINTS` |  |  | yes | 3 | NetSend and NetReceive log nothing |
| `MW2_NO_STATE_7_POWER_TEST` |  |  | yes | 1 | Key 0x3e asks for power down in power state 7 too, where 1.1 ignores it |
| `MW2_LAYOUT_GAUGE_FUNCTIONS` |  |  | yes | 4 | The map layouts hold their gauge functions from the start, not indices in g_cockpitGauges |
| `MW2_INLINE_HELPERS` |  |  | yes | 12 | *Artefact.* The edition's headers define FixedMul16, FixedDiv16, MulDiv64, HashName and DebugBreakpoint __inline (its /Ox expands them in every caller), and MemCopy and MemSet are C |
| `MW2_DEBUG_LOG_FORMATS` |  |  | yes | 2 | The debug log's own formats (the sky's colours on one line, the armor as %f) |

## Where each feature is tested

- `MW2_FLOAT_SCALARS`: `MW2/include/ammobin.h`: `AmmoBin`; `MW2/include/bandpoly.h`: file scope (2); `MW2/include/classentry.h`: file scope; `MW2/include/clock.h`: file scope (2); `MW2/include/cockpit.h`: file scope; `MW2/include/eyepoint.h`: `Eyepoint`; `MW2/include/filevector3.h`: file scope; `MW2/include/filexform.h`: file scope; `MW2/include/fixedfloat.h`: file scope; `MW2/include/fixedtrig.h`: file scope (2); `MW2/include/inradius.h`: the definition of `IsWithinRadius`; `MW2/include/integrate.h`: file scope; `MW2/include/maneuvers.h`: file scope; `MW2/include/mapview.h`: file scope; `MW2/include/projectedvertex.h`: file scope; `MW2/include/queuedpolygon.h`: `QueuedPolygon`; `MW2/include/ramp.h`: file scope (3); `MW2/include/screenscale.h`: file scope; `MW2/include/shape.h`: file scope; `MW2/include/targeting.h`: file scope; `MW2/include/vertex.h`: `Vertex` (2); `MW2/include/view.h`: file scope; `MW2/include/weaponslot.h`: `WeaponBin`, file scope (2); `MW2/src/ai.c`: `AddClamped`, `AiStateMove` (2), `FindNearestTarget`, `GetApproachThrottle`, `SteerToTarget`, `SweepTorso` (2); `MW2/src/aiweapons.c`: `DecideAIFire`; `MW2/src/bandpoly.c`: file scope; `MW2/src/bargauges.c`: `DrawHeatBar`, `DrawHeatRateBar`, `DrawThrottleGauge`; `MW2/src/brightnessmenu.c`: `GetBrightnessFraction`; `MW2/src/classtable.c`: `ChoosePlayerDetailLevels`; `MW2/src/clock.c`: the definition of `Hypot2D`, the definitions of `InitSinAtanTables`, `InitSlopeTables`; `MW2/src/cockpit.c`: `DrawMapContents` (3), `DrawMapFieldOfView` (3), `DrawMapNavPoints` (3), `DrawMapTarget` (3), `DrawMapUnits` (3), `DrawMapViewText` (5), `GetMapHeightShade`, `ZoomMapView` (3); `MW2/src/cockpitpanels.c`: `UpdateCockpit`; `MW2/src/collision.c`: `IntersectRayFace`, `IsBelowFacePlane`, `IsPointInFace`, `IsPointInFaceXY` (2), `IsPointInFaceXZ` (2), `IsPointInFaceYZ` (2), `TestSegmentCollision`; `MW2/src/config.c`: `LoadReels` (2); `MW2/src/damagepanel.c`: `DrawArmorBars` (4), `DrawDamageOutline` (2), file scope; `MW2/src/debris.c`: `BlowOffChunk`, `DamageChunksInRadius`, `PushDebrisPiece`, `ThrowDebrisPiece`, `UpdateDebrisPiece` (4); `MW2/src/depthsort.c`: `QueueShapeLod`; `MW2/src/door.c`: `LateUpdateDoor` (2), `UpdateDoor`; `MW2/src/environment.c`: `FirstEnvironment`, `QuadrupleMechCooling`, `QuarterMechCooling`; `MW2/src/eyepoint.c`: `GetCameraFloor`, `RestoreView`, `SaveView`, `UpdateEyepoint` (2), `UpdateFreeEyeView`, `UpdateTrackView`; `MW2/src/fadepal.c`: `EmitWreckSmoke`, `FlashZappedPaletteLevel`, `PlayPlayerHitFeedback`; `MW2/src/fixedsqrt.c`: the definition of `NormalizeVectorGuarded`; `MW2/src/gamekeys.c`: `RunGameKey`; `MW2/src/geocache.c`: `ApplyBlockXform`, `BeginBlock`, `LoadQuadtreeBoxes`, `ToggleBlockBoxes`; `MW2/src/gpanim.c`: `UpdateMotion`; `MW2/src/horizon.c`: `HorizonXAtY`, `HorizonYAtX`, `IsAboveHorizon`; `MW2/src/hud.c`: `DrawAltimeter` (6), `DrawCompass`, `DrawCompassMarkers`, `DrawObjectBrackets`, `DrawPlayerBrackets`, `InitHudGauges` (2); `MW2/src/inputmap.c`: `UpdateAxisFromKeys`, `UpdateInputs` (2); `MW2/src/inradius.c`: the definition of `IsWithinRadius`; `MW2/src/integrate.c`: the definition of `IntegrateMidpoint`; `MW2/src/maneuvers.c`: `AvoidObstacles`, `BuildProbeRay`, `CanJump`, `ChooseFlankPlace`, `DodgeShot` (2), `GetAvoidSide`, `GetHeadingTo`, `GetOffsetPoint` (4), `IsSharpTurn`, `IsStandableShape`, `JumpToTurn`, `ManeuverAchick` (2), `ManeuverAdfa` (2), `ManeuverBehind`, `ManeuverStupid` (2), `PlaceOffsetNav`, `RunManeuver`, `StartManeuver`, file scope; `MW2/src/mapicon.c`: `DrawMapIcon` (4); `MW2/src/mapview.c`: `BeginMapView` (2), `CullMapViewShape` (3), `ProjectMapPoint` (2), `ProjectMapViewVertex`, the definition of `ProjectMapCoordinate`; `MW2/src/mechclass.c`: `FirstMech`, `LateUpdateMech` (8), the definition of `UpdateMech`; `MW2/src/mechcollision.c`: `ApplyCollisionDamage`, `CollideWithBuildings` (2), `CollideWithMechs` (6), `DamageMechHittingShape`, `DamageMechsInCollision`, `MoveMechWithCollisions` (3); `MW2/src/mechdamage.c`: `ApplyDamageToMech` (3), `CalculateHeat` (2), `DestroyCriticalSlot`, `PunchInAutoHeading`, `RunAutopilot` (2); `MW2/src/mekfile.c`: `LoadMechConfig` (5); `MW2/src/network.c`: `ReceiveCollisionMsg`, `ReceiveStateMsg`, file scope (2); `MW2/src/normalizerotation.c`: `NormalizeRotation`, `ScaleVectorToLength`; `MW2/src/objectanim.c`: `OrbitTask` (4), `PathTask` (2), `SpinTask` (2); `MW2/src/objective.c`: `DoFirstObjtv`; `MW2/src/palette.c`: `StartPaletteFade`; `MW2/src/quadtree.c`: `TestQuadtreeRay`; `MW2/src/ramp.c`: `SetWrappedRampTarget`, `UpdateWrappedRamp` (3), file scope, the definitions of `StartFloatRamp`, `UpdateFloatRamp`; `MW2/src/ray.c`: `AdvanceRayStart`, `BuildRayFixed`, `BuildRayFloat`, `BuildRayFromDirection`, `ClipRaySlab`, `ClipRayToGround`, `SetRayLength`; `MW2/src/recordstacks.c`: `InitializeDrawBuffer`; `MW2/src/resource.c`: `CreateObjectNode`, `PathRecord`, file scope; `MW2/src/screenscale.c`: `DrawGaugeEllipse`, `GetEllipseEdgeAtAngle` (2), `GetEllipseNeedleAt` (2), `GetEllipseNeedleToward`, `GetLineSlope` (2), `GetRectEdgeAtSlope`, `GetRectNeedleAt` (2), `IsInsideGaugeEllipse` (3), `ScaleRectToLowRes`, file scope; `MW2/src/setres.c`: `SetPixelAspect`; `MW2/src/shape.c`: `AddShapeVertex`, `ComputeFaceNormal` (2); `MW2/src/shapebounds.c`: `ComputeShapeBounds` (2); `MW2/src/shapecollision.c`: `TestPointAboveFaces`; `MW2/src/shapegeom.c`: `ApproximateShapeDistance`, `RayShapeDistance`, `TransformModel`, `TransformShapeCenter`, the definition of `ComputeTriangleNormal`, the definition of `DivDifference17`, the definition of `SolvePlaneY`; `MW2/src/shots.c`: `DamageMechsInRadius`, `DamageThingsInRadius`, `GuideMissileToTarget` (2), `HeatMechsNearFires`, `SwayShot`, `UpdateShot` (3); `MW2/src/soundfx.c`: `CalculateSamplePan`, `PlayPositionalSound` (3), `UpdateAmbientSound` (2); `MW2/src/statuspanels.c`: `DrawSpeedPanel`; `MW2/src/targeting.c`: `UpdateTarget`, file scope; `MW2/src/targetpanel.c`: `DrawTargetPanel` (2), `DrawTargetPanelText`; `MW2/src/transform.c`: `BuildMatrixEx`, `GetMatrixAngles`, `MultiplyRotations`, `OrthogonalizeMatrixColumn`, `RotatePoint`, `SetIdentityMatrix`, `TransformPoint`; `MW2/src/view.c`: `CullSceneShape` (6), `CullShapeToFrustum` (5), `ProjectWorldPoint` (2), `SelectEyepoint`, `SetFarPlane`, `SetNearPlane`, `UpdateProjection`, file scope; `MW2/src/weapons.c`: `GetEyeAimDirection` (2), `SpawnLaunchFx` (3), `SpawnShot`, `UpdateAimDistance`, `UpdateWeaponLock`; `MW2/src/world.c`: `BwdExecuteStream` (6); `MW2/src/wtbshapes.c`: `LoadShapeRecord` (3), `SetShapeScale`
- `MW2_A3D`: `MW2/include/animation.h`: file scope (3); `MW2/include/animframe.h`: `AnimFrame`; `MW2/include/bandpoly.h`: file scope (2); `MW2/include/cockpit.h`: file scope; `MW2/include/objectanim.h`: file scope; `MW2/include/palette.h`: file scope; `MW2/include/polydraw.h`: file scope (4); `MW2/include/queuedpolygon.h`: `QueuedPolygon`; `MW2/include/rendersettings.h`: `RenderSettings` (2); `MW2/include/shape.h`: file scope; `MW2/include/vertex.h`: `Vertex`; `MW2/src/animation.c`: `DrawAnimatedPolygon`, file scope (4), the definition of `RotateTexturePoint`, the definitions of `CalcAverageBitmapColors`, `FUN_100519b0`, `FUN_10051ba1`, `FUN_10051c07`, `FUN_10051dfe`, `FUN_10051ea2`; `MW2/src/bandpoly.c`: `DrawBand` (2), the definition of `DrawBandPolygon`, the definition of `DrawBandWithFlash`; `MW2/src/cockpit.c`: `DrawMapView`, `SatelliteDrawPolygon` (2), file scope, the definition of `FUN_100751a9`; `MW2/src/depthsort.c`: file scope (2), the definition of `DrawQueuedPolygon`, the definitions of `ClipEdgeToColumn`, `ClipEdgeToRow`, `ClipPolygonToScreen`, `DrawQueuedPolygon`, `GetEdgeCrossingDepth`; `MW2/src/faceshade.c`: `GetFaceColor`; `MW2/src/fadepal.c`: `RenderViewToPane` (2); `MW2/src/objectanim.c`: the definitions of `ClipEdgeToNearPlane`, `Difference`, `GetFaceShade`, `GetViewVertex`, `Interpolate`, `Product`, `ProjectAxis`, `ProjectVertex`, `QueueFace`, `Sum`, `ViewRow`; `MW2/src/palette.c`: `ApplyPaletteResource`, `StartPaletteFade`; `MW2/src/polydraw.c`: `DrawPentagon`, `DrawQuad`, `DrawScenePolygon` (2), `DrawTriangle`, `g_renderSettings`, file scope (3), the definition of `DrawSkyAndGround`, the definition of `FUN_10067b90`, the definitions of `DrawLineTo`, `DrawPolygonOrLine`, the definitions of `FUN_10068856`, `FUN_100688fe`, `FUN_100689f5`, `FUN_10068b57`, `FUN_10068e33`, `FUN_10069172`, `FUN_100692db`, `LoadSkyGroundSettings`, `ReadTextLine`; `MW2/src/recordstacks.c`: `AllocProjectedVertex`, `AllocQueuedPolygon`, `InitializeDrawBuffer` (2); `MW2/src/render.c`: file scope (2); `MW2/src/shape.c`: `AddShapeVertex` (2); `MW2/src/simmain.c`: file scope; `MW2/src/targetpanel.c`: `DrawTargetPanel`
- `MW2_VERTEX_SHADING`: `MW2/include/projectedvertex.h`: `ProjectedVertex`; `MW2/include/wtbface.h`: file scope; `MW2/include/wtbvertex.h`: file scope; `MW2/src/shapegeom.c`: `TransformModel`; `MW2/src/wtbshapes.c`: `LoadShapeRecord` (8), file scope
- `MW2_VFX16`: `MW2/include/accel/vfx16.h`: file scope; `MW2/include/menu.h`: file scope; `MW2/include/palette.h`: file scope (2); `MW2/src/anim2d.c`: file scope; `MW2/src/bargauges.c`: file scope; `MW2/src/cockpit.c`: file scope; `MW2/src/damagepanel.c`: `DrawDamageOutline`, `InitDamagePanel`, file scope; `MW2/src/depthsort.c`: file scope; `MW2/src/hud.c`: file scope; `MW2/src/mapicon.c`: file scope; `MW2/src/mechviewpanel.c`: file scope; `MW2/src/menu.c`: file scope (2); `MW2/src/menucontrols.c`: file scope; `MW2/src/overlay.c`: file scope; `MW2/src/palette.c`: `ApplyPaletteResource` (2), file scope; `MW2/src/palidentity.c`: `ResetTextColors`; `MW2/src/pausebanner.c`: file scope; `MW2/src/render.c`: file scope; `MW2/src/screenscale.c`: `DrawGaugeEllipse`, file scope; `MW2/src/simmain.c`: file scope; `MW2/src/statuspanels.c`: file scope; `MW2/src/targetpanel.c`: `DrawTargetPanel`, `DrawTargetPanelText`, file scope; `MW2/src/timedoverlays.c`: file scope; `MW2/src/weaponpanel.c`: `DrawWeaponPanel` (2), file scope
- `MW2_VFX_STOCK`: `3rdparty/vfx/VFX.INC`: `RMOVSB32`, `RSTOSB32`, `RXLAT32`; `3rdparty/vfx/VFX3D.ASM`: `VFX_flat_polygon` (2); `3rdparty/vfx/VFXA.ASM`: `VFX_character_draw`, `VFX_pane_wipe` (2)
- `MW2_VFX_LOOKASIDE_WRITE`: `3rdparty/vfx/VFXA.ASM`: file scope (2)
- `MW2_LARGER_POLYGON_BUFFERS`: `MW2/src/render.c`: `FirstRender`
- `MW2_NO_DISSOLVE`: `MW2/src/palette.c`: `StartPalettes`
- `MW2_DISPLAYDIB_DISABLED`: `MW2/src/dispdib.c`: `DispDibBegin`
- `MW2_REFRESH_MODE_RETRY`: `MW2/src/refreshmode.c`: `InitRefreshMode` (2)
- `MW2_RAW_SCREENSHOTS`: `MW2/src/config.c`: `SaveScreenshot` (2); `MW2/src/gamekeys.c`: `RunGameKey`
- `MW2_GRAPHICS_MENU`: `MW2/src/mainmenu.c`: `g_mainMenuPage`, file scope (5)
- `MW2_REGISTRY_SETTINGS`: `MW2/src/simmain.c`: file scope, the definition of `FUN_1008fad0`
- `MW2_DEFAULT_DRAW_MODE`: `MW2/src/render.c`: file scope
- `MW2_HUD_ART`: `MW2/include/setres.h`: file scope (2); `MW2/src/hud.c`: `DrawAltimeter`, file scope (2); `MW2/src/mechviewpanel.c`: file scope; `MW2/src/render.c`: `ShowBanner` (2); `MW2/src/setres.c`: `ChooseArtResolution`, file scope
- `MW2_HUD_SHAPE_IDS`: `MW2/src/pausebanner.c`: `DrawPausedBanner`; `MW2/src/targetpanel.c`: `DrawTargetPanel` (5)
- `MW2_SLIDER_SHAPES`: `MW2/src/mainmenu.c`: `g_sliderShapes`
- `MW2_MENU_BACKGROUND_0X118`: `MW2/src/mainmenu.c`: `g_mainMenu`
- `MW2_PRJ_CEL_IDS`: `MW2/src/animation.c`: `g_preloadCels`
- `MW2_BRIGHT_MAP_COLORS`: `MW2/src/targeting.c`: file scope (3)
- `MW2_COCKPIT_POINT_ONLY`: `MW2/include/cockpit.h`: file scope; `MW2/src/cockpit.c`: file scope; `MW2/src/setres.c`: `SetRes`
- `MW2_UNRESOLVED_GAUGE_INDICES`: `MW2/src/cockpit.c`: `LoadCockpitLayout` (2)
- `MW2_DEBUG_OVERLAY_COLOR`: `MW2/src/overlay.c`: file scope
- `MW2_NO_EYE_POSITION_OVERLAY`: `MW2/src/overlay.c`: `DrawDebugOverlays`
- `MW2_NO_BOUNDING_SPHERES`: `MW2/src/render.c`: `DrawScene`
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
- `MW2_JOYSTICK_OPEN_POSEX`: `MW2/src/joystick.c`: `JoystickOpenDevice` (2)
- `MW2_JOYINFOEX_UNCLEARED`: `MW2/src/joystick.c`: `JoystickPoll`
- `MW2_AXIS_RATE_POINTER_TEST`: `MW2/src/inputmap.c`: `UpdateInputs`
- `MW2_NO_FULLSCREEN_KEY`: `MW2/src/inputmap.c`: `UpdateInputs`
- `MW2_LEMMY_CHEAT`: `MW2/src/gamekeys.c`: `HandleCheatInput`
- `MW2_NO_DORCS_CHEAT`: `MW2/src/gamekeys.c`: `HandleCheatInput`
- `MW2_EXTRA_DEBUG_PRINTS`: `MW2/src/logwindow.c`: file scope, the definition of `MonoPrintHex`; `MW2/src/mechdamage.c`: `DestroyMech`, file scope; `MW2/src/recordstacks.c`: `InitializeDrawBuffer`
- `MW2_PIXEL_FORMAT`: `MW2/src/palette.c`: `ApplyPaletteResource`; `MW2/src/polydraw.c`: file scope
- `MW2_KEY_COLOR`: `MW2/include/hud.h`: file scope; `MW2/include/palette.h`: file scope; `MW2/src/cockpit.c`: `DrawMapView`; `MW2/src/cockpitpanels.c`: `DrawPanelAnim`; `MW2/src/damagepanel.c`: `DrawDamageOutline`, file scope; `MW2/src/hud.c`: `DrawAltimeter` (2), `DrawCompass`, `DrawHudShape` (2), file scope, the definitions of `ClipRectToPane`, `FUN_1001e01a`; `MW2/src/mechviewpanel.c`: `DrawMechViewPanel` (7), file scope; `MW2/src/menu.c`: `DrawAndRunMenu` (2); `MW2/src/pausebanner.c`: `DrawPausedBanner`; `MW2/src/statuspanels.c`: `DrawAutopilotPanel`, `DrawHeatPanel`, `DrawHeatRatePanel`, `DrawJetsPanel`, `DrawKillsPanel`, `DrawMascPanel`, `DrawNetworkPanel` (3), `DrawObjectivesPanel`, `DrawSpeedPanel`; `MW2/src/targetpanel.c`: `DrawTargetPanel` (7), `DrawTargetPanelText`; `MW2/src/timedoverlays.c`: `DrawTextBox`, `DrawTimedOverlays`; `MW2/src/weaponpanel.c`: `DrawWeaponPanel`, `DrawWeaponPanelStartup`
- `MW2_VFXREND_CS_SEGMENT`: `3rdparty/vfx/VFXREND.ASM`: `VFX_polygon_clip_XY_and_render`, `VFX_polygon_render` (2)
- `MW2_LOD_PAR`: `MW2/src/classtable.c`: `ChoosePlayerDetailLevels` (2), file scope; `MW2/src/targetpanel.c`: `DrawTargetPanel` (2), file scope (2), the definition of `IsPlayerInView`; `MW2/src/view.c`: `UpdateProjection` (2), file scope
- `MW2_TEXTURE_FILTER`: `MW2/src/accel/graphicsmenu.c`: file scope (4), the definitions of `GetFilterTextures`, `SetFilterTextures`; `MW2/src/simmain.c`: `FUN_1008fad0` (2)
- `MW2_DEPTH_ENTRY_TAG`: `MW2/include/depthsort.h`: `DepthEntry`; `MW2/src/depthsort.c`: file scope
- `MW2_RES_640X400`: `MW2/src/setres.c`: `ChooseArtResolution`
- `MW2_DETAIL_LEVEL_IN_LOADER`: `MW2/src/classtable.c`: `ChoosePlayerDetailLevels`, `LoadClassLevel`
- `MW2_SKIP_LOADED_LEVEL`: `MW2/src/classtable.c`: `LoadClassEntryShape`
- `MW2_SHOT_DAMAGE_AFTER_RECORD`: `MW2/src/shots.c`: `UpdateShot` (2)
- `MW2_POWER_STATE_7_KEYS`: `MW2/src/gamekeys.c`: `RunGameKey` (3)
- `MW2_JOYSTICK_AXIS_SUMS`: `MW2/src/joystick.c`: file scope, the definition of `JoystickPoll`
- `MW2_FULLSCREEN_SAVES_MODE`: `MW2/src/refreshmode.c`: `ToggleFullScreen`
- `MW2_MOUSE_PEEK_RETURNS`: `MW2/src/simmain.c`: `HandleMessages`
- `MW2_STDCALL_TIMER_CALLBACK`: `MW2/src/supanim.c`: `SupAnimTimerCallback`, file scope
- `MW2_DEBUG_LOG_TEXT`: `MW2/src/refreshmode.c`: file scope; `MW2/src/simmain.c`: `UpdatePauseState` (2)
- `MW2_RES_800X600`: `MW2/src/accel/graphicsmenu.c`: `g_graphicsPage`, file scope (4), the definitions of `GetRes800x600`, `SetRes800x600`; `MW2/src/anim2d.c`: `DrawAnim2d`, file scope, the definition of `TileShapeFrame`; `MW2/src/menu.c`: `LayoutMenu` (2), file scope, the definition of `ScaleMenuPointToFrame`; `MW2/src/simmain.c`: `FUN_1008fad0` (2), the definition of `SetGameResolution`
- `MW2_ART_RESOLUTION_NEAREST`: `MW2/src/setres.c`: the definition of `ChooseArtResolution`
- `MW2_SKY_TILE_DEFAULTS`: `MW2/src/polydraw.c`: `LoadSkyGroundSettings`, file scope
- `MW2_SKY_GROUND_COLORS`: `MW2/src/polydraw.c`: `DrawSkyAndGround`
- `MW2_MESSAGE_BACKGROUND_SHAPES`: `MW2/src/timedoverlays.c`: file scope
- `MW2_OUTLINE_CLEARS_FLAGS`: `MW2/src/depthsort.c`: `DrawQueuedPolygon`
- `MW2_NARROW_TORSO_PAN`: `MW2/src/inputmap.c`: `g_inputSinks`
- `MW2_NO_REVERSE_THROTTLE`: `MW2/src/inputmap.c`: `g_inputSinks`
- `MW2_AI_TILT_DEGREES`: `MW2/src/aiweapons.c`: `DecideAIFire` (2)
- `MW2_HIT_FEEDBACK_FLAG_CHECK`: `MW2/src/shots.c`: `UpdateShot`
- `MW2_NET_RETURN_ZERO`: `MW2/src/network.c`: `FindSession`, `StopExternalIO`
- `MW2_NO_NET_DEBUG_PRINTS`: `MW2/src/netio.c`: `NetReceive`, `NetSend` (2)
- `MW2_NO_STATE_7_POWER_TEST`: `MW2/src/gamekeys.c`: `RunGameKey`
- `MW2_LAYOUT_GAUGE_FUNCTIONS`: `MW2/src/targeting.c`: `g_largeMapLayout`, `g_satelliteLayout`, `g_smallMapLayout`, file scope
- `MW2_INLINE_HELPERS`: `MW2/include/debugbreak.h`: the definition of `DebugBreakpoint`; `MW2/include/fixeddiv.h`: the definition of `FixedDiv16`; `MW2/include/fixedmul.h`: the definition of `FixedMul16`; `MW2/include/muldiv.h`: the definition of `MulDiv64`; `MW2/include/namehash.h`: the definition of `HashName`; `MW2/src/debugbreak.c`: the definition of `DebugBreakpoint`; `MW2/src/fixeddiv.c`: the definition of `FixedDiv16`; `MW2/src/fixedmul.c`: the definition of `FixedMul16`; `MW2/src/loadres.c`: `MemCopy`, `MemSet`; `MW2/src/muldiv.c`: the definition of `MulDiv64`; `MW2/src/namehash.c`: the definition of `HashName`
- `MW2_DEBUG_LOG_FORMATS`: `MW2/src/mekfile.c`: `LoadMechConfig`; `MW2/src/polydraw.c`: `DrawSkyAndGround`

## Functions with a definition per edition

Where an edition's version of a function differs in its control flow or in more than a few statements, each version is a definition of its own; the comment above each says what that edition's does.

| Function | File | Selected by | Definitions |
|---|---|---|---|
| `ChooseArtResolution` | `MW2/src/setres.c` | `defined(MW2_ART_RESOLUTION_NEAREST)` | 3Dfx; 1.1 and Matrox |
| `DrawSkyAndGround` | `MW2/src/polydraw.c` | `defined(MW2_A3D)` | Matrox and 3Dfx; 1.1 |
| `JoystickPoll` | `MW2/src/joystick.c` | `defined(MW2_JOYSTICK_AXIS_SUMS)` | Matrox; 1.1 and 3Dfx |
| `SetGameResolution` | `MW2/src/simmain.c` | `defined(MW2_RES_800X600)` | 3Dfx; 1.1 and Matrox |
| `SimMain` | `MW2/src/simmain.c` | `defined(MW2_MATROX)` | Matrox; 3Dfx; 1.1 |
| `SimWindowProc` | `MW2/src/simmain.c` | `defined(MW2_MATROX)` | Matrox; 3Dfx; 1.1 |
| `UpdateMech` | `MW2/src/mechclass.c` | `defined(MW2_FLOAT_SCALARS)` | Matrox and 3Dfx; 1.1 |

## Sites that test an edition

What tests `MW2_ACCEL`, `MW2_MATROX` or `MW2_3DFX` itself rather than a feature, sorted by what the site holds (the tool's reading of its content).

### `MW2_ACCEL` (84 sites)

- In a function: the edition's back end, what the shared code does for it, or the same computation written another way (7): `MW2/src/aiweapons.c`: `DecideAIFire`; `MW2/src/cockpit.c`: `SatelliteFaceColor`; `MW2/src/mekfile.c`: `LoadMechConfig`; `MW2/src/ramp.c`: `SetWrappedRampTarget`, `UpdateWrappedRamp`; `MW2/src/world.c`: `BwdExecuteStream`; `MW2/src/wtbshapes.c`: `LoadShapeRecord`
- An edition's own definition of a function, or functions only some editions' objects have (7): `MW2/src/accel/queueface.c`: the definitions of `ClipEdgeToNearPlane`, `ClipProjectedEdge`, `GetFaceShade`, `GetViewVertex`, `ProjectPolygon`, `ProjectVertex`, `QueueFace`; `MW2/src/shape.c`: the definition of `GetShapeLoadFlags`, the definition of `SelectModelByKey`, the definition of `SelectNextModel`, the definitions of `FUN_1003a859`, `FUN_1003a889`, `GetModelKey`, the definitions of `GetNewShapeFlags`, `SelectFirstModel`, `SetNewShapeFlags`, the definitions of `GetShapeFace`, `GetTransformedVertex`, `GetVertexPosition`
- A fragment included at the position of the object that holds it (22): `MW2/src/ai.c`: file scope (7); `MW2/src/clock.c`: file scope; `MW2/src/cockpit.c`: file scope (3); `MW2/src/config.c`: file scope; `MW2/src/depthsort.c`: file scope (2); `MW2/src/joystick.c`: file scope (2); `MW2/src/shape.c`: file scope (4); `MW2/src/staticmem.c`: file scope; `MW2/src/transform.c`: file scope
- The same definition at another address or in another section (34): `MW2/src/cockpit.c`: file scope; `MW2/src/inputmap.c`: file scope (32); `MW2/src/recordstacks.c`: file scope
- Data and declarations that differ (10): `MW2/include/simmain.h`: file scope; `MW2/src/gdi.c`: file scope; `MW2/src/inputmap.c`: file scope (8)
- Includes (4): `MW2/include/depthsort.h`: file scope; `MW2/src/classtable.c`: file scope; `MW2/src/simmain.c`: file scope; `MW2/src/view.c`: file scope

### `MW2_MATROX` (13 sites)

- In a function: the edition's back end, what the shared code does for it, or the same computation written another way (2): `MW2/src/animation.c`: `DrawAnimatedPolygon`; `MW2/src/shots.c`: `DamageThingsInRadius`
- An edition's own definition of a function, or functions only some editions' objects have (3): `MW2/src/animation.c`: the definitions of `FUN_10051c07`, `FUN_10051dfe`, `FUN_10051ea2`; `MW2/src/simmain.c`: the definition of `SimMain`, the definition of `SimWindowProc`
- Data and declarations that differ (5): `MW2/include/accel/a3d.h`: file scope; `MW2/src/refreshmode.c`: `g_displayBackends`, `g_refreshModes`; `MW2/src/simmain.c`: file scope (2)
- Includes (3): `MW2/src/refreshmode.c`: file scope; `MW2/src/simmain.c`: file scope (2)

### `MW2_3DFX` (63 sites)

- In a function: the edition's back end, what the shared code does for it, or the same computation written another way (45): `MW2/src/accel/floatmath.c`: `InitSlopeTables` (2); `MW2/src/accel/queueface.c`: `ProjectPolygon`, `ProjectVertex` (2); `MW2/src/aiweapons.c`: `RunAIWeapons`; `MW2/src/animation.c`: `CalcAverageBitmapColors`, `DrawAnimatedPolygon`, `FUN_100519b0`, `FUN_10069591`; `MW2/src/cockpit.c`: `FUN_100751a9`; `MW2/src/horizon.c`: `HorizonYAtX` (2); `MW2/src/loadres.c`: `LoadCachedResource` (2); `MW2/src/maneuvers.c`: `InitializeManeuvers` (5); `MW2/src/mapview.c`: `BeginMapView`; `MW2/src/mechclass.c`: `UpdateMech`; `MW2/src/mechcollision.c`: `MoveMechWithCollisions`; `MW2/src/mechdamage.c`: `ApplyDamageToMech`; `MW2/src/menucontrols.c`: `RunMenuStatus` (2); `MW2/src/mw2prj.c`: `CachePreloads`; `MW2/src/objectanim.c`: `ReelMotionTask`; `MW2/src/overlay.c`: `HideCacheInfo`; `MW2/src/pausebanner.c`: `HandleDebugKey`; `MW2/src/polydraw.c`: `FUN_10068856`, `FUN_100688fe` (2), `FUN_100689f5`, `FUN_10068b57` (3), `FUN_10068e33`, `FUN_10069172`, `FUN_100692db`; `MW2/src/screenscale.c`: `GetEllipseNeedleAt` (2), `GetRectNeedleAt`; `MW2/src/shots.c`: `SpawnEffectEx`, `UpdateShot`
- An edition's own definition of a function, or functions only some editions' objects have (6): `MW2/src/animation.c`: the definition of `FUN_1002cfd0`, the definition of `FUN_1002d760`, the definition of `FUN_10051dfe`, the definitions of `FUN_10051c07`, `FUN_10051ea2`; `MW2/src/simmain.c`: the definition of `SimMain`, the definition of `SimWindowProc`
- Data and declarations that differ (8): `MW2/include/accel/a3d.h`: file scope; `MW2/include/accel/a3dvertex.h`: file scope; `MW2/include/animation.h`: file scope; `MW2/src/animation.c`: file scope; `MW2/src/bwdkeywords.c`: file scope; `MW2/src/damagepanel.c`: file scope; `MW2/src/refreshmode.c`: `g_displayBackends`, `g_refreshModes`
- Includes (2): `MW2/src/refreshmode.c`: file scope; `MW2/src/simmain.c`: file scope
- Pragmas that reproduce a unit's flags (2): `MW2/src/accel/queueface.c`: file scope (2)

<!-- generated: end -->
