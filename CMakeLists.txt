cmake_minimum_required(VERSION 3.25...4.2 FATAL_ERROR)

# MSVC runtime library flags are selected by an abstraction
cmake_policy(SET CMP0091 NEW)

project(demech2 C CXX)

if (NOT WIN32)
  message(FATAL_ERROR "Windows is the only supported platform (for now...)")
endif()

include(CheckCXXSourceCompiles)
include(CMakePushCheckState)
include("${CMAKE_CURRENT_LIST_DIR}/cmake/reccmp.cmake")

set(DEMECH2_SOURCE_DIR "${PROJECT_SOURCE_DIR}")

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
option(ENABLE_CLANG_TIDY "Enable clang-tidy")
if (ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_BIN NAMES "clang-tidy")
  set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_BIN}")
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_BIN}")
endif()

math(EXPR bits "8 * ${CMAKE_SIZEOF_VOID_P}")
message(STATUS "Building ${bits}-bit MechWarrior 2")
if (NOT bits EQUAL 32)
  message(WARNING "Only 32-bit executables are supported")
endif()

set(MSVC_FOR_DECOMP FALSE)
if (MSVC)
  # Visual C++ 4.x -> cl version 10.x (VC++ 4.1, the original toolchain, is 10.10)
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "10.0" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.0")
    set(MSVC_FOR_DECOMP TRUE)
  endif()
endif()

message(STATUS "MSVC for decompilation: ${MSVC_FOR_DECOMP}")

include("${DEMECH2_SOURCE_DIR}/cmake/masm.cmake")
# A unit of hand-written assembly is listed as <dir>/<name>.${asm_or_c}: MW2's .asm, or its .c
# of portable C. The VFX objects both DLLs link are ${vfxa}, ${vfx3d} and ${vfxrend}: VFX's source,
# or common/src's portable C and stubs, declared in common/include.

option(DEMECH2_WERROR "Treat warnings as errors" OFF)
option(DEMECH2_DECOMP_ASSERT "Assert struct size" ${MSVC_FOR_DECOMP})

function(add_warning WARNING)
  if (DEMECH2_WERROR)
    set(compiler_option "-Werror=${WARNING}")
  else()
    set(compiler_option "-W${WARNING}")
  endif()
  string(MAKE_C_IDENTIFIER "COMPILER_SUPPORTS${compiler_option}" varname)

  cmake_push_check_state(RESET)
  set(CMAKE_REQUIRED_FLAGS "${compiler_option} ")
  if (MSVC)
    string(APPEND CMAKE_REQUIRED_FLAGS "/WX")
  else()
    string(APPEND CMAKE_REQUIRED_FLAGS "-Werror")
  endif()
  check_cxx_source_compiles("int main() { return 0; }" ${varname})
  cmake_pop_check_state()

  if (${varname})
    add_compile_options(${compiler_option})
  endif()
endfunction()

add_warning(parentheses)

find_program(IWYU_BIN NAMES "include-what-you-use")
if (IWYU_BIN)
  set(IWYU_ARGS "-target i686-pc-windows-msvc" "-Xiwyu" "--mapping_file=${PROJECT_SOURCE_DIR}/tools/iwyu.imp")
  set(CMAKE_C_INCLUDE_WHAT_YOU_USE "${IWYU_BIN}" ${IWYU_ARGS})
  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${IWYU_BIN}" ${IWYU_ARGS})
endif()

# --- Compiler and linker flags ---
#
# Both DLLs compile their game code at /Od /Oi in every configuration: it is the only
# setting that matches. CMake's defaults for VC++ 4.1 add codegen flags of their own
# (/GX for all C++, /Ob0 in Debug, /GZ which 4.1 rejects, /Zm1000), link flags LINK 3.10
# doesn't know (/pdbtype:sept) or that change the output (/INCREMENTAL:YES), and eleven
# default import libraries (.idata descriptor order follows library processing order).
# Replace them all; each target sets its own codegen extras, runtime and link flags.
if (MSVC_FOR_DECOMP)
  include("${DEMECH2_SOURCE_DIR}/cmake/decomp_flags.cmake")
endif()

if (DEMECH2_DECOMP_ASSERT)
  message(STATUS "Decomp asserts enabled")
endif()

# --- Third-party import libraries ---
#
# Generated from the .def files in 3rdparty/ (cmake/import_library.cmake). The VC++ 2.2
# sub-project generates the ones its targets link again, from the same .def files.
include("${PROJECT_SOURCE_DIR}/cmake/import_library.cmake")

demech2_add_import_library(ddraw DEF "${PROJECT_SOURCE_DIR}/3rdparty/ddraw/ddraw.def"
  SOURCES 3rdparty/ddraw/ddraw.c)
demech2_add_import_library(dplay DEF "${PROJECT_SOURCE_DIR}/3rdparty/dplay/dplay.def"
  SOURCES 3rdparty/dplay/dplay.c)
demech2_add_import_library(wail32 DEF "${PROJECT_SOURCE_DIR}/3rdparty/mss/wail32.def"
  SOURCES 3rdparty/mss/wail32.c)
demech2_add_import_library(smackw32 DEF "${PROJECT_SOURCE_DIR}/3rdparty/smacker/smackw32.def")
demech2_add_import_library(msi95 DEF "${PROJECT_SOURCE_DIR}/3rdparty/msi95/msi95.def")

# --- Game DLLs ---
#
# demech2_add_dll and its helpers are in cmake/game_dll.cmake, which vc40/ uses too.
include("${DEMECH2_SOURCE_DIR}/cmake/game_dll.cmake")

# MW2SHELL.DLL: /Od /Oi /G5 /Ob1 (C++ adds /GX), the release CRT (4.1's LIBCMT). The C units
# need /Ob1 too: mouse.c expands an __inline bounds test, with a jmp per return.
# /G5 shows in the shift encoding: the original's signed `/ 2` is `sar eax, 1` in the short
# form (d1 f8) at every site, which 4.1 only emits under /G5 (c1 f8 01 otherwise).
# The original is a plain /DLL link. /DEBUG gives reccmp its PDB, at the cost of a debug
# directory the original doesn't have; /OPT:REF keeps the plain link's CRT shape, which
# /DEBUG would otherwise turn off, and /INCREMENTAL:no the direct exports.
set(mw2shell_link_options "")
if (MSVC_FOR_DECOMP)
  foreach(option /DEBUG /OPT:REF /INCREMENTAL:no)
    list(APPEND mw2shell_link_options "$<$<CONFIG:Debug,RelWithDebInfo>:${option}>")
  endforeach()
endif()
demech2_add_dll(mw2shell
  ID MW2SHELL
  OUTPUT_NAME MW2SHELL
  RUNTIME MultiThreaded
  SOURCES
    MW2SHELL/src/debrief.cpp
    MW2SHELL/src/mechvariant.cpp
    MW2SHELL/src/keyboard.c
    MW2SHELL/src/resourcename.c
    MW2SHELL/src/font.cpp
    MW2SHELL/src/stringcollection.c
    MW2SHELL/src/videodriver.cpp
    MW2SHELL/src/mechbay.cpp
    MW2SHELL/src/shellmain.cpp
    MW2SHELL/src/refreshmode.c
    MW2SHELL/src/directdraw.c
    MW2SHELL/src/resourcecache.c
    MW2SHELL/src/clanhall.cpp
    MW2SHELL/src/rosterscreen.cpp
    MW2SHELL/src/debugprint.c
    MW2SHELL/src/video.cpp
    MW2SHELL/src/debugout.c
    ${vfxrend}
    MW2SHELL/src/archivereader.cpp
    ${vfx3d}
    MW2SHELL/src/pilotroster.cpp
    MW2SHELL/src/textpages.cpp
    MW2SHELL/src/projectarchive.cpp
    MW2SHELL/src/dispdib.c
    MW2SHELL/src/prjfile.c
    MW2SHELL/src/stringutil.c
    MW2SHELL/src/cdcheck.cpp
    MW2SHELL/src/gdi.c
    MW2SHELL/src/resourcefile.c
    MW2SHELL/src/input.c
    ${vfxa}
    MW2SHELL/src/missionui.cpp
    MW2SHELL/src/tokenstream.c
    MW2SHELL/src/simhandoff.cpp
    MW2SHELL/src/readyroom.cpp
    MW2SHELL/src/mousestate.cpp
    MW2SHELL/src/joystick.c
    MW2SHELL/src/windowstate.c
    MW2SHELL/src/mw2prj.c
    MW2SHELL/src/collection.c
    MW2SHELL/src/cadettraining.cpp
    MW2SHELL/src/audio.cpp
    MW2SHELL/src/mainmenu.cpp
    MW2SHELL/src/textglyphlist.cpp
    MW2SHELL/src/hallofhonor.cpp
    MW2SHELL/src/cockpitcontrols.cpp
    MW2SHELL/src/menudata.cpp
    MW2SHELL/src/options.cpp
    MW2SHELL/src/keyboardinput.cpp
    MW2SHELL/src/page.cpp
    MW2SHELL/src/shellglobals.cpp
    MW2SHELL/src/popuppicture.cpp
    MW2SHELL/src/brightness.c
    MW2SHELL/src/briefing.cpp
    MW2SHELL/src/mouse.c
    MW2SHELL/src/credits.cpp
    MW2SHELL/src/textglyph.cpp
    MW2SHELL/src/tmpackdatabase.cpp
    MW2SHELL/src/buttonmenu.cpp
    MW2SHELL/MW2SHELL.def
    MW2SHELL/mw2shell.rc
  COMPILE_OPTIONS "$<$<COMPILE_LANGUAGE:C,CXX>:/G5;/Ob1>" "$<$<COMPILE_LANGUAGE:CXX>:/GX>"
  LINK_OPTIONS ${mw2shell_link_options}
  LIBRARIES winmm.lib ddraw wail32 smackw32 kernel32.lib user32.lib gdi32.lib advapi32.lib
)

include("${PROJECT_SOURCE_DIR}/cmake/extract_resource.cmake")

# The shell's dialogs show icon 103, which stays out of the repository: it is extracted
# from the original MW2SHELL.DLL at build time. Without the original, the dialogs build
# with an empty icon slot.
set(DEMECH2_MW2SHELL_ORIGINAL "${PROJECT_SOURCE_DIR}/MW2SHELL.DLL" CACHE FILEPATH
  "Original MW2SHELL.DLL (1.1 patch) to take the shell's icon from")
set(mw2shell_original_sha256 1078ccb07fd45388bfd525719a8206ce7a01d6536776d30a5655ccb928900879)
set(mw2shell_icon FALSE)
if (EXISTS "${DEMECH2_MW2SHELL_ORIGINAL}")
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${DEMECH2_MW2SHELL_ORIGINAL}")
  file(SHA256 "${DEMECH2_MW2SHELL_ORIGINAL}" sha256)
  if (sha256 STREQUAL mw2shell_original_sha256)
    set(mw2shell_icon TRUE)
  else()
    message(WARNING "${DEMECH2_MW2SHELL_ORIGINAL} is not the 1.1 MW2SHELL.DLL; building the shell without its icon")
  endif()
endif()
if (mw2shell_icon)
  message(STATUS "Shell icon: from ${DEMECH2_MW2SHELL_ORIGINAL}")
  demech2_add_extract_resource()
  set(icon_dir "${CMAKE_CURRENT_BINARY_DIR}/MW2SHELL")
  add_custom_command(OUTPUT "${icon_dir}/mw2shell.ico"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${icon_dir}"
    COMMAND extract_resource icon "${DEMECH2_MW2SHELL_ORIGINAL}" 103 "${icon_dir}/mw2shell.ico"
    DEPENDS extract_resource "${DEMECH2_MW2SHELL_ORIGINAL}"
    VERBATIM
  )
  set_source_files_properties(MW2SHELL/mw2shell.rc PROPERTIES
    COMPILE_DEFINITIONS DEMECH2_ORIGINAL_ICON
    INCLUDE_DIRECTORIES "${icon_dir}"
    OBJECT_DEPENDS "${icon_dir}/mw2shell.ico"
  )
else()
  message(STATUS "Shell icon: none (no original MW2SHELL.DLL at ${DEMECH2_MW2SHELL_ORIGINAL})")
endif()

# MW2.DLL: C at /Od /Oi /G5, the debug CRT (4.1's LIBCMTD; /MTd predefines _DEBUG). The
# original was linked /DEBUG /INCREMENTAL:no in every configuration. /G5 shows in the shift
# encoding, as in the shell: StartPaletteFade's `>>= 1` is the short form (d1 7d 0c).
set(mw2_link_options "")
if (MSVC_FOR_DECOMP)
  set(mw2_link_options /DEBUG /INCREMENTAL:no)
endif()
include("${DEMECH2_SOURCE_DIR}/MW2/sources.cmake")
demech2_add_dll(mw2
  ID MW2
  OUTPUT_NAME MW2
  RUNTIME MultiThreadedDebug
  SOURCES
    ${mw2_sources}
    MW2/MW2.def
  COMPILE_OPTIONS "$<$<COMPILE_LANGUAGE:C,CXX>:/G5>"
  LINK_OPTIONS ${mw2_link_options}
  LIBRARIES winmm.lib wail32 ddraw dplay kernel32.lib user32.lib gdi32.lib advapi32.lib
)

# --- VC++ 2.2 targets ---
#
# MECH2.EXE and NETMECHW.DLL were built with VC++ 2.2, which one CMake project can't mix
# with 4.1: the vc22/ sub-project builds them, configured and built from here as a nested
# build under <build>/vc22 with 2.2's cl (DEMECH2_MSVC22_ROOT, e.g. a clone of
# archaic-msvc/msvc220) and 4.1's LINK (see vc22/CMakeLists.txt). The nested build gets its
# PATH, INCLUDE and LIB from here, so it doesn't matter which VCVARS32.BAT ran. Without
# VC++ 2.2, or with a modern compiler, this project builds the same targets itself
# (vc22/targets.cmake): a working launcher and NetMech lobby, which can't match the originals.
set(DEMECH2_MSVC22_ROOT "" CACHE PATH "VC++ 2.2 root directory (bin, include, lib), for MECH2.EXE and NETMECHW.DLL")
if (MSVC_FOR_DECOMP AND DEMECH2_MSVC22_ROOT)
  message(STATUS "VC++ 2.2: ${DEMECH2_MSVC22_ROOT} (vc22/, built in ${PROJECT_BINARY_DIR}/vc22)")
  include(ExternalProject)
  set(DEMECH2_MECH2_ORIGINAL "${PROJECT_SOURCE_DIR}/MECH2.EXE" CACHE FILEPATH
    "Original MECH2.EXE (retail release) to take the launcher's icons from")
  set(DEMECH2_NETMECHW_ORIGINAL "${PROJECT_SOURCE_DIR}/NETMECHW.DLL" CACHE FILEPATH
    "Original NETMECHW.DLL (retail release) to take the lobby's icons and bitmaps from")
  # The nested commands run through a generated script that sets the environment:
  # 2.2's bin ahead of this build's PATH, and INCLUDE and LIB as 2.2's VCVARS32.BAT sets
  # them (winres.h is in mfc\include).
  file(TO_NATIVE_PATH "${DEMECH2_MSVC22_ROOT}" msvc22)
  set(env_path "${msvc22}\\bin;$ENV{PATH}")
  set(env_include "${msvc22}\\include;${msvc22}\\mfc\\include")
  set(env_lib "${msvc22}\\lib;${msvc22}\\mfc\\lib")
  set(vc22_env_script "${PROJECT_BINARY_DIR}/vc22-env.cmake")
  configure_file("${PROJECT_SOURCE_DIR}/cmake/nested-env.cmake.in" "${vc22_env_script}" @ONLY)
  set(vc22_env "${CMAKE_COMMAND}" -P "${vc22_env_script}" --)
  # This build's NMake by its full path: 2.2's bin comes first on the nested build's PATH.
  find_program(vc22_make_program NAMES "${CMAKE_MAKE_PROGRAM}" NO_CACHE REQUIRED)
  ExternalProject_Add(vc22
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/vc22"
    BINARY_DIR "${PROJECT_BINARY_DIR}/vc22"
    CONFIGURE_COMMAND ${vc22_env} "${CMAKE_COMMAND}" -S <SOURCE_DIR> -B <BINARY_DIR>
      -G "${CMAKE_GENERATOR}"
      "-DCMAKE_MAKE_PROGRAM=${vc22_make_program}"
      "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
      "-DCMAKE_C_COMPILER=${DEMECH2_MSVC22_ROOT}/bin/cl.exe"
      "-DCMAKE_CXX_COMPILER=${DEMECH2_MSVC22_ROOT}/bin/cl.exe"
      "-DCMAKE_RC_COMPILER=${DEMECH2_MSVC22_ROOT}/bin/rc.exe"
      "-DCMAKE_LINKER=${CMAKE_LINKER}"
      "-DDEMECH2_MECH2_ORIGINAL=${DEMECH2_MECH2_ORIGINAL}"
      "-DDEMECH2_NETMECHW_ORIGINAL=${DEMECH2_NETMECHW_ORIGINAL}"
    BUILD_COMMAND ${vc22_env} "${CMAKE_COMMAND}" --build <BINARY_DIR>
    INSTALL_COMMAND ""
    BUILD_ALWAYS TRUE
  )
  # The nested build's outputs, for reccmp-build.yml (single-configuration generators only,
  # like the rest of this build)
  reccmp_add_external_target(NETMECHW
    PATH "${PROJECT_BINARY_DIR}/vc22/NETMECHW.dll"
    PDB "${PROJECT_BINARY_DIR}/vc22/NETMECHW.pdb"
  )
  reccmp_add_external_target(MECH2
    PATH "${PROJECT_BINARY_DIR}/vc22/MECH2.exe"
    PDB "${PROJECT_BINARY_DIR}/vc22/MECH2.pdb"
  )
else()
  if (MSVC_FOR_DECOMP)
    message(STATUS "VC++ 2.2: none (DEMECH2_MSVC22_ROOT); building MECH2.EXE and NETMECHW.DLL with this compiler, which can't match the originals")
  endif()
  set(DEMECH2_VC22 FALSE)
  include("${PROJECT_SOURCE_DIR}/vc22/targets.cmake")
  demech2_add_mech2()
  demech2_add_netmechw()
endif()

# --- VC++ 4.0 targets ---
#
# MW2_MATROX.DLL, the Matrox Mystique edition's simulator, was built with VC++ 4.0: the vc40/
# sub-project builds it with 4.0's cl, LINK and libraries (DEMECH2_MSVC40_ROOT, e.g. a clone
# of itsmattkc/MSVC400), configured and built from here as a nested build under <build>/vc40,
# like vc22/. Without VC++ 4.0, the 4.1 build builds the same target itself
# (vc40/targets.cmake), which only differs in stack slots and the CRT. Modern compilers don't
# build it: it is MW2.DLL's code.
set(DEMECH2_MSVC40_ROOT "" CACHE PATH "VC++ 4.0 root directory (bin, include, lib), for MW2_MATROX.DLL")
if (MSVC_FOR_DECOMP AND DEMECH2_MSVC40_ROOT)
  message(STATUS "VC++ 4.0: ${DEMECH2_MSVC40_ROOT} (vc40/, built in ${PROJECT_BINARY_DIR}/vc40)")
  include(ExternalProject)
  file(TO_NATIVE_PATH "${DEMECH2_MSVC40_ROOT}" msvc40)
  set(env_path "${msvc40}\\bin;$ENV{PATH}")
  set(env_include "${msvc40}\\include")
  set(env_lib "${msvc40}\\lib")
  set(vc40_env_script "${PROJECT_BINARY_DIR}/vc40-env.cmake")
  configure_file("${PROJECT_SOURCE_DIR}/cmake/nested-env.cmake.in" "${vc40_env_script}" @ONLY)
  set(vc40_env "${CMAKE_COMMAND}" -P "${vc40_env_script}" --)
  find_program(vc40_make_program NAMES "${CMAKE_MAKE_PROGRAM}" NO_CACHE REQUIRED)
  ExternalProject_Add(vc40
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/vc40"
    BINARY_DIR "${PROJECT_BINARY_DIR}/vc40"
    CONFIGURE_COMMAND ${vc40_env} "${CMAKE_COMMAND}" -S <SOURCE_DIR> -B <BINARY_DIR>
      -G "${CMAKE_GENERATOR}"
      "-DCMAKE_MAKE_PROGRAM=${vc40_make_program}"
      "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
      "-DCMAKE_C_COMPILER=${DEMECH2_MSVC40_ROOT}/bin/cl.exe"
      "-DCMAKE_CXX_COMPILER=${DEMECH2_MSVC40_ROOT}/bin/cl.exe"
      "-DCMAKE_LINKER=${DEMECH2_MSVC40_ROOT}/bin/link.exe"
      "-DCMAKE_ASM_MASM_COMPILER=${CMAKE_ASM_MASM_COMPILER}"
      "-DDEMECH2_DECOMP_ASSERT=${DEMECH2_DECOMP_ASSERT}"
    BUILD_COMMAND ${vc40_env} "${CMAKE_COMMAND}" --build <BINARY_DIR>
    INSTALL_COMMAND ""
    BUILD_ALWAYS TRUE
  )
  reccmp_add_external_target(MW2MATROX
    PATH "${PROJECT_BINARY_DIR}/vc40/MW2_MATROX.dll"
    PDB "${PROJECT_BINARY_DIR}/vc40/MW2_MATROX.pdb"
  )
elseif (MSVC_FOR_DECOMP)
  message(STATUS "VC++ 4.0: none (DEMECH2_MSVC40_ROOT); building MW2_MATROX.DLL with VC++ 4.1, which only differs in stack slots and the CRT")
  include("${PROJECT_SOURCE_DIR}/vc40/targets.cmake")
  demech2_add_mw2matrox()
endif()

# --- Modern compiler compatibility ---
if (MSVC)
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "15")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  endif()
  # Visual Studio 2017 version 15.7 needs "/Zc:__cplusplus" for __cplusplus
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.14.26428")
    add_compile_options(-Zc:__cplusplus)
  endif()
endif()

# --- Tests ---
#
# The portable C that replaces the hand-written assembly, against the assembly (ctest)
enable_testing()
add_subdirectory(tests/asmequiv)

find_program(CLANGFORMAT_BIN NAMES clang-format)
if (EXISTS "${CLANGFORMAT_BIN}")
  execute_process(COMMAND "${CLANGFORMAT_BIN}" --version
    OUTPUT_VARIABLE "CLANGFORMAT_VERSION_OUTPUT"
    RESULT_VARIABLE "CLANGFORMAT_RESULT"
  )
  if (CLANGFORMAT_RESULT EQUAL 0 AND CLANGFORMAT_VERSION_OUTPUT MATCHES "version ([0-9\\.]+)")
    set(CLANGFORMAT_VERSION "${CMAKE_MATCH_1}")
    set(CLANGFORMAT_VERSION_REQUIRED "17.0")
    message(DEBUG "Found clang-format version ${CLANGFORMAT_VERSION} (needs ${CLANGFORMAT_VERSION_REQUIRED}")
    if (CLANGFORMAT_VERSION VERSION_GREATER_EQUAL "${CLANGFORMAT_VERSION_REQUIRED}")
      file(GLOB_RECURSE demech2_sources CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/MECH2/*.c"
        "${PROJECT_SOURCE_DIR}/MECH2/*.cpp"
        "${PROJECT_SOURCE_DIR}/MECH2/*.h"
        "${PROJECT_SOURCE_DIR}/NETMECHW/*.c"
        "${PROJECT_SOURCE_DIR}/NETMECHW/*.cpp"
        "${PROJECT_SOURCE_DIR}/NETMECHW/*.h"
        "${PROJECT_SOURCE_DIR}/MW2SHELL/*.c"
        "${PROJECT_SOURCE_DIR}/MW2SHELL/*.cpp"
        "${PROJECT_SOURCE_DIR}/MW2SHELL/*.h"
        "${PROJECT_SOURCE_DIR}/MW2/*.c"
        "${PROJECT_SOURCE_DIR}/MW2/*.cpp"
        "${PROJECT_SOURCE_DIR}/MW2/*.h"
      )
      if (demech2_sources)
        string(REPLACE ";" "\n" demech2_source_lines "${demech2_sources}")
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/demech2_sources.txt" "${demech2_source_lines}\n")
        add_custom_target(clang-format ${CLANGFORMAT_BIN} -i "--files=${CMAKE_CURRENT_BINARY_DIR}/demech2_sources.txt")
      endif()
    endif()
  endif()
endif()

find_library(CLANG_LIBRARY NAMES "clang" DOC "libclang from host (NOT target) system")
find_package(Python3 COMPONENTS Interpreter QUIET)
if (CLANG_LIBRARY AND TARGET Python3::Interpreter)
  set(NCC_SOURCE_ROOTS "")
  set(NCC_INCLUDES util common/include)
  foreach(root MW2SHELL MW2 NETMECHW MECH2)
    if (EXISTS "${PROJECT_SOURCE_DIR}/${root}")
      list(APPEND NCC_SOURCE_ROOTS ${root})
      if (EXISTS "${PROJECT_SOURCE_DIR}/${root}/include")
        list(APPEND NCC_INCLUDES ${root}/include)
      endif()
      list(APPEND NCC_INCLUDES ${root})
    endif()
  endforeach()
  list(APPEND NCC_SOURCE_ROOTS common)
  if (NCC_SOURCE_ROOTS)
    add_custom_target(ncc
      Python3::Interpreter "${PROJECT_SOURCE_DIR}/tools/ncc/ncc.py"
        --clang-lib "${CLANG_LIBRARY}"
        --recurse
        --style tools/ncc/ncc.style
        --skip tools/ncc/skip.yml
        --definition DEFINE_GUID PASCAL WINAPI FAR BOOL=int CALLBACK HWND__=HWND
        --include
          ${NCC_INCLUDES}
        --path ${NCC_SOURCE_ROOTS}
      COMMENT "Verifying naming conventions (ncc)"
      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    )
  endif()
endif()

reccmp_configure()

set(RELEASE_URL "https://github.com/anpage/demech2/releases/download/continuous")
add_custom_target(download-main-progress)
add_custom_target(show-progress)

get_property(RECCMP_TARGETS GLOBAL PROPERTY RECCMP_TARGETS)
foreach(target ${RECCMP_TARGETS})
  get_property(reccmp_id TARGET "${target}" PROPERTY INTERFACE_RECCMP_ID)

  set(progress_local "${PROJECT_BINARY_DIR}/${reccmp_id}-progress-main.json")
  set(download_command COMMAND "${CMAKE_COMMAND}"
    "-DURL=${RELEASE_URL}"
    "-DFILENAME_ONLINE=${reccmp_id}PROGRESS.json"
    "-DFILENAME_LOCAL=${progress_local}"
    "-P" "${PROJECT_SOURCE_DIR}/cmake/download-file.cmake"
  )
  add_custom_target(download-main-progress-${reccmp_id} ${download_command})
  add_dependencies(download-main-progress download-main-progress-${reccmp_id})
  add_custom_command(OUTPUT "${progress_local}" COMMAND ${download_command})
  add_custom_target(show-progress-${reccmp_id}
    COMMENT "Compare progress of ${reccmp_id} with current git main"
    COMMAND reccmp-reccmp --target ${reccmp_id} --diff "${progress_local}"
    DEPENDS "${progress_local}"
  )
  add_dependencies(show-progress show-progress-${reccmp_id})
endforeach()
