# Equivalence tests of the portable C that replaces the hand-written assembly (PORTABLE_C in
# util/compat.h) against the assembly itself.
#
# - asmref.dll: the routines as the game builds them, the matched assembly (the reference). Built
#   by x86 MSVC: VC++ 4.1 in the decomp build, the modern compiler's inline assembler otherwise.
# - asmport.dll: the same routines' portable C, from the same sources (the candidate).
# - asmequiv.exe: runs every case through both and compares them; writes the golden vectors.
# - asmgolden: checks the portable C, linked in, against the committed golden vectors
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
)
list(TRANSFORM asmequiv_routine_sources PREPEND "${DEMECH2_SOURCE_DIR}/")

set(asmequiv_include_dirs
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${DEMECH2_SOURCE_DIR}/util"
  "${DEMECH2_SOURCE_DIR}/MW2/include"
)

function(asmequiv_target TARGET)
  target_include_directories(${TARGET} PRIVATE ${asmequiv_include_dirs})
  set_target_properties(${TARGET} PROPERTIES MSVC_RUNTIME_LIBRARY MultiThreaded)
  if (WIN32)
    target_link_libraries(${TARGET} PRIVATE kernel32.lib)
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

add_executable(asmgolden golden.c cases.c ${asmequiv_routine_sources})
target_compile_definitions(asmgolden PRIVATE PORTABLE_C)
asmequiv_target(asmgolden)
add_test(NAME asmgolden COMMAND asmgolden "${golden}")

if (MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 4)
  foreach(side ref port)
    add_library(asm${side} SHARED ${asmequiv_routine_sources} asmequiv.def)
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
  target_compile_definitions(asmport PRIVATE PORTABLE_C)

  add_executable(asmequiv asmequiv.c cases.c)
  asmequiv_target(asmequiv)
  add_dependencies(asmequiv asmref asmport)

  add_test(NAME asmequiv
    COMMAND asmequiv $<TARGET_FILE:asmref> $<TARGET_FILE:asmport>
      -golden "${CMAKE_CURRENT_BINARY_DIR}/golden.txt"
  )
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
