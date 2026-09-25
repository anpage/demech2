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
  foreach(lang C CXX)
    set(CMAKE_${lang}_FLAGS "/DWIN32 /D_WINDOWS /W3 /Od /Oi")
    set(CMAKE_${lang}_FLAGS_DEBUG "/Zi")
    set(CMAKE_${lang}_FLAGS_RELEASE "/DNDEBUG")
    set(CMAKE_${lang}_FLAGS_RELWITHDEBINFO "/Zi /DNDEBUG")
    set(CMAKE_${lang}_FLAGS_MINSIZEREL "/DNDEBUG")
    set(CMAKE_${lang}_STANDARD_LIBRARIES "")
  endforeach()
  foreach(kind SHARED EXE)
    set(CMAKE_${kind}_LINKER_FLAGS "/machine:I386")
    foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
      set(CMAKE_${kind}_LINKER_FLAGS_${config} "")
    endforeach()
  endforeach()
endif()

if (DEMECH2_DECOMP_ASSERT)
  message(STATUS "Decomp asserts enabled")
endif()

# --- Third-party import libraries ---
#
# Import libraries for the DLLs the game links against but we don't build. Each is
# generated from a .def file. LIB 3.10 can't express a .def export whose import name
# differs from its symbol (DirectDrawCreate vs. _DirectDrawCreate@12), so .def names
# are resolved against a stub object that carries the __stdcall decorations. The stub
# object is never linked into a game DLL.
function(demech2_add_import_library TARGET)
  cmake_parse_arguments(ARGS "" "DEF" "SOURCES" ${ARGN})
  set(lib "${CMAKE_CURRENT_BINARY_DIR}/3rdparty/${TARGET}.lib")
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/3rdparty")
  set(objects "")
  if (ARGS_SOURCES)
    add_library(${TARGET}-symbols OBJECT ${ARGS_SOURCES})
    set(objects "$<TARGET_OBJECTS:${TARGET}-symbols>")
  endif()
  add_custom_command(OUTPUT "${lib}"
    COMMAND "${CMAKE_LINKER}" /lib /nologo /machine:ix86 "/def:${ARGS_DEF}" "/out:${lib}" ${objects}
    DEPENDS "${ARGS_DEF}" ${objects}
    COMMAND_EXPAND_LISTS
    VERBATIM
  )
  add_custom_target(${TARGET}-implib DEPENDS "${lib}")
  # An IMPORTED library keeps its place in a link line (an INTERFACE library's items
  # would be moved after the plain .lib names).
  add_library(${TARGET} STATIC IMPORTED GLOBAL)
  set_target_properties(${TARGET} PROPERTIES IMPORTED_LOCATION "${lib}")
  add_dependencies(${TARGET} ${TARGET}-implib)
endfunction()

demech2_add_import_library(ddraw DEF "${PROJECT_SOURCE_DIR}/3rdparty/ddraw/ddraw.def"
  SOURCES 3rdparty/ddraw/ddraw.c)
demech2_add_import_library(dplay DEF "${PROJECT_SOURCE_DIR}/3rdparty/dplay/dplay.def"
  SOURCES 3rdparty/dplay/dplay.c)
demech2_add_import_library(wail32 DEF "${PROJECT_SOURCE_DIR}/3rdparty/miles/wail32.def"
  SOURCES 3rdparty/miles/wail32.c)
demech2_add_import_library(smackw32 DEF "${PROJECT_SOURCE_DIR}/3rdparty/smacker/smackw32.def")

# --- Game DLLs ---
#
# Adds a game DLL and registers it with reccmp. SOURCES must follow the original link
# order: object order determines function addresses. LIBRARIES are the import
# libraries, in the order of the original's .idata descriptors (library processing
# order decides it). RUNTIME is the MSVC_RUNTIME_LIBRARY value, COMPILE_OPTIONS and
# LINK_OPTIONS the target's own codegen and link flags.
function(demech2_add_dll TARGET)
  cmake_parse_arguments(ARGS "" "ID;OUTPUT_NAME;RUNTIME"
    "SOURCES;LIBRARIES;COMPILE_OPTIONS;LINK_OPTIONS" ${ARGN})
  add_library(${TARGET} SHARED ${ARGS_SOURCES} util/decomp.cpp)
  set_target_properties(${TARGET} PROPERTIES
    OUTPUT_NAME "${ARGS_OUTPUT_NAME}"
    MSVC_RUNTIME_LIBRARY "${ARGS_RUNTIME}"
  )
  target_include_directories(${TARGET} PRIVATE
    "${PROJECT_SOURCE_DIR}/util"
    "${PROJECT_SOURCE_DIR}/${ARGS_ID}/include"
  )
  target_include_directories(${TARGET} SYSTEM PRIVATE
    "${PROJECT_SOURCE_DIR}/3rdparty/dx2/INC"
    "${PROJECT_SOURCE_DIR}/3rdparty/miles"
  )
  target_compile_options(${TARGET} PRIVATE ${ARGS_COMPILE_OPTIONS})
  if (DEMECH2_DECOMP_ASSERT)
    target_compile_definitions(${TARGET} PRIVATE ENABLE_DECOMP_ASSERTS)
  endif()
  if (MSVC)
    target_link_options(${TARGET} PRIVATE
      "/MAP:${CMAKE_CURRENT_BINARY_DIR}/${ARGS_OUTPUT_NAME}.map"
      ${ARGS_LINK_OPTIONS}
    )
  endif()
  target_link_libraries(${TARGET} PRIVATE ${ARGS_LIBRARIES})
  reccmp_add_target(${TARGET} ID ${ARGS_ID})
endfunction()

# MW2SHELL.DLL: C++ at /Od /Oi /GX /Ob1, C at /Od /Oi, the release CRT (4.1's LIBCMT).
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
    MW2SHELL/src/brasslantern0x414.cpp
    MW2SHELL/src/videodriver.cpp
    MW2SHELL/src/mechbay.cpp
    MW2SHELL/src/shellmain.cpp
    MW2SHELL/src/clanhall.cpp
    MW2SHELL/src/unk10014b60.cpp
    MW2SHELL/src/debugprint.c
    MW2SHELL/src/video.cpp
    MW2SHELL/src/debugout.c
    MW2SHELL/src/pilotroster.cpp
    MW2SHELL/src/cedarknot0x10.cpp
    MW2SHELL/src/cdcheck.cpp
    MW2SHELL/src/missionui.cpp
    MW2SHELL/src/simhandoff.cpp
    MW2SHELL/src/unk10039de0.cpp
    MW2SHELL/src/mousestate.cpp
    MW2SHELL/src/unk1003c7e0.cpp
    MW2SHELL/src/audio.cpp
    MW2SHELL/src/mainmenu.cpp
    MW2SHELL/src/unk1003e2f0.cpp
    MW2SHELL/src/cockpitcontrols.cpp
    MW2SHELL/src/options.cpp
    MW2SHELL/src/hollowreed0x110.cpp
    MW2SHELL/src/unk10046200.cpp
    MW2SHELL/src/unk10046e80.cpp
    MW2SHELL/src/unk10047370.cpp
    MW2SHELL/src/tmpackdatabase.cpp
    MW2SHELL/MW2SHELL.def
    MW2SHELL/mw2shell.rc
  COMPILE_OPTIONS "$<$<COMPILE_LANGUAGE:CXX>:/GX>" "$<$<COMPILE_LANGUAGE:CXX>:/Ob1>"
  LINK_OPTIONS ${mw2shell_link_options}
  LIBRARIES winmm.lib ddraw wail32 smackw32 kernel32.lib user32.lib gdi32.lib advapi32.lib
)

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
  add_executable(extract_icon tools/extract_icon/extract_icon.c)
  set_target_properties(extract_icon PROPERTIES MSVC_RUNTIME_LIBRARY MultiThreaded)
  target_link_libraries(extract_icon PRIVATE kernel32.lib)
  set(icon_dir "${CMAKE_CURRENT_BINARY_DIR}/MW2SHELL")
  add_custom_command(OUTPUT "${icon_dir}/mw2shell.ico"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${icon_dir}"
    COMMAND extract_icon "${DEMECH2_MW2SHELL_ORIGINAL}" 103 "${icon_dir}/mw2shell.ico"
    DEPENDS extract_icon "${DEMECH2_MW2SHELL_ORIGINAL}"
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

# MW2.DLL: C at /Od /Oi, the debug CRT (4.1's LIBCMTD; /MTd predefines _DEBUG). The
# original was linked /DEBUG /INCREMENTAL:no in every configuration.
set(mw2_link_options "")
if (MSVC_FOR_DECOMP)
  set(mw2_link_options /DEBUG /INCREMENTAL:no)
endif()
demech2_add_dll(mw2
  ID MW2
  OUTPUT_NAME MW2
  RUNTIME MultiThreadedDebug
  SOURCES
    MW2/src/palette.c
    MW2/src/fixedmul.c
    MW2/src/muldiv.c
    MW2/src/gpanim.c
    MW2/src/supanim.c
    MW2/src/debris.c
    MW2/src/audio.c
    MW2/src/missionaudio.c
    MW2/src/pausebanner.c
    MW2/src/world.c
    MW2/src/network.c
    MW2/src/eyepoint.c
    MW2/src/render.c
    MW2/src/loadres.c
    MW2/src/objective.c
    MW2/src/cmdline.c
    MW2/src/geocache.c
    MW2/src/logwindow.c
    MW2/src/error.c
    MW2/src/menu.c
    MW2/src/keyboard.c
    MW2/src/staticmem.c
    MW2/src/fadepal.c
    MW2/src/resource.c
    MW2/src/ai.c
    MW2/src/gamma.c
    MW2/src/overlay.c
    MW2/src/speech.c
    MW2/src/gamekeys.c
    MW2/src/setres.c
    MW2/src/rendertarget.c
    MW2/src/simmain.c
    MW2/src/animation.c
    MW2/src/shots.c
    MW2/src/players.c
    MW2/src/timedoverlays.c
    MW2/src/config.c
    MW2/src/startup.c
    MW2/src/mw2log.c
    MW2/src/random.c
    MW2/src/perf.c
    MW2/src/drawmode.c
    MW2/src/ddfill.c
    MW2/src/input.c
    MW2/src/clock.c
    MW2/src/callbacks.c
    MW2/src/environment.c
    MW2/MW2.def
  LINK_OPTIONS ${mw2_link_options}
  LIBRARIES winmm.lib wail32 ddraw dplay kernel32.lib user32.lib gdi32.lib advapi32.lib
)

# MECH2.EXE (source only, not a reccmp target) is added with its source.

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
  set(NCC_INCLUDES util)
  foreach(root MECH2 MW2SHELL MW2)
    if (EXISTS "${PROJECT_SOURCE_DIR}/${root}")
      list(APPEND NCC_SOURCE_ROOTS ${root})
      if (EXISTS "${PROJECT_SOURCE_DIR}/${root}/include")
        list(APPEND NCC_INCLUDES ${root}/include)
      endif()
      list(APPEND NCC_INCLUDES ${root})
    endif()
  endforeach()
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
