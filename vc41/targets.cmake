# The VC++ 4.1 target with optimized units: MW2_S3.DLL, the simulator of the S3 ViRGE edition.
#
# Included by vc41/CMakeLists.txt, a directory of the top-level project (VC++ 4.1). Expects
# DEMECH2_SOURCE_DIR, asm_or_c and ${vfxa} (cmake/masm.cmake), demech2_add_dll
# (cmake/game_dll.cmake) and the ddraw, dplay, wail32 and s3dtk import library targets.

# MW2_S3.DLL (linked 1996-07-12 by LINK 3.10): the simulator of the S3 ViRGE edition, drawing
# through S3's toolkit (S3DTKW.DLL) on DirectDraw surfaces, three months after the ATI edition. Its
# C is the accelerated editions' (the float conversion, the 16-bit VFXA port, the A3D interface),
# built the way the ATI and 3Dfx editions were but with VC++ 4.1: /Ox /Ow /G5 /Gr /Gy, with
# network.c, shots.c and mechreload.c at /Od, and 4.1's single-threaded LIBC (/ML; 4.1 leaves the
# Pentium division guard off, where 2.2 has it on). It links VFXA and the 16-bit port but neither
# VFX3D nor VFXREND. MW2_ACCEL and MW2_S3 are defined. The units' order is the ATI edition's
# until they are placed; s3/a3d.c is its renderer, over the S3 toolkit.
function(demech2_add_mw2s3)
  set(od
    "${DEMECH2_SOURCE_DIR}/MW2/src/network.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/shots.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/mechreload.c"
  )
  set(sources
    "${DEMECH2_SOURCE_DIR}/MW2/src/mapview.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/palidentity.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/recttransition.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/dispdib.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/anim2d.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/setres.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/refreshmode.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/timedoverlays.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/cockpit.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/render.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/gdi.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/cockpitlayouts.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/directdraw.c"
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
    "${DEMECH2_SOURCE_DIR}/MW2/src/door.c"
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
    "${DEMECH2_SOURCE_DIR}/MW2/src/bwdkeywords.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/wtbshapes.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/resource.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/weapons.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/shots.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/weapondata.c"
    ${vfxa}
    "${DEMECH2_SOURCE_DIR}/MW2/src/palfade.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/accel/vfx16.asm"
    "${DEMECH2_SOURCE_DIR}/MW2/src/brightnessmenu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/commandpointmenu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/commandmenu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/commandpoint2menu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/dorcs.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/lancemenu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/mainmenu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/menucontrols.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/settings.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/perf.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/audiomenu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/menu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/accel/graphicsmenu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/s3/a3d.c"
    # Units not placed yet, linked after the others in the ATI edition's order.
    "${DEMECH2_SOURCE_DIR}/MW2/src/namehash.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixedmul.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixeddiv.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/muldiv.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/integrate.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/inradius.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/emptyfns.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/muldiv14.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixedsqrt.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/sqrtguess.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixeddiv29.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixedmul29.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/scaledelta.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/crossdiv.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/lerp.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixeddot27.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixeddot29.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/shiftdiv.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixeddivu.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/fixedmul30.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/mulratio.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/mulnorm16.c"
    "${DEMECH2_SOURCE_DIR}/MW2/src/muladddiv.c"
  )
  foreach(source IN LISTS sources)
    if (source MATCHES "\\.c$" AND NOT source IN_LIST od)
      set_source_files_properties("${source}" PROPERTIES DEMECH2_OPTIMIZATION "/Ox;/Ow")
    endif()
  endforeach()
  # cmake/masm.cmake's VFX properties, which CMake keeps in the top-level directory
  set_source_files_properties(${vfxa} PROPERTIES
    LANGUAGE ASM_MASM
    COMPILE_OPTIONS "/Cx;/DMASM;/DFLAT_MODEL;/I${DEMECH2_SOURCE_DIR}/3rdparty/vfx")
  demech2_edition_masm_defines(s3_masm_defines MW2_ACCEL MW2_S3)
  demech2_add_dll(mw2s3
    ID MW2S3
    SOURCE_DIR MW2
    OUTPUT_NAME MW2_S3
    LINK_DIR s3
    RUNTIME ""
    SOURCES
      ${sources}
      "${DEMECH2_SOURCE_DIR}/MW2/MW2.def"
    COMPILE_OPTIONS "$<$<COMPILE_LANGUAGE:C,CXX>:/ML;/G5;/Gr;/Gy;/DMW2_ACCEL;/DMW2_S3>"
      "$<$<COMPILE_LANGUAGE:ASM_MASM>:${s3_masm_defines}>"
    LINK_OPTIONS /DEBUG /INCREMENTAL:no
    LIBRARIES kernel32.lib user32.lib gdi32.lib advapi32.lib winmm.lib dplay ddraw wail32 s3dtk
  )
  target_include_directories(mw2s3 SYSTEM PRIVATE "${DEMECH2_SOURCE_DIR}/3rdparty/s3dtk")
endfunction()
