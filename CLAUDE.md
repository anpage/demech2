# demech2 — MechWarrior 2 (Windows 95) Decompilation

Decompilation of MechWarrior 2 (Windows 95 release) using Microsoft Visual C++ 4.1 (`cl` 10.10.6038, `_MSC_VER` 1010). Modeled after the [LEGO Island](https://github.com/isledecomp/isle) and [LEGO Racers](https://github.com/isledecomp/racers) decompilations.

Two reccmp targets, both from the freely downloadable 1.1 patch:

- **MW2SHELL** — `MW2SHELL.DLL`: shell (FMV, menus, settings, metagame). A mixture of C and C++.
- **MW2** — `MW2.DLL`: simulator (in-mission gameplay). Probably all C.

One source-only target:

- **MECH2** — `MECH2.EXE`, the launcher that loads the DLLs. Its source lives in `MECH2/` **without annotations and outside reccmp**: it was built with an older toolchain (VC++ 2.x), so it cannot byte-match under VC++ 4.1 and is never scored. It is kept compiling so the recompiled DLLs can be launched.

Future target: `NETMECHW.DLL` (NetMech shell counterpart) — the layout, CMake and CI are designed so it can be added as a third reccmp target without restructuring.

Not targets: the third-party DLLs `WAIL32.DLL` (Miles Sound System) and `SMACKW32.DLL` (Smacker), which we only link against.

## Local Instructions

Machine-specific instructions live in `CLAUDE.local.md` at the repository root (gitignored): how to invoke the toolchain and reccmp on this machine (Wine build, prefix, paths), where the originals live, which tools are available, and the contributor's workflow preferences. **Agents: if `CLAUDE.local.md` exists, read it before building or running reccmp; where it conflicts with the generic instructions here, it wins.** Claude Code loads it automatically; other agents reading `AGENTS.md` must open it themselves. Contributors: create it to fit your environment; it is never committed.

## Building

```
<path-to-msvc41>\BIN\VCVARS32.BAT
mkdir build && cd build
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build .
```

Portable VC++ 4.1: https://github.com/madebr/msvc410. CMake 3.26.6 (i386) is known to drive the VC++ 4.1 NMake generator.

Build configuration:

| Target   | Language                 | `cl` flags         | CRT                                        | Link                                                |
| -------- | ------------------------ | ------------------ | ------------------------------------------ | --------------------------------------------------- |
| MW2      | C                        | `/Od /Oi`          | `/MTd` (static debug, predefines `_DEBUG`) | `/DLL /DEBUG /INCREMENTAL:no`                       |
| MW2SHELL | C++ (C files: no `/GX`)  | `/Od /Oi /G5 /Ob1 /GX` | `/MT` (static)                             | `/DLL` (comparison build adds `/DEBUG` for the PDB) |

The CRT is selected through `MSVC_RUNTIME_LIBRARY` under `CMP0091 NEW` (`MultiThreadedDebug` for MW2, `MultiThreaded` for MW2SHELL). No CRT patching is needed: the originals match 4.1's `LIBCMTD.LIB` (MW2) and `LIBCMT.LIB` (MW2SHELL) as-is.

The shell's icon 103 (shown by four dialogs) is not committed: CMake builds `tools/extract_icon` and extracts it from the original `MW2SHELL.DLL` (`DEMECH2_MW2SHELL_ORIGINAL`, default: the repository root; SHA-256 checked). Without the original, `mw2shell.rc` builds without the icon. The launcher's window icons 103 and 108 (NetMech) work the same way, from the original `MECH2.EXE` (`DEMECH2_MECH2_ORIGINAL`) into `mech2.rc`.

Source order in each target must follow the **original link order** (object order determines function addresses).

## reccmp

```bash
pip install -r tools/requirements.txt

# Compare (run from build/ directory)
reccmp-reccmp --target MW2SHELL --print-rec-addr
reccmp-reccmp --target MW2 --verbose 0x10003580 --print-rec-addr

# Game-code progress, as CI reports it
reccmp-reccmp --target MW2SHELL --silent --nolib --total 770
reccmp-reccmp --target MW2 --silent --nolib --total 1537

# Compare global variable data values
reccmp-datacmp --target MW2SHELL --verbose --print-rec-addr
reccmp-datacmp --target MW2 --verbose --print-rec-addr

# Lint annotations (pass source dirs to avoid scanning gitignored files)
reccmp-decomplint --module MW2SHELL --warnfail <path-to-MW2SHELL>
reccmp-decomplint --module MW2 --warnfail <path-to-MW2>
```

`reccmp-user.yml` (gitignored) points to the original binaries for local comparison. Progress counts **game code only**: `--nolib` drops the `LIBRARY` entries from both the matched count and the denominator, and `--total` is the Ghidra original's game-code function count, everything below the shell's import thunks (`0x100492a2`) or MW2's CRT (`0x10080490`): **770** for MW2SHELL and **1537** for MW2. Without these flags, reccmp divides by the annotated functions only, which overstates progress. `--total` is only a floor — reccmp uses whichever is larger, the annotated function count or `--total`. The CRT is left out until the game code is done: its `LIBRARY` entries only match once the game code that calls them is linked. The workflow keeps the list in `.github/workflows/build.yml` (`targets` job).

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

A `// GLOBAL:` marks the address of the pointer variable itself. If the variable points at data (e.g. a `char*` string literal), the data address belongs in a `reccmp/` data-source CSV (added as needed, following the reccmp convention).

Run `reccmp-datacmp` after adding/modifying globals with non-zero initial values to verify they match.

When a shared function carries annotations for both DLLs, MW2SHELL's annotation comes first (the target order in `reccmp-project.yml`).

**CRT names differ between the debug and release CRT**: LIBCMTD (MW2) carries CodeView procedure symbols, so reccmp sees the C name (`_vsnprintf`, `_DllMainCRTStartup`); LIBCMT (MW2SHELL) has only publics, so it sees the decorated name (`__vsnprintf`, `__DllMainCRTStartup@12`). `library_msvc.h` therefore matches public CRT functions by their public symbol, which both libraries share: `// LIBRARY: MW2 0x10080490 SYMBOL` followed by `// _sprintf`. Static CRT functions have no public symbol: MW2 names them by their CodeView C name (when unique), and MW2SHELL's stay unmatched but still mark library addresses. At an alias address (`__chkstk`/`__alloca_probe`), use the symbol reccmp keeps.

**reccmp reads annotations from the source tree, not the build**: every annotated file must be in the build, or reccmp logs errors for the functions it can't find.

## Class Pattern (C++, MW2SHELL)

C++ exists in MW2SHELL and MECH2. MW2 is plain C — see the struct pattern below.

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
- **Annotation ordering:** when a function has both MW2SHELL and MW2 annotations, MW2SHELL comes first.
- **No redundant `this->`.** Write `m_member`, `Method()`, `BaseClass::VirtualMethod()` directly.
- **Win32 API: prefer un-suffixed names.** Use `CreateWindowEx`, `DEVMODE`, `MSG`, `WIN32_FIND_DATA`, etc. — NOT `CreateWindowExA`/`DEVMODEA`. The un-suffixed names are macros that resolve to the `A` form when `UNICODE` is undefined; the compiled binary still imports the `A` symbols. Disassemblers show the resolved `*A` symbol — translate back to the macro.
- **Pointer/bool constants:** use `NULL` for null pointer assignments/returns, `TRUE`/`FALSE` for boolean values, and plain `0` only for scalar values and status codes.
- **Blank lines inside functions:** write functions in readable paragraphs. Keep tight sequences of the same kind together, separate declarations/setup, the main operation, and result handling with single blank lines. If an `if`/loop block closes and the next statement is a `return`, put a blank line before the `return`.
- **Enums for magic numbers:** hoist fixed enumerations (flag bits, event tags, state codes) into a named `enum` at class or namespace scope. `c_camelCase` per NCC.
- **No leading `const` on return-by-value** (`const RetType Get() const` — meaningless, trips NCC).
- **Language per translation unit.** A unit is `.c` or `.cpp`, decided by which one matches — not by target. If a unit won't match as C, try C++ (and vice versa) before contorting the source.
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
10. **Sync Ghidra** through the Ghidra MCP server, if available: push the matched names and types (see "Ghidra ↔ Source").

**Language oracle.** At `/Od` the C++ front end ends a `void` function with a `jmp` to the epilogue; the C front end doesn't. Functions that return a value compile identically either way. Use this to decide `.c` vs `.cpp` when a unit is ambiguous.

## Hand-written Assembly (VC++ 4.1)

The original game contains hand-written assembly, reproduced as C/C++ source that the MSVC inline assembler emits byte-for-byte. Three forms:

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

**2. Whole assembly routines** become `__declspec(naked)` functions whose body is one `__asm { … }` block, annotated `// FUNCTION: <MODULE> 0x…` like any other function. Branch targets are labels named after the original address. Non-MSVC builds put the naked functions inside `#ifdef COMPAT_MODE … #else … #endif` with `STUB()` bodies on the `COMPAT_MODE` side. Reference pattern, preserved from the upstream LEGO Racers decomp (`GolDP/src/render/golrasterizers1.cpp`, with its original GOLDP annotation — in this project the annotation carries the real MW2/MW2SHELL address of the routine being matched):

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

**3. Data owned by assembly routines** (lookup tables, jump tables) gets `// GLOBAL:` annotations and is checked with datacmp.

**VC++ 4.1 inline-assembler limits (verified):** it accepts every 386/486 and x87 instruction, and rejects everything Pentium and later (`cpuid`, `rdtsc`, `cmpxchg8b`, `cmovcc`, MMX…) as well as data directives (`db`/`dw`/`dd`). Rejected opcodes must be emitted with `_emit`. Because there are no data directives, a routine with an embedded jump table of label addresses cannot be written as one naked function (`_emit` bytes can't carry relocations): either split the table out as a `// GLOBAL:` array of function addresses, or use a real assembler for that object. Decide when it happens.

DOS-era routines may use custom register calling conventions (inputs in `ebx`/`edx`). Callers in C then need the same inline-asm setup, or are themselves asm. Original MASM/TASM objects may have their own section alignment or padding between functions; if addresses drift around an asm block, that is the first suspect.

**How to spot assembly** — apply per function, when it is picked up. At `/Od`, VC++ 4.1 gives **every** C and C++ function the same frame:

```
push ebp; mov ebp, esp; [sub esp, N;] push ebx; push esi; push edi
…
pop edi; pop esi; pop ebx; leave; ret [N]
```

`ebx`/`esi`/`edi` are saved even when unused; an empty function is exactly this, `0xb` bytes. Checklist, strongest signal first:

1. **Outside the game-code range?** CRT functions (MW2 from `0x10080490`, MW2SHELL from `0x100492e0`) and the shell's import thunks (`0x100492a2`–`0x100492de`) are library code, not game assembly. Use `// LIBRARY:`.
2. **No `/Od` frame at all** (missing `push ebp; mov ebp, esp`, or missing/partial `push ebx; push esi; push edi`) → a whole assembly routine → `__declspec(naked)`. A frame that saves only the registers it clobbers suggests MASM `PROC … USES`. _Exceptions that are still C:_ SEH functions (`push -1; push <scopetable>; push __except_handler3; mov eax, fs:[0]` after `mov ebp, esp`, as in `SimMain`), their `__finally` funclets, and C++ EH functions (`ShellMain`: EH state counter and unwind funclets).
3. **`/Od` frame, but the body doesn't look like `/Od` output** → C function with an `__asm` body (the `FixedMul16` pattern). `/Od` spills every value to `[ebp-N]` and reloads it for each statement; it never keeps results in registers across statements. Instructions it doesn't generate are a giveaway: `shrd`/`shld`, `adc`/`sbb` after arithmetic, `bswap`, `xlat`, `lods`/`stos`/`movs`/`scas` (with or without `rep`) outside the CRT, `loop`/`jecxz`, `pushad`/`popad`, `in`/`out`, `cli`/`sti`, and x87 transcendental forms (`fsin`, `fpatan`, `fyl2x`…). Bare (non-`rep`) string instructions in particular: `/Oi` intrinsics always `rep`-prefix variable-size forms and inline constant-size forms as plain `mov`s — the shell's blit region (`FUN_10034622` etc.) is the first candidate area.
4. **Register-based parameters:** callers load `eax`/`ebx`/`edx`… right before a `call` without pushing arguments, or the callee reads registers it never set. Both caller and callee are then likely assembly.
5. **Layout hints:** inside a 16-byte-aligned unit, VC++ packs functions back to back with no padding. Padding _inside_ a unit (`0x90`, `0x00`, or alignment between functions) points to an assembler-built object. So do non-switch data blobs inside `.text` (VC++ puts switch jump tables inside the function body; those are C). MW2SHELL has two: `0x10017990`–`0x100286a6` and the fixed-point table at `0x10035af0`.
6. **Inline `__asm` doesn't compile?** Check the limits above: `_emit` for 586+ opcodes; label-address tables need the split decision.

Record each confirmed routine in the file's header comment, so the list grows as a by-product of decompilation. **An honest C near-miss beats an asm transcription** — don't convert a function to `__asm` just because the C doesn't match yet.

## Codegen Patterns (VC++ 4.1 `/Od`)

This section only grows as patterns are **proven by matches**:

- **Unoptimized code is literal.** Expect near-1:1 statement mapping; matching is about exact types, statement order and local declaration order, not register coaxing. Dead stores and `jmp`-to-next come from source constructs (e.g. a `break` at the end of a `switch` case, or an empty `else`) and are clues, not noise.
- **Empty/thin functions:** the universal `/Od` frame (see above), `0xb` bytes for an empty function.
- **Comparison operand order follows symbol order, not source order.** `a > b` and `b < a` compile identically; 4.1 chooses which side of `<`/`>`/`==` goes in `eax` from the symbol table. If a comparison is reversed vs. the original, reordering the operands' _declarations_ fixes it — even the count of extra symbols declared ahead of the function in the TU can flip it. Global _definitions_ count the same way: reordering the `// GLOBAL:` definitions in the TU fixed `KeyboardReadKeyCode` and `SimMain`'s objective loop. Array indexing shows the same effect: `p->m_items[i]` loads the base first in some functions and the index first in others (`CollectionGet`, `ClearCollection`), and declaration order didn't flip those — accept after one attempt.
- **`int -= int` after `&=` negates twice.** `n &= 0xfff; rem -= n;` with both `int` compiles to `xor eax, eax; sub eax, [n]; neg eax; sub [rem], eax`; an unsigned `n` gives the plain `mov`/`sub` (`TMPackDataBase::GetDBItemLZ`).
- **A global's signedness picks the `|=` form.** `g |= 0x80000000` becomes the `mov`/`or`/`mov` round-trip when `g` is signed, the direct `or dword ptr [g]` when unsigned.
- **An unreachable `break` after `return` still emits its `jmp`** (`SimWindowProc`'s last case is literally `return 0; break;`).
- **A trailing `break` in a `switch`'s last label emits its own `jmp`**, ahead of the jump over the dispatch; a last label without `break` has only the latter (`VideoDriver::FUN_10006a99`/`FUN_10006da9` end in a `default:` with no `break`).
- **Every named local has its own stack home, so the slots give the local count.** A value stored to `[ebp-N]` and reloaded is a named local; a call result tested straight from `eax` (`test eax, eax`) is an expression used in place. `if (Find(x) >= 0)` and `index = Find(x); if (index >= 0)` differ exactly this way (`CopperLedger0x04::FUN_1003e19b`). Likewise `item = items[i]; if (item) f(item);` (`ClearCollection`), and a pointer re-read into the same local after a block (`items = p->m_items;` in `ExpandCollection`). Count the distinct slots in the `sub esp, N` frame before writing the body.
- **CRT data needs a library annotation too.** A CRT global renders as `<OFFSETn>` on the original side, a diff on every use, until `library_msvc.h` names it: `stderr` is `_iob + 0x40`, fixed by `// GLOBAL: MW2SHELL 0x10074a60` / `// _iob`. When the only diff is an `<OFFSETn>` against a named CRT symbol, the source is right; annotate the data.
- **Chained assignment reloads through memory:** `a = b = x;` stores `b`, reloads it, then stores `a` (`VideoDriver::VideoDriver`, the stack views in `VideoDriver::FUN_10006ed4`).
- **A conditional expression evaluates into a stack temporary:** `(c ? f() : -1) == 0` compiles to `cmp; je; call; mov [tmp], eax; jmp; mov [tmp], -1; cmp [tmp], 0`, with no `jmp` after the false arm. That tells it apart from an `/Ob1`-expanded inline function, which keeps a `jmp` per `return` (`VideoDriver`'s `ACQUIRE_FRAMEBUFFER()` macro).
- **Both front ends reorder commutative operands**, so source order can't be read back from the disassembly.
- **`/Oi` expands exactly:** `strlen`, `strcpy`, `strcat`, `strcmp`, `memcmp` (`repe cmpsb`; `VideoDriver::FUN_10006c50`), `memcpy` (any size, constants included; a variable size becomes `rep movsb`), `memset` (any size), `sqrt`→`__CIsqrt`, `pow`→`__CIpow` — and nothing else. `memmove`, `strncpy`, `stricmp` etc. stay calls. The `__CI*` entry points are tiny thunks; they score 0% until their targets are annotated too.
- **Language per TU matters:** the C and C++ front ends differ in integer promotion around `char`, the enum type, and default `__cdecl` name handling (plus the `void`-return `jmp` oracle above).
- **SEH (C)** matches from `__try`/`__finally`: the `push -1; push scopetable; push __except_handler3; fs:[0]` prologue, trylevel updates at `[ebp-4]`, the `push ret; jmp funclet` call shape, and `AbnormalTermination()` → `__abnormal_termination`. **EH (C++)** matches from `/GX`: an EH state per `new T(...)`, unwind funclets placed in reverse state order before the epilogue, `mov eax, FuncInfo; jmp __CxxFrameHandler`.
- **Inline expansion at `/Od`:** plain `/Od` behaves as `/Ob0` — even `__inline` functions stay calls. `/Ob1` expands `inline`-marked functions (and the scalar deleting destructor in `delete g_x`; MW2SHELL needs it). The shell's C units are built with `/Ob1` as well: `mouse.c`'s `FUN_10046bd9` expands an `__inline` bounds test (a pointer parameter to a local expands to direct `[ebp-N]` access). The auto-inlining that distinguishes `/Ob2` only runs with the optimizer, which neither DLL uses.
- **Stack-slot permutation:** 4.1's slot assignment reacts to tiny source changes, and a function can match except for a consistent permutation of `[ebp-N]` slots. reccmp scores every permuted access as a diff. Declaration order doesn't steer it (`SimMain`, `ShellMain`, `ShellWindowProc`, `CreateCollection`); probe compiles show the slots follow the locals' _names_, but no usable rule has come out of investigating it. **It is a closed question: don't reorder declarations, rename locals, or write probe compiles to fix a slot permutation.** Once the only remaining diff is permuted `[ebp-N]` slots, name the permuted locals in a comment above the annotation and move on.
- **An `/Ob1`-expanded inline function keeps its `return` as a `jmp`.** A class-body getter (`GetBackBuffer()`) expands to the member load plus a `jmp` past it, so it doesn't compile like direct member access (`ShellWindowProc`'s `memset` of `VideoDriver::m_backBuffer`).
- **Globals the original initializes live in raw `.data`; give them explicit initializers.** An address below the end of the section's raw data (e.g. MW2 `0x100bce00`) was defined with an initializer, even `= 0`. Datacmp reports an uninitialized (BSS) definition there as a diff. Past the raw end, leave the definition uninitialized. A nonzero byte partway through a "buffer" means the buffer is shorter (`g_paletteColors` is `0x300` bytes, not `0x400`).
- **`/G5` picks the short shift encoding.** 4.1 compiles signed `x / 2` to `cdq; sub eax, edx; sar eax, 1` and encodes the shift as `c1 f8 01`, or as `d1 f8` under `/G5`. Every site in MW2SHELL (and in MW2's game code) uses `d1 f8`, so MW2SHELL builds with `/G5` (`EmberGlyph0x3e::EmberGlyph0x3e`; the flag changed no other shell function). MW2 still builds without it until a match needs it.
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

## Ghidra ↔ Source

- **Bootstrap is minimal.** Don't import Ghidra names wholesale. Start with `// LIBRARY:` annotations for identified CRT functions plus the export entry points (`ShellMain`, `ShellWindowProc`, `SimMain`, `SimWindowProc`) as the first units to decompile. No generated `// STUB:` skeletons — stubs are written by hand as callees of the function being worked on.
- **Bring Ghidra names over, but don't treat them as gospel.** When a function is decompiled, its Ghidra names come with it, translated into NCC form. They are provisional: rename freely when the code contradicts a name; prefer names corroborated in the binary (e.g. a debug string naming the callee — `DebugLog("LoadWorld()\n")` just before the call); treat an uncorroborated Ghidra name as a working label, not a finding. Types and struct layouts still need the usual corroboration.
- **Source is authoritative for anything annotated.** Push only what matched code backs. Agents: if the Ghidra MCP server is available, keep Ghidra in sync as you work: after a match, rename the functions, globals, parameters and locals, and apply the corroborated types, prototypes and struct layouts through the MCP tools. Don't run `reccmp-ghidra-import` (or its `reccmp-import-ghidra` build targets).
- **Bulk import (humans only).** `reccmp-ghidra-import` is wired up in `cmake/reccmp.cmake` (the `RECCMP_<ID>_GHIDRA_LOCAL_PROJECT_PATH` / `_FILE` cache variables). It overwrites existing names, so back up the Ghidra project before the first import.

## Prioritize Constructors, Destructors, and SDDs (MW2SHELL)

When starting a new class, match ctor + dtor + scalar-deleting-destructor first. Ctors reveal member init order, base chain (via vtable sets), and types; dtors reveal cleanup order and virtual calls (confirming vtable layout); SDDs verify class size and dtor linkage. Matching these three gives high confidence that size/inheritance/vtable/members are correct before tackling methods. (Verified for VC++ 4.1: `/Ob1` inlines the SDD into `delete g_x` — `t = g_x; if (t) { t->~T(); operator delete(t); }` with a redundant `jmp`.)

## Decompilation Principles

- **Every type must be corroborated by matched code.** A type is proven only when a `// FUNCTION:` using it reaches 100%, or its only remaining diff is a documented compiler artifact. Until then, `undefined`/`undefined4`.
- **No raw pointer arithmetic as a substitute for types.** Casts + subtractions mean the types are wrong; find the real class so the cast is legitimate C++.
- **Split mixed compilation units until unexplained address gaps are gone.** A large address jump usually means a function is assigned to the wrong file, or multiple classes are mashed into one source unit. A 16-aligned start with `0xCC` padding before it is an object boundary; use the boundaries to draft the file split (about 150 TUs in MW2, 51 in MW2SHELL). Never split one class's methods across `.cpp` files just to tidy address order.
- **One root type per header.** A header should define at most one top-level class or struct; forward declarations do not count.
- **Nest one-owner helper types.** Loader params, callback shims and small related records belong inside the primary class that owns them.
- **Ground polymorphic classes.** When a concrete polymorphic class is identified, add its `VTABLE`, ctor, dtor, and scalar-deleting-destructor annotations instead of leaving it as an unannotated interface shape.
- **Read the original binary directly** (Python/struct/capstone) for vtable entries, call targets, and function addresses when the disassembler mislabels them. Disassembler dumps stop at the first `ret` — a function can continue past it.
- **Every annotation has a real address** — no placeholders.
- **`// FUNCTION:` scores.** `// FUNCTION:` marks a complete decompilation that reccmp compares; aim for 100%. A diff usually means the code is wrong: investigate the root cause (layout, types, missing base). A sub-100% `// FUNCTION:` is acceptable only when the remaining diff is a known compiler artifact: a stack-slot permutation (accept it immediately, no attempts), or a comparison/index operand order that one declaration-order attempt didn't flip. Say so in a comment above the annotation, naming the locals or the comparison. Incomplete code stays `// STUB:`. The reccmp diff against main in each PR is the regression gate.
- **Re-verify inherited matches.** Claimed matches are not verified matches; re-measure with reccmp; fix sub-100% claims, or demote them unless a comment documents the remaining compiler artifact.
- **Validate vtables explicitly** (verbose compare on the vtable address) in addition to function compares.
- **Shared code:** start each shared file as per-target copies (MW2 + MW2SHELL annotations); move a file into `common/` only once one source matches both addresses.

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
MECH2/        # MECH2.EXE source only: no annotations, not a reccmp target
MW2SHELL/     # MW2SHELL.DLL (include/, src/, MW2SHELL.def, library_msvc.h, mw2shell.rc: only the resources it needs)
MW2/          # MW2.DLL      (include/, src/, MW2.def, library_msvc.h; loads no resources)
              # NETMECHW/ added later with the same shape
3rdparty/     # import-library .def files (DDRAW, DPLAY, WAIL32, SMACKW32; the .libs are generated at build time),
              # DirectX 2 SDK headers, and our own Miles/Smacker declarations (no SDK: only what the game uses)
util/         # decomp.h, compat.h, types.h
cmake/        # reccmp CMake integration
tools/        # ncc, lint scripts, requirements
reccmp/       # reccmp data sources (CSVs, added as needed)
docker/       # VC++ 4.1 + CMake under Wine build image
assets/       # progress report icons
```
