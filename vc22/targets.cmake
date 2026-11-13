# The targets the originals built with VC++ 2.2: MECH2.EXE, the launcher, NETMECHW.DLL,
# NetMech's lobby, and MW2_3DFX.DLL, the 3Dfx edition's simulator.
#
# Included by vc22/CMakeLists.txt, the VC++ 2.2 sub-project, and by the top-level project
# when it has no VC++ 2.2 to hand them to: a VC++ 4.1 build without DEMECH2_MSVC22_ROOT, or a
# modern compiler (the NCC/clang-tidy build). Expects DEMECH2_SOURCE_DIR (the repository
# root), DEMECH2_VC22 (TRUE in the VC++ 2.2 build) and the dplay import library target.

include("${DEMECH2_SOURCE_DIR}/cmake/extract_resource.cmake")

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
    demech2_add_extract_resource()
    set(icon_dir "${CMAKE_CURRENT_BINARY_DIR}/MECH2")
    set(icon_files "")
    foreach(icon 103 108)
      add_custom_command(OUTPUT "${icon_dir}/mech2_${icon}.ico"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${icon_dir}"
        COMMAND extract_resource icon "${DEMECH2_MECH2_ORIGINAL}" ${icon} "${icon_dir}/mech2_${icon}.ico"
        DEPENDS extract_resource "${DEMECH2_MECH2_ORIGINAL}"
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
    "${root}/NETMECHW/src/unk10001070.cpp"
    "${root}/NETMECHW/src/sessionlist.cpp"
    "${root}/NETMECHW/src/unk10002140.cpp"
    "${root}/NETMECHW/src/unk100025f0.cpp"
    "${root}/NETMECHW/src/unk10003660.cpp"
    "${root}/NETMECHW/src/unk10006060.cpp"
    "${root}/NETMECHW/src/unk10006b20.cpp"
    "${root}/NETMECHW/src/unk10006ce0.cpp"
    "${root}/NETMECHW/src/unk10007c30.c"
    "${root}/NETMECHW/src/unk100089d0.cpp"
    "${root}/NETMECHW/src/unk10009230.cpp"
    "${root}/NETMECHW/src/unk1000a650.c"
    "${root}/NETMECHW/src/unk1000aa90.cpp"
    "${root}/NETMECHW/src/unk1000b400.cpp"
    "${root}/NETMECHW/src/chatlog.cpp"
    "${root}/NETMECHW/src/unk1000d250.cpp"
    "${root}/NETMECHW/src/unk1000e410.cpp"
    "${root}/NETMECHW/src/unk1000f0f0.cpp"
    "${root}/NETMECHW/src/bwdwriter.c"
    "${root}/NETMECHW/src/unk1000f8e0.cpp"
    "${root}/NETMECHW/src/unk10010460.cpp"
    "${root}/NETMECHW/src/unk10010e50.cpp"
    "${root}/NETMECHW/src/unk10011030.cpp"
    "${root}/NETMECHW/src/unk10011120.cpp"
    "${root}/NETMECHW/src/mw2prj.c"
    "${root}/NETMECHW/src/prjfile.c"
    "${root}/NETMECHW/src/resourcecache.c"
    "${root}/NETMECHW/src/resourcename.c"
    "${root}/NETMECHW/src/weapondata.c"
    "${root}/NETMECHW/src/resourcefile.c"
    "${root}/NETMECHW/src/bwdkeywords.c"
    "${root}/NETMECHW/src/debugout.c"
    "${root}/NETMECHW/NETMECHW.def"
    "${root}/NETMECHW/netmechw.rc"
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
  target_include_directories(netmechw SYSTEM PRIVATE "${root}/3rdparty/dplay")
  if (DEMECH2_VC22 OR DEMECH2_DECOMP_ASSERT)
    target_compile_definitions(netmechw PRIVATE ENABLE_DECOMP_ASSERTS)
  endif()
  if (MSVC)
    # The map gives tools/check_units.py the object of each symbol.
    target_link_options(netmechw PRIVATE "/MAP:${CMAKE_CURRENT_BINARY_DIR}/NETMECHW.map")
  endif()
  if (DEMECH2_VC22)
    target_link_options(netmechw PRIVATE /DEBUG /INCREMENTAL:no)
  endif()
  if (DEMECH2_VC22 OR MSVC_FOR_DECOMP)
    # The objects, in link order, for cmake/link_objects.cmake: it copies them to
    # o/<name>/ under their base names and writes <name>.objects.rsp, which
    # CMAKE_<LANG>_CREATE_SHARED_LIBRARY reads instead of <OBJECTS> in the VC++ 2.2
    # sub-project and in a VC++ 4.1 build (vc22/CMakeLists.txt, CMakeLists.txt).
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

  # The lobby's icons (groups 104 and 105) and bitmaps stay out of the repository, like
  # the shell's and the launcher's icons: they are extracted from the original NETMECHW.DLL
  # at build time. Without the original, the lobby has no icons or pictures.
  set(DEMECH2_NETMECHW_ORIGINAL "${root}/NETMECHW.DLL" CACHE FILEPATH
    "Original NETMECHW.DLL (retail release) to take the lobby's icons and bitmaps from")
  set(netmechw_original_sha256 3f4d1508238d847213127e46623a85b249985686d6053a3d9e1c6deea87c0095)
  set(netmechw_images FALSE)
  if (EXISTS "${DEMECH2_NETMECHW_ORIGINAL}")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${DEMECH2_NETMECHW_ORIGINAL}")
    file(SHA256 "${DEMECH2_NETMECHW_ORIGINAL}" sha256)
    if (sha256 STREQUAL netmechw_original_sha256)
      set(netmechw_images TRUE)
    else()
      message(WARNING "${DEMECH2_NETMECHW_ORIGINAL} is not the retail NETMECHW.DLL; building the lobby without its icons and bitmaps")
    endif()
  endif()
  if (netmechw_images)
    message(STATUS "Lobby images: from ${DEMECH2_NETMECHW_ORIGINAL}")
    demech2_add_extract_resource()
    set(image_dir "${CMAKE_CURRENT_BINARY_DIR}/NETMECHW")
    set(image_files "")
    foreach(icon 104 105)
      add_custom_command(OUTPUT "${image_dir}/netmechw_${icon}.ico"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${image_dir}"
        COMMAND extract_resource icon "${DEMECH2_NETMECHW_ORIGINAL}" ${icon} "${image_dir}/netmechw_${icon}.ico"
        DEPENDS extract_resource "${DEMECH2_NETMECHW_ORIGINAL}"
        VERBATIM
      )
      list(APPEND image_files "${image_dir}/netmechw_${icon}.ico")
    endforeach()
    foreach(bitmap 105 112 113 114 213 214 215 216 217 218 219 220 221 222 899 906 910 911)
      add_custom_command(OUTPUT "${image_dir}/netmechw_${bitmap}.bmp"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${image_dir}"
        COMMAND extract_resource bitmap "${DEMECH2_NETMECHW_ORIGINAL}" ${bitmap} "${image_dir}/netmechw_${bitmap}.bmp"
        DEPENDS extract_resource "${DEMECH2_NETMECHW_ORIGINAL}"
        VERBATIM
      )
      list(APPEND image_files "${image_dir}/netmechw_${bitmap}.bmp")
    endforeach()
    set_source_files_properties("${root}/NETMECHW/netmechw.rc" PROPERTIES
      COMPILE_DEFINITIONS DEMECH2_ORIGINAL_IMAGES
      INCLUDE_DIRECTORIES "${image_dir}"
      OBJECT_DEPENDS "${image_files}"
    )
  else()
    message(STATUS "Lobby images: none (no original NETMECHW.DLL at ${DEMECH2_NETMECHW_ORIGINAL})")
  endif()
endfunction()

# MW2_3DFX.DLL (linked 1996-10-21 by LINK 2.55): the simulator of the 3Dfx edition (the Diamond
# Monster disc's MW2.DLL), drawing through Glide (GLIDE.DLL). It is the C the accelerated editions
# share with the Matrox edition's (the float conversion, the 16-bit VFXA port), built with VC++ 2.2
# and optimized: /Ox /Oa /G5 /Gr,
# which reproduce the original exactly, with __fastcall the default calling convention (/Gr).
# network.c, shots.c and mechreload.c are /Od (still /Gr). Every function is a COMDAT of its own
# (/Gy): the linker pads between them with int3, inside an object as between objects. Its CRT is 2.2's LIBCMT, and it links
# 3dfx's Texus (not built yet). MW2_ACCEL and MW2_3DFX are defined: MW2_ACCEL selects the code
# the accelerated editions share (MW2/src/accel), MW2_3DFX this edition's own.
#
# The sources are MW2's and the accelerated editions' (MW2/src/accel), in this edition's link
# order as far as its functions place them. Its own back end (the Glide renderer, at 0x10066570)
# isn't written yet: the Matrox edition's A3D and MSI back ends (MW2/src/matrox) are linked in its
# place. Only the VC++ 2.2 sub-project builds this target: it needs ML, and its sources'
# optimization flags would apply to MW2's targets in the top-level project.
function(demech2_add_mw23dfx)
  # 3dfx's Texus, which the renderer (3dfx/a3d.c) converts its textures with.
  include("${DEMECH2_SOURCE_DIR}/3rdparty/texus/texus.cmake")
  demech2_add_texus(texus)
  set(od
    "${DEMECH2_SOURCE_DIR}/MW2/src/network.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/shots.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/mechreload.c"
  )
  set(sources
    "${DEMECH2_SOURCE_DIR}/MW2/src/mapview.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/recttransition.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/dispdib.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/anim2d.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/setres.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/refreshmode.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/timedoverlays.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/cockpit.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/render.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/gdi.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/3dfx/glidedraw.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/screenscale.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/brightness.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/supanim.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/environment.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/gamecd.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/logwindow.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/clock.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/callbacks.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/debugprint.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/missionaudio.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/simmain.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/objective.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/startup.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/ray.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/gpanim.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/mechcollision.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/quadtree.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/shapecollision.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/boundbox.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/shapelists.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/objectanim.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/object.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/collision.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/gridobject.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/mechviewpanel.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/ai.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/lineofsight.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/debris.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/mechdamage.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/statuspanels.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/players.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/team.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/mechclass.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/damagepanel.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fadepal.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/bargauges.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/targeting.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/weaponpanel.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/maneuvers.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/targetpanel.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/artillery.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/hud.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/aiweapons.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/cockpitpanel.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/mechreload.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/staticmem.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/loadres.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/geocache.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/recordstacks.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/resourcename.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/classtable.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/polydraw.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/horizon.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/shape.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/depthsort.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/faceshade.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/bandpoly.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/palette.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/accel/queueface.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/animation.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/camerashake.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/eyepoint.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/transform.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/view.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/accel/floatmath.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/soundlimits.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/cdaudio.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/speech.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/audio.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/soundfx.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/midi.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/sndunpack.${asm_or_c}"
    "${DEMECH2_SOURCE_DIR}/MW2/src/debugbreak.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/network.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/mouse.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/pausebanner.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/gamekeys.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/keyboard.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/cmdline.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/inputmap.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/netio.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/joystick.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/inifile.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/screenshot.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/ramp.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/gifsave.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/overlay.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/error.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/mw2prj.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/prjfile.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/random.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/mw2log.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/ticks.${asm_or_c}"
    "${DEMECH2_SOURCE_DIR}/MW2/src/accel/registry.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/world.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/bwdnames.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/missionsetup.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/config.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/bwd.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/poolsizes.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/mekfile.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/wtbshapes.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/resource.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/weapons.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/shots.c"
    ${vfx3d}
    ${vfxrend}
    ${vfxa}
    "${DEMECH2_SOURCE_DIR}/MW2/src/palfade.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/accel/vfx16.asm"
    "${DEMECH2_SOURCE_DIR}/MW2/src/dorcs.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/lancemenu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/menucontrols.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/settings.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/perf.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/menu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/accel/graphicsmenu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/3dfx/a3d.c"
    # Not placed yet: units with nothing found in the 3Dfx edition (fixed-point helpers it
    # replaced), linked after the others in the Matrox edition's order. Its display back end and
    # renderer (3dfx/glidedraw.c, 3dfx/a3d.c) replace directdraw.c and the Matrox edition's.
    "${DEMECH2_SOURCE_DIR}/MW2/src/namehash.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/mainmenu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/brightnessmenu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixedmul.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/bwdkeywords.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixeddiv.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/palidentity.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/door.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/muldiv.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/weapondata.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/integrate.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/inradius.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/emptyfns.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/commandpointmenu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/muldiv14.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixedsqrt.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/sqrtguess.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixeddiv29.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixedmul29.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/audiomenu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/scaledelta.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/crossdiv.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/lerp.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixeddot27.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixeddot29.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/shiftdiv.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixeddivu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixedmul30.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/commandmenu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/mulratio.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/mulnorm16.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/muladddiv.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/texpoly.c"
  )
  foreach(source IN LISTS sources)
    if (source MATCHES "\\.c$" AND NOT source IN_LIST od)
      set_source_files_properties("${source}" PROPERTIES DEMECH2_OPTIMIZATION "/Ox;/Oa")
    endif()
  endforeach()
  demech2_add_dll(mw23dfx
    ID MW23DFX
    SOURCE_DIR MW2
    OUTPUT_NAME MW2_3DFX
    LINK_DIR 3d
    # The single-threaded CRT (2.2's LIBC, /ML): the original imports no TLS or critical
    # section functions, and its fclose takes no lock. CMake has no MSVC_RUNTIME_LIBRARY value
    # for /ML: an empty one adds no flag.
    RUNTIME ""
    SOURCES
      ${sources}
      "${DEMECH2_SOURCE_DIR}/MW2/MW2.def"
    COMPILE_OPTIONS "$<$<COMPILE_LANGUAGE:C,CXX>:/ML;/G5;/Gr;/Gy;/DMW2_ACCEL;/DMW2_3DFX>"
      "$<$<COMPILE_LANGUAGE:ASM_MASM>:/DMW2_ACCEL>"
    LINK_OPTIONS /DEBUG /INCREMENTAL:no
    LIBRARIES texus kernel32.lib user32.lib gdi32.lib advapi32.lib winmm.lib dplay glide wail32
  )
  target_include_directories(mw23dfx SYSTEM PRIVATE
    "${DEMECH2_SOURCE_DIR}/3rdparty/glide"
    "${DEMECH2_SOURCE_DIR}/3rdparty/texus"
  )
endfunction()
