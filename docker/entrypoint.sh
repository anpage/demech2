#!/usr/bin/env bash

# Builds the source mounted at /demech2 into /build and, when a folder of original binaries is
# mounted at /originals, certifies the build against them (tools/verify.py). The exit status is
# the build's, or the verification's.

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

# Configure build with CMake. MSVC 2.2 builds MECH2.EXE, NETMECHW.DLL and MW2_3DFX.DLL in a nested
# build (build/vc22), MSVC 4.0 MW2_MATROX.DLL in another (build/vc40).
# ML.EXE is passed explicitly: MASM's BIN also holds a 16-bit LINK, LIB and NMAKE
# The default build type is the one the originals are compared with.
wine cmake -B build demech2 -G "NMake Makefiles" -DCMAKE_ASM_MASM_COMPILER=C:/masm/BIN/ML.EXE -DDEMECH2_MSVC22_ROOT=C:/msvc22 -DDEMECH2_MSVC40_ROOT=C:/msvc40 ${CMAKE_FLAGS:--DCMAKE_BUILD_TYPE=RelWithDebInfo}

# Start compiling MechWarrior 2
wine cmake --build build
status=$?

# Certify the build against the originals, if any are mounted: every function byte-identical,
# proven by its witness record, or unproven (the lists go to build/states).
if [ "$status" -eq 0 ] && [ -d /originals ]; then
  # reccmp reads the annotations. The image has it at the commit its tools/requirements.txt
  # pinned; a source tree that pins another gets its own (this needs the network).
  if ! cmp -s demech2/tools/requirements.txt /opt/requirements.txt; then
    echo "tools/requirements.txt differs from the image's: installing it"
    /opt/venv/bin/pip install --quiet --force-reinstall --no-deps -r demech2/tools/requirements.txt
  fi
  /opt/venv/bin/python demech2/tools/verify.py --originals /originals --build /build --out /build/states $VERIFY_FLAGS
  status=$?
fi

# Restore directory ownership
chown -R "$SRC_DIR_OWNERSHIP" demech2
chown -R "$BUILD_DIR_OWNERSHIP" build

exit $status
