# The target whose original was built with VC++ 4.0: MW2_MATROX.DLL, the simulator of the
# Matrox Mystique edition.
#
# Included by vc40/CMakeLists.txt, the VC++ 4.0 sub-project, and by the top-level project
# when it has no VC++ 4.0 to hand it to (a VC++ 4.1 build without DEMECH2_MSVC40_ROOT).
# Expects DEMECH2_SOURCE_DIR, mw2_sources (MW2/sources.cmake), demech2_add_dll
# (cmake/game_dll.cmake) and the ddraw, dplay and wail32 import library targets.

# MW2_MATROX.DLL (linked 1996-08-22 by LINK 3.00): the same C as MW2.DLL at /Od /Oi /G5, with
# the release CRT (4.0's LIBCMT, which its CRT matches far better than 4.1's), drawing through
# Matrox's MSI95.DLL instead of DirectDraw. Most of its functions are 1.1's, byte for byte or
# with their stack slots permuted, and carry MW2MATROX annotations next to MW2's
# (tools/port_annotations.py). For now it builds 1.1's sources in 1.1's link order: reccmp
# compares the shared functions, wherever the linker puts them. MW2_MATROX is defined for the
# edition's own code.
function(demech2_add_mw2matrox)
  demech2_add_dll(mw2matrox
    ID MW2MATROX
    SOURCE_DIR MW2
    OUTPUT_NAME MW2_MATROX
    RUNTIME MultiThreaded
    SOURCES
      ${mw2_sources}
      "${DEMECH2_SOURCE_DIR}/MW2/MW2.def"
    COMPILE_OPTIONS "$<$<COMPILE_LANGUAGE:C,CXX>:/G5;/DMW2_MATROX>"
    LINK_OPTIONS /DEBUG /INCREMENTAL:no
    LIBRARIES winmm.lib wail32 ddraw dplay kernel32.lib user32.lib gdi32.lib advapi32.lib
  )
endfunction()
