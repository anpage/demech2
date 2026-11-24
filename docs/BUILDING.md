# Building

This project uses the [CMake](https://cmake.org/) build system. For the most accurate results, Microsoft Visual C++ 4.1 (the same compiler used to build the original 1.1 DLLs) is recommended. Since we're trying to match the output of this code to the original executables as closely as possible, all contributions will be graded with the output of the original compilers.

These instructions outline how to compile this repository using Visual C++ 4.1 into highly accurate binaries that closely match the originals. If you wish, you can try using other compilers, but this is at your own risk and won't be covered in this guide.

[TARGETS.md](TARGETS.md) lists every target with its compiler, its flags and where its binary is written.

## On Windows

#### Prerequisites

You will need the following software installed:

- Microsoft Visual C++ 4.1. A [portable version](https://github.com/madebr/msvc410) is available that can be downloaded and used quickly.
- MASM 6.11 (ML), for the original's hand-written assembly. A [ready-to-use copy](https://github.com/shengyanli1982/MASM611) is available; the build only needs its `BIN\ML.EXE`. Don't put its `BIN` on your `PATH`: it also holds a 16-bit `LINK`, `LIB` and `NMAKE` that would shadow Visual C++ 4.1's.
- Microsoft Visual C++ 2.2, for the targets built with it (optional). A [portable version](https://github.com/archaic-msvc/msvc220) is available too. Without it, `MECH2.EXE` and `NETMECHW.DLL` are built with Visual C++ 4.1 instead, which isn't their original toolchain, and the simulator editions that need 2.2 aren't built.
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
- To build the Visual C++ 2.2 targets with that compiler, add `-DDEMECH2_MSVC22_ROOT=<path-to-msvc22>`. CMake builds them in a nested build and sets up Visual C++ 2.2's environment itself, so only Visual C++ 4.1's `VCVARS32.BAT` needs to run.
- To build `MW2_MATROX.DLL` with Visual C++ 4.0, add `-DDEMECH2_MSVC40_ROOT=<path-to-msvc40>`: another nested build, set up the same way.
- The shell's dialogs show an icon that is not included in this repository. If the original 1.1 `MW2SHELL.DLL` is in the repository root, the build takes the icon from it; point `-DDEMECH2_MW2SHELL_ORIGINAL=<path>` at a copy elsewhere. Without it, the shell still builds and runs, and those dialogs just show no icon. The same goes for the launcher's window icons and the original `MECH2.EXE` (`-DDEMECH2_MECH2_ORIGINAL=<path>`), and for the NetMech lobby's icons and bitmaps and the original `NETMECHW.DLL` (`-DDEMECH2_NETMECHW_ORIGINAL=<path>`).
- The 1.1 patch's shell has a broken word in two of its dialogs ("shrieki ng", "co nfident"). The shell on the retail "with NetMech" and Softkey discs is the same build with those two texts fixed; add `-DDEMECH2_MW2SHELL_RETAIL=ON` to build its texts instead. The code is the same either way. Either shell can provide the icon.

1. Build the project by running `nmake` or `cmake --build <build-folder>`
2. When this is done, the recompiled binaries are in the build folder: those built with Visual C++ 4.1 at its root (the optimized S3 edition in its `vc41` subfolder), those built with Visual C++ 2.2 in `vc22`, and the Matrox edition in `vc40` when built with Visual C++ 4.0. [TARGETS.md](TARGETS.md#build-configuration) gives each binary's path.

The comparison builds of the Visual C++ 2.2 targets use Visual C++ 4.1's linker with Visual C++ 2.2's libraries, so that reccmp can read their PDBs.

## With Docker

We support Docker as a method of compilation. This is ideal for users on Linux and macOS who do not wish to manually configure a Wine environment for compiling this project: the image holds the three compilers, MASM and CMake under Wine, and the verification tools.

One command builds every binary and, given your own copies of the originals, checks every function against them:

```
docker run --rm \
	-v <path-to-source>:/demech2:rw \
	-v <build-folder>:/build:rw \
	-v <originals-folder>:/originals:ro \
	ghcr.io/anpage/demech2:latest
```

`<path-to-source>` should be replaced with the path to the source code directory (ie: the root of this repository).
`<build-folder>` should be replaced with the path to the build folder you'd like CMake to use during compilation.
`<originals-folder>` should be replaced with a folder holding the original binaries you have, under the names [TARGETS.md](TARGETS.md#the-originals) gives them (`MW2.DLL`, `MW2SHELL.DLL`, `MW2_GBL.DLL`…, in any case). Leave that line out to only build.

After the build, each target whose original is there is certified (`tools/verify.py`, which runs `tools/certify.py` per target: see [VERIFYING.md](VERIFYING.md)): the file is checked to be the one this project targets, every unit is compiled again, the witness records are applied, and every function is compared with the original. It ends with the counts the [proof states page](https://anpage.github.io/demech2/STATES.HTML) shows:

```
Target     Original        Functions Byte-identical  Proven  Unproven
MW2SHELL   MW2SHELL.DLL          798            644     154         0
MW2        MW2.DLL              1558            999     559         0
MW2MATROX  MW2_MATROX.DLL  no original: skipped
```

The list of every function with its state is written to `<build-folder>/states` (`<TARGET>STATES.json`, and `<TARGET>STATES.md` for the functions that aren't byte-identical, with the recipe that proves each or its first difference); `-e VERIFY_FLAGS=--functions` prints it too. The command fails if the build does, if a file isn't the original it is named for, or if a witness record doesn't give the original's bytes.

The build is configured as `RelWithDebInfo`, which is what the originals are compared with. To configure it otherwise, pass CMake's flags in the `CMAKE_FLAGS` environment variable (`-e CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Release"`).
