# Included by the top-level project and the nested VC++ builds (vc40/). Expects
# DEMECH2_SOURCE_DIR and MSVC_FOR_DECOMP.

# The original's hand-written assembly objects are assembled with MASM 6.11 (ML). Pass ML.EXE as
# CMAKE_ASM_MASM_COMPILER rather than putting MASM's BIN on PATH: it also holds a 16-bit LINK,
# LIB and NMAKE that would shadow VC++ 4.1's. Other compilers build the units' C stubs instead.
if (MSVC_FOR_DECOMP)
  # CMake writes the path into a generated script, where a Windows path's backslashes are escapes
  if (CMAKE_ASM_MASM_COMPILER)
    file(TO_CMAKE_PATH "${CMAKE_ASM_MASM_COMPILER}" CMAKE_ASM_MASM_COMPILER)
  endif()
  enable_language(ASM_MASM)
  set(CMAKE_ASM_MASM_FLAGS "/nologo /coff")
  set(CMAKE_ASM_MASM_FLAGS_DEBUG "/Zi")
  set(CMAKE_ASM_MASM_FLAGS_RELEASE "")
  set(CMAKE_ASM_MASM_FLAGS_RELWITHDEBINFO "/Zi")
  set(CMAKE_ASM_MASM_FLAGS_MINSIZEREL "")
  set(asm_or_c asm)
  # Miles Design's VFX (3rdparty/vfx): built with ML's MASM syntax and flat model, publics in their
  # declared case
  set(vfxa ${DEMECH2_SOURCE_DIR}/3rdparty/vfx/VFXA.ASM)
  set(vfx3d ${DEMECH2_SOURCE_DIR}/3rdparty/vfx/VFX3D.ASM)
  set(vfxrend ${DEMECH2_SOURCE_DIR}/3rdparty/vfx/VFXREND.ASM)
  set_source_files_properties(${vfxa} ${vfx3d} ${vfxrend} PROPERTIES
    LANGUAGE ASM_MASM
    COMPILE_OPTIONS "/Cx;/DMASM;/DFLAT_MODEL;/I${DEMECH2_SOURCE_DIR}/3rdparty/vfx")
else()
  set(asm_or_c c)
  set(vfxa ${DEMECH2_SOURCE_DIR}/common/src/vfxa.c)
  set(vfx3d ${DEMECH2_SOURCE_DIR}/common/src/vfx3d.c)
  set(vfxrend ${DEMECH2_SOURCE_DIR}/common/src/vfxrend.c)
endif()
