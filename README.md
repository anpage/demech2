# MechWarrior 2 Decompilation

This is a work-in-progress decompilation of the Windows 95 release of MechWarrior 2. It aims to be as accurate as possible, matching the recompiled instructions to the original machine code as much as possible. The goal is to provide a workable codebase that can be modified, improved, and ported to other platforms later on.

This project is modeled after the [LEGO Island](https://github.com/isledecomp/isle) and [LEGO Racers](https://github.com/isledecomp/racers) decompilations.

> **Note:** This repository is for decompilation only and its code is true to the original release. It will not compile for targets other than 32-bit Windows. A modern, portable adaptation of the MechWarrior 2 codebase is in progress as [ReMech2](https://github.com/anpage/remech2-rs).

## Status

<a href="https://anpage.github.io/demech2/MECH2PROGRESS.HTML"><img src="https://anpage.github.io/demech2/MECH2PROGRESS.SVG" width="50%"></a><a href="https://anpage.github.io/demech2/MW2SHELLPROGRESS.HTML"><img src="https://anpage.github.io/demech2/MW2SHELLPROGRESS.SVG" width="50%"></a>
<a href="https://anpage.github.io/demech2/NETMECHWPROGRESS.HTML"><img src="https://anpage.github.io/demech2/NETMECHWPROGRESS.SVG" width="50%"></a><a href="https://anpage.github.io/demech2/MW2PROGRESS.HTML"><img src="https://anpage.github.io/demech2/MW2PROGRESS.SVG" width="50%"></a>
<a href="https://anpage.github.io/demech2/MW2MATROXPROGRESS.HTML"><img src="https://anpage.github.io/demech2/MW2MATROXPROGRESS.SVG" width="50%"></a><a href="https://anpage.github.io/demech2/MW23DFXPROGRESS.HTML"><img src="https://anpage.github.io/demech2/MW23DFXPROGRESS.SVG" width="50%"></a>

Progress only counts game code. The statically linked C runtime and the import thunks are left out of both the matched and the total counts. The totals come from Ghidra’s analysis of the original binaries and may grow slightly as decompilation turns up missed functions.

What counts is proof, not similarity. `tools/certify.py` gives every annotated function one of three states:

- **byte-identical**: the source compiles to the original's code, every relocated address pointing where the original's does;
- **proven**: it does once its stack locals are renamed and/or a number of symbols are declared ahead of its unit, as a committed record (`tools/witnesses/`) says. Neither is recoverable from the original: its locals' names are lost, and its headers declared another number of things, both of which only move stack slots, operand order and (in optimized code) registers. The source keeps its readable names; the record is the proof;
- **unproven**: neither. `tools/audit.py` says which of these still differ only by compiler entropy (classified, by rewrites that reproduce the original exactly), and `tools/known_diffs.txt` lists the rest, with the reason where it is known. reccmp's percentage still shows how close each is.

| Binary | Functions | Byte-identical | Proven | Unproven |
|---|---|---|---|---|
| `MW2SHELL.DLL` (1.1) | 798 | 639 | 126 | 33 |
| `MW2.DLL` (1.1) | 1557 | 995 | 480 | 82 |
| `NETMECHW.DLL` (retail) | 228 | 128 | 87 | 13 |
| `MECH2.EXE` (retail) | 21 | 15 | 6 | 0 |
| `MW2_MATROX.DLL` (Matrox Mystique) | 1608 | 996 | 482 | 130 |
| `MW2_3DFX.DLL` (3Dfx) | 1548 | 1141 | 204 | 203 |

These are the annotated functions as of November 2026; each target's current list (`<TARGET>STATES.md` and `.json`) is published with the progress reports in the [continuous release](https://github.com/anpage/demech2/releases/tag/continuous). A match proves the code, the types, the data layout and the constants; it proves nothing about names, comments and descriptions, which are readings of the code. The original's hand-written assembly is reproduced as MASM source (its own objects) or inline assembly, and every function with inline assembly is listed in `tools/asm_allowlist.txt` with what in the original shows it was hand-written: everything else matches as C or C++.

Every game-code function of the four binaries of the 1.1 patch and the retail release is decompiled. Two more targets, the simulators of the Matrox Mystique edition (`MW2_MATROX.DLL`) and the 3Dfx edition (`MW2_3DFX.DLL`), are built from the same simulator source, where their differences (they keep the simulation's values as floats, and draw through Matrox's API and 3Dfx's Glide; the 3Dfx edition is optimized code) are being worked in. Beyond those editions, two kinds of work remain. One is raising the match: the unproven functions above, whose differences are real until shown otherwise. The other is understanding the code: much of it still carries placeholder names (`FUN_10003580`, `m_unk0x04`, `g_unk0x100a00dc`) that need to be studied and replaced with meaningful names and types. Contributions are welcome.

The continuous release ([`continuous`](https://github.com/anpage/demech2/releases/tag/continuous)) carries the latest recompiled binaries from `main`, with their PDBs and progress reports.

## Building

This project uses the [CMake](https://cmake.org/) build system. For the most accurate results, Microsoft Visual C++ 4.1 (the same compiler used to build the original DLLs) is recommended. Since we're trying to match the output of this code to the original executables as closely as possible, all contributions will be graded with the output of this compiler.

These instructions will outline how to compile this repository using Visual C++ 4.1 into highly accurate binaries that closely match the originals. If you wish, you can try using other compilers, but this is at your own risk and won't be covered in this guide.

#### Prerequisites

You will need the following software installed:

- Microsoft Visual C++ 4.1. A [portable version](https://github.com/madebr/msvc410) is available that can be downloaded and used quickly.
- MASM 6.11 (ML), for the original's hand-written assembly. A [ready-to-use copy](https://github.com/shengyanli1982/MASM611) is available; the build only needs its `BIN\ML.EXE`. Don't put its `BIN` on your `PATH`: it also holds a 16-bit `LINK`, `LIB` and `NMAKE` that would shadow Visual C++ 4.1's.
- Microsoft Visual C++ 2.2, for `MECH2.EXE`, `NETMECHW.DLL` and the 3Dfx edition's `MW2_3DFX.DLL` (optional). A [portable version](https://github.com/archaic-msvc/msvc220) is available too. Without it, the first two are built with Visual C++ 4.1 instead, which isn't their original toolchain, and the 3Dfx edition isn't built.
- Microsoft Visual C++ 4.0, for the Matrox edition's `MW2_MATROX.DLL` (optional). A [portable version](https://github.com/itsmattkc/MSVC400) is available. Without it, the Matrox edition is built with Visual C++ 4.1, whose stack-slot assignment and C runtime differ from its original toolchain's.
- [CMake](https://cmake.org/). A copy is often included with the "Desktop development with C++" workload in newer versions of Visual Studio; however, it can also be installed as a standalone app. Version 3.26.6 (i386) is known to work with the VC++ 4.1 NMake generator.

#### Compiling

1. Open a Command Prompt (`cmd`).
2. From Visual C++ 4.1, run `BIN\VCVARS32.BAT` to populate the path and other environment variables for compiling with MSVC.
3. Make a folder for compiled objects to go, such as a `build` folder inside the source repository (the folder you cloned/downloaded to).
4. In your Command Prompt, `cd` to the build folder.
5. Configure the project with CMake by running:

```
cmake <path-to-source> -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_ASM_MASM_COMPILER=<path-to-masm>\BIN\ML.EXE
```

- **Visual C++ 4.1 has issues with paths containing spaces**. If you get configure or build errors, make sure neither CMake, the repository, nor Visual C++ 4.1 is in a path that contains spaces.
- Replace `<path-to-source>` with the source repository. This can be `..` if your build folder is inside the source repository.
- `RelWithDebInfo` is recommended because it will produce debug symbols useful for further decompilation work.
- `NMake Makefiles` is most recommended because it will be immediately compatible with Visual C++ 4.1.
- To build `MECH2.EXE`, `NETMECHW.DLL` and `MW2_3DFX.DLL` with Visual C++ 2.2, add `-DDEMECH2_MSVC22_ROOT=<path-to-msvc22>`. CMake builds them in a nested build and sets up Visual C++ 2.2's environment itself, so only Visual C++ 4.1's `VCVARS32.BAT` needs to run.
- To build `MW2_MATROX.DLL` with Visual C++ 4.0, add `-DDEMECH2_MSVC40_ROOT=<path-to-msvc40>`: another nested build, set up the same way.
- The shell's dialogs show an icon that is not included in this repository. If the original 1.1 `MW2SHELL.DLL` is in the repository root, the build takes the icon from it; point `-DDEMECH2_MW2SHELL_ORIGINAL=<path>` at a copy elsewhere. Without it, the shell still builds and runs, and those dialogs just show no icon. The same goes for the launcher's window icons and the original `MECH2.EXE` (`-DDEMECH2_MECH2_ORIGINAL=<path>`), and for the NetMech lobby's icons and bitmaps and the original `NETMECHW.DLL` (`-DDEMECH2_NETMECHW_ORIGINAL=<path>`).

1. Build the project by running `nmake` or `cmake --build <build-folder>`
2. When this is done, there should be a recompiled `MW2SHELL.DLL`, `MW2.DLL`, `NETMECHW.DLL`, `MECH2.EXE` and `MW2_MATROX.DLL` in the build folder (`NETMECHW.DLL` and `MECH2.EXE` in its `vc22` subfolder when built with Visual C++ 2.2, with `MW2_3DFX.DLL`, and `MW2_MATROX.DLL` in its `vc40` subfolder when built with Visual C++ 4.0).

The build configuration for each binary (comparison builds use Visual C++ 4.1's linker with Visual C++ 2.2's libraries for the Visual C++ 2.2 targets, so reccmp can read their PDBs):

| Target           | Language                   | `cl` flags                                        | CRT                     | Link                             |
| ---------------- | -------------------------- | ------------------------------------------------- | ----------------------- | -------------------------------- |
| `MW2.DLL`        | C                          | `/Od /Oi /G5`                                     | `/MTd` (static debug)   | `/DLL /DEBUG /INCREMENTAL:no`    |
| `MW2SHELL.DLL`   | C++ (some C files)         | `/Od /Oi /G5 /Ob1 /GX` (C files: no `/GX`)        | `/MT` (static)          | `/DLL`                           |
| `NETMECHW.DLL`   | C and C++ (Visual C++ 2.2) | `/Od /Oi /Z7` (no `/GX`)                          | `/MT` (static)          | `/DLL`, `/DEBUG /INCREMENTAL:no` |
| `MECH2.EXE`      | C (Visual C++ 2.2)         | `/Od /Oi /G5 /Z7`                                 | `/ML` (single-threaded) | incremental EXE, `/DEBUG`        |
| `MW2_MATROX.DLL` | C (Visual C++ 4.0)         | `/Od /Oi /G5`                                     | `/MT` (static)          | `/DLL /DEBUG /INCREMENTAL:no`    |
| `MW2_3DFX.DLL`   | C (Visual C++ 2.2)         | `/Ox /Oa /Oi /G5 /Gr /Gy /Z7` (three files `/Od`) | `/MT` (static)          | `/DLL /DEBUG /INCREMENTAL:no`    |

### Docker

Alternatively, we support Docker as a method of compilation. This is ideal for users on Linux and macOS who do not wish to manually configure a Wine environment for compiling this project.

Compilation should be as simple as configuring and running the following command:

```
docker run -d \
	-e CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=RelWithDebInfo" \
	-v <path-to-source>:/demech2:rw \
	-v <build-folder>:/build:rw \
	ghcr.io/anpage/demech2:latest
```

`<path-to-source>` should be replaced with the path to the source code directory (ie: the root of this repository).
`<build-folder>` should be replaced with the path to the build folder you'd like CMake to use during compilation.

You can pass as many CMake flags as you'd like in the `CMAKE_FLAGS` environment variable, but the default configuration provided in the command is already ideal for building highly-accurate binaries.

### Verification

To verify your build against the original binaries, install the [reccmp](https://github.com/isledecomp/reccmp) tooling:

```
pip install -r tools/requirements.txt
```

This installs a [fork](https://github.com/anpage/reccmp) pinned to a commit: `MECH2.EXE` was linked incrementally but has no debug directory, and reccmp only looks for the incremental thunks in images that have one.

Create `reccmp-user.yml` in the project root pointing to the original binaries:

```yaml
targets:
  MW2SHELL:
    path: path/to/MW2SHELL.DLL
  MW2:
    path: path/to/MW2.DLL
  NETMECHW:
    path: path/to/NETMECHW.DLL
  MECH2:
    path: path/to/MECH2.EXE
  MW2MATROX:
    path: path/to/MW2_MATROX.DLL
  MW23DFX:
    path: path/to/MW2_3DFX.DLL
```

Then run:

```
reccmp-reccmp --target MW2SHELL -S MW2SHELLPROGRESS.SVG
reccmp-reccmp --target MW2 -S MW2PROGRESS.SVG
reccmp-reccmp --target NETMECHW --nolib --total 244
reccmp-reccmp --target MECH2 --nolib --total 21
reccmp-reccmp --target MW2MATROX --nolib --total 1597
reccmp-reccmp --target MW23DFX --nolib --total 1574
```

To certify the build (from the repository root, after building; it compiles each unit again with the build's own commands, applies the witness records, and compares every function with the original):

```
python tools/certify.py --target MW2 [--build <build-folder>] [-v]
```

`-v` lists the unproven functions with their first difference. Under Wine, pass `--wine <path-to-wine>`, with `WINEPREFIX` set.

#### Continuous integration

Pull requests are built and verified with Visual C++ 4.1 and 2.2 on GitHub Actions. The 1.1 patch's `MW2SHELL.DLL` and `MW2.DLL` are downloaded by the Build workflow itself. The retail `NETMECHW.DLL` and `MECH2.EXE` aren't freely downloadable, so a second workflow (`verify-retail.yml`) runs after Build with access to a private repository holding the originals: it compares the Build run's binaries against them with reccmp and appends the results to the Build workflow's PR comment. Both certify every function: a pull request fails when a function `main` has byte-identical or proven is neither any more, unless its description lists it (`Gate exception: <TARGET> <function> <reason>`). Pull requests from forks get the same report.

## Project Structure

- `MECH2/` - Decompilation of the retail `MECH2.EXE` launcher (a reccmp target with Visual C++ 2.2)
- `MW2SHELL/` - Decompilation of `MW2SHELL.DLL`
- `MW2/` - Decompilation of `MW2.DLL`
- `NETMECHW/` - Decompilation of the retail `NETMECHW.DLL`, NetMech's lobby (a reccmp target with Visual C++ 2.2)
- `util/` - Utility headers for decompilation
- `3rdparty/` - Import libraries (`.def` files), our own DirectDraw, DirectPlay, Glide, MSI95, Miles and Smacker declarations, and the third-party code listed under "License"
- `cmake/` - CMake modules
- `tools/` - Python tools and requirements
- `reccmp/` - reccmp data sources
- `docker/` - VC++ 4.1 + VC++ 2.2 + CMake under Wine build image
- `vc22/` - The VC++ 2.2 sub-project that builds `MECH2.EXE` and `NETMECHW.DLL`
- `assets/` - Progress report icons

## Target Binaries

| Binary         | Role                                  | Size          | SHA256                                                             | Modified           |
| -------------- | ------------------------------------- | ------------- | ------------------------------------------------------------------ | ------------------ |
| `MW2SHELL.DLL` | Shell: FMV, menus, settings, metagame | 545,792 bytes | `1078ccb07fd45388bfd525719a8206ce7a01d6536776d30a5655ccb928900879` | October 9, 1996    |
| `MW2.DLL`      | Simulator (in-mission gameplay)       | 827,392 bytes | `6212d542f8f915a594b278ab189f20a27e522e7c08ac57ce68bf47f45b17bbb5` | September 19, 1996 |

Both DLLs come from the [freely downloadable 1.1 patch](https://archive.org/details/mw2patch).

| Binary         | Role          | Size          | SHA256                                                             | Modified         |
| -------------- | ------------- | ------------- | ------------------------------------------------------------------ | ---------------- |
| `NETMECHW.DLL` | NetMech lobby | 433,152 bytes | `3f4d1508238d847213127e46623a85b249985686d6053a3d9e1c6deea87c0095` | December 8, 1995 |
| `MECH2.EXE`    | Launcher      | 53,248 bytes  | `064a9f1f45cfd18f0bef1dea9711b7fd582755cfbb8a8e9694f8cef2e5c3690c` | December 5, 1995 |

Both come from the retail release, not the patch. Comparing them requires the original retail files and a build with Visual C++ 2.2 (`-DDEMECH2_MSVC22_ROOT`). Without that compiler the project still builds both, but the results cannot be compared to the originals.

## Contributing

Contributions are welcome. Please follow the conventions established in the codebase:

- Use reccmp annotations (`FUNCTION:`, `STUB:`, `GLOBAL:`) for all decompiled code
- Functions in a compilation unit must be ordered by their address in ascending order
- Follow the clang-format configuration
- Use NCC naming conventions (`FUN_XXXXXXXX` for unknown functions, `g_unk0xXXXXXXXX` for unknown globals)
- Keep pull requests small and focused

## License

The project's own code is licensed under the **GNU Lesser General Public License, version 3** ([LICENSE](LICENSE)). That covers everything in this repository except the third-party files listed below: the decompiled sources, the headers, the build scripts, the tools, and the declarations in `3rdparty/` that were written for this project (the import-library definitions and the DirectPlay, DirectDraw, DisplayDib, Glide, MSI95, Miles Sound System and Smacker declarations).

The LGPL comes with one **additional permission**, granted under section 7 of the GPL, version 3, which the LGPL builds on. The project may be combined with code under the 3DFX GLIDE Source Code General Public License, the licence of 3dfx's Texus library, which the 3Dfx edition's simulator links. The result may be distributed, with that code staying under its own licence. That licence's terms don't count as extra restrictions on the rest of the project. The exact wording is in [NOTICE](NOTICE).

**Third-party code** in `3rdparty/` keeps its own terms:

| Directory        | What it is                                                                   | Terms                                                                                                                                                                               |
| ---------------- | ---------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `3rdparty/vfx`   | Miles Design's VFX graphics library, by John Miles                           | Open-source freeware since July 2000: usable for any purpose without restriction ([README.TXT](3rdparty/vfx/README.TXT))                                                            |
| `3rdparty/texus` | 3dfx Interactive's Texus texture library, from the 1999 Glide source release | 3DFX GLIDE Source Code General Public License ([glide_license.txt](3rdparty/texus/glide_license.txt), [README](3rdparty/texus/README)). Changes are marked and dated in the source. |

**Not included:** MechWarrior 2 itself. The game's binaries, data and resources aren't in this repository and aren't covered by any of these licences. The build reads a few icons and bitmaps from your own copy of the original binaries, if you have them, and they're never committed. The game and its trademarks belong to their owners. This project isn't affiliated with Activision, 3dfx, Matrox, Microsoft or Miles Design.
