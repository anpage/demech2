# MechWarrior 2 Decompilation

This is a work-in-progress decompilation of the Windows 95 release of MechWarrior 2. It aims to be as accurate as possible, matching the recompiled instructions to the original machine code as much as possible. The goal is to provide a workable codebase that can be modified, improved, and ported to other platforms later on.

This project is modeled after the [LEGO Island](https://github.com/isledecomp/isle) and [LEGO Racers](https://github.com/isledecomp/racers) decompilations.

> **Note:** This repository is for decompilation only and its code is true to the original release. It will not compile for targets other than 32-bit Windows. A modern, portable adaptation of the MechWarrior 2 codebase is planned.

## Status

<a href="https://anpage.github.io/demech2/MW2SHELLPROGRESS.HTML"><img src="https://anpage.github.io/demech2/MW2SHELLPROGRESS.SVG" width="50%"></a><a href="https://anpage.github.io/demech2/MW2PROGRESS.HTML"><img src="https://anpage.github.io/demech2/MW2PROGRESS.SVG" width="50%"></a>

This project is in its early stages. `MW2SHELL.DLL` and `MW2.DLL` are being decompiled with the goal of eventually reaching full accuracy against the original binaries. Contributions are welcome.

## Building

This project uses the [CMake](https://cmake.org/) build system. For the most accurate results, Microsoft Visual C++ 4.1 (the same compiler used to build the original DLLs) is recommended. Since we're trying to match the output of this code to the original executables as closely as possible, all contributions will be graded with the output of this compiler.

These instructions will outline how to compile this repository using Visual C++ 4.1 into highly-accurate binaries where the majority of functions are instruction-matching with retail. If you wish, you can try using other compilers, but this is at your own risk and won't be covered in this guide.

#### Prerequisites

You will need the following software installed:

- Microsoft Visual C++ 4.1. A [portable version](https://github.com/madebr/msvc410) is available that can be downloaded and used quickly.
- [CMake](https://cmake.org/). A copy is often included with the "Desktop development with C++" workload in newer versions of Visual Studio; however, it can also be installed as a standalone app. Version 3.26.6 (i386) is known to work with the VC++ 4.1 NMake generator.

#### Compiling

1. Open a Command Prompt (`cmd`).
2. From Visual C++ 4.1, run `BIN\VCVARS32.BAT` to populate the path and other environment variables for compiling with MSVC.
3. Make a folder for compiled objects to go, such as a `build` folder inside the source repository (the folder you cloned/downloaded to).
4. In your Command Prompt, `cd` to the build folder.
5. Configure the project with CMake by running:

```
cmake <path-to-source> -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

- **Visual C++ 4.1 has issues with paths containing spaces**. If you get configure or build errors, make sure neither CMake, the repository, nor Visual C++ 4.1 is in a path that contains spaces.
- Replace `<path-to-source>` with the source repository. This can be `..` if your build folder is inside the source repository.
- `RelWithDebInfo` is recommended because it will produce debug symbols useful for further decompilation work.
- `NMake Makefiles` is most recommended because it will be immediately compatible with Visual C++ 4.1.
- The shell's dialogs show an icon that is not included in this repository. If the original 1.1 `MW2SHELL.DLL` is in the repository root, the build takes the icon from it; point `-DDEMECH2_MW2SHELL_ORIGINAL=<path>` at a copy elsewhere. Without it, the shell still builds and runs, and those dialogs just show no icon. The same goes for the launcher's window icons and the original `MECH2.EXE` (`-DDEMECH2_MECH2_ORIGINAL=<path>`).

1. Build the project by running `nmake` or `cmake --build <build-folder>`
2. When this is done, there should be a recompiled `MW2SHELL.DLL`, `MW2.DLL` and `MECH2.EXE` in the build folder.

The build configuration identified during the project's initial investigation:

| Target         | Language           | `cl` flags                              | CRT                   | Link                          |
| -------------- | ------------------ | --------------------------------------- | --------------------- | ----------------------------- |
| `MW2.DLL`      | C                  | `/Od /Oi`                               | `/MTd` (static debug) | `/DLL /DEBUG /INCREMENTAL:no` |
| `MW2SHELL.DLL` | C++ (some C files) | `/Od /Oi /GX /Ob1` (C files: `/Od /Oi`) | `/MT` (static)        | `/DLL`                        |

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

Create `reccmp-user.yml` in the project root pointing to the original binaries:

```yaml
targets:
  MW2SHELL:
    path: path/to/MW2SHELL.DLL
  MW2:
    path: path/to/MW2.DLL
```

Then run:

```
reccmp-reccmp --target MW2SHELL -S MW2SHELLPROGRESS.SVG
reccmp-reccmp --target MW2 -S MW2PROGRESS.SVG
```

## Project Structure

- `MECH2/` - Source of `MECH2.EXE` (no annotations, not a reccmp target)
- `MW2SHELL/` - Decompilation of `MW2SHELL.DLL`
- `MW2/` - Decompilation of `MW2.DLL`
- `util/` - Utility headers for decompilation
- `3rdparty/` - Import libraries (`.def` files), DirectX 2 SDK headers, and our own Miles/Smacker declarations
- `cmake/` - CMake modules
- `tools/` - Python tools and requirements
- `reccmp/` - reccmp data sources
- `docker/` - VC++ 4.1 + CMake under Wine build image
- `assets/` - Progress report icons

## Target Binaries

| Binary         | Role                                  | Size          | SHA256                                                             | Modified           |
| -------------- | ------------------------------------- | ------------- | ------------------------------------------------------------------ | ------------------ |
| `MW2SHELL.DLL` | Shell: FMV, menus, settings, metagame | 545,792 bytes | `1078ccb07fd45388bfd525719a8206ce7a01d6536776d30a5655ccb928900879` | October 9, 1996    |
| `MW2.DLL`      | Simulator (in-mission gameplay)       | 827,392 bytes | `6212d542f8f915a594b278ab189f20a27e522e7c08ac57ce68bf47f45b17bbb5` | September 19, 1996 |

Both DLLs come from the [freely downloadable 1.1 patch](https://archive.org/details/mw2patch) and are the reccmp targets.

| Binary      | Role     | Size         | SHA256                                                             | Modified         |
| ----------- | -------- | ------------ | ------------------------------------------------------------------ | ---------------- |
| `MECH2.EXE` | Launcher | 53,248 bytes | `064a9f1f45cfd18f0bef1dea9711b7fd582755cfbb8a8e9694f8cef2e5c3690c` | December 5, 1995 |

`MECH2.EXE` is **source-only**: its source lives in `MECH2/` without annotations and outside reccmp. It was built with an older toolchain (VC++ 2.x) than the DLLs, so it cannot byte-match under the project toolchain and is not scored.

## Contributing

Contributions are welcome. Please follow the conventions established in the codebase:

- Use reccmp annotations (`FUNCTION:`, `STUB:`, `GLOBAL:`) for all decompiled code
- Functions in a compilation unit must be ordered by their address in ascending order
- Follow the clang-format configuration
- Use NCC naming conventions (`FUN_XXXXXXXX` for unknown functions, `g_unk0xXXXXXXXX` for unknown globals)
- Keep pull requests small and focused
