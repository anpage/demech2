# The targets the originals built with VC++ 2.2: MECH2.EXE, the launcher, and NETMECHW.DLL,
# NetMech's lobby.
#
# Included by vc22/CMakeLists.txt, the VC++ 2.2 sub-project, and by the top-level project
# when it has no VC++ 2.2 to hand them to: a VC++ 4.1 build without DEMECH2_MSVC22_ROOT, or a
# modern compiler (the NCC/clang-tidy build). Expects DEMECH2_SOURCE_DIR (the repository
# root), DEMECH2_VC22 (TRUE in the VC++ 2.2 build) and the dplay import library target.

include("${DEMECH2_SOURCE_DIR}/cmake/extract_icon.cmake")

# MECH2.EXE: C at /Od /Oi /G5, the single-threaded CRT (2.2's LIBC, /ML). The original is an
# incremental EXE link without /DEBUG; the comparison build keeps the incremental link and
# adds /DEBUG for the PDB. Built by another compiler, it is only the launcher, with the
# static multithreaded CRT (/ML is gone from modern MSVC).
function(demech2_add_mech2)
  set(root "${DEMECH2_SOURCE_DIR}")
  add_executable(mech2 WIN32
    "${root}/MECH2/src/winmain.c"
    "${root}/MECH2/src/cdcheck.c"
    "${root}/MECH2/src/debug.c"
    "${root}/MECH2/mech2.rc"
  )
  set_target_properties(mech2 PROPERTIES OUTPUT_NAME MECH2)
  target_include_directories(mech2 PRIVATE
    "${root}/util"
    "${root}/MECH2/include"
  )
  if (DEMECH2_VC22)
    # CMake has no MSVC_RUNTIME_LIBRARY value for /ML: an empty one adds no flag.
    set_target_properties(mech2 PROPERTIES MSVC_RUNTIME_LIBRARY "")
    target_compile_options(mech2 PRIVATE /ML /G5)
    # No /MAP: LINK turns the incremental link off for it.
    target_link_options(mech2 PRIVATE /DEBUG /INCREMENTAL:yes)
  else()
    set_target_properties(mech2 PROPERTIES MSVC_RUNTIME_LIBRARY MultiThreaded)
  endif()
  target_link_libraries(mech2 PRIVATE kernel32.lib user32.lib gdi32.lib)

  # The launcher's window icons (103, and 108 for NetMech) stay out of the repository, like
  # the shell's: they are extracted from the original MECH2.EXE at build time. Without the
  # original, the window has no icon.
  set(DEMECH2_MECH2_ORIGINAL "${root}/MECH2.EXE" CACHE FILEPATH
    "Original MECH2.EXE (retail release) to take the launcher's icons from")
  set(mech2_original_sha256 064a9f1f45cfd18f0bef1dea9711b7fd582755cfbb8a8e9694f8cef2e5c3690c)
  set(mech2_icons FALSE)
  if (EXISTS "${DEMECH2_MECH2_ORIGINAL}")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${DEMECH2_MECH2_ORIGINAL}")
    file(SHA256 "${DEMECH2_MECH2_ORIGINAL}" sha256)
    if (sha256 STREQUAL mech2_original_sha256)
      set(mech2_icons TRUE)
    else()
      message(WARNING "${DEMECH2_MECH2_ORIGINAL} is not the retail MECH2.EXE; building the launcher without its icons")
    endif()
  endif()
  if (mech2_icons)
    message(STATUS "Launcher icons: from ${DEMECH2_MECH2_ORIGINAL}")
    demech2_add_extract_icon()
    set(icon_dir "${CMAKE_CURRENT_BINARY_DIR}/MECH2")
    set(icon_files "")
    foreach(icon 103 108)
      add_custom_command(OUTPUT "${icon_dir}/mech2_${icon}.ico"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${icon_dir}"
        COMMAND extract_icon "${DEMECH2_MECH2_ORIGINAL}" ${icon} "${icon_dir}/mech2_${icon}.ico"
        DEPENDS extract_icon "${DEMECH2_MECH2_ORIGINAL}"
        VERBATIM
      )
      list(APPEND icon_files "${icon_dir}/mech2_${icon}.ico")
    endforeach()
    set_source_files_properties("${root}/MECH2/mech2.rc" PROPERTIES
      COMPILE_DEFINITIONS DEMECH2_ORIGINAL_ICONS
      INCLUDE_DIRECTORIES "${icon_dir}"
      OBJECT_DEPENDS "${icon_files}"
    )
  else()
    message(STATUS "Launcher icons: none (no original MECH2.EXE at ${DEMECH2_MECH2_ORIGINAL})")
  endif()
endfunction()

# NETMECHW.DLL: C and C++ at /Od /Oi (no /G5, no /GX), the multithreaded CRT (2.2's LIBCMT,
# /MT). The original is a plain /DLL link; the comparison build adds /DEBUG for the PDB and
# /INCREMENTAL:no for the direct exports. SOURCES follow the original link order: object
# order determines function addresses. LIBRARIES are in the order of the original's .idata
# descriptors.
function(demech2_add_netmechw)
  set(root "${DEMECH2_SOURCE_DIR}")
  add_library(netmechw SHARED
    "${root}/NETMECHW/src/dllmain.cpp"
    "${root}/NETMECHW/src/unk10003660.cpp"
    "${root}/NETMECHW/NETMECHW.def"
    "${root}/util/decomp.cpp"
  )
  set_target_properties(netmechw PROPERTIES
    OUTPUT_NAME NETMECHW
    MSVC_RUNTIME_LIBRARY MultiThreaded
  )
  target_include_directories(netmechw PRIVATE
    "${root}/util"
    "${root}/NETMECHW/include"
  )
  target_include_directories(netmechw SYSTEM PRIVATE "${root}/3rdparty/dx2/INC")
  if (DEMECH2_VC22 OR DEMECH2_DECOMP_ASSERT)
    target_compile_definitions(netmechw PRIVATE ENABLE_DECOMP_ASSERTS)
  endif()
  if (MSVC)
    # The map gives tools/check_units.py the object of each symbol.
    target_link_options(netmechw PRIVATE "/MAP:${CMAKE_CURRENT_BINARY_DIR}/NETMECHW.map")
  endif()
  if (DEMECH2_VC22)
    target_link_options(netmechw PRIVATE /DEBUG /INCREMENTAL:no)
    # The objects, in link order, for cmake/link_objects.cmake: it copies them to
    # o/<name>/ under their base names and writes <name>.objects.rsp, which
    # CMAKE_<LANG>_CREATE_SHARED_LIBRARY reads instead of <OBJECTS> (vc22/CMakeLists.txt).
    set(dir "$<TARGET_FILE_DIR:netmechw>")
    set(name "$<TARGET_FILE_BASE_NAME:netmechw>")
    file(GENERATE OUTPUT "${dir}/${name}.objects.list"
      CONTENT "$<JOIN:$<TARGET_OBJECTS:netmechw>,\n>\n")
    add_custom_command(TARGET netmechw PRE_LINK
      COMMAND "${CMAKE_COMMAND}"
        "-DLIST=${dir}/${name}.objects.list" "-DDEST=${dir}/o/${name}"
        "-DRSP=${dir}/${name}.objects.rsp" "-DPREFIX=o\\${name}"
        -P "${root}/cmake/link_objects.cmake"
      COMMENT "Copying NETMECHW objects to o/${name}"
      VERBATIM
    )
  endif()
  target_link_libraries(netmechw PRIVATE dplay kernel32.lib user32.lib gdi32.lib comctl32.lib advapi32.lib)
endfunction()
