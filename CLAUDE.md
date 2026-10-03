# demech2 — MechWarrior 2 (Windows 95) Decompilation

Decompilation of MechWarrior 2 (Windows 95 release) using Microsoft Visual C++ 4.1 (`cl` 10.10.6038, `_MSC_VER` 1010). Modeled after the [LEGO Island](https://github.com/isledecomp/isle) and [LEGO Racers](https://github.com/isledecomp/racers) decompilations.

Four reccmp targets:

- **MW2SHELL** — `MW2SHELL.DLL`: shell (FMV, menus, settings, metagame), from the freely downloadable 1.1 patch. A mixture of C and C++.
- **MW2** — `MW2.DLL`: simulator (in-mission gameplay), from the same patch. Probably all C.
- **NETMECHW** — `NETMECHW.DLL`: NetMech's lobby (the shell's multiplayer counterpart: `MECH2.EXE` calls its `Launcher` export, then runs `MW2.DLL` with the launch record it fills in), from the retail release, built with VC++ 2.2 (`cl` 9.10, `_MSC_VER` 900). A mixture of C and C++ like the shell, 244 game-code functions in 42 objects, no hand-written assembly. Every game function is decompiled: the units it shares with the shell are per-target copies (`debugout.c`, `mw2prj.c`, `prjfile.c`, `resourcecache.c`, `resourcefile.c`, `resourcename.c`, and the CD check, which NETMECHW links inside the results dialog's object, `unk10010460.cpp`), and so are MW2's data-only `bwdkeywords.c` and `weapondata.c`. The rest is the lobby: `Launcher`, the lobby window and the pane switch (`unk10003660.cpp`), one dialog per pane (connection, sessions, host options, guest lobby, player slots, briefing, mech selection, launch, results), the DirectPlay session list, threads and messages, the chat, the player table, the `.MEK` mech files and their network form (`unk10007c30.c`), and the mission files left for the simulator (`unk1000f0f0.cpp`, `bwdwriter.c`).
- **MECH2** — `MECH2.EXE`: retail launcher that loads the DLLs, built with VC++ 2.2 too. Its 21 game-code functions are annotated for reccmp.

The build uses 2.2 for the last two when configured (see "Building"); otherwise it builds them with the project's compiler, which cannot match the originals.

Not targets: the third-party DLLs `WAIL32.DLL` (Miles Sound System) and `SMACKW32.DLL` (Smacker), which we only link against.

## Local Instructions

Machine-specific instructions live in `CLAUDE.local.md` at the repository root (gitignored): how to invoke the toolchain and reccmp on this machine (Wine build, prefix, paths), where the originals live, which tools are available, and the contributor's workflow preferences. **Agents: if `CLAUDE.local.md` exists, read it before building or running reccmp; where it conflicts with the generic instructions here, it wins.** Claude Code loads it automatically; other agents reading `AGENTS.md` must open it themselves. Contributors: create it to fit your environment; it is never committed.

## Building

```
<path-to-msvc41>\BIN\VCVARS32.BAT
mkdir build && cd build
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DDEMECH2_MSVC22_ROOT=<path-to-msvc22>
cmake --build .
```

Portable VC++ 4.1: https://github.com/madebr/msvc410. MASM 6.11 (ML, for the MASM objects; see "Hand-written Assembly"): https://github.com/shengyanli1982/MASM611, passed to the configure step as `-DCMAKE_ASM_MASM_COMPILER=<path-to-masm>\BIN\ML.EXE` (never put its `BIN` on `PATH`: it also holds a 16-bit `LINK`, `LIB` and `NMAKE`). CMake 3.26.6 (i386) is known to drive the VC++ 4.1 NMake generator.

**VC++ 2.2** (for `NETMECHW.DLL` and `MECH2.EXE`): https://github.com/archaic-msvc/msvc220 at `c0fff2a`. One CMake project can use only one compiler per language, so the 2.2 targets live in the `vc22/` sub-project, which the top-level build configures and builds as a nested build in `<build>/vc22` (`ExternalProject`), setting 2.2's `PATH`/`INCLUDE`/`LIB` itself (through the generated `<build>/vc22-env.cmake`): only the 4.1 `VCVARS32.BAT` needs to run. The targets are defined once, in `vc22/targets.cmake`: without `DEMECH2_MSVC22_ROOT`, or with a modern compiler, the top-level project builds them itself. The comparison build compiles with 2.2's `cl` and `/Z7` and links with **4.1's LINK** against 2.2's libraries: reccmp's `cvdump` can't read a 2.x PDB, but reads LINK 3.10's. Never `/Zi` for 2.2 objects.

Build configuration:

| Target   | Language                | `cl` flags             | CRT                                        | Link                                                |
| -------- | ----------------------- | ---------------------- | ------------------------------------------ | --------------------------------------------------- |
| MW2      | C                       | `/Od /Oi /G5`          | `/MTd` (static debug, predefines `_DEBUG`) | `/DLL /DEBUG /INCREMENTAL:no`                       |
| MW2SHELL | C++ (C files: no `/GX`) | `/Od /Oi /G5 /Ob1 /GX` | `/MT` (static)                             | `/DLL` (comparison build adds `/DEBUG` for the PDB) |
| NETMECHW | C and C++ (VC++ 2.2; no `/GX`) | `/Od /Oi /Z7`   | `/MT` (static `LIBCMT`)                    | `/DLL` (comparison build adds `/DEBUG /INCREMENTAL:no`) |
| MECH2    | C (VC++ 2.2)            | `/Od /Oi /G5 /Z7`      | `/ML` (single-threaded `LIBC`)             | incremental EXE (comparison build adds `/DEBUG`)    |

The CRT is selected through `MSVC_RUNTIME_LIBRARY` under `CMP0091 NEW` (`MultiThreadedDebug` for MW2, `MultiThreaded` for MW2SHELL and NETMECHW; CMake has no value for `/ML`, so MECH2 sets an empty one and passes the flag). No CRT patching is needed: the originals match 4.1's `LIBCMTD.LIB` (MW2) and `LIBCMT.LIB` (MW2SHELL), and 2.2's `LIBCMT.LIB` (NETMECHW) and `LIBC.LIB` (MECH2) as-is.

The shell's icon 103 (shown by four dialogs) is not committed: CMake builds `tools/extract_resource` and extracts it from the original `MW2SHELL.DLL` (`DEMECH2_MW2SHELL_ORIGINAL`, default: the repository root; SHA-256 checked). Without the original, `mw2shell.rc` builds without the icon. The launcher's window icons 103 and 108 (NetMech) work the same way, from the original `MECH2.EXE` (`DEMECH2_MECH2_ORIGINAL`) into `mech2.rc`, and so do the lobby's two icon groups and 18 bitmaps, from the original `NETMECHW.DLL` (`DEMECH2_NETMECHW_ORIGINAL`) into `netmechw.rc`. Text resources (dialogs, string tables, menus, version blocks) are committed in the `.rc` files, recovered from the originals so that RC compiles each byte-identical.

Source order in each target must follow the **original link order** (object order determines function addresses).

## reccmp

```bash
pip install -r tools/requirements.txt

# Compare (run from build/ directory)
reccmp-reccmp --target MW2SHELL --print-rec-addr
reccmp-reccmp --target MW2 --verbose 0x10003580 --print-rec-addr
reccmp-reccmp --target NETMECHW --print-rec-addr
reccmp-reccmp --target MECH2 --print-rec-addr

# Game-code progress, as CI reports it
reccmp-reccmp --target MW2SHELL --silent --nolib --total 770
reccmp-reccmp --target MW2 --silent --nolib --total 1537
reccmp-reccmp --target NETMECHW --silent --nolib --total 244
reccmp-reccmp --target MECH2 --silent --nolib --total 21

# Compare global variable data values
reccmp-datacmp --target MW2SHELL --verbose --print-rec-addr
reccmp-datacmp --target MW2 --verbose --print-rec-addr
reccmp-datacmp --target NETMECHW --verbose --print-rec-addr
reccmp-datacmp --target MECH2 --verbose --print-rec-addr

# Lint annotations (pass source dirs to avoid scanning gitignored files)
reccmp-decomplint --module MW2SHELL --warnfail <path-to-MW2SHELL>
reccmp-decomplint --module MW2 --warnfail <path-to-MW2>
reccmp-decomplint --module NETMECHW --warnfail <path-to-NETMECHW>
reccmp-decomplint --module MECH2 --warnfail <path-to-MECH2>

# Blocks of globals (see "Annotations"): the original's block operations against the
# recompiled layout (from build/), and the source's (from the repository root; needs the
# libclang Python bindings)
python ../tools/check_block_layout.py --target MW2SHELL
python tools/check_block_sizes.py --msvc-include <path-to-msvc410>/include MW2SHELL MW2 NETMECHW MECH2

# The source split against the original's object layout (from build/; see "Decompilation Principles")
python ../tools/check_units.py --target MW2SHELL
```

`tools/requirements.txt` pins a fork of reccmp (`anpage/reccmp`): `isledecomp/reccmp` reads an incremental link's thunk table only when the image has a debug directory, and `MECH2.EXE` has none, so every call through its thunks would score as a diff. pip doesn't reinstall a git requirement whose version number hasn't changed: after the pin moves, update an existing venv with `pip install --force-reinstall --no-deps -r tools/requirements.txt`.

`reccmp-user.yml` (gitignored) points to the original binaries for local comparison. Progress counts **game code only**: `--nolib` drops the `LIBRARY` entries from both the matched count and the denominator, and `--total` is the Ghidra original's game-code function count, everything below the shell's import thunks (`0x100492a2`) or MW2's CRT (`0x10080490`): **770** for MW2SHELL, **1537** for MW2, **244** for the retail NETMECHW.DLL (before the DPLAY import thunks at `0x10013b70`; its CRT starts at `0x10013b7c`), and **21** for the retail MECH2.EXE (before its import thunks at `0x004022be`). Without these flags, reccmp divides by the annotated functions only, which overstates progress. `--total` is only a floor — reccmp uses whichever is larger, the annotated function count or `--total`. The CRT is left out until the game code is done: its `LIBRARY` entries only match once the game code that calls them is linked. The public-target CI list is in `.github/workflows/build.yml` (`targets` job). The retail targets (NETMECHW, MECH2) are listed in `.github/workflows/verify-retail.yml`, a `workflow_run` workflow that runs after Build: it checks the originals out of a private repository and runs the base branch's reccmp and check scripts against the PR's sources and the Build run's artifact. It also posts the PR comment and, on pushes to main, publishes every target's progress report (release and GitHub Pages).

Linux notes: reccmp runs `wine cvdump.exe` and `winepath`; `winepath` must be on PATH, tool output must go to a file rather than a pipe, and a persistent wineserver avoids per-call stalls. Run reccmp from the build directory: it finds `reccmp-build.yml` by searching the current directory and its parents. A build under Wine (e.g. the Docker image) writes `project: 'Z:/…'` into `reccmp-build.yml`, but the target paths are relative to the file, so Linux reccmp can read the build as long as the build directory sits inside the repository (it finds `reccmp-project.yml` by searching upward). A build directory outside the repository needs its `project:` path rewritten.

## Annotations

Functions in a compilation unit must be ordered by address (ascending). Object boundaries: a 16-aligned start is an object boundary only when `0xCC` padding precedes it; aligned starts flush against the previous function are mostly mid-object.

```cpp
// FUNCTION: MW2 0x10003580        — complete decompilation, compared by reccmp
// STUB: MW2 0x10003600            — incomplete, skipped by reccmp
// LIBRARY: MW2 0x10080490         — CRT/3rd-party (in library_msvc.h, inside #ifdef 0)
// SYNTHETIC: MW2SHELL 0x10007040  — compiler-generated (e.g. scalar deleting destructors)
// GLOBAL: MW2 0x100acb2c          — global variable
// VTABLE: MW2SHELL 0x10064000     — virtual function table
// SIZE 0xc8                       — struct/class size assertion
```

**The colon is required** for every annotation except `// SIZE`. reccmp silently ignores a colon-less `// VTABLE MODULE 0x…`; vtable set sites inside ctors/dtors then show as `<OFFSET2>` instead of `ClassName::vftable`, costing 5–10% match.

**A `//` comment line must not end in a backslash.** It continues the comment onto the next line, so a path like `MEK\` at the end of the line above an annotation swallows the annotation (and clang-format then reflows it into the comment); reccmp loses the function without an error. Reword the line.

A `// GLOBAL:` marks the address of the pointer variable itself. If the variable points at data (e.g. a `char*` string literal), the data address belongs in a `reccmp/` data-source CSV (added as needed, following the reccmp convention).

Run `reccmp-datacmp` after adding/modifying globals with non-zero initial values to verify they match.

**Blocks of globals are one struct.** When the original reads, writes, copies or clears a run of globals as one unit (`fread(&first, 0x3c, 1, file)`), declare the run as a single struct global. Separate globals only sit together by accident, in the declared order and within one translation unit, and not at all in the modern-compiler builds; datacmp and reccmp compare symbol by symbol and pass either way. `SoundConfig g_soundConfig` (MW2SND.CFG) is the example. CI enforces this from both sides: `tools/check_block_sizes.py` checks the source's constant-size block calls against their objects, and `tools/check_block_layout.py` checks the original's block calls and inline `rep stos`/`rep movs` against the recompiled object at each address. The same tool fails on a global declared larger than the original's room for it before the next known address: in the original, code reaching its last elements touches the neighbour (the modifier bits in the last word of the key states, a NULL terminator the original's table doesn't have), and datacmp can't see that either.

When a shared function carries annotations for more than one target, they follow the target order in `reccmp-project.yml`: MW2SHELL, MW2, NETMECHW, MECH2.

**CRT names differ between the debug and release CRT**: LIBCMTD (MW2) carries CodeView procedure symbols, so reccmp sees the C name (`_vsnprintf`, `_DllMainCRTStartup`); LIBCMT (MW2SHELL) has only publics, so it sees the decorated name (`__vsnprintf`, `__DllMainCRTStartup@12`). `library_msvc.h` therefore matches public CRT functions by their public symbol, which both libraries share: `// LIBRARY: MW2 0x10080490 SYMBOL` followed by `// _sprintf`. Static CRT functions have no public symbol: MW2 names them by their CodeView C name (when unique), and MW2SHELL's stay unmatched but still mark library addresses. The 2.2 CRTs (NETMECHW's `LIBCMT`, MECH2's `LIBC`) have only publics too, so they follow MW2SHELL's rules. At an alias address (`__chkstk`/`__alloca_probe`), use the symbol reccmp keeps.

**reccmp reads annotations from the source tree, not the build**: every annotated file must be in the build, or reccmp logs errors for the functions it can't find.

**Don't name a unit after a CRT source file.** reccmp matches line records by file name, so a unit sharing a name with a CRT object's source (the CRT's `input.c` is the scanf engine) picks up its line records, and a function lands in "Debug data out of sync" and drops out of the compare. MW2's input unit is `inputmap.c` for this reason.

## Class Pattern (C++, MW2SHELL)

C++ exists in MW2SHELL and NETMECHW. MW2 and MECH2 are plain C — see the struct pattern below.

```cpp
// header:
// VTABLE: MW2SHELL 0x10064000
// SIZE 0x1d6c
class RandomName0x1d6c {
public:
    virtual ~RandomName0x1d6c();    // vtable+0x00
    virtual void VTable0x04();      // vtable+0x04

    // SYNTHETIC: MW2SHELL 0x10007040
    // RandomName0x1d6c::`scalar deleting destructor'

private:
    int m_unk0x04;                      // 0x04
    float m_unk0x08;                    // 0x08
    undefined m_unk0x0c[0x100 - 0x0c];  // 0x0c
};

// source:
DECOMP_SIZE_ASSERT(RandomName0x1d6c, 0x1d6c)
```

Member offset comments (`// 0xNN`) and vtable offset comments (`// vtable+0xNN`) are required.

**Gap members.** Use a subtraction for gap arrays so the size is self-documenting:

```cpp
undefined m_unk0x05[0x7dc - 0x05];   // 0x05
undefined m_unk0x92c[0x944 - 0x92c]; // 0x92c
```

**Overrides.** Use `override` instead of `virtual` for derived methods (`void VTable0x04() override;`). `override` is defined as empty for pre-C++11 compilers in `compat.h`.

**Struct pattern (C, MW2).** No vtables, SDDs or `this`. Identify the big global structures and the free functions that operate on them; struct members still take `m_` (ncc's `StructMember` rule), parameters `p_`, globals `g_`. State is heavily global — recover the `.data` layout alongside the functions. Prioritize functions that pin down a struct's layout.

## Code Style

- **Bit tests:** `if (flags & c_flagCached)` / `if (!(flags & c_flagCached))` — no `!= 0` / `== 0`.
- **Address padding:** 8 hex digits, lowercase: `0x10003580`.
- **Annotation ordering:** when a function has annotations for several targets, they go in the order MW2SHELL, MW2, NETMECHW, MECH2.
- **No redundant `this->`.** Write `m_member`, `Method()`, `BaseClass::VirtualMethod()` directly.
- **Win32 API: prefer un-suffixed names.** Use `CreateWindowEx`, `DEVMODE`, `MSG`, `WIN32_FIND_DATA`, etc. — NOT `CreateWindowExA`/`DEVMODEA`. The un-suffixed names are macros that resolve to the `A` form when `UNICODE` is undefined; the compiled binary still imports the `A` symbols. Disassemblers show the resolved `*A` symbol — translate back to the macro.
- **Pointer/bool constants:** use `NULL` for null pointer assignments/returns, `TRUE`/`FALSE` for boolean values, and plain `0` only for scalar values and status codes.
- **Blank lines inside functions:** write functions in readable paragraphs. Keep tight sequences of the same kind together, separate declarations/setup, the main operation, and result handling with single blank lines. If an `if`/loop block closes and the next statement is a `return`, put a blank line before the `return`.
- **Enums for magic numbers:** hoist fixed enumerations (flag bits, event tags, state codes) into a named `enum` at class or namespace scope. `c_camelCase` per NCC.
- **No leading `const` on return-by-value** (`const RetType Get() const` — meaningless, trips NCC).
- **Language per translation unit.** A unit is `.c` or `.cpp`, decided by which one matches — not by target. If a unit won't match as C, try C++ (and vice versa) before contorting the source.
- **Declarations live in headers.** A unit whose functions or globals other units use has a header named after it (`video.cpp` → `video.h`; a class unit adds them to its class header), declaring them as the definitions do. The unit includes its own header, and C units' headers wrap the declarations in `extern "C"` guards. Source files don't redeclare another unit's functions or globals. Adding a declaration can flip a comparison's operand order in any unit that includes the header (see "Codegen Patterns"), so rerun the full compare after header changes.
- **clang-format the files you touch.** NCC naming runs in CI only.

## Naming Conventions

Uses LEGO Island NCC rules (`tools/ncc/ncc.style`), enforced in CI, for C and C++ alike:

- Functions: `FUN_XXXXXXXX` (8 hex digits, lowercase)
- Globals: `g_unk0xXXXXXXXX`
- Members: `m_unk0xXX` (by offset) — includes C struct members
- Parameters: `p_unk0xXX`
- Unknown classes/structs: `RandomName0xSize` (random PascalCase + `0x` + hex size, e.g. `NeonCactus0x1d6c`)
- Virtual methods: `VTable0xXX` (by vtable offset; C++ only)
- Enum constants: `c_` prefix
- The `p_`/`m_`/`g_` prefixes apply to _all_ parameters/members/globals, not just unknown placeholders.
- Names must match across prefixes when assigned: `m_hInstance = p_hInstance`, never `m_hInstance = p_something`
- **Unused parameters** must be unnamed in the definition (C++ only: C requires every parameter in a definition to be named).

**Ghidra names** often won't follow these rules. Translate them into NCC form when adopted: a Ghidra global `mechCount` becomes `g_mechCount`, a Ghidra struct field `hp` becomes `m_hp`, a Ghidra `local_`/`param_`/`DAT_` becomes the `m_unk0x…`/`p_unk0x…`/`g_unk0x…` placeholder rather than an invented name.

## Types

Use the project vocabulary from `util/types.h` for game code:

```c
typedef signed char MechS8;    typedef unsigned char MechU8;
typedef short MechS16;        typedef unsigned short MechU16;
typedef int MechS32;          typedef unsigned int MechU32;
typedef float MechFloat;      typedef double MechDouble;
typedef char MechChar;        /* plain char for text */
```

- Typedefs are codegen-neutral; the _underlying_ type (signed vs. unsigned, `char` vs. `int`, `float` vs. `double`) is what affects `/Od` codegen, so keep the width and signedness visible at every declaration.
- **Unproven types:** use `undefined`/`undefined2`/`undefined4`/`undefined4*` from `decomp.h`. Do not guess `int`/`float`/`void*` until usage context or reccmp proves it.
- Keep original types at API boundaries (Win32, DirectX, Miles, Smacker, CRT); `void*` can remain.
- Boolean typedefs (`MechBool`…) are added only when a match proves the width and signedness.

## Decompiling a New Function

1. **Find the decompilation and check the byte budget.** Read the body; note called functions and globals. The gap to the next function's address bounds the implementation — a rich decompiled body that cannot fit means the function is a thin wrapper and the real logic lives in a callee. Disassemble suspicious gaps before writing code.
2. **Check calling conventions.** Per-call-site guesses can differ from the real definition — cross-reference. `mov ecx, X; call F` indicates `__thiscall`, even if F never reads `this`. `__stdcall` for the DLL exports (`SimMain`, `ShellMain`, the window procs).
3. **Identify classes from `__thiscall` on a global** (MW2SHELL). That global is a class instance; declare a class with `undefined m_unk0x00[size]`.
4. **STUB every unknown callee.** Required for build + reccmp. Use the `STUB(0xADDRESS)` macro (from `decomp.h`) in stub bodies; stubs are ordered by address ascending per file.
5. **Write clean C/C++, not disassembler pseudocode.** Translate `*(_DWORD*)(this + 4)` into proper member access, method calls, named variables. No gotos, no raw float bit patterns.
6. **Build**, then `reccmp-reccmp --target MW2 --verbose 0xADDRESS --print-rec-addr`. Iterate toward 100%; stop at a documented compiler artifact (see "`// FUNCTION:` scores" below).
   6b. **Verify global data** with `reccmp-datacmp`.
7. **Validate vtables** (MW2SHELL): `reccmp-reccmp --verbose 0xVTABLE_ADDR`. Every declared virtual needs a matching annotation with its real address from the original binary.
8. **Check for regressions.** Re-verify previously matched functions that touch modified classes.
9. **Lint.** `reccmp-decomplint` from `build/`, passing the source directory as a path argument.

**Language oracle.** At `/Od` the C++ front end ends a `void` function with a `jmp` to the epilogue; the C front end doesn't, unless the source has an explicit `return;` (`dispdib.c`'s empty `FUN_1002ee30` is `{ return; }`). Functions that return a value compile identically either way, except around `/Ob1`-expanded inline functions (see "Inline expansion" below). Use this to decide `.c` vs `.cpp` when a unit is ambiguous.

## Hand-written Assembly (VC++ 4.1)

The original game contains hand-written assembly. Whole MASM objects are reproduced as MASM 6.11 source (form 4); assembly inside compiled objects as C/C++ source that the MSVC inline assembler emits byte-for-byte (forms 1 and 2). Four forms:

**1. Inline `__asm` inside an otherwise-C function** (the common case; verified: MW2's `FixedMul16` and `MulDiv64` are exactly this):

```c
// FUNCTION: MW2 0x10003580
MechS32 FixedMul16(MechS32 p_a, MechS32 p_b)
{
	__asm {
		mov eax, p_a
		mov ebx, p_b
		imul ebx
		shrd eax, edx, 16
		adc eax, 0
	}
}
```

If the function ends without a `return` statement (result left in `eax`), add `#pragma warning(disable : 4035)`.

For non-MSVC builds, guard with a compiler-version check and a portable fallback (reference pattern from the LEGO Racers decomp, `common/src/golcpu.cpp`):

```cpp
#if defined(_MSC_VER) && _MSC_VER < 1100
#define DETECT_ASSEMBLY 1   // VC++ 4.x: use __asm
#else
#define DETECT_ASSEMBLY 0   // portable fallback (__cpuid, memcpy, ...)
#endif
```

**2. Whole assembly routines inside a compiled object** (no MASM object boundary before them) become `__declspec(naked)` functions whose body is one `__asm { … }` block, annotated `// FUNCTION: <MODULE> 0x…` like any other function. Branch targets are labels named after the original address. Non-MSVC builds put the naked functions inside `#ifdef COMPAT_MODE … #else … #endif` with `STUB()` bodies on the `COMPAT_MODE` side. `COMPAT_MODE` comes from `compat.h`, so a unit with naked functions must include it: otherwise libclang (NCC, the block-size check, clang-tidy) parses the `__asm` bodies, and it crashes outright on `push offset symbol` (`rendertarget.c`). Keep such instructions out of C functions with `__asm` bodies for the same reason: make the routine naked. Reference pattern, preserved from the upstream LEGO Racers decomp (`GolDP/src/render/golrasterizers1.cpp`, with its original GOLDP annotation — in this project the annotation carries the real MW2/MW2SHELL address of the routine being matched):

```cpp
// FUNCTION: GOLDP 0x10033270
__declspec(naked) void FUN_10033270()
{
	__asm {
		lea edi, [ebx + edx*2]
		and edx, 1
		mov eax, dword ptr [esp + 0x168]
		je jmp_10033288
		mov word ptr [edi], ax
		dec ecx
		lea edi, [edi + 2]
		js jmp_1003329b
jmp_10033288:
		dec ecx
		js jmp_10033298
jmp_1003328b:
		mov dword ptr [edi], eax
		sub ecx, 2
		lea edi, [edi + 4]
		jns jmp_1003328b
		inc ecx
		jne jmp_1003329b
jmp_10033298:
		mov word ptr [edi], ax
jmp_1003329b:
		ret
	}
}
```

**3. Data owned by assembly routines** (lookup tables, jump tables) gets `// GLOBAL:` annotations and is checked with datacmp. A MASM object's data, including what it keeps in `.text`, is defined in its `.asm` (form 4).

**4. MASM objects** become `.asm` sources assembled with MASM 6.11 (ML) in the VC++ 4.1 build: MW2's `ticks.asm` and `sndunpack.asm`. The three MASM objects both DLLs link are a third-party library, Miles Design's VFX, built from its own source (see "Miles Design VFX" below). An object was MASM when it starts at a boundary a compiled object can't have (4-aligned and flush against the previous object, where VC++ pads to 16 with `0xCC`) and has MASM fingerprints: short jumps wherever they reach (the inline assembler can't emit a short conditional jump), `cs: mov eax, eax` or `mov eax, eax` padding before a label, data or jump tables inside `.text`, frames that save only the registers they use, or ML's own frame. ML's `PROC` frame is exactly the `/Od` frame: `push ebp; mov ebp, esp; add esp, -N` for `LOCAL`s (the compiler writes `sub esp, N`); `push` of the `USES` registers; and `pop`s, `leave`, `ret` at every `ret`. So a `/Od`-framed function without locals inside a MASM object's range is a `PROC` too.

`tools/asm2masm.py` writes the source from the original and checks it:

```sh
python tools/asm2masm.py --target MW2 START END --data DSTART DEND --comments OLD -o unit.asm \
    --check --ml <path-to-masm>/BIN/ML.EXE [--wine <wine>]
```

START/END is the object's code (trailing `int3` padding is dropped), DSTART/DEND its `.data` and `.bss`. Routines start at the module's annotations (C sources and by-name header markers); `--split A,B` adds unannotated starts. It rebuilds each routine as `Name proc uses …, p_x:dword` with `local`s wherever ML's frame reproduces it (parameter names from the C declaration), names every relocated address from the annotations, labels jump targets `jmp_<address>` (an operand the code patches as `jmp_<instruction>+k`), and turns padding into `align 4`. `--comments` carries the comments over from an existing `.asm` or `.c`, so regenerating after an annotation change keeps them; the committed `.asm` files are its output. `--check` assembles the result and compares the code, the data and every relocation (presence and target) with the original; it reports what it can't name as `UNK_<address>` (annotate it and rerun). Regenerate rather than hand-edit instructions.

- **Annotations by name in the unit's header** (`ticks.h`, `sndunpack.h`, and `common/include`'s `vfxa.h`/`vfx3d.h`/`vfxrend.h`): reccmp only reads C/C++ sources, so they carry `// FUNCTION: MW2 0x…` / `// GetTicks` pairs and `// GLOBAL:` pairs. ML's `/Zi` gives each `PROC` an `S_GPROC32` record with its real length, so reccmp compares whole bodies.
- **Data typing.** ML's debug information types a data definition as one element, so datacmp and `check_block_layout.py` would see a table's first element only. The generator wraps arrays in a `STRUCT` (datacmp compares the whole struct) and types addresses as pointers (`CODEPTR`/`DATAPTR`), one member each: datacmp compares a typed pointer by the symbol it points to, an untyped one by its value.
- **ML's operand conventions**, which the generator applies: of an unscaled `[r1+r2]` ML takes `r2` as the base, and `xchg`/`test r, r` put the first operand in the ModRM `reg` field.
- **Build.** A unit is listed in `CMakeLists.txt` as `<dir>/<name>.${asm_or_c}`: the VC++ 4.1 build assembles the `.asm`, other compilers (`COMPAT_MODE`) compile the `.c` of data definitions and the routines' portable C (`ticks.c`, `sndunpack.c`; "Portable C" below), which carries no annotations. ML takes no C options: a target's C flags go inside `$<$<COMPILE_LANGUAGE:C,CXX>:…>` (ML stops parsing at `/G5` and runs the linker).
- `tools/check_units.py` accepts a 4-aligned, flush start for a `.asm` unit.

### Miles Design VFX (`3rdparty/vfx`)

Three MASM objects both DLLs link, byte for byte the same in both, are John Miles's VFX graphics library: MW2's credits (`0x100af438`) thank him for "the graphics and sound packages" and "the world of PANES". His July 2000 open-source release (`3rdparty/vfx/README.TXT`) has their source, and the build assembles it: `VFXA.ASM` (VFX 1.15's 2D primitives: pixels, lines, shapes, panes, ellipses, fonts, ILBM/PCX/GIF, fades), `VFX3D.ASM` (its polygon fillers) and `VFXREND.ASM` (VFXREND 1.06, the polygon renderer and its self-patching primitives, the "code block"), with `VFX.INC` and `RENDOPTS.INC`. `VFX.H` and `VFXREND.H` document the C API and aren't built.

- **Changes to the release** sit between `; demech2: begin change.` and `; demech2: end change` comments that say why, and leave the objects identical to the game's (code, data and every relocation's target). MW2's own: `VFX.INC`'s `RSTOSB32`/`RMOVSB32` align the destination to a dword (the shape drawers), `RXLAT32` isn't unrolled, `VFX_pane_wipe` and `VFX_flat_polygon` align their fills, `VFX_character_draw` copies bytes, `RENDOPTS.INC` builds MW2's 25 primitives in its order (the table's entries give each one's flags), and VFXREND has `GetCodeBlock` (not in 1.06, name unknown). For the tools: VFXREND writes its table in order (ML 6.11 misplaces the fixups of a `dd` written after an `ORG` back), outside any macro (ML's debug information makes bytes a macro emits outside a `PROC` into an unnamed `$$$` procedure, which then names the table), and `VFXA.ASM` makes three statics public for the tests. The files are LF and UTF-8 (the release's are CRLF and code page 437).
- **Build:** ML `/coff /Cx /DMASM /DFLAT_MODEL`, with `3rdparty/vfx` on the include path (`CMakeLists.txt`'s `${vfxa}`, `${vfx3d}`, `${vfxrend}`). `COMPAT_MODE` builds compile the portable C in `common/src` (`vfxa.c`, `vfx3d.c`; "Portable C" below) and `common/src/vfxrend.c`'s stubs. Both DLLs include the headers from `common/include`, and reccmp reads annotations there too (`common` is in both targets' `source-root`).
- **Names are VFX's**, as the assembly defines them: functions (`VFX_pane_copy`, `DrawShapeUnclipped`, `GIF_getb`, `poly_1681`…), the driver's entry points (`VFX_describe_driver`…), `PANE` (a rectangle of a `WINDOW`, `common/include/pane.h`), `WINDOW` (`window.h`; the game keeps its bitmap info where VFX has the stencil) and `VFX_TEXTURE`, members and parameters in NCC form (`m_x0`, `p_shapeTable`). `tools/ncc/ncc.style` accepts VFX's function and type names, `skip.yml` its exported globals. The C's own helpers and statics keep NCC names.
- **Annotations:** `common/include`'s `vfxa.h`, `vfx3d.h` and `vfxrend.h` annotate the routines and VFX's public data by name, each marker naming both DLLs' addresses (MW2SHELL's, then MW2's). The objects' other data are static labels, several at one address and some with the same name in two objects, so `reccmp/<target>-vfx.csv` names them at their original addresses instead, under the name reccmp takes from the rebuild: regenerate it with `tools/vfx_names.py` after changing a VFX data label.
- **reccmp's limit:** `VFX_map_polygon` scores about 93%. VFX keeps its table of span routines (`__map_logic`) and the span routines inside the procedure, and reccmp reads everything from the table on as data, so the relocated operands in that code compare by value; the object itself is identical. `check_block_layout.py` leaves the VFX objects out: VFX declares its buffers as labels and untyped data, which ML's debug information sizes as one element, and its blocks of separately named variables are by design (`VFX_register_driver` copies a driver's table over its 13 entry points).

**VC++ 4.1 inline-assembler limits (verified):** it accepts every 386/486 and x87 instruction, and rejects everything Pentium and later (`cpuid`, `rdtsc`, `cmpxchg8b`, `cmovcc`, MMX…) as well as data directives (`db`/`dw`/`dd`). Rejected opcodes must be emitted with `_emit`. Its operand order for `xchg r, r` and `test r8, r8` is fixed (`87 da` for `xchg ebx, edx`, and `xchg edx, ebx` is the same instruction to it), so the other encoding (`87 d3`), which MASM gives for `xchg edx, ebx`, needs `_emit`. Because there are no data directives, a routine with an embedded jump table of label addresses cannot be written as one naked function (`_emit` bytes can't carry relocations): assemble that object with ML (form 4; `VFX_map_polygon` in `VFX3D.ASM`). A C initializer can't express `function + offset`: 4.1 silently drops the addend. Its operand parser also drops a register written before a symbol: `[ebx+g_table]` assembles as `[g_table]`, so write the symbol first (`[g_table+ebx]`). A `jmp` to a label assembles as `rel32` unless written `jmp short`, backward as well as forward, while conditional jumps stay `rel32` either way; every short `jmp` in the original needs the `short` (`FUN_100698de`, `unk100696c0.c`). Short conditional jumps can't be written at all: a routine with them came from a real assembler (form 4). `near` and `far` are keywords to 4.1, so they can't name locals. Nor can register names: a local `dx` is read as the register (`mov eax, dx` fails with C2443, operand size conflict; `unk10071930.c`). An `__asm` block can jump to a C label, which then holds the C `return` (`FUN_10071930`). clang-format mangles an `__asm` block that follows a C label right after another `__asm` block; fence such a run with `// clang-format off`/`on` (`FUN_10049155`).

DOS-era routines may use custom register calling conventions (inputs in `ebx`/`edx`). Callers in C then need the same inline-asm setup, or are themselves asm. Original MASM/TASM objects may have their own section alignment or padding between functions; if addresses drift around an asm block, that is the first suspect.

**How to spot assembly** — apply per function, when it is picked up. At `/Od`, VC++ 4.1 gives **every** C and C++ function the same frame:

```
push ebp; mov ebp, esp; [sub esp, N;] push ebx; push esi; push edi
…
pop edi; pop esi; pop ebx; leave; ret [N]
```

`ebx`/`esi`/`edi` are saved even when unused; an empty function is exactly this, `0xb` bytes. Checklist, strongest signal first:

1. **Outside the game-code range?** CRT functions (MW2 from `0x10080490`, MW2SHELL from `0x100492e0`) and the shell's import thunks (`0x100492a2`–`0x100492de`) are library code, not game assembly. Use `// LIBRARY:`.
2. **No `/Od` frame at all** (missing `push ebp; mov ebp, esp`, or missing/partial `push ebx; push esi; push edi`) → a whole assembly routine → `__declspec(naked)`, or form 4 when its object is MASM. A frame that saves only the registers it clobbers suggests MASM `PROC … USES`. _Exceptions that are still C:_ SEH functions (`push -1; push <scopetable>; push __except_handler3; mov eax, fs:[0]` after `mov ebp, esp`, as in `SimMain`), their `__finally` funclets, and C++ EH functions (`ShellMain`: EH state counter and unwind funclets).
3. **`/Od` frame, but the body doesn't look like `/Od` output** → C function with an `__asm` body (the `FixedMul16` pattern). `/Od` spills every value to `[ebp-N]` and reloads it for each statement; it never keeps results in registers across statements. Instructions it doesn't generate are a giveaway: `adc`/`sbb` after arithmetic (`shrd eax, edx, n; adc eax, 0` rounds with the bit shifted out; C spells that as a second `shrd` plus `and`/`add`), a `div`/`idiv` of a 64-bit dividend (C's `__int64` division calls `__alldiv`/`__aulldiv`), `bswap`, `xlat`, `lods`/`stos`/`movs`/`scas` (with or without `rep`) outside the CRT, `loop`/`jecxz`, `pushad`/`popad`, `in`/`out`, `cli`/`sti`, and x87 transcendental forms (`fsin`, `fpatan`, `fyl2x`…). `shld`/`shrd` alone prove nothing: 4.1 emits them inline at `/Od` for `__int64` shifts (`(__int64) a * b >> 30` is `imul; mov ecx, edx; shrd eax, ecx, 30`). A body that falls into the epilogue with its result in `eax` and no `jmp` is an `__asm` block without a `return`: a C `return` always ends in a `jmp` to the epilogue (`FixedDivU16`, `FixedMul30`). Bare (non-`rep`) string instructions in particular: `/Oi` intrinsics always `rep`-prefix variable-size forms and inline constant-size forms as plain `mov`s.
4. **Register-based parameters:** callers load `eax`/`ebx`/`edx`… right before a `call` without pushing arguments, or the callee reads registers it never set. Both caller and callee are then likely assembly.
5. **Layout hints:** inside a 16-byte-aligned unit, VC++ packs functions back to back with no padding. Padding _inside_ a unit (`0x90`, `0x00`, or alignment between functions) points to an assembler-built object. So do non-switch data blobs inside `.text` (VC++ puts switch jump tables inside the function body; those are C). In MW2SHELL they are VFXREND's table of primitives and VFXA's cosine table (`0x10035af0`).
6. **Inline `__asm` doesn't compile?** Check the limits above: `_emit` for 586+ opcodes; label-address tables need ML (form 4).

`tools/asm2c.py` prints a routine of the original as inline-asm text following these rules (`python tools/asm2c.py --target MW2 START END`, or `--body --params …` for a C function with an `__asm` body): short jumps as `_emit` pairs, the ModRM-swapped instructions as `_emit` bytes, and annotated functions and globals by name. It prints what it can't name as `/* ??? */`; define or stub those and rerun. It is for forms 1 and 2, assembly inside compiled objects; a MASM object goes through `tools/asm2masm.py` (form 4).

Record each confirmed routine in the file's header comment, so the list grows as a by-product of decompilation. **An honest C near-miss beats an asm transcription** — don't convert a function to `__asm` just because the C doesn't match yet.

### Portable C (modern builds)

The modern-compiler builds replace the hand-written assembly with portable C, proven equivalent by testing (`tests/asmequiv`); the 4.1 build keeps the assembly, so reccmp is unaffected. Done so far: every MW2 function with an `__asm` body or block (the fixed-point helpers, `approxlen.c`, `sqrtguess.c`, `namehash.c`, `integrate.c`, the `unk*.c` arithmetic helpers, `MemCopy`/`MemSet` in `loadres.c`, the matrices of `transform.c`, the shapes of `unk10039a30.c`, the projection and clipping of `unk10046750.c`, the trigonometry in `unk100696c0.c`, the horizon in `unk10071930.c`, the record stacks in `unk1007d120.c`), the MASM objects `ticks.asm` and `sndunpack.asm`, and VFX's `VFXA.ASM` and `VFX3D.ASM` (`common/src/vfxa.c`, `vfx3d.c`; VFXREND still has stubs). None of these is x87 code. `debugbreak.c`'s `int 3` already compiles only for x86 MSVC.

- **Where:** an `__asm` function gets an `#ifdef PORTABLE_C` branch inside its body, above the `#else` that keeps the assembly (`fixedmul.c`). Where the function's C would overflow in standard C (the 4.1 code wraps), the branch replaces the whole body (`unk10071930.c`, `FUN_1006975b`). Newer compilers reject an `__asm` block that jumps to a C label, so such a function's portable C is selected by `defined(PORTABLE_C) || !defined(_MSC_VER) || _MSC_VER >= 1100` (`PORTABLE_C_LABELS` in `unk10039a30.c` and `unk10046750.c`), and `asmequiv`'s `HasReference` skips it with a newer compiler's reference. A MASM object's portable C is its `COMPAT_MODE` `.c` (`ticks.c`): the reference DLL assembles the `.asm` (4.1 build only: modern builds have no MASM, and `asmequiv` skips those routines there), the candidate compiles the `.c`. Its register-convention helpers are static functions of the `.c`, tested through the entry points. So are its working variables, where no call reads what an earlier one left (VFX3D's, all but its lookaside table; VFXA's row buffers, row tables and run-length encoder): the C keeps them in locals. What a later call reads stays (VFXA's driver entry points and name, its lookaside table and the fade's error terms, which the tests read and set by name; VFX3D's lookaside table, which they read back through `VFX_map_polygon`). Return values the assembly leaves behind without setting them (a local it never set, a pointer it last computed) aren't reproduced, and the tests don't compare them. `compat.h` defines `PORTABLE_C` in every `COMPAT_MODE` build; a 4.1 build selects it with `/DPORTABLE_C`, and an x86 MSVC build keeps the assembly with `/DREFERENCE_ASM` (the tests use both).
- **Defined behaviour only**, in the project's types: `MechS64`/`MechU64` and the helpers for register-level operations (`PortableS32`, `PortableSar32`, `PortableBsr`, …) are in `util/portable.h`, not `types.h`: a typedef in `types.h` is a symbol in every unit, and adding two shifted the scores of about 120 functions across the four targets. Inputs where the original faults (`idiv` by zero or overflowing) are out of domain: the test's predicate must match the reference's faults exactly. So are inputs where its result is undefined (`bsr` of 0): neither side is called. The C asserts every precondition with `PORTABLE_ASSERT`, active in the tests (`PORTABLE_ASSERTS`) and in debug builds of the portable C (`_DEBUG`), and may do anything there otherwise; every division goes through `PortableIdiv`/`PortableDiv`, which assert that it can't fault.
- **Tests** (`tests/asmequiv/CMakeLists.txt`): `asmequiv.exe` loads `asmref.dll` (the assembly) and `asmport.dll` (the portable C), built from the same sources, and compares every case of each routine (boundary values, then seeded random ones, each generated from its index alone); `asmgolden` checks the portable C, linked in, against `golden.txt`, which the 4.1 reference writes; ctest runs it as eight shards (`asmgolden-0` to `-7`, `-shard K N`: every Nth line of the vectors), which CI's `ctest -j4` runs in parallel. `asmequiv -exhaustive` runs the routines with one argument word (`FixedSqrtGuess`) on all 2³² inputs; it takes about a minute, so it's a ctest test (`asmequiv-exhaustive`) only with `-DASMEQUIV_EXHAUSTIVE=ON`, and CI doesn't run it: run it after changing such a routine. `ctest` runs the others in both Windows jobs; the `portable` CI job builds `tests/asmequiv` on its own (a standalone project) on Linux x86-64 with GCC and Clang under UBSan and ASan. A routine that takes more than `MechS32` arguments has a runner in `routines.c`: it builds the state the case reads from the case's argument words (buffers filled with random bytes, guard bands included; tables; globals, which the DLLs export as `DATA`), calls the routine and records its return value and every word it may write. Generate from one random draw per statement: the order of two calls in one expression is unspecified, and MSVC and GCC differ. Runners call each routine through its own function type (Clang's UBSan checks), and `asmequiv` calls through a thunk that restores `ebx`, `esi`, `edi`, `ebp` and `es` (`ResetTicks` returns with `ebx` and `ecx` swapped, `VFX_pane_scroll`'s fill with `es` 0). Generators produce what the game could pass: convex polygons, well-formed shapes, fonts and pictures (with a ByteRun1 and an LZW encoder), and nothing the original faults or loops forever on (a domain predicate excludes those it can compute, a comment above the runner documents the rest). A case that passes a pointer in a 32-bit word (`VFX_line_draw`'s table and callback modes) is `c_domainPointers`: `asmequiv` compares it, the golden vectors only count it. A runner outputs pointers as the region and offset they point to (`Locate`), and structures with pointers member by member, so that the outputs are the same on every platform. `FUN_10049155` copies pointers into the 32-bit polygon records, which overlap where pointers are wider, so `asmgolden` checks it on x86 only (`RunsHere`). Hooks a routine calls (`g_unk0x100a6cc8`'s) are harness functions that log what they're passed; they're called from inside the module, so they never call through `asmequiv`'s thunk. A new routine goes into the units list (a MASM object's into `asmequiv_masm_units`), `asmequiv.def`, the routine table in `routines.c` (with its domain and runner) and the symbol table in `golden.c`; whatever its unit references beyond the routines goes into `stubs.c` (and `posix/windows.h` for the Linux build); then copy `<build>/tests/asmequiv/golden.txt` over the committed one (with LF line endings).
- **Coverage:** in the 4.1 build, the `asmequiv` test also checks that the cases reach every basic block of the reference, in the same run that writes the golden vectors: `tools/asmblocks.py` lists the blocks of `asmref.dll` (the `asmequiv-blocks` test, with capstone; CI's 4.1 job installs it), and `asmequiv -coverage` puts an `int3` on each. Without a Python that has capstone (the Wine build), run them by hand: `python tools/asmblocks.py <build>/tests/asmequiv/asmref.dll -o blocks.txt`, then `asmequiv asmref.dll asmport.dll -coverage blocks.txt`. Block coverage doesn't test boundaries: a comparison's equality case needs cases of its own (`ArcsineArgument` puts the sine on table entries). A routine's blocks include its helpers: the calls into code without an export between it and the next export, every call to such code from the routines in `asmblocks.py`'s `HELPER_CALLERS` (whose helpers ML put elsewhere, even before them: negative offsets), and after an indirect `jmp` the code a jump table in its range leads to (`VFX_map_polygon`'s span routines). Blocks no input reaches are listed with the reason in `asmequiv.c`'s `c_deadBlocks`.

## Codegen Patterns (VC++ 4.1 `/Od`)

This section only grows as patterns are **proven by matches**:

- **Unoptimized code is literal.** Expect near-1:1 statement mapping; matching is about exact types, statement order and local declaration order, not register coaxing. Dead stores and `jmp`-to-next come from source constructs (e.g. a `break` at the end of a `switch` case, or an empty `else`) and are clues, not noise.
- **Empty/thin functions:** the universal `/Od` frame (see above), `0xb` bytes for an empty function.
- **Comparison operand order follows symbol order, not source order.** `a > b` and `b < a` compile identically; 4.1 chooses which side of `<`/`>`/`==` goes in `eax` from the symbol table. If a comparison is reversed vs. the original, reordering the operands' _declarations_ fixes it — even the count of extra symbols declared ahead of the function in the TU can flip it. Global _definitions_ count the same way: reordering the `// GLOBAL:` definitions in the TU fixed `KeyboardReadKeyCode` and `SimMain`'s objective loop. Array indexing shows the same effect: `p->m_items[i]` loads the base first in some functions and the index first in others (`CollectionGet`, `ClearCollection`), and declaration order didn't flip those — accept after one attempt.
- **`x * -1` is not `-x`.** `p_height * -1` compiles to `mov ecx, eax; add eax, eax; sub eax, ecx; neg eax`; `-p_height` and `0 - p_height` give a lone `neg` and `xor`/`sub` (`InitBitmapInfo`).
- **`int -= int` after `&=` negates twice.** `n &= 0xfff; rem -= n;` with both `int` compiles to `xor eax, eax; sub eax, [n]; neg eax; sub [rem], eax`; an unsigned `n` gives the plain `mov`/`sub` (`TMPackDataBase::GetDBItemLZ`). The same double negation shows `-=` against a plain subtraction chain: `c -= p + 1` computes `p + 1` in `ecx` (`mov ecx, [p]; inc ecx; sub eax, ecx`), while `c = c - p - 1` compiles to `xor eax, eax; sub eax, [p]; dec eax; neg eax; sub [c], eax` (`FUN_100020fa`).
- **`x < 0` and `x <= -1` compile differently.** On a signed global, `< 0` tests with `jns`; `<= -1` compiles to `cmp x, -1; jg` (`EndBlock`).
- **An unsigned operand keeps `a = a - b` and `a = a + b` plain.** With a signed `b`, both fold into the compound forms (`xor`/`sub`/`neg`/`sub [a]`, `add [a], eax`); with an unsigned `b` they stay `mov`/`sub`/`mov` and `mov`/`add`/`mov`. `StaticPoolAlloc`'s size parameter is unsigned, so its signed comparison needs a `(MechS32)` cast.
- **A global's signedness picks the `|=` form.** `g |= 0x80000000` becomes the `mov`/`or`/`mov` round-trip when `g` is signed, the direct `or dword ptr [g]` when unsigned.
- **An unreachable `break` after `return` still emits its `jmp`** (`SimWindowProc`'s last case is literally `return 0; break;`).
- **A trailing `break` in a `switch`'s last label emits its own `jmp`**, ahead of the jump over the dispatch; a last label without `break` has only the latter (`VideoDriver::FUN_10006a99`/`FUN_10006da9` end in a `default:` with no `break`).
- **Every named local has its own stack home, so the slots give the local count.** A value stored to `[ebp-N]` and reloaded is a named local; a call result tested straight from `eax` (`test eax, eax`) is an expression used in place. `if (Find(x) >= 0)` and `index = Find(x); if (index >= 0)` differ exactly this way (`TextGlyphList::FUN_1003e19b`). Likewise `item = items[i]; if (item) f(item);` (`ClearCollection`), and a pointer re-read into the same local after a block (`items = p->m_items;` in `ExpandCollection`). Count the distinct slots in the `sub esp, N` frame before writing the body.
- **CRT data needs a library annotation too.** A CRT global renders as `<OFFSETn>` on the original side, a diff on every use, until `library_msvc.h` names it: `stderr` is `_iob + 0x40`, fixed by `// GLOBAL: MW2SHELL 0x10074a60` / `// _iob`. When the only diff is an `<OFFSETn>` against a named CRT symbol, the source is right; annotate the data.
- **Chained assignment reloads through memory:** `a = b = x;` stores `b`, reloads it, then stores `a` (`VideoDriver::VideoDriver`, the stack views in `VideoDriver::FUN_10006ed4`). A float assignment used as an operand doesn't: `dx /= length = sqrt(...)` is `fst [length]; fdivr [dx]; fstp [dx]` (`BuildRayFloat`). The same goes for a float compound assignment in a comparison: `(s += fabs(c)) > 0.0001` compares before it stores, `fadd [s]; fcom; fstp [s]` (`FUN_1004fe0f`), and `a = (vz = z3 - y1) * uy - …` stores `vz` with `fst` (`FUN_1004ff16`).
- **Constant folding follows the grouping.** `(n - 1) * 3` folds to `lea eax, [eax + eax*2 - 3]`; `n * 3 - 3` stays `lea` then `sub eax, 3` (`RotatePaletteCycle`). Array addressing folds the same way: `&p->m_sections[i - 1]` scales `i - 1` (`lea eax, [eax + eax*4 - 5]; shl eax, 3`), while `p->m_sections + i - 1` scales `i` and subtracts the element size afterwards (`FUN_10008938`).
- **A returned conditional expression goes straight to `eax`:** `return c ? i : -1;` compiles to `je; mov eax, [i]; jmp; mov eax, -1; jmp epilogue`, with no stack temporary (`FindInputDevice`); `if (c) { return i; } else { return -1; }` puts a `jmp` to the epilogue after each arm instead.
- **A conditional expression evaluates into a stack temporary:** `(c ? f() : -1) == 0` compiles to `cmp; je; call; mov [tmp], eax; jmp; mov [tmp], -1; cmp [tmp], 0`, with no `jmp` after the false arm. That tells it apart from an `/Ob1`-expanded inline function, which keeps a `jmp` per `return` (`VideoDriver`'s `ACQUIRE_FRAMEBUFFER()` macro).
- **Both front ends reorder commutative operands**, so source order can't be read back from the disassembly.
- **`/Oi` expands exactly:** `strlen`, `strcpy`, `strcat`, `strcmp`, `memcmp` (`repe cmpsb`; `VideoDriver::FUN_10006c50`), `memcpy` (any size, constants included; a variable size becomes `rep movsb`), `memset` (any size), `abs` (`cdq; xor eax, edx; sub eax, edx`; `FUN_1005d44e`), and the math functions to their `__CI*` entry points: `sqrt`→`__CIsqrt`, `pow`→`__CIpow`, `sin`→`__CIsin`, `atan`→`__CIatan`, `atan2`→`__CIatan2`, `asin`→`__CIasin` (`clock.c`'s tables). `memmove`, `strncpy`, `stricmp` etc. stay calls. The `__CI*` entry points are tiny thunks; they score 0% until their targets are annotated too.
- **Language per TU matters:** the C and C++ front ends differ in integer promotion around `char`, the enum type, and default `__cdecl` name handling (plus the `void`-return `jmp` oracle above).
- **SEH (C)** matches from `__try`/`__finally`: the `push -1; push scopetable; push __except_handler3; fs:[0]` prologue, trylevel updates at `[ebp-4]`, the `push ret; jmp funclet` call shape, and `AbnormalTermination()` → `__abnormal_termination`. **EH (C++)** matches from `/GX`: an EH state per `new T(...)`, unwind funclets placed in reverse state order before the epilogue, `mov eax, FuncInfo; jmp __CxxFrameHandler`.
- **Inline expansion at `/Od`:** plain `/Od` behaves as `/Ob0` — even `__inline` functions stay calls. `/Ob1` expands `inline`-marked functions (and the scalar deleting destructor in `delete g_x`; MW2SHELL needs it). The shell's C units are built with `/Ob1` as well: `mouse.c`'s `FUN_10046bd9` expands an `__inline` bounds test (a pointer parameter to a local expands to direct `[ebp-N]` access). Constant arguments are substituted into the expansion, only the others get `[ebp-N]` temporaries, and a conditional on a constant keeps its dead arm behind a `jmp` (`DISPDIB.H`'s `DisplayDibWindowCreate`/`DisplayDibWindowMessage`, reproduced in `3rdparty/dispdib/dispdib.h`). `g = f(...)` of an expanded function returning a value is a language signal: C stores each `return` straight into `g`, C++ goes through an extra temporary (`DispDibBegin` only matched as C). The auto-inlining that distinguishes `/Ob2` only runs with the optimizer, which neither DLL uses.
- **Stack-slot permutation:** 4.1's slot assignment reacts to tiny source changes, and a function can match except for a consistent permutation of `[ebp-N]` slots. reccmp scores every permuted access as a diff. Declaration order doesn't steer it (`SimMain`, `ShellMain`, `ShellWindowProc`, `CreateCollection`); probe compiles show the slots follow the locals' _names_, but no usable rule has come out of investigating it. **It is a closed question: don't reorder declarations, rename locals, or write probe compiles to fix a slot permutation.** Once the only remaining diff is permuted `[ebp-N]` slots, name the permuted locals in a comment above the annotation and move on.
- **Without `/Ob1`, an `__inline` function is emitted out of line after the unit's functions**, in its own 16-aligned section with `0xCC` padding before it (MW2's copies of the shell's `DdrawUnlock`/`DdrawRestore`, `IsInsideWindow` and the DisplayDib header's helpers). `tools/check_units.py` reports each as a split, so they're listed in its expected findings; define them at the end of the file so the annotations stay in address order, and annotate helpers from a `3rdparty/` header by name in the unit's own header (by-name markers are only allowed in headers).
- **An `/Ob1`-expanded inline function keeps its `return` as a `jmp`.** A class-body getter (`GetBackBuffer()`) expands to the member load plus a `jmp` past it, so it doesn't compile like direct member access (`ShellWindowProc`'s `memset` of `VideoDriver::m_backBuffer`).
- **Globals the original initializes live in raw `.data`; give them explicit initializers.** An address below the end of the section's raw data (e.g. MW2 `0x100bce00`) was defined with an initializer, even `= 0`. Datacmp reports an uninitialized (BSS) definition there as a diff. Past the raw end, leave the definition uninitialized. A nonzero byte partway through a "buffer" means the buffer is shorter (`g_paletteColors` is `0x300` bytes, not `0x400`).
- **`/G5` picks the short shift encoding.** 4.1 compiles signed `x / 2` to `cdq; sub eax, edx; sar eax, 1` and encodes the shift as `c1 f8 01`, or as `d1 f8` under `/G5`. Every site in MW2SHELL (and in MW2's game code) uses `d1 f8`, so both DLLs build with `/G5` (`TextGlyph::TextGlyph`, where the flag changed no other shell function; MW2's `StartPaletteFade`, whose `p_duration >>= 1` is `d1 7d 0c`).
- **`lea reg, [global]` means an offset into a larger object.** 4.1 loads a global's address with `mov reg, OFFSET g` when the access starts at the symbol, and with `lea reg, [g+N]` for a non-zero offset (a member or element past the first). `extern` makes no difference. So a `lea` of an apparently standalone address marks a member of a bigger global: the `mw2prm.cfg` record `SimHandoffState` at `0x10090288` was found this way from `lea edi, [0x10090298]` (`FUN_10002d30`).
- **`/Ob1` drops an inline's repeated test.** An `__inline` helper that tests `g_x` expands without the test when called inside `if (g_x) { … }`, but keeps its dead `else` arm behind a `jmp` (`DdrawPaletteFade`'s `DdrawUnlock()`).
- **A conditional between two address constants is branch-free**, even at `/Od`: `c ? "a" : "b"` compiles to `xor edx, edx; cmp [c], 0; setne dl; dec edx; sub ecx, eax; and edx, ecx; add edx, eax` (`FUN_1003c966`, `CpcScreenTick`).
- **`&=` with an unsigned constant round-trips.** On a signed local, `id &= 0x7fffffff` compiles to a direct `and dword ptr [ebp-N]`; `id &= ~0x80000000` gives `mov`/`and`/`mov` (`FUN_10009da9`).
- **A named local, not a conditional, stores both arms to a word slot.** `v = c ? -2 : f(); n.x = v;` with `MechS16 v`; a bare conditional assigned to a member stores straight into the member (`PrjBuildMechVariantTemplate`).
- **A `default: break;` adds a `jmp`** after the last case's `break`, while the dispatch still jumps straight to the switch's end (`FUN_100010ed`). In a jump table, the entries for the missing values point at that `jmp`; without a `default` they point at the switch's end (`FUN_1006b1fb`, where case 0xb ends in `return;` and `default: break;` follows).
- **A dispatch jumps past a case that only breaks.** In a compare-tree switch, the test for a case whose body is just `break` jumps straight to the switch's end, though the body's `jmp` is still emitted (`FUN_1005b22f`, `PlayNewCdAudio`).
- **An array member of a global decays with `mov` + `add`.** `(MechU16*) g_bitmapInfo.m_colors` and `g_gdiLogPalette.m_entries` compile to `mov eax, OFFSET g; add eax, N`, not the `lea` of a scalar member (`GdiBegin`, `GdiRealizePalette`). The address of a constant element of a global array compiles the same way: `&g_cockpitSpeech[11]` is `mov eax, OFFSET g_cockpitSpeech; add eax, 0x84` (`FUN_10059f6e`), while element 0 is a plain `push OFFSET`. So does the address of a struct member past the first: `&g.m_z` is `mov`/`add` too. A `push OFFSET g+8` immediate therefore means `g+8` is a global of its own: the collision normals at `0x100a5520` are nine scalar globals, not three vectors (`FUN_10035423`).
- **A narrowing cast loads narrow.** `(MechS16) p->m_int` assigned to an `int` compiles to `movsx eax, word ptr [...]`: `FUN_1004bc2e` reads the eyepoint's `0x4c`/`0x50` as words, `FUN_100024f0` as dwords.
- **An early `return` reads as `jne`/`jmp`.** `if (!p) { return; }` compiles to `cmp; jne past; jmp epilogue`, while `if (p) { … }` is a single `je` over the body. The shape code tests its selected model this way throughout (`FUN_1003a7f9`, `FUN_1006e9e6`); `if (!a || b) { return; }` jumps both tests to one shared `jmp` (`FUN_1003aba5`).
- **A postfix decrement in a loop test goes through a temporary.** `while (n--)` and `for (n = c; n--; p++)` copy `n` to an extra stack slot, decrement, then test the copy (`FUN_1006eb80`, `FUN_1001e429`), so the frame has one slot more than the named locals.
- **`memcpy` of a short constant loads the destination first.** `memcpy(msg->m_tag, "DA", 2)` is `mov eax, [msg]; mov cx, word ptr ["DA"]; mov [eax], cx`; the cast store `*(MechU16*) msg->m_tag = *(MechU16*) "DA"` loads the literal first (`network.c`'s message tags). A local initialized from a literal, `MechChar msg[] = "GO";`, copies it as a word and a byte through `eax`/`ecx` (`FUN_1000fca8`).
- **A struct assignment copies with `rep movsd`.** `g = p_xform;` for a 0x24-byte struct parameter is `lea esi, [ebp+8]; mov edi, OFFSET g; mov ecx, 9; rep movsd` (`ApplyBlockXform`).
- **An 8-byte struct moves through two registers.** `*p_out = result;` for a `Point` local is `mov eax, [x]; mov ecx, [y]; mov edx, [p_out]; mov [edx], eax; mov [edx+4], ecx` (`FUN_10057bf3`), and a `Point` passed by value pushes `mov eax, [x]; mov ecx, [y]; push ecx; push eax` (`FUN_10056fcf`'s scale). Two adjacent locals loaded as a pair like this are one struct.
- **`memcpy` of an 8-byte struct computes both addresses first.** `memcpy(&dst[n], &src[i], 8)` is `src` into `eax`, `dst` into `ecx`, then `mov edx, [eax]; mov [ecx], edx; mov eax, [eax+4]; mov [ecx+4], eax`; the struct assignment `dst[n] = src[i]` loads both dwords before computing `dst` (`FUN_10033a06`).
- **A local array with a brace initializer is stored element by element.** `MechChar shifted[16] = {'<', '_', ...};` compiles to sixteen `mov byte ptr [ebp-N], imm` (`FUN_1005bf7c`); a string initializer copies from a literal instead (`FUN_1000fca8`).
- **4.1 drops a retest of a condition the enclosing `if` already proved,** even at `/Od`: `if (g) { if (g) { } }` and `g && g` emit one test. A loop head is a join point that keeps it: `if (g) { do { if (g) { } } while (0); }` gives `cmp; je +0xd; cmp; je 0`, the shape of a compiled-out debug macro (`overlay.c`'s `DRAW_DEBUG_TEXT`). An empty `if (g) { }` on its own emits `cmp; je 0`, and `if (0) { … }` a `jmp` over the dead body (`FUN_10058958`).
- **C bitfields read back through `and`/`shl`/`or`.** Assigning a call result to a 1-bit field stores the result in a stack temporary, then `and eax, 1; shl eax, n; mov ecx, [f]; and ecx, ~mask; or eax, ecx`; setting the bit to 1 is `mov`/`or`/`mov`, clearing it a direct `and dword ptr [f], ~mask` (`GaugeQuadrant` in `FUN_10057ac4`).
- **A search loop's test sits in the `for` condition.** `for (i = 0; i < n && a[i].m_obj; i++) { }` compiles to `jge exit; cmp; je exit; jmp inc`; the same test as `if (!a[i].m_obj) { break; }` in the body adds a `jne`/`jmp` pair (`debris.c`'s slot searches).
- **`va_end` emits a store:** `mov dword ptr [args], 0` after the call (`FUN_1003a3df`, `FUN_1003a432`).
- **A right shift compared with a constant becomes a masked compare.** `(abs(t) >> 16) > 45` compiles to `and eax, 0xffff0000; mov ecx, 0x2d0000; and ecx, 0xffff0000; cmp eax, ecx; jle`, with the constant's mask left unfolded (`FUN_10053811`).
- **reccmp can't name a table in an indexed call.** `call dword ptr [eax*4 + table]` keeps raw addresses on both sides (its call operand regex only matches a bare `[address]`), so a call through a function-pointer table always scores as a diff; say so above the annotation (`ai.c`'s `g_aiStateFns`).
- **A byte mask compares the byte.** `(x & 0xff) == c` on a memory operand compiles to `cmp byte ptr [x], c` (`MapFaceId`), and `(w & 0xff00)` to `cmp byte ptr [w+1], 0`; a cast of the shifted value, `(MechU8) (w >> 8)`, loads the byte into a register instead: `xor edx, edx; mov dl, byte ptr [w+1]; test edx, edx` (`StartSample`). Neither `(MechU8) x == c` nor `(char) x == c` gives the `cmp byte ptr`. On a `MechS16` parameter, `(k & 0xff00) != 0x700` loads the word and compares the high byte in a register: `movsx eax, word ptr [k]; cmp ah, 7` (`FUN_1005b7c0`).
- **Signed `% 2^n` masks through the sign.** `c = c % 0x100000` on a signed `int` is `cdq; xor eax, edx; sub eax, edx; and eax, 0xfffff; xor eax, edx; sub eax, edx` (`LoadShapeRecord`'s checksum).
- **A call site shows the parameter's width.** A `MechU16` value passed to a `MechU32`/`int` parameter is widened before the push (`xor edx, edx; mov dx, word ptr [...]; push edx`); passed to a `MechU8` parameter it is pushed as loaded (`mov ax, word ptr [...]; push eax`). The callee's own code can't tell `MechU8` from a wider parameter it stores as a byte, so the call site decides (`FUN_1003ab34`).
- **reccmp compares only as many bytes as the recompiled function has.** When a stack-slot permutation gives the recompiled function shorter `[ebp-N]` encodings (disp8 instead of disp32), the original's tail is cut off and shows as missing lines; that is part of the permutation, not a separate diff (`LoadShapeRecord`).
- **reccmp pairs identical strings in address order.** String literals with the same text are matched first-to-first, so a literal of an undecompiled function that comes earlier (the original's `"MASC"` at `0x100a1458`) shifts every pair after it and datacmp reports the pointers to them. Define that earlier string as an annotated global in its unit until its function is decompiled (`unk10004f40.c`'s `g_unk0x100a1458`).
- **An empty `else` emits a `jmp` to the next statement.** `if (c) { x += a / c; } else { }` gives the original's `jmp` after the assignment; without the `else` there is none (`FUN_100147d0`).
- **A switch's empty last label has no `jmp` of its own.** In a jump table, an entry that points at the jump over the dispatch (after the previous case's `break`) is an empty label written last, `case 3:;`; a missing value points at the switch's end instead (`RunMenuChoice`, `RunMenuSlider`).
- **A returned conditional's arms follow the comparison as written.** `return f() > t ? t : 0x7fffffff;` and `return f() <= t ? 0x7fffffff : t;` compile identically; `return t >= f() ? 0x7fffffff : t;` gives the other arm order (`FUN_1003a096`).
- **A double local stays on the FPU stack between statements.** `d = x; d -= m; d *= 2.0; d /= r; d -= 65536.0;` compiles to one chain that stores `d` with `fst` and never reloads it, and an unrelated statement after it can sit between the last `fst` and the `__ftol` that converts `st0` (`UpdateAxisFromKeys`, `UpdateInputs`).
- **A shifted index into a dword array folds its scale.** `&s.m_buttons[n >> 5]` on a `MechU32` array compiles to `and eax, ~0x1f; sar eax, 3`; the byte offset written out, `(n & ~0x1f) >> 3`, folds the mask to `and eax, ~0x18` instead (`ParseInputConditions`).
- **Bitfield reads compile like mask tests; only the writes differ.** `!p->m_skillFlag0` is `test byte ptr [x], 1` and `p->m_skillFlag2` is `shr eax, 2; test al, 1`, exactly like `!(x & 1)` and `(x >> 2) & 1`. Setting a 1-bit field is `mov`/`or`/`mov`, where `x |= 1` on a plain `MechU32` is a direct `or dword ptr` (`FUN_100139e9`).
- **An index offset folds into the element only for an array at the struct's start.** `e->m_list[2 + r]` with the list at offset 0 is `lea eax, [eax*2 + 4]` added to the element address; the same element read as a member array at offset 4, `e->m_next[r]`, keeps the 4 as a displacement (`FUN_10013d81`).
- **`x += -(a * b)` keeps its `neg`.** On a `MechU16` it compiles to `imul; neg; add`, where `x -= a * b` is a `sub` (`FUN_1005e534`).
- **Switch jump tables** are embedded in the function body inside `.text` (e.g. the 5-entry table at `0x10006824` in `FUN_10006760`); at `/Od` a sparse switch becomes a compare tree on a stack temporary.

Float-literal and folding behavior are **not** documented here — they must be re-derived for VC++ 4.1 at `/Od` from matches before any rule is written down.

### Leads carried over from MSVC 6.0 (unverified on VC++ 4.1 — re-derive before trusting)

The upstream LEGO Racers decomp documented extensive `cl` 12.00 `/O2` codegen lore. Most of it concerns the optimizer — register-allocation levers, strength-reduced loops and walking pointers, CSE, cold-branch inversion, cross-jumping, tail calls, ICF/COMDAT fold pools — and none of that applies at `/Od` (no optimizer, no `/Gy`). Don't re-import those. What follows are the pieces that plausibly transfer because they are front-end or language semantics rather than optimizer behavior. Treat each as a hypothesis: promote it into the proven list above (or delete it) once a match settles it. And remember that old MSVC is known to move around more than stack slots — declaration position, statement position across adjacent blocks, and even the count of symbols declared ahead of a function can all flip codegen; when stuck near a match on comparison operand order, try those once (stack-slot order is excluded: see "Stack-slot permutation" above).

- **Return type inference.** `mov eax, <literal>` in the epilogue ⇒ the function returns that literal (often a success sentinel); declaring it `void` will mismatch. No `mov eax` at all and callers ignore `eax` ⇒ `void` (combine with the language oracle above).
- **Permissive `for` scoping.** Pre-standard MSVC lets two `for (MechS32 i = ...)` loops in the _same_ scope redeclare `i`; the original source may rely on it. Sibling branch arms may each declare their own counter — shared stack home ⇒ one function-top declaration, distinct homes ⇒ per-arm `for`-init declarations.
- **Vtable/SDD reading (MW2SHELL).** The vtable set lands after all base ctors; a set that appears _after_ some member init means that init belongs to an inlined base ctor, and a double `mov [ecx], &vtable` identifies the inheritance chain. Other virtuals declared before `~Derived()` in the class body push the SDD slot later — declaration order matters. `delete obj` and calling the SDD slot with argument `1` compile to identical bytes. The exact out-of-line SDD body shape differs from MSVC 6.0's — derive 4.1's from a real match before pattern-matching on it.
- **Typed sub-object members auto-emit ctor/dtor calls.** `SubClass m_member;` emits the sub-ctor in the outer ctor (declaration order) and the sub-dtor in the outer dtor (reverse order). With the sub-ctor/dtor STUBbed at the correct address, the outer can match without implementing the sub-object — the primary tool for verifying class layout.
- **EH state counts.** We verified one EH state per `new T(...)` in `ShellMain`. The MSVC 6.0 analog — a ctor with N dtor-bearing members writes N+1 EH states — is a lead for shell ctors under `/GX`; so is whether 4.1 elides the EH frame entirely when sub-ctors are provably no-throw within the same TU.
- **Call-site signals that are compiler-agnostic.** Static member functions emit bytes identical to free `__cdecl` functions. Byte-identical call setup with a different `call rel32` target ⇒ annotate the import thunk in `library_msvc.h`, don't distort the source. `__purecall` in a derived vtable slot ⇒ the derived class re-pure-virtualized a concrete base virtual. Disassembler symbol databases can mislabel local functions as STL/CRT; if the name isn't in the import table, follow the `e8 <rel32>`. Old MSVC rejects `__thiscall` on function-pointer typedefs — declare a named virtual at the slot instead.
- **Layout diagnostics.** A member access at `[this+N]` at or past the believed `SIZE` means the struct is larger (usually absorbing an adjacent member). A uniform Δ-byte shift of every `[reg+disp]` against the original means the struct base is off by Δ. One contiguous zero-store spanning two members can come from a single `memset` sized `sizeof(first) + sizeof(second)` (`/Oi` inlines `memset` at any size — verified). `ZeroMemory(&s, sizeof(s))` can beat `{0}` for Win32/DirectX structs.
- **Inline helpers (MW2SHELL `/Ob1` only).** Whether class-body-defined (implicitly `inline`) methods actually expand under 4.1's `/Ob1` is itself unverified — check before relying on any inline-helper lever. If they do expand, the MSVC 6.0 lessons apply: expanded control flow is not CSE'd (a condition evaluating the same check twice, branchy both times, is an inline helper called in each operand — a ternary compiles branch-free and folds to one evaluation), and promoting `ptr->m_foo` to an inline `ptr->GetFoo()` (or the reverse) changes the expansion even though the body is identical.
- **Float constants.** The MSVC 6.0 literal-pool vs. named-`const`-global rules do not carry over. Re-derive for 4.1 at `/Od` with the shape oracle: which operand is loaded first (`fld [x]; fadd [c]` vs. `fld [c]; fadd [x]`), and whether stores/pushes are immediates or go through a register. MW2's float probes match, so plain float arithmetic is settled; the datum-classification question stays open until a match forces it.

## VC++ 2.2 (NETMECHW, MECH2)

`NETMECHW.DLL` and `MECH2.EXE` were built with VC++ 2.2 (`cl` 9.10, LINK 2.55). Everything above about 4.1 at `/Od` holds for them, except where noted here.

- **Codegen at `/Od` is 4.1's**, apart from stack-slot assignment, which follows a different hash: the same local names get different slots than under 4.1. The rule is unchanged: a slot permutation is a closed question. Comparison operand order still follows declaration order (`MonoPrintLine`'s `i < length` needs `length` declared before `i`), and the C/C++ `jmp` oracle holds (NETMECHW's C++ units end `void` functions with a `jmp`, its C units don't). Commutative operand order can differ from 4.1's for the same source: `LoadCachedResource`'s hash adds the type bytes in source order (`p_type[2]` first) in both originals, and 4.1 reproduces it for the shell, while 2.2 compiles the NETMECHW copy to load them from `p_type[0]` up. Treat it as entropy, as under 4.1.
- **Data: an array's string literals come first.** 2.2 emits all the strings of a `char*` array's initializer, then the array. NETMECHW's copy of the shell's resource tag tables puts each string right before its pointer instead (`0x10023958`), which no array initializer reproduces, so they stay unannotated in `NETMECHW/src/mw2prj.c` (nothing in the DLL reads them).
- **`/QIfdiv` is on by default** in 2.2 and off in 4.1, so the 2.2 targets can't be built with 4.1. NETMECHW's three x87 divisions carry the guard (`pushfd; cmp [__adjust_fdiv], 0; jne; fdiv; jmp; push; push; call __adj_fdiv_m64; popfd`); don't try to write it in the source.
- **No `/G5` in NETMECHW:** all its signed `/ 2` shifts are the long form (`c1 f8 01`), where MECH2's are the short one. **No `/GX`:** 2.2's `/GX` emits no EH frame or state counter, and NETMECHW has no `__CxxFrameHandler`; its C++ is `__thiscall` methods, `new`/`delete` of plain classes (`SessionList`'s scalar deleting destructor at `0x10006020`), methods on global objects, and their static initialization. 2.2 constructs a unit's global objects in one function at the start of the object (`$$1000`, `0x10003660`), and destroys them in reverse order through `$$2000`, which `$$3000` registers with `atexit`; `$$4000`, the `.CRT$XCU` entry, calls `$$1000` and then `$$3000`. `$$2000` to `$$4000` sit at the end of the object, and the four are `// SYNTHETIC:` by name in `unk10003660.h`. Methods defined in a class body are emitted out of line, after the unit's functions in their own 16-aligned sections, in the first object that calls them (`SessionList::Lock`): they are listed as splits in `tools/check_units_expected.txt`.
- **A `char` bitfield reads in a byte register.** `CheckDlgButton(h, id, flags.m_option1)` on a `MechU8 : 1` field compiles to `mov al, [flags]; shr al, 1; and al, 1; xor ecx, ecx; mov cl, al`; the same mask on a plain `MechU8`, `(flags >> 1) & 1`, works in `eax` (`FUN_100089d0`, whose options byte other code writes whole: a union of the byte and the bits).
- **A signed target keeps `*p += n` as a load/add/store.** With `MechS32* p`, `*p += sizeof(T)` compiles to `mov eax, [p]; mov eax, [eax]; add eax, n; mov ecx, [p]; mov [ecx], eax`; a `DWORD*`, or a plain `int` constant added to the signed target, gives a direct `add dword ptr [eax], n` (`FUN_100086b1`, whose header size is `offsetof(MechFile, m_sections)` for this reason).
- **A switch on a signed value compares signed.** A compare tree whose first test is `jg` switches on an `int`: the hot-key dialogs that do write `switch ((MechS32) p_wParam)` (`FUN_1000268a`, `FUN_100098a2`); the others switch on the `WPARAM` and compare with `ja`.
- **`LoadImage`'s 0x4000 flag** (`LR_COPYFROMRESOURCE`) postdates 2.2's `winuser.h`; `unk10003660.h` defines it for the lobby's `LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE` calls.
- **A test of a player slot is a short-circuit `||` with a constant-false operand.** `if (g_players[i].m_flags == 0 || FALSE)` compiles to `cmp; je THEN; jmp ELSE`, with the then-arm first; `if (!g_players[i].m_flags)` gives a single `jne` (`IS_PLAYER_SLOT_USED` in `unk1000aa90.h`; `FUN_1000f0f0`, `FUN_10005b5d`).
- **An unreferenced local still takes a stack slot**, so it counts toward the frame size (the `unused2` of `FUN_100089d0` and `FUN_10011120`).
- **The `/QIfdiv` guard reads `_adjust_fdiv`**, a CRT global annotated in `library_msvc.h`; without the annotation, every guarded division scores a diff.
- **Inline assembler:** the same limits as 4.1. Neither binary has hand-written assembly.
- **Headers:** the build uses 2.2's own CRT and SDK headers, which differ from 4.1's. `MAKEINTRESOURCE` casts through `WORD`, which the original's `LoadIcon` call doesn't (`CreateGameWindow` casts the ID to `LPCSTR` directly). `_P_OVERLAY` expands to the CRT variable `_p_overlay` rather than a constant, so it is a `// GLOBAL:` in `library_msvc.h`.
- **RC 2.2** compiles `netmechw.rc` byte-identical to the original's resources, with two quirks: its `RADIOBUTTON`/`AUTORADIOBUTTON` shortcuts carry no `WS_TABSTOP` (the other button shortcuts do), and a string's carriage returns must be written as `\r` (a literal CR is line-ending whitespace to it). Check a recovered resource by rebuilding and comparing its bytes with the original's.
- **NETMECHW's link** is a plain `/DLL` link (`NETMECHW.def` exports `_DllMain@12 @1` and `Launcher @2`): objects start 16-aligned after `0xCC` padding, as with 4.1, so object boundaries, the unit split (42 objects, `tools/check_units.py` with the nested build's map) and `--print-rec-addr` work as for the 4.1 DLLs. Its CRT objects aren't 16-aligned (2.2's `LIBCMT` packs them), so the game/CRT boundary is the DPLAY import thunks at `0x10013b70`, not padding. Import libraries in `.idata` order: `DPLAY, KERNEL32, USER32, GDI32, COMCTL32, ADVAPI32`; the DPLAY imports are by ordinal, which the existing `3rdparty/dplay/dplay.def` provides.
- **NETMECHW shares code with the shell and the simulator**: 43 of its game functions are byte-identical (relocations aside) to MW2SHELL/MW2 ones (`debugout.c`, `resourcecache.c`, `prjfile.c`, `resourcefile.c`, `mw2prj.c`, the CD check), and the repository's shell source compiled with 2.2 matches 32 of the 35 it shares, the rest being slot permutations. Per "Shared code" under "Decompilation Principles", start them as per-target copies with NETMECHW annotations; a `common/` source has to match both compilers' slot hashes, so some files may stay split. The copies are in: 2.2's slot hash permutes 18 of their 58 functions, and the NETMECHW originals differ from the shell's in three places (`DebugPrintInternal`'s log mode and `default:`, `FillMemoryFast`'s fill word), noted in the copies. With that many permutations under one compiler's hash or the other, these units stay per-target copies.
- **MECH2's incremental link.** The original is an incremental EXE without `/DEBUG`: calls go through a table of `jmp` thunks at the start of `.text`, and each object is padded with `0xCC` to `align16(size * 1.25)`, the linker's reserve for relinking. The comparison build (LINK 3.10, `/INCREMENTAL:yes /DEBUG`) pads differently, so recompiled addresses drift from the original's past the first object: always pass `--print-rec-addr`. `tools/check_units.py` doesn't understand this layout yet, and the nested build writes no map file for it, so it doesn't run on MECH2.
- **MECH2's object order** is `winmain.c`, `cdcheck.c`, `debug.c`.

## Ghidra ↔ Source

- **Bootstrap is minimal.** Don't import Ghidra names wholesale. Start with `// LIBRARY:` annotations for identified CRT functions plus the export entry points (`ShellMain`, `ShellWindowProc`, `SimMain`, `SimWindowProc`) as the first units to decompile. No generated `// STUB:` skeletons — stubs are written by hand as callees of the function being worked on.
- **Bring Ghidra names over, but don't treat them as gospel.** When a function is decompiled, its Ghidra names come with it, translated into NCC form. They are provisional: rename freely when the code contradicts a name; prefer names corroborated in the binary (e.g. a debug string naming the callee — `DebugLog("LoadWorld()\n")` just before the call); treat an uncorroborated Ghidra name as a working label, not a finding. Types and struct layouts still need the usual corroboration.
- **Source is authoritative for anything annotated.** Names and types reach Ghidra from the source through `reccmp-ghidra-import`, which a human runs (below). Agents don't write to Ghidra: use the Ghidra MCP server, if available, to read the binary only (no renames, types or comments), and don't run `reccmp-ghidra-import` (or its `reccmp-import-ghidra` build targets).
- **Bulk import (humans only).** `reccmp-ghidra-import` is wired up in `cmake/reccmp.cmake` (the `RECCMP_<ID>_GHIDRA_LOCAL_PROJECT_PATH` / `_FILE` cache variables). It overwrites existing names, so back up the Ghidra project before the first import.

## Prioritize Constructors, Destructors, and SDDs (MW2SHELL)

When starting a new class, match ctor + dtor + scalar-deleting-destructor first. Ctors reveal member init order, base chain (via vtable sets), and types; dtors reveal cleanup order and virtual calls (confirming vtable layout); SDDs verify class size and dtor linkage. Matching these three gives high confidence that size/inheritance/vtable/members are correct before tackling methods. (Verified for VC++ 4.1: `/Ob1` inlines the SDD into `delete g_x` — `t = g_x; if (t) { t->~T(); operator delete(t); }` with a redundant `jmp`.)

## Decompilation Principles

- **Every type must be corroborated by matched code.** A type is proven only when a `// FUNCTION:` using it reaches 100%, or its only remaining diff is a documented compiler artifact. Until then, `undefined`/`undefined4`.
- **No raw pointer arithmetic as a substitute for types.** Casts + subtractions mean the types are wrong; find the real class so the cast is legitimate C++.
- **Split mixed compilation units until unexplained address gaps are gone.** A large address jump usually means a function is assigned to the wrong file, or multiple classes are mashed into one source unit. A 16-aligned start with `0xCC` padding before it is an object boundary; use the boundaries to draft the file split (about 150 TUs in MW2, 51 in MW2SHELL). Never split one class's methods across `.cpp` files just to tidy address order. CI checks the split (`tools/check_units.py`): each object's functions, initialized data and `.bss` must each form one run in the original, in the build's link order; a padding boundary inside a file, a file starting mid-object, a `const` mismatch between `.rdata` and `.data`, and a C tentative definition the original allocates among the objects' own `.bss` (it was `static`) all fail. Known findings are listed with a reason in `tools/check_units_expected.txt`; the check also fails on an entry that no longer occurs, so remove it with the fix.
- **One root type per header.** A header should define at most one top-level class or struct; forward declarations do not count.
- **Nest one-owner helper types.** Loader params, callback shims and small related records belong inside the primary class that owns them.
- **Ground polymorphic classes.** When a concrete polymorphic class is identified, add its `VTABLE`, ctor, dtor, and scalar-deleting-destructor annotations instead of leaving it as an unannotated interface shape.
- **Read the original binary directly** (Python/struct/capstone) for vtable entries, call targets, and function addresses when the disassembler mislabels them. Disassembler dumps stop at the first `ret` — a function can continue past it.
- **Every annotation has a real address** — no placeholders.
- **`// FUNCTION:` scores.** `// FUNCTION:` marks a complete decompilation that reccmp compares; aim for 100%. A diff usually means the code is wrong: investigate the root cause (layout, types, missing base). A sub-100% `// FUNCTION:` is acceptable only when the remaining diff is a known compiler artifact: a stack-slot permutation (accept it immediately, no attempts), or a comparison/index operand order that one declaration-order attempt didn't flip. Say so in a comment above the annotation, naming the locals or the comparison. Incomplete code stays `// STUB:`. The reccmp diff against main in each PR is the regression gate.
- **Re-verify inherited matches.** Claimed matches are not verified matches; re-measure with reccmp; fix sub-100% claims, or demote them unless a comment documents the remaining compiler artifact.
- **Validate vtables explicitly** (verbose compare on the vtable address) in addition to function compares.
- **Shared code:** start each shared file as per-target copies (MW2 + MW2SHELL annotations); move a file into `common/` only once one source matches both addresses. Third-party code both DLLs link goes to `3rdparty/` with its own source (`3rdparty/vfx`), and its C and headers to `common/` (`common/src`, `common/include`).

## Naming from Matched Code

A member name is proven when a `// FUNCTION:` match forces a specific semantic interpretation. Rename the `m_unk0xNN` placeholder once a match corroborates usage; types still follow the `undefined` rule.

Rename a function from `VTable0xNN` / `FUN_XXXXXXXX` to a semantic name when evidence is strong:

- **Clear pair with a named counterpart.** `VTable0x20` calls `Shutdown()`; adjacent `VTable0x1c` calls the init helper with hInstance/hWnd → `VTable0x1c` is `InitializeInput`.
- **Symmetric with a named method.** A method that undoes every action `Initialize()` took, and is also the `~Class()` body → `Destroy`.
- **Body leaves no interpretation.** A one-line tail call, or a loop opening every line of a newline list and publishing them as globals.

Do not rename on weak evidence. If multiple plausible names exist, keep `FUN_XXXXXXXX`. A misleading name is worse than a neutral placeholder. Renaming a virtual does not affect codegen — the vtable is slot-indexed — so it's safe if all call sites and overrides are updated together.

**Lifecycle vocabulary** (match existing names so the codebase shares one vocabulary):

- `Initialize()` — explicit init separate from the ctor.
- `Run()` — the main loop / per-instance driver.
- `Shutdown()` — release live resources but leave the object reusable.
- `Destroy()` — full teardown: invokes `Shutdown` plus everything else, leaving the post-construction state.
- `Reset()` — return to pristine zero-state.

When a class has a small subsystem-teardown method AND a larger full-destroy wrapper, the small one is `Shutdown` and the big one is `Destroy`. Don't invent `Cleanup` / `Teardown` / `StopServices`.

## Project Structure

```
MECH2/        # MECH2.EXE reccmp target (include/, src/, library_msvc.h, mech2.rc; built by vc22/)
MW2SHELL/     # MW2SHELL.DLL (include/, src/, MW2SHELL.def, library_msvc.h, mw2shell.rc: only the resources it needs)
MW2/          # MW2.DLL      (include/, src/, MW2.def, library_msvc.h; loads no resources)
NETMECHW/     # NETMECHW.DLL reccmp target (include/, src/, NETMECHW.def, library_msvc.h; built by vc22/)
common/       # what both DLLs share: Miles Design VFX's headers (include/) and portable C and stubs (src/)
3rdparty/     # import-library .def files (DDRAW, DPLAY, WAIL32, SMACKW32; the .libs are generated at build time),
              # DirectX 2 SDK headers, our own Miles Sound System (mss/) and Smacker declarations (no SDK: only
              # what the game uses), and Miles Design VFX's source (vfx/: VFXA.ASM, VFX3D.ASM, VFXREND.ASM, …)
              # from its July 2000 open-source release, as both DLLs link it (changes marked)
util/         # decomp.h, compat.h, types.h
vc22/         # the VC++ 2.2 sub-project (CMakeLists.txt) and the 2.2 targets' definitions (targets.cmake): NETMECHW, MECH2
cmake/        # reccmp CMake integration, shared CMake helpers
tools/        # ncc, lint scripts, requirements
tests/        # asmequiv: the portable C against the hand-written assembly (ctest; also a standalone project)
reccmp/       # reccmp data sources (CSVs, added as needed)
docker/       # VC++ 4.1 + VC++ 2.2 + CMake under Wine build image
assets/       # progress report icons
```
