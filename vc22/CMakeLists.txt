# The VC++ 2.2 sub-project: the targets whose originals were built with VC++ 2.2 (cl 9.10,
# LINK 2.55), MECH2.EXE and NETMECHW.DLL. One CMake project can use only one compiler per
# language, so the top-level (VC++ 4.1) build configures and builds this one as a nested
# build under <build>/vc22, with 2.2's cl and the environment set (see "VC++ 2.2 targets"
# in the top-level CMakeLists.txt). It is not meant to be configured on its own.
#
# The comparison build compiles with 2.2's cl and /Z7 (CodeView in the objects), and links
# with 4.1's LINK (CMAKE_LINKER) against 2.2's libraries (LIB): reccmp's cvdump can't read a
# 2.x PDB, but reads the one LINK 3.10 writes.
cmake_minimum_required(VERSION 3.25...4.2 FATAL_ERROR)

# MSVC runtime library flags are selected by an abstraction
cmake_policy(SET CMP0091 NEW)

project(demech2_vc22 C CXX)

get_filename_component(DEMECH2_SOURCE_DIR "${PROJECT_SOURCE_DIR}/.." ABSOLUTE)

# Visual C++ 2.2 -> cl version 9.x (9.10; CMake reports "MSVC 9.0")
if (NOT MSVC OR CMAKE_C_COMPILER_VERSION VERSION_LESS "9.0" OR CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL "10.0")
  message(FATAL_ERROR "The vc22 sub-project needs VC++ 2.2 (cl 9.x), found ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
endif()
set(DEMECH2_VC22 TRUE)

# --- Compiler and linker flags ---
#
# As in the top-level build: CMake's defaults (/GX, /O2, /Ob1 or /Ob0, /Zm1000; on the link
# /pdbtype:sept, /INCREMENTAL:YES and the default import libraries) are replaced, and each
# target sets its own codegen extras, runtime and link flags. /Z7 instead of /Zi, so the
# debug information stays in the objects for LINK 3.10 (CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
# adds the flag; CMake's default would be /Zi).
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")
foreach(lang C CXX)
  set(CMAKE_${lang}_FLAGS "/DWIN32 /D_WINDOWS /W3 /Od /Oi")
  set(CMAKE_${lang}_FLAGS_DEBUG "")
  set(CMAKE_${lang}_FLAGS_RELEASE "/DNDEBUG")
  set(CMAKE_${lang}_FLAGS_RELWITHDEBINFO "/DNDEBUG")
  set(CMAKE_${lang}_FLAGS_MINSIZEREL "/DNDEBUG")
  set(CMAKE_${lang}_STANDARD_LIBRARIES "")
endforeach()
foreach(kind SHARED EXE)
  set(CMAKE_${kind}_LINKER_FLAGS "/machine:I386")
  foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_${kind}_LINKER_FLAGS_${config} "")
  endforeach()
endforeach()
# LINK 3.10's ~4K limit on the object names it is given (see "Compiler and linker flags" in
# the top-level CMakeLists.txt): a DLL links from flat copies of its objects, listed in a
# response file that demech2_add_netmechw writes before each link.
foreach(lang C CXX)
  string(REPLACE "<OBJECTS>" "@<TARGET_BASE>.objects.rsp"
    CMAKE_${lang}_CREATE_SHARED_LIBRARY "${CMAKE_${lang}_CREATE_SHARED_LIBRARY}")
endforeach()

# The import libraries the targets link, generated here with 4.1's LINK from the same .def
# files as the top-level build's (cmake/import_library.cmake).
include("${DEMECH2_SOURCE_DIR}/cmake/import_library.cmake")
demech2_add_import_library(dplay DEF "${DEMECH2_SOURCE_DIR}/3rdparty/dplay/dplay.def"
  SOURCES "${DEMECH2_SOURCE_DIR}/3rdparty/dplay/dplay.c")

include("${PROJECT_SOURCE_DIR}/targets.cmake")
demech2_add_mech2()
demech2_add_netmechw()
