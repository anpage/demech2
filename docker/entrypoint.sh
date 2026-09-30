#!/usr/bin/env bash

# Cache the ownership status of the passed
# directories before Docker starts messing with it
SRC_DIR_OWNERSHIP="$(stat -c '%u:%g' demech2)"
BUILD_DIR_OWNERSHIP="$(stat -c '%u:%g' build)"

# Silence debug warnings in wine (creates noise during compile)
export WINEDEBUG=-all

# Populate the Windows path inside of the wineprefix
# TODO: This is in here because writing to the registry seems
# to fail when performed in the Dockerfile itself; investigate
wine reg ADD 'HKCU\Environment' /v PATH /d 'C:\msvc\bin;C:\cmake\bin;C:\windows\system32' /f
wine reg ADD 'HKCU\Environment' /v INCLUDE /d 'C:\msvc\include' /f
wine reg ADD 'HKCU\Environment' /v LIB /d 'C:\msvc\lib' /f
wine reg ADD 'HKCU\Environment' /v TMP /d 'Z:\build' /f
wine reg ADD 'HKCU\Environment' /v TEMP /d 'Z:\build' /f

# Configure build with CMake
# ML.EXE is passed explicitly: MASM's BIN also holds a 16-bit LINK, LIB and NMAKE
wine cmake -B build demech2 -G "NMake Makefiles" -DCMAKE_ASM_MASM_COMPILER=C:/masm/BIN/ML.EXE $CMAKE_FLAGS

# Start compiling MechWarrior 2
wine cmake --build build

# Restore directory ownership
chown -R "$SRC_DIR_OWNERSHIP" demech2
chown -R "$BUILD_DIR_OWNERSHIP" build
