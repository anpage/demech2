"""Compile a target's units the way its build does, from any copy of the sources, and read COFF.

Shared by `compare_objects.py` and `witness.py`. The compile commands come from the build tree's
NMake files (`flags.make`, `build.make`): the nested builds (`vc22/`, `vc40/`) write no
`compile_commands.json`. Each command is rerun with the source tree swapped for another one, so the
same flags compile a copy of the repository (another revision, a scratch copy with renamed locals).

Off Windows, the compilers run under Wine: pass its path (`--wine`), with `WINEPREFIX` set.
"""

import os
import re
import shutil
import struct
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
WINDOWS = sys.platform == "win32"

# The CMake target directory of each reccmp target, relative to the build tree, and the nested
# build's environment file, if any.
TARGETS = {
    "MW2SHELL": ("CMakeFiles/mw2shell.dir", None),
    "MW2": ("CMakeFiles/mw2.dir", None),
    "MW2MATROX": ("vc40/CMakeFiles/mw2matrox.dir", "vc40-env.cmake"),
    "MW23DFX": ("vc22/CMakeFiles/mw23dfx.dir", "vc22-env.cmake"),
    "NETMECHW": ("vc22/CMakeFiles/netmechw.dir", "vc22-env.cmake"),
    "MECH2": ("vc22/CMakeFiles/mech2.dir", "vc22-env.cmake"),
}


def to_host(path):
    """A path of the build tree's makefiles (a Wine Z: path off Windows) as a host path."""
    if not WINDOWS and re.match(r"^[A-Za-z]:[\\/]", path):
        return path[2:].replace("\\", "/")
    return path


def to_tool(path):
    """A host path as the compiler sees it."""
    return str(path) if WINDOWS else "Z:" + str(path).replace("/", "\\")


def source_root(build):
    """The source tree the build tree was configured from, as the makefiles spell it."""
    for line in (build / "CMakeCache.txt").read_text(errors="replace").split("\n"):
        if line.startswith("CMAKE_HOME_DIRECTORY:INTERNAL="):
            return line.split("=", 1)[1].strip()
    raise SystemExit("%s: no CMAKE_HOME_DIRECTORY" % build)


class Unit:
    """One compile of a target: a C, C++ or MASM source and its options."""

    def __init__(self, target, compiler, language, arguments, source, root):
        self.target = target
        self.compiler = compiler
        self.language = language  # "C", "CXX" or "ASM_MASM"
        self.arguments = arguments  # the options, the source tree as `root` spells it
        self.root = root
        self.source = source  # relative to the source tree, with forward slashes
        self.name = source.split("/")[-1]

    def __repr__(self):
        return "%s:%s" % (self.target, self.source)

    def options(self, tree):
        """The options with the source tree replaced by `tree` (a host path)."""
        text = self.arguments
        new = to_tool(tree)
        for old in (self.root, self.root.replace("/", "\\")):
            text = text.replace(old, new if "\\" in old or WINDOWS else new.replace("\\", "/"))
        return text

    def path(self, tree):
        return Path(tree) / self.source

    def files(self, tree):
        """The unit's source and the .c fragments it includes, by name (host paths in `tree`)."""
        found, queue = {}, [self.path(tree)]
        while queue:
            path = queue.pop()
            if path.name in found or not path.exists():
                continue
            found[path.name] = path
            for name in re.findall(r'^\s*#\s*include\s+"([^"]+\.c(?:pp)?)"', path.read_text(encoding="utf-8"), re.M):
                queue.append(path.parent / name)
        return found


def read_env(build, env_file):
    """INCLUDE, LIB and the compiler's bin directory for a nested build, from its environment file."""
    if env_file is None:
        return None
    text = (build / env_file).read_text()
    env = {}
    for name in ("PATH", "INCLUDE", "LIB"):
        m = re.search(r"set\(ENV\{%s\} \[==\[(.*?)\]==\]\)" % name, text)
        if m:
            env[name] = m.group(1)
    return env


class Target:
    def __init__(self, name, build):
        self.name = name
        self.build = Path(build)
        directory, env_file = TARGETS[name]
        self.directory = self.build / directory
        if not (self.directory / "build.make").exists():
            raise SystemExit("%s: no %s (configure and build it first)" % (name, self.directory / "build.make"))
        nested = self.build / directory.split("/")[0] if env_file else self.build
        self.root = source_root(self.build)
        self.env = read_env(self.build, env_file)
        self.units = self.read_units(nested)

    def read_units(self, nested):
        flags = {}
        for line in (self.directory / "flags.make").read_text().splitlines():
            m = re.match(r"(\w+) = (.*)", line)
            if m:
                flags[m.group(1)] = m.group(2).strip()
        units, compiler = [], None
        cl_rule = re.compile(r" /nologo ((?:/T\w )?)\$\((C|CXX)_DEFINES\) \$\(\2_INCLUDES\) \$\(\2_FLAGS\) (.*?)/Fo\S+ /Fd\S+ -c (\S+)$")
        ml_rule = re.compile(r"\t(\S+) \$\(ASM_MASM_DEFINES\) \$\(ASM_MASM_INCLUDES\) \$\(ASM_MASM_FLAGS\) (.*?)-c -Fo \S+ (\S+)$")
        roots = (self.root, self.root.replace("/", "\\"))

        def relative(source):
            for root in roots:
                if source.startswith(root):
                    return source[len(root) :].lstrip("\\/").replace("\\", "/")
            return None

        for line in (self.directory / "build.make").read_text().splitlines():
            m = re.match(r"\t(\S+) @<<$", line)
            if m:
                compiler = m.group(1)
                continue
            m = cl_rule.match(line)
            if m:
                kind, language, extra, source = m.groups()
                arguments = kind + " ".join(flags[language + s] for s in ("_DEFINES", "_INCLUDES", "_FLAGS")) + " " + extra
                rel = relative(source)
                if rel:
                    units.append(Unit(self.name, compiler, language, arguments.strip(), rel, self.root))
                continue
            m = ml_rule.match(line)
            if m:
                ml, extra, source = m.groups()
                arguments = " ".join(flags["ASM_MASM" + s] for s in ("_DEFINES", "_INCLUDES", "_FLAGS")) + " " + extra
                rel = relative(source)
                if rel:
                    units.append(Unit(self.name, ml, "ASM_MASM", arguments.strip(), rel, self.root))
        return units

    def environment(self, unit):
        env = dict(os.environ)
        if self.env:
            env["INCLUDE"], env["LIB"] = self.env["INCLUDE"], self.env["LIB"]
            path = self.env["PATH"]
        else:
            root = re.sub(r"\\bin\\[^\\]+$", "", unit.compiler if unit.language != "ASM_MASM" else "C:\\msvc\\bin\\CL.EXE", flags=re.I)
            env["INCLUDE"], env["LIB"] = root + "\\include", root + "\\lib"
            path = root + "\\bin"
        if WINDOWS:
            env["PATH"] = path + ";" + env.get("PATH", "")
        else:
            env.update(WINEDEBUG="-all", WINEPATH=path)
        return env


class Request:
    """A compile: `unit` from `tree`, with `files` (a name of the unit's files -> new text) written
    to a scratch copy, and `prefix` put ahead of the unit's own source."""

    def __init__(self, unit, tree, tag, files=None, prefix="", listing=False):
        self.unit, self.tree, self.tag = unit, Path(tree), tag
        self.files, self.prefix, self.listing = files or {}, prefix, listing
        self.obj = self.asm = self.error = None


class Compiler:
    """Runs batches of compiles in parallel, each batch in a scratch directory of its own."""

    def __init__(self, scratch, wine="wine", jobs=4):
        self.scratch = Path(scratch)
        self.wine = wine
        self.jobs = jobs
        self.count = 0
        self.runs = 0

    def run(self, target, requests):
        """Compile the requests (all of one target). Sets each request's `obj`, `asm` or `error`."""
        requests = list(requests)
        batches = [requests[i :: self.jobs] for i in range(self.jobs)]
        processes = []
        run = self.scratch / ("run%d" % self.runs)
        self.runs += 1
        shutil.rmtree(run, ignore_errors=True)
        for n, batch in enumerate(b for b in batches if b):
            work = run / ("batch%d" % n)
            shutil.rmtree(work, ignore_errors=True)
            work.mkdir(parents=True)
            lines = ["@echo off"]
            for k, request in enumerate(batch):
                unit = request.unit
                source = unit.path(request.tree)
                include = ""
                if request.files or request.prefix:
                    # A scratch copy of the unit's files; the unit's own directory follows on the
                    # include path, for its headers and the fragments the copy doesn't replace.
                    where = work / ("src%d" % k)
                    where.mkdir()
                    texts = {name: path.read_text(encoding="utf-8") for name, path in unit.files(request.tree).items()}
                    texts.update(request.files)
                    texts[unit.name] = request.prefix + texts[unit.name]
                    for name, text in texts.items():
                        (where / name).write_text(text, encoding="utf-8", newline="\n")
                    source = where / unit.name
                    include = "/I%s " % to_tool(unit.path(request.tree).parent)
                obj = work / ("%d.obj" % k)
                if unit.language == "ASM_MASM":
                    lines.append("%s %s %s-c -Fo %s %s" % (unit.compiler, unit.options(request.tree), include, to_tool(obj), to_tool(source)))
                else:
                    listing = ""
                    if request.listing:
                        request.asm = work / ("%d.asm" % k)
                        listing = "/Fa%s " % to_tool(request.asm)
                    response = work / ("%d.rsp" % k)
                    # One option a line: VC++ 2.2's cl cuts a response file's line at 1024 characters.
                    text = "/nologo %s %s/Fo%s /Fd%s\\ %s-c %s" % (unit.options(request.tree), include, to_tool(obj), to_tool(work), listing, to_tool(source))
                    response.write_text("\n".join(re.findall(r'(?:"[^"]*"|\S)+', text)) + "\n")
                    lines.append("%s @%s" % (unit.compiler, to_tool(response)))
                request.obj = obj
                self.count += 1
            batch_file = work / "run.bat"
            batch_file.write_text("\r\n".join(lines) + "\r\n")
            env = target.environment(batch[0].unit)
            command = ["cmd", "/c", str(batch_file)] if WINDOWS else [self.wine, "cmd", "/c", to_tool(batch_file)]
            log = open(work / "log.txt", "w")
            processes.append((subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT, env=env, cwd=work), log))
        for process, log in processes:
            process.wait()
            log.close()
        for request in requests:
            if not request.obj.exists():
                text = (request.obj.parent / "log.txt").read_text(errors="replace")
                errors = [line.strip() for line in text.split("\n") if re.search(r"\b(fatal )?error [A-Z]+\d+|error A\d+|Can't recognize", line)]
                request.error = "compile failed: " + (errors[0] if errors else "no object")
                request.obj = None
        return requests

    def discard(self, requests):
        """Delete the scratch files of a run's requests."""
        for directory in {r.obj.parent.parent for r in requests if r.obj} | {r.asm.parent.parent for r in requests if r.asm}:
            shutil.rmtree(directory, ignore_errors=True)


# ---------------------------------------------------------------------------------------------
# COFF objects

IMAGE_SCN_CNT_CODE = 0x20
IMAGE_SCN_LNK_COMDAT = 0x1000
IMAGE_SYM_CLASS_EXTERNAL = 2
IMAGE_SYM_CLASS_STATIC = 3
IMAGE_SYM_CLASS_FUNCTION = 101  # .bf, .lf, .ef: line-number records, not code
IMAGE_SYM_CLASS_FILE = 103
IMAGE_REL_I386_DIR32 = 6
IMAGE_REL_I386_REL32 = 20

# Names the compiler numbers with a counter of its own: string literals ($SG123), labels ($L123),
# temporaries ($T123), statics of a function (_x$S123), a nested scope's duplicate (_i$99).
NUMBERED = re.compile(r"\$(SG|L|T|S|M)?(\d+)")


class Symbol:
    def __init__(self, index, name, value, section, kind, storage, aux):
        self.index, self.name, self.value, self.section = index, name, value, section
        self.kind, self.storage, self.aux = kind, storage, aux
        self.canonical = name


class Section:
    def __init__(self, number, name, flags, data, size, relocations):
        self.number, self.name, self.flags, self.data, self.size = number, name, flags, data, size
        self.relocations = relocations  # [(offset, Symbol, type)]

    @property
    def debug(self):
        return self.name.startswith(".debug")

    @property
    def code(self):
        return bool(self.flags & IMAGE_SCN_CNT_CODE)


class Function:
    def __init__(self, symbol, section, start, code, relocations):
        self.symbol, self.section, self.start = symbol, section, start
        self.code = code  # the function's bytes
        self.relocations = relocations  # offset in the function -> (Symbol, type)

    @property
    def name(self):
        return self.symbol.name


class Object:
    def __init__(self, path):
        data = Path(path).read_bytes()
        _, section_count, _, symbols_at, symbol_count, optional, _ = struct.unpack_from("<HHIIIHH", data, 0)
        strings = symbols_at + symbol_count * 18
        self.symbols, by_index, i = [], {}, 0
        while i < symbol_count:
            raw, value, section, kind, storage, aux_count = struct.unpack_from("<8sIhHBB", data, symbols_at + i * 18)
            if raw[:4] == b"\0\0\0\0":
                at = strings + struct.unpack_from("<I", raw, 4)[0]
                name = data[at : data.index(b"\0", at)].decode("latin1")
            else:
                name = raw.rstrip(b"\0").decode("latin1")
            aux = data[symbols_at + (i + 1) * 18 : symbols_at + (i + 1 + aux_count) * 18]
            by_index[i] = Symbol(i, name, value, section, kind, storage, aux)
            self.symbols.append(by_index[i])
            i += 1 + aux_count
        self.sections = []
        for n in range(section_count):
            at = 20 + optional + n * 40
            name, _, _, size, pointer, relocations_at, _, relocation_count, _, flags = struct.unpack_from("<8sIIIIIIHHI", data, at)
            name = name.rstrip(b"\0").decode("latin1")
            if name.startswith("/"):
                offset = int(name[1:])
                name = data[strings + offset : data.index(b"\0", strings + offset)].decode("latin1")
            body = data[pointer : pointer + size] if pointer else None
            relocations = []
            for r in range(relocation_count):
                offset, symbol, kind = struct.unpack_from("<IIH", data, relocations_at + r * 10)
                relocations.append((offset, by_index[symbol], kind))
            self.sections.append(Section(n + 1, name, flags, body, size, relocations))
        self.canonicalize()
        self.functions = {}
        for section in self.sections:
            if not section.code or section.data is None:
                continue
            starts = sorted((s.value, s.index, s) for s in self.symbols if s.section == section.number and s.kind == 0x20)
            for k, (start, _, symbol) in enumerate(starts):
                end = starts[k + 1][0] if k + 1 < len(starts) else len(section.data)
                code = section.data[start:end]
                if k + 1 == len(starts):
                    code = code.rstrip(b"\xcc")
                inside = {o - start: (s, t) for o, s, t in section.relocations if start <= o < start + len(code)}
                self.functions[symbol.name] = Function(symbol, section, start, code, inside)

    def canonicalize(self):
        """Renumber the compiler's counters by order of appearance, so that a symbol declared
        ahead of the unit, which advances them all, doesn't make every name differ."""
        seen = {}

        def renumber(m):
            key = m.group(1) or ""
            numbers = seen.setdefault(key, {})
            return "$%s#%d" % (key, numbers.setdefault(m.group(2), len(numbers)))

        for symbol in self.symbols:
            symbol.canonical = NUMBERED.sub(renumber, symbol.name)

    def section_of(self, symbol):
        if 0 < symbol.section <= len(self.sections):
            return self.sections[symbol.section - 1]
        return None

    def data_at(self, symbol, addend, limit=256):
        """The initialized bytes a data symbol (plus addend) starts, up to the next symbol, or None
        when there are none or they hold a relocation."""
        section = self.section_of(symbol)
        if section is None or section.data is None:
            return None
        start = symbol.value + addend
        later = [s.value for s in self.symbols if s.section == symbol.section and s.value > start]
        end = min(min(later) if later else len(section.data), len(section.data), start + limit)
        if any(start <= o < end for o, _, _ in section.relocations):
            return None
        return section.data[start:end]


# ---------------------------------------------------------------------------------------------
# PE images


class Image:
    def __init__(self, path):
        self.data = Path(path).read_bytes()
        pe = struct.unpack_from("<I", self.data, 0x3C)[0]
        count, optional = struct.unpack_from("<H", self.data, pe + 6)[0], struct.unpack_from("<H", self.data, pe + 20)[0]
        self.base = struct.unpack_from("<I", self.data, pe + 24 + 28)[0]
        self.sections = []
        for n in range(count):
            at = pe + 24 + optional + n * 40
            virtual_size, address, size, pointer = struct.unpack_from("<IIII", self.data, at + 8)
            self.sections.append((self.base + address, max(virtual_size, size), size, pointer))

    def at(self, address, count):
        for start, virtual_size, size, pointer in self.sections:
            if start <= address < start + virtual_size:
                offset = address - start
                raw = self.data[pointer + offset : pointer + min(offset + count, size)] if offset < size else b""
                return raw + b"\0" * (min(count, virtual_size - offset) - len(raw))
        return b""
