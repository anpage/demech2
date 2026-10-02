# MechWarrior 2 Decompilation

This is a work-in-progress decompilation of the Windows 95 release of MechWarrior 2. It aims to be as accurate as possible, matching the recompiled instructions to the original machine code as much as possible. The goal is to provide a workable codebase that can be modified, improved, and ported to other platforms later on.

This project is modeled after the [LEGO Island](https://github.com/isledecomp/isle) and [LEGO Racers](https://github.com/isledecomp/racers) decompilations.

> **Note:** This repository is for decompilation only and its code is true to the original release. It will not compile for targets other than 32-bit Windows. A modern, portable adaptation of the MechWarrior 2 codebase is planned.

## Status

<a href="https://anpage.github.io/demech2/MECH2PROGRESS.HTML"><img src="https://anpage.github.io/demech2/MECH2PROGRESS.SVG" width="50%"></a><a href="https://anpage.github.io/demech2/MW2SHELLPROGRESS.HTML"><img src="https://anpage.github.io/demech2/MW2SHELLPROGRESS.SVG" width="50%"></a>
<a href="https://anpage.github.io/demech2/NETMECHWPROGRESS.HTML"><img src="https://anpage.github.io/demech2/NETMECHWPROGRESS.SVG" width="50%"></a><a href="https://anpage.github.io/demech2/MW2PROGRESS.HTML"><img src="https://anpage.github.io/demech2/MW2PROGRESS.SVG" width="50%"></a>

Progress only counts game code. The statically linked C runtime and the import thunks are left out of both the matched and the total counts. The totals come from Ghidra’s analysis of the original binaries and may grow slightly as decompilation turns up missed functions.

Every game-code function of all four binaries is decompiled. Two kinds of work remain. One is raising the match: plenty of functions are still short of byte-identical, and many of those differ only by compiler artifacts (stack-slot assignment, operand order), which reccmp still scores as differences. The other is understanding the code: much of it still carries placeholder names (`FUN_10003580`, `m_unk0x04`, `g_unk0x100acb2c`) that need to be studied and replaced with meaningful names and types. Contributions are welcome.

The continuous release ([`continuous`](https://github.com/anpage/demech2/releases/tag/continuous)) carries the latest recompiled binaries from `main`, with their PDBs and progress reports.

## Building

This project uses the [CMake](https://cmake.org/) build system. For the most accurate results, Microsoft Visual C++ 4.1 (the same compiler used to build the original DLLs) is recommended. Since we're trying to match the output of this code to the original executables as closely as possible, all contributions will be graded with the output of this compiler.

These instructions will outline how to compile this repository using Visual C++ 4.1 into highly accurate binaries that closely match the originals. If you wish, you can try using other compilers, but this is at your own risk and won't be covered in this guide.

#### Prerequisites

You will need the following software installed:

- Microsoft Visual C++ 4.1. A [portable version](https://github.com/madebr/msvc410) is available that can be downloaded and used quickly.
- MASM 6.11 (ML), for the original's hand-written assembly. A [ready-to-use copy](https://github.com/shengyanli1982/MASM611) is available; the build only needs its `BIN\ML.EXE`. Don't put its `BIN` on your `PATH`: it also holds a 16-bit `LINK`, `LIB` and `NMAKE` that would shadow Visual C++ 4.1's.
- Microsoft Visual C++ 2.2, for `MECH2.EXE` and `NETMECHW.DLL` (optional). A [portable version](https://github.com/archaic-msvc/msvc220) is available too. Without it, both are built with Visual C++ 4.1 instead, which isn't their original toolchain.
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
- To build `MECH2.EXE` and `NETMECHW.DLL` with Visual C++ 2.2, add `-DDEMECH2_MSVC22_ROOT=<path-to-msvc22>`. CMake builds them in a nested build and sets up Visual C++ 2.2's environment itself, so only Visual C++ 4.1's `VCVARS32.BAT` needs to run.
- The shell's dialogs show an icon that is not included in this repository. If the original 1.1 `MW2SHELL.DLL` is in the repository root, the build takes the icon from it; point `-DDEMECH2_MW2SHELL_ORIGINAL=<path>` at a copy elsewhere. Without it, the shell still builds and runs, and those dialogs just show no icon. The same goes for the launcher's window icons and the original `MECH2.EXE` (`-DDEMECH2_MECH2_ORIGINAL=<path>`), and for the NetMech lobby's icons and bitmaps and the original `NETMECHW.DLL` (`-DDEMECH2_NETMECHW_ORIGINAL=<path>`).

1. Build the project by running `nmake` or `cmake --build <build-folder>`
2. When this is done, there should be a recompiled `MW2SHELL.DLL`, `MW2.DLL`, `NETMECHW.DLL` and `MECH2.EXE` in the build folder (the last two in its `vc22` subfolder when built with Visual C++ 2.2).

The build configuration for each binary (comparison builds use Visual C++ 4.1's linker with Visual C++ 2.2's libraries for the Visual C++ 2.2 targets, so reccmp can read their PDBs):

| Target         | Language                   | `cl` flags                                 | CRT                     | Link                             |
| -------------- | -------------------------- | ------------------------------------------ | ----------------------- | -------------------------------- |
| `MW2.DLL`      | C                          | `/Od /Oi /G5`                              | `/MTd` (static debug)   | `/DLL /DEBUG /INCREMENTAL:no`    |
| `MW2SHELL.DLL` | C++ (some C files)         | `/Od /Oi /G5 /Ob1 /GX` (C files: no `/GX`) | `/MT` (static)          | `/DLL`                           |
| `NETMECHW.DLL` | C and C++ (Visual C++ 2.2) | `/Od /Oi /Z7` (no `/GX`)                   | `/MT` (static)          | `/DLL`, `/DEBUG /INCREMENTAL:no` |
| `MECH2.EXE`    | C (Visual C++ 2.2)         | `/Od /Oi /G5 /Z7`                          | `/ML` (single-threaded) | incremental EXE, `/DEBUG`        |

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
```

Then run:

```
reccmp-reccmp --target MW2SHELL -S MW2SHELLPROGRESS.SVG
reccmp-reccmp --target MW2 -S MW2PROGRESS.SVG
reccmp-reccmp --target NETMECHW --nolib --total 244
reccmp-reccmp --target MECH2 --nolib --total 21
```

#### Continuous integration

Pull requests are built and verified with Visual C++ 4.1 and 2.2 on GitHub Actions. The 1.1 patch's `MW2SHELL.DLL` and `MW2.DLL` are downloaded by the Build workflow itself. The retail `NETMECHW.DLL` and `MECH2.EXE` aren't freely downloadable, so a second workflow (`verify-retail.yml`) runs after Build with access to a private repository holding the originals: it compares the Build run's binaries against them with reccmp and appends the results to the Build workflow's PR comment. Pull requests from forks get the same report.

## Project Structure

- `MECH2/` - Decompilation of the retail `MECH2.EXE` launcher (a reccmp target with Visual C++ 2.2)
- `MW2SHELL/` - Decompilation of `MW2SHELL.DLL`
- `MW2/` - Decompilation of `MW2.DLL`
- `NETMECHW/` - Decompilation of the retail `NETMECHW.DLL`, NetMech's lobby (a reccmp target with Visual C++ 2.2)
- `util/` - Utility headers for decompilation
- `3rdparty/` - Import libraries (`.def` files), DirectX 2 SDK headers, and our own Miles/Smacker declarations
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
