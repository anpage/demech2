# Verifying

## The proof states

What counts is proof, not similarity. `tools/certify.py` gives every annotated function one of three states:

- **byte-identical**: the source compiles to the original's code, every relocated address pointing where the original's does;
- **proven**: it does once its stack locals are renamed and/or a number of symbols are declared ahead of its unit, as a committed record (`tools/witnesses/`) says. Neither is recoverable from the original: its locals' names are lost, and its headers declared another number of things, both of which only move stack slots, operand order and (in optimized code) registers. The source keeps its readable names; the record is the proof;
- **unproven**: neither. `tools/audit.py` says which of these still differ only by compiler entropy (classified, by rewrites that reproduce the original exactly), and `tools/known_diffs.txt` lists the rest, with the reason where it is known. reccmp's percentage still shows how close each is.

The current counts for every binary, and each function that isn't byte-identical with its record or its reason, are on the [proof states page](https://anpage.github.io/demech2/STATES.HTML), which CI rewrites on every push to `main`. Each target's list (`<TARGET>STATES.md` and `.json`) is also published with the progress reports in the [continuous release](https://github.com/anpage/demech2/releases/tag/continuous). A match proves the code, the types, the data layout and the constants; it proves nothing about names, comments and descriptions, which are readings of the code. The original's hand-written assembly is reproduced as MASM source (its own objects) or inline assembly, and every function with inline assembly is listed in `tools/asm_allowlist.txt` with what in the original shows it was hand-written: everything else matches as C or C++.

## Comparing with the originals

To verify your build against the original binaries, install the [reccmp](https://github.com/isledecomp/reccmp) tooling:

```
pip install -r tools/requirements.txt
```

This installs a [fork](https://github.com/anpage/reccmp) pinned to a commit: `MECH2.EXE` was linked incrementally but has no debug directory, and reccmp only looks for the incremental thunks in images that have one.

Create `reccmp-user.yml` in the project root pointing to the original binaries you have ([TARGETS.md](TARGETS.md#reccmp-useryml) has one to start from):

```yaml
targets:
  MW2SHELL:
    path: path/to/MW2SHELL.DLL
  MW2:
    path: path/to/MW2.DLL
```

Then, from the build folder, with a target's name and its game-code total from [TARGETS.md](TARGETS.md#game-code):

```
reccmp-reccmp --target MW2SHELL --nolib --total 770
reccmp-reccmp --target MW2 --nolib --total 1537 -S MW2PROGRESS.SVG
```

To certify the build (from the repository root, after building; it compiles each unit again with the build's own commands, applies the witness records, and compares every function with the original):

```
python tools/certify.py --target MW2 [--build <build-folder>] [-v]
```

`-v` lists the unproven functions with their first difference. Under Wine, pass `--wine <path-to-wine>`, with `WINEPREFIX` set.

`python tools/verify.py [--originals <folder>]` does this for every target whose original it finds (in the folder, or through `reccmp-user.yml`) and prints one table of the counts; it is what the Docker image runs.

## Continuous integration

Pull requests are built and verified with Visual C++ 4.1, 4.0 and 2.2 on GitHub Actions. The public originals (the 1.1 patch's `MW2SHELL.DLL` and `MW2.DLL`, and Ghost Bear's Legacy's `MW2_GBL.DLL`) are downloaded by the Build workflow itself. The retail ones (`NETMECHW.DLL`, `MECH2.EXE` and the 3D-card editions' simulators) aren't freely downloadable, so a second workflow (`verify-retail.yml`) runs after Build with access to a private repository holding the originals: it compares the Build run's binaries against them with reccmp and appends the results to the Build workflow's PR comment. Both certify every function: a pull request fails when a function `main` has byte-identical or proven is neither any more, unless its description lists it (`Gate exception: <TARGET> <function> <reason>`). Pull requests from forks get the same report.

CI also checks that the documents generated from `targets.toml` are current and that `reccmp-project.yml` and the retail workflow's list agree with it (`python tools/targets.py --check`).
