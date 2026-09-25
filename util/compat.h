#ifndef COMPAT_H
#define COMPAT_H

// Various macros to enable compiling with other/newer compilers.

// Visual C++ 4.1 (the original toolchain) -> _MSC_VER 1010
#define MSVC410_VERSION 1010

#if defined(__MINGW32__) || defined(__clang__) || defined(__GNUC__) || (defined(_MSC_VER) && _MSC_VER > MSVC410_VERSION)
#define COMPAT_MODE
#endif

#if defined(_MSC_VER)
// Disable "identifier was truncated to '255' characters" warning.
// Impossible to avoid this if using STL map or set.
// This removes most (but not all) occurrences of the warning.
#pragma warning(disable : 4786)
#endif

// We use `override` so newer compilers can tell us our vtables are valid,
// however this keyword was added in C++11, so we define it as empty for
// compatibility with older compilers. C has no such keyword, so the shims
// are only defined for C++.
#if defined(__cplusplus) && __cplusplus < 201103L
#define override
#define static_assert(expr, msg)
#elif defined(__cplusplus)
#define override override
#endif

#endif // COMPAT_H
