# Equivalence tests of the portable C that replaces the hand-written assembly (PORTABLE_C in
# util/compat.h) against the assembly itself.
#
# - asmref.dll: the routines as the game builds them, the matched assembly (the reference). Built
#   by x86 MSVC: VC++ 4.1 in the decomp build, the modern compiler's inline assembler otherwise.
# - asmport.dll: the same routines' portable C, from the same sources (the candidate).
# - asmequiv.exe: runs every case through both and compares them; writes the golden vectors.
# - asmequiv-exhaustive (with -DASMEQUIV_EXHAUSTIVE=ON; not in CI, it takes about a minute): the
#   routines with one argument word, on every input.
# - coverage (decomp build, with a Python that has capstone): the asmequiv run also checks that
#   the cases reach every basic block of the reference (tools/asmblocks.py lists them).
# - asmgolden-0 to -7: check the portable C, linked in, against the committed golden vectors
#   (golden.txt). Builds anywhere: this directory is also a standalone project, for compilers and
#   platforms the game doesn't build on yet (cmake -S tests/asmequiv, with
#   -DASMEQUIV_SANITIZE=ON for UBSan and ASan).
#
# The decomp build checks that golden.txt is what its reference writes; to update it after
# adding cases or routines, copy <build>/tests/asmequiv/golden.txt over it.
if (NOT DEFINED DEMECH2_SOURCE_DIR)
  cmake_minimum_required(VERSION 3.25...4.2 FATAL_ERROR)
  project(asmequiv C)
  set(DEMECH2_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
  set(MSVC_FOR_DECOMP FALSE)
  enable_testing()
  option(ASMEQUIV_SANITIZE "Build asmgolden with UBSan and ASan (GCC, Clang)" OFF)
  # The units' C predates C23, where an empty parameter list means no parameters
  # (RenderSettings's hooks)
  set(CMAKE_C_STANDARD 17)
endif()

# The units under test, from the game's sources
set(asmequiv_routine_sources
  MW2/src/fixedmul.c
  MW2/src/fixedmul30.c
  MW2/src/fixeddiv.c
  MW2/src/fixeddiv29.c
  MW2/src/fixeddivu.c
  MW2/src/fixeddot27.c
  MW2/src/fixeddot29.c
  MW2/src/muldiv.c
  MW2/src/approxlen.c
  MW2/src/sqrtguess.c
  MW2/src/namehash.c
  MW2/src/integrate.c
  MW2/src/inradius.c
  MW2/src/muldiv14.c
  MW2/src/fixedmul29.c
  MW2/src/scaledelta.c
  MW2/src/crossdiv.c
  MW2/src/lerp.c
  MW2/src/shiftdiv.c
  MW2/src/mulratio.c
  MW2/src/mulnorm16.c
  MW2/src/muladddiv.c
  MW2/src/loadres.c
  MW2/src/fixedtrig.c
  MW2/src/horizon.c
  MW2/src/recordstacks.c
  MW2/src/transform.c
  MW2/src/shapegeom.c
  MW2/src/objectanim.c
)
list(TRANSFORM asmequiv_routine_sources PREPEND "${DEMECH2_SOURCE_DIR}/")
# MASM objects: the reference assembles the .asm where the build has MASM (the decomp build), and
# the candidate compiles the portable C of the unit's .c
set(asmequiv_masm_units
  MW2/src/sndunpack
  MW2/src/ticks
)
if (NOT DEFINED asm_or_c)
  set(asm_or_c c)
endif()
list(TRANSFORM asmequiv_masm_units PREPEND "${DEMECH2_SOURCE_DIR}/")
list(TRANSFORM asmequiv_masm_units APPEND ".c" OUTPUT_VARIABLE asmequiv_masm_c)
list(TRANSFORM asmequiv_masm_units APPEND ".${asm_or_c}" OUTPUT_VARIABLE asmequiv_masm_ref)
# Miles Design's VFX objects (3rdparty/vfx), whose portable C is in common/src
list(APPEND asmequiv_masm_c
  "${DEMECH2_SOURCE_DIR}/common/src/vfx3d.c"
  "${DEMECH2_SOURCE_DIR}/common/src/vfxa.c"
  "${DEMECH2_SOURCE_DIR}/common/src/vfxrend.c"
)
if (asm_or_c STREQUAL "asm")
  set(asmequiv_vfx_ref
    "${DEMECH2_SOURCE_DIR}/3rdparty/vfx/VFX3D.ASM"
    "${DEMECH2_SOURCE_DIR}/3rdparty/vfx/VFXA.ASM"
    "${DEMECH2_SOURCE_DIR}/3rdparty/vfx/VFXREND.ASM"
  )
  set_source_files_properties(${asmequiv_vfx_ref} PROPERTIES
    LANGUAGE ASM_MASM
    COMPILE_OPTIONS "/Cx;/DMASM;/DFLAT_MODEL;/I${DEMECH2_SOURCE_DIR}/3rdparty/vfx")
else()
  set(asmequiv_vfx_ref
    "${DEMECH2_SOURCE_DIR}/common/src/vfx3d.c"
    "${DEMECH2_SOURCE_DIR}/common/src/vfxa.c"
    "${DEMECH2_SOURCE_DIR}/common/src/vfxrend.c"
  )
endif()
list(APPEND asmequiv_masm_ref ${asmequiv_vfx_ref})
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" AND NOT MSVC)
  # The units' other functions: C names every parameter, and the debug logs print sizes with %d
  set_source_files_properties(${asmequiv_routine_sources} ${asmequiv_masm_c} PROPERTIES
    COMPILE_OPTIONS "-Wno-unused-parameter;-Wno-format")
endif()
# What the units reference beyond the routines: ray.c (FUN_1003a096 reads a ray's length through
# it), clock.c's tables and stand-ins
list(APPEND asmequiv_routine_sources
  "${DEMECH2_SOURCE_DIR}/MW2/src/ray.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/stubs.c"
)

set(asmequiv_include_dirs
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${DEMECH2_SOURCE_DIR}/util"
  "${DEMECH2_SOURCE_DIR}/MW2/include"
  "${DEMECH2_SOURCE_DIR}/common/include"
  "${DEMECH2_SOURCE_DIR}/3rdparty/mss"
)
if (NOT WIN32)
  # The little of <windows.h> the units include
  list(APPEND asmequiv_include_dirs "${CMAKE_CURRENT_SOURCE_DIR}/posix")
endif()

function(asmequiv_target TARGET)
  target_include_directories(${TARGET} PRIVATE ${asmequiv_include_dirs})
  set_target_properties(${TARGET} PROPERTIES MSVC_RUNTIME_LIBRARY MultiThreaded)
  if (WIN32)
    target_link_libraries(${TARGET} PRIVATE kernel32.lib)
  endif()
  if (NOT WIN32)
    target_link_libraries(${TARGET} PRIVATE m)
  endif()
  if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" AND NOT MSVC)
    # The routines' 4.1 pragmas
    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wno-unknown-pragmas)
    if (ASMEQUIV_SANITIZE)
      target_compile_options(${TARGET} PRIVATE
        -fsanitize=undefined,address -fno-sanitize-recover=all -fno-omit-frame-pointer)
      target_link_options(${TARGET} PRIVATE -fsanitize=undefined,address)
    endif()
  endif()
endfunction()

set(golden "${CMAKE_CURRENT_SOURCE_DIR}/golden.txt")

add_executable(asmgolden golden.c cases.c routines.c ${asmequiv_routine_sources} ${asmequiv_masm_c})
target_compile_definitions(asmgolden PRIVATE PORTABLE_C PORTABLE_ASSERTS)
asmequiv_target(asmgolden)
# In shards, which ctest -j runs in parallel: each checks every eighth line of the vectors
set(asmgolden_shards 8)
math(EXPR asmgolden_last "${asmgolden_shards} - 1")
foreach(shard RANGE ${asmgolden_last})
  add_test(NAME asmgolden-${shard} COMMAND asmgolden "${golden}" -shard ${shard} ${asmgolden_shards})
endforeach()

if (MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(asmequiv_masm_sources_ref ${asmequiv_masm_ref})
  set(asmequiv_masm_sources_port ${asmequiv_masm_c})
  foreach(side ref port)
    add_library(asm${side} SHARED ${asmequiv_routine_sources} ${asmequiv_masm_sources_${side}} asmequiv.def)
    asmequiv_target(asm${side})
    if (TARGET mw2)
      # The game's codegen flags (see MW2.DLL)
      target_compile_options(asm${side} PRIVATE $<TARGET_PROPERTY:mw2,COMPILE_OPTIONS>)
    endif()
    if (COMMAND demech2_link_object_copies)
      demech2_link_object_copies(asm${side})
    endif()
  endforeach()
  target_compile_definitions(asmref PRIVATE REFERENCE_ASM)
  target_compile_definitions(asmport PRIVATE PORTABLE_C PORTABLE_ASSERTS)

  add_executable(asmequiv asmequiv.c cases.c routines.c)
  asmequiv_target(asmequiv)
  add_dependencies(asmequiv asmref asmport)

  # Coverage (decomp build): the cases reach every basic block of the reference.
  # tools/asmblocks.py lists the blocks, with capstone, and the asmequiv run that writes the golden
  # vectors checks them; without a Python that has capstone, coverage is left out.
  set(asmequiv_coverage)
  if (MSVC_FOR_DECOMP)
    find_package(Python3 COMPONENTS Interpreter QUIET)
    set(capstone_result 1)
    if (Python3_FOUND)
      execute_process(COMMAND "${Python3_EXECUTABLE}" -c "import capstone"
        RESULT_VARIABLE capstone_result OUTPUT_QUIET ERROR_QUIET)
    endif()
    if (capstone_result EQUAL 0)
      set(blocks "${CMAKE_CURRENT_BINARY_DIR}/blocks.txt")
      add_test(NAME asmequiv-blocks
        COMMAND "${Python3_EXECUTABLE}" "${DEMECH2_SOURCE_DIR}/tools/asmblocks.py"
          $<TARGET_FILE:asmref> -o "${blocks}"
      )
      set_tests_properties(asmequiv-blocks PROPERTIES FIXTURES_SETUP asmequiv_blocks)
      set(asmequiv_coverage -coverage "${blocks}")
    else()
      message(STATUS "asmequiv: no Python with capstone, so no coverage check")
    endif()
  endif()

  add_test(NAME asmequiv
    COMMAND asmequiv $<TARGET_FILE:asmref> $<TARGET_FILE:asmport>
      -golden "${CMAKE_CURRENT_BINARY_DIR}/golden.txt" ${asmequiv_coverage}
  )
  if (asmequiv_coverage)
    set_tests_properties(asmequiv PROPERTIES FIXTURES_REQUIRED asmequiv_blocks)
  endif()
  # The routines with one argument word, on all 2^32 inputs: about a minute, so not in CI
  option(ASMEQUIV_EXHAUSTIVE "Add asmequiv-exhaustive, the one-argument routines on every input" OFF)
  if (ASMEQUIV_EXHAUSTIVE)
    add_test(NAME asmequiv-exhaustive
      COMMAND asmequiv $<TARGET_FILE:asmref> $<TARGET_FILE:asmport> -exhaustive
    )
  endif()
  if (MSVC_FOR_DECOMP)
    # The committed golden vectors are the 4.1 reference's
    set_tests_properties(asmequiv PROPERTIES FIXTURES_SETUP asmequiv_golden)
    add_test(NAME asmequiv-golden-current
      COMMAND "${CMAKE_COMMAND}" -E compare_files --ignore-eol
        "${golden}" "${CMAKE_CURRENT_BINARY_DIR}/golden.txt"
    )
    set_tests_properties(asmequiv-golden-current PROPERTIES FIXTURES_REQUIRED asmequiv_golden)
  endif()
endif()
