# The VC++ 4.0 sub-project: the target whose original was built with VC++ 4.0 (cl 10.00,
# LINK 3.00), MW2_MATROX.DLL. One CMake project can use only one compiler per language, so
# the top-level (VC++ 4.1) build configures and builds this one as a nested build under
# <build>/vc40, with 4.0's cl, LINK and libraries and the environment set (see "VC++ 4.0
# targets" in the top-level CMakeLists.txt). It is not meant to be configured on its own.
#
# 4.0 compiles and links like 4.1 (cmake/decomp_flags.cmake), and reccmp's cvdump reads the
# PDB LINK 3.00 writes.
cmake_minimum_required(VERSION 3.25...4.2 FATAL_ERROR)

# MSVC runtime library flags are selected by an abstraction
cmake_policy(SET CMP0091 NEW)

project(demech2_vc40 C CXX)

get_filename_component(DEMECH2_SOURCE_DIR "${PROJECT_SOURCE_DIR}/.." ABSOLUTE)

# Visual C++ 4.0 -> cl version 10.00 (CMake reports "MSVC 10.0"; 4.1 is 10.10)
if (NOT MSVC OR NOT CMAKE_C_COMPILER_VERSION VERSION_EQUAL "10.0")
  message(FATAL_ERROR "The vc40 sub-project needs VC++ 4.0 (cl 10.00), found ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
endif()
set(MSVC_FOR_DECOMP TRUE)

include("${DEMECH2_SOURCE_DIR}/cmake/masm.cmake")
include("${DEMECH2_SOURCE_DIR}/cmake/decomp_flags.cmake")

option(DEMECH2_DECOMP_ASSERT "Assert struct size" TRUE)

# The import libraries the target links, generated here with 4.0's LINK from the same .def
# files as the top-level build's (cmake/import_library.cmake).
include("${DEMECH2_SOURCE_DIR}/cmake/import_library.cmake")
demech2_add_import_library(ddraw DEF "${DEMECH2_SOURCE_DIR}/3rdparty/ddraw/ddraw.def"
  SOURCES "${DEMECH2_SOURCE_DIR}/3rdparty/ddraw/ddraw.c")
demech2_add_import_library(dplay DEF "${DEMECH2_SOURCE_DIR}/3rdparty/dplay/dplay.def"
  SOURCES "${DEMECH2_SOURCE_DIR}/3rdparty/dplay/dplay.c")
demech2_add_import_library(wail32 DEF "${DEMECH2_SOURCE_DIR}/3rdparty/mss/wail32.def"
  SOURCES "${DEMECH2_SOURCE_DIR}/3rdparty/mss/wail32.c")

include("${DEMECH2_SOURCE_DIR}/cmake/game_dll.cmake")
include("${PROJECT_SOURCE_DIR}/targets.cmake")
demech2_add_mw2matrox()
