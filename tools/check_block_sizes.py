#!/usr/bin/env python3
"""Flag block operations that run past the object they are given.

The original game reads, writes and copies some runs of globals as one block
(`fread(&first, 0x3c, 1, file)`). The run only exists if every member of it is
declared, in order, in one object; otherwise the call writes into whatever the
compiler put next. Per-symbol checks (reccmp, datacmp) cannot see this, so this
tool checks it at the source level: for each call to a block function whose
pointer argument is a named object and whose size argument is a constant, the
size must fit in what is left of the object from the pointer onwards.

Members of one struct may be spanned (the struct's layout is fixed): the room
counted for `&g.m_member` runs to the end of `g`. Pointers into unknown objects
(`p->m_member`, a pointer parameter) are not checked.

Sources are parsed as 32-bit MSVC code against the VC++ 4.1 headers, so type
sizes match the real build. The headers are case-insensitive on Windows only:
the tool makes a lowercase copy of them (and of the DirectX 2 SDK headers), with oaidl.h's member named `bool`
renamed so that the C++ parse succeeds.

A call can be exempted with a `check_block_sizes: allow` comment on its line.
"""

import argparse
import ctypes
import os
import re
import shutil
import sys
import tempfile

import clang.cindex
from clang.cindex import CursorKind, Index, TypeKind

# Block functions: (pointer argument indices, size argument indices). The size
# is the product of the size arguments (fread and fwrite take size and count).
BLOCK_FUNCTIONS = {
    "memcpy": ((0, 1), (2,)),
    "memmove": ((0, 1), (2,)),
    "memset": ((0,), (2,)),
    "memcmp": ((0, 1), (2,)),
    "strncpy": ((0,), (2,)),
    "fread": ((0,), (1, 2)),
    "fwrite": ((0,), (1, 2)),
    "_read": ((1,), (2,)),
    "_write": ((1,), (2,)),
    "ReadFile": ((1,), (2,)),
    "WriteFile": ((1,), (2,)),
}

ALLOW_MARKER = "check_block_sizes: allow"

SOURCE_EXTENSIONS = (".c", ".cpp")

# Per-target compiler settings, as in CMakeLists.txt: the CRT choice predefines
# _MT, and MW2's static debug CRT (/MTd) predefines _DEBUG too.
TARGET_DEFINES = {
    "MW2": ["_MT", "_DEBUG"],
    "MW2SHELL": ["_MT"],
    "MECH2": ["_MT"],
}

COMMON_INCLUDES = [
    "util",
    "3rdparty/dispdib",
    "3rdparty/miles",
    "3rdparty/smacker",
]

COMMON_DEFINES = ["WIN32", "_WINDOWS"]

# Header directories with uppercase file names, folded into the lowercase copy.
UPPERCASE_INCLUDES = ["3rdparty/dx2/INC"]


def setup_evaluate(lib):
    """Register the constant-evaluation entry points of libclang."""
    lib.clang_Cursor_Evaluate.argtypes = [clang.cindex.Cursor]
    lib.clang_Cursor_Evaluate.restype = ctypes.c_void_p
    lib.clang_EvalResult_getKind.argtypes = [ctypes.c_void_p]
    lib.clang_EvalResult_getKind.restype = ctypes.c_int
    lib.clang_EvalResult_getAsLongLong.argtypes = [ctypes.c_void_p]
    lib.clang_EvalResult_getAsLongLong.restype = ctypes.c_longlong
    lib.clang_EvalResult_dispose.argtypes = [ctypes.c_void_p]
    lib.clang_EvalResult_dispose.restype = None


def evaluate_int(lib, cursor):
    """The value of an integer constant expression, or None."""
    result = lib.clang_Cursor_Evaluate(cursor)
    if not result:
        return None
    try:
        # CXEval_Int
        if lib.clang_EvalResult_getKind(result) != 1:
            return None
        return lib.clang_EvalResult_getAsLongLong(result)
    finally:
        lib.clang_EvalResult_dispose(result)


def prepare_headers(include_dirs, out_dir):
    """Lowercase copy of the VC++ 4.1 and DirectX 2 headers, with oaidl.h patched."""
    for include_dir in include_dirs:
        copy_lowercase(include_dir, out_dir)


def copy_lowercase(include_dir, out_dir):
    for name in os.listdir(include_dir):
        src = os.path.join(include_dir, name)
        if not os.path.isfile(src):
            continue
        dst = os.path.join(out_dir, name.lower())
        if name.lower() == "oaidl.h":
            with open(src, "rb") as f:
                text = f.read()
            text = re.sub(rb"\bVARIANT_BOOL(\s+)bool;", rb"VARIANT_BOOL\1bool_;", text)
            with open(dst, "wb") as f:
                f.write(text)
        else:
            shutil.copyfile(src, dst)


def strip_expr(cursor):
    """Skip implicit conversions, parentheses and casts."""
    while cursor.kind in (
        CursorKind.UNEXPOSED_EXPR,
        CursorKind.PAREN_EXPR,
        CursorKind.CSTYLE_CAST_EXPR,
        CursorKind.CXX_STATIC_CAST_EXPR,
        CursorKind.CXX_REINTERPRET_CAST_EXPR,
        CursorKind.CXX_CONST_CAST_EXPR,
    ):
        children = [c for c in cursor.get_children() if c.kind.is_expression()]
        if not children:
            break
        cursor = children[-1]
    return cursor


def first_token(cursor):
    for token in cursor.get_tokens():
        return token.spelling
    return None


def operator_token(cursor):
    """The operator of a binary expression: the first token after the left operand."""
    children = list(cursor.get_children())
    if len(children) != 2:
        return None
    left_end = children[0].extent.end.offset
    for token in cursor.get_tokens():
        if token.extent.start.offset >= left_end:
            return token.spelling
    return None


def object_size(cursor):
    """Size of a named object, from its definition if the declaration is incomplete."""
    size = cursor.type.get_size()
    if size > 0:
        return size
    definition = cursor.get_definition()
    if definition is not None:
        size = definition.type.get_size()
        if size > 0:
            return size
    return None


def object_room(lib, expr):
    """(name, bytes available from the pointer onwards) for a pointer expression
    into a named object, or None when the object cannot be determined."""
    expr = strip_expr(expr)

    if expr.kind == CursorKind.DECL_REF_EXPR:
        # An array decays to a pointer to its first element; a pointer variable
        # points somewhere unknown.
        decl = expr.referenced
        if decl is None or decl.kind not in (CursorKind.VAR_DECL,):
            return None
        if decl.type.get_canonical().kind not in (
            TypeKind.CONSTANTARRAY,
            TypeKind.INCOMPLETEARRAY,
        ):
            return None
        size = object_size(decl)
        return (decl.spelling, size) if size else None

    if expr.kind == CursorKind.STRING_LITERAL:
        size = expr.type.get_size()
        return (expr.spelling, size) if size > 0 else None

    decays = expr.type.get_canonical().kind in (TypeKind.CONSTANTARRAY, TypeKind.INCOMPLETEARRAY)
    if expr.kind in (CursorKind.MEMBER_REF_EXPR, CursorKind.ARRAY_SUBSCRIPT_EXPR) and decays:
        # An array member or element decays to a pointer to its first element.
        return lvalue_room(lib, expr)

    if expr.kind == CursorKind.UNARY_OPERATOR and first_token(expr) == "&":
        operand = [c for c in expr.get_children() if c.kind.is_expression()]
        if not operand:
            return None
        return lvalue_room(lib, strip_expr(operand[0]))

    if expr.kind == CursorKind.BINARY_OPERATOR and operator_token(expr) == "+":
        left, right = list(expr.get_children())
        base = object_room(lib, left)
        index = evaluate_int(lib, right)
        pointee = strip_expr(left).type.get_canonical()
        if pointee.kind in (TypeKind.CONSTANTARRAY, TypeKind.INCOMPLETEARRAY):
            element = pointee.get_array_element_type().get_size()
        else:
            element = pointee.get_pointee().get_size()
        if base is None or index is None or element <= 0:
            return None
        return (base[0], base[1] - index * element)

    return None


def lvalue_room(lib, expr):
    """(name, bytes from the start of an lvalue to the end of its object)."""
    if expr.kind == CursorKind.DECL_REF_EXPR:
        decl = expr.referenced
        if decl is None or decl.kind != CursorKind.VAR_DECL:
            return None
        size = object_size(decl)
        return (decl.spelling, size) if size else None

    if expr.kind == CursorKind.MEMBER_REF_EXPR:
        # Spanning later members of the same object is fine: count the room to
        # the end of the outermost named object.
        offset = 0
        node = expr
        while node.kind == CursorKind.MEMBER_REF_EXPR:
            field = node.referenced
            if field is None or first_token_is_arrow(node):
                return None
            field_offset = field.get_field_offsetof()
            if field_offset < 0:
                return None
            offset += field_offset // 8
            children = [c for c in node.get_children() if c.kind.is_expression()]
            if not children:
                return None
            node = strip_expr(children[0])
        base = lvalue_room(lib, node)
        if base is None:
            return None
        return (base[0] + expr_suffix(expr), base[1] - offset)

    if expr.kind == CursorKind.ARRAY_SUBSCRIPT_EXPR:
        array, index_expr = list(expr.get_children())
        if strip_expr(array).type.get_canonical().kind not in (TypeKind.CONSTANTARRAY, TypeKind.INCOMPLETEARRAY):
            # Subscripting a pointer: the object behind it is unknown.
            return None
        index = evaluate_int(lib, index_expr)
        base = lvalue_room(lib, strip_expr(array))
        element = expr.type.get_size()
        if base is None or element <= 0:
            return None
        if index is None:
            # A variable index: count from the first element, the most room any
            # index can have, so only a certain overrun is reported.
            return ("%s[...]" % base[0], base[1])
        return ("%s[%d]" % (base[0], index), base[1] - index * element)

    return None


def first_token_is_arrow(member_expr):
    """True for `p->m`, whose object is only known through a pointer."""
    children = [c for c in member_expr.get_children() if c.kind.is_expression()]
    if not children:
        return True
    base_end = children[0].extent.end.offset
    for token in member_expr.get_tokens():
        if token.extent.start.offset >= base_end:
            return token.spelling == "->"
    return True


def expr_suffix(member_expr):
    return "." + member_expr.spelling


def callee_name(call):
    name = call.spelling
    if name:
        return name
    ref = call.referenced
    return ref.spelling if ref is not None else None


def check_file(index, lib, path, args, lines_cache, stats):
    tu = index.parse(
        path,
        args=args,
        options=0,
    )
    # Format the diagnostics now: they refer into the translation unit.
    fatal = ["%s" % d for d in tu.diagnostics if d.severity >= 4]
    findings = []
    for cursor in tu.cursor.walk_preorder():
        if cursor.kind != CursorKind.CALL_EXPR:
            continue
        location = cursor.location
        if location.file is None or os.path.abspath(location.file.name) != os.path.abspath(path):
            continue
        name = callee_name(cursor)
        if name not in BLOCK_FUNCTIONS:
            continue
        call_args = list(cursor.get_arguments())
        pointer_args, size_args = BLOCK_FUNCTIONS[name]
        if len(call_args) <= max(pointer_args + size_args):
            continue

        size = 1
        for i in size_args:
            value = evaluate_int(lib, call_args[i])
            if value is None:
                size = None
                break
            size *= value
        stats["calls"] += 1
        if size is None:
            stats["unknown size"] += 1
            continue

        for i in pointer_args:
            room = object_room(lib, call_args[i])
            if room is None or room[1] is None:
                stats["unknown object"] += 1
                if stats["verbose"]:
                    text = " ".join(t.spelling for t in call_args[i].get_tokens())
                    print("unresolved: %s:%d %s(%s)" % (path, location.line, name, text), file=sys.stderr)
                continue
            stats["checked"] += 1
            if size <= room[1]:
                continue
            line = lines_cache(path)[location.line - 1]
            if ALLOW_MARKER in line:
                continue
            findings.append(
                "%s:%d: %s() uses %d bytes (0x%x) through %s, which has %d bytes (0x%x) from there"
                % (path, location.line, name, size, size, room[0], room[1], room[1])
            )
    return findings, fatal


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--clang-lib", help="Path to libclang (.so/.dll)")
    parser.add_argument("--msvc-include", required=True, help="The VC++ 4.1 include directory")
    parser.add_argument("--root", default=".", help="Repository root")
    parser.add_argument("targets", nargs="+", help="Source directories to check (MW2SHELL, MW2, MECH2)")
    parser.add_argument(
        "--verbose", action="store_true", help="Report parse errors, unresolved pointers and coverage too"
    )
    args = parser.parse_args()

    if args.clang_lib:
        clang.cindex.Config.set_library_file(args.clang_lib)
    index = Index.create()
    lib = clang.cindex.conf.lib
    setup_evaluate(lib)

    cache = {}

    def lines(path):
        if path not in cache:
            with open(path, encoding="latin-1") as f:
                cache[path] = f.read().split("\n")
        return cache[path]

    findings = []
    parse_errors = 0
    stats = {"calls": 0, "unknown size": 0, "unknown object": 0, "checked": 0, "verbose": args.verbose}
    with tempfile.TemporaryDirectory() as headers:
        prepare_headers([args.msvc_include] + [os.path.join(args.root, d) for d in UPPERCASE_INCLUDES], headers)
        for target in args.targets:
            target_dir = os.path.join(args.root, target)
            name = os.path.basename(os.path.normpath(target))
            base_args = [
                "-target", "i686-pc-windows-msvc",
                "-fms-extensions",
                "-fms-compatibility",
                "-fms-compatibility-version=10.10",
                "-fno-wchar",
                "-nostdsysteminc",
                "-nostdinc++",
                "-isystem", headers,
            ]
            for inc in COMMON_INCLUDES + [os.path.join(target, "include")]:
                base_args.append("-I" + os.path.join(args.root, inc))
            for define in COMMON_DEFINES + TARGET_DEFINES.get(name, []):
                base_args.append("-D" + define)

            for dirpath, _, filenames in os.walk(target_dir):
                for filename in sorted(filenames):
                    if not filename.endswith(SOURCE_EXTENSIONS):
                        continue
                    path = os.path.join(dirpath, filename)
                    language = ["-x", "c++"] if filename.endswith(".cpp") else ["-x", "c"]
                    file_findings, fatal = check_file(index, lib, path, base_args + language, lines, stats)
                    findings += file_findings
                    if fatal:
                        parse_errors += 1
                        if args.verbose:
                            for d in fatal[:5]:
                                print("parse error: %s" % d, file=sys.stderr)

    for finding in findings:
        print(finding)
    if args.verbose:
        print(
            "%(calls)d block calls: %(checked)d pointer arguments checked; skipped %(unknown size)d calls "
            "with a variable size and %(unknown object)d pointers into unknown objects" % stats,
            file=sys.stderr,
        )
    if parse_errors:
        print("%d file(s) had parse errors; their unparsed calls were not checked" % parse_errors, file=sys.stderr)
    if findings:
        print("%d block operation(s) run past their object" % len(findings))
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
