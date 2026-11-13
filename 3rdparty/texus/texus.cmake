# 3dfx's Texus texture library (3rdparty/texus), as the 3Dfx edition's simulator (MW2_3DFX.DLL)
# links it: a static library of the objects the DLL has, for the vc22/ sub-project.
#
# Sources: in the order of their objects in the DLL. The order LINK pulls the members in follows
# the references (txInit3dfInfo and txConvert, then what they call), which gives the DLL's order
# whatever the order of the archive, but the list keeps it for the reader.
#
# Flags: 3dfx built Texus itself, not with the game's flags. VC++ 2.2 with /Ox /G5, __cdecl (no
# /Gr): the functions are aligned with npad padding inside each object (no /Gy, whose COMDATs the
# linker would pad with int3, as in the game's code), the objects keep their duplicate strings (no
# /Gf), and none of the x87 divisions has 2.2's FDIV check (/QIfdiv-, which 2.2 has on by
# default). The comparison build adds /Z7 (CMAKE_MSVC_DEBUG_INFORMATION_FORMAT), as for the other
# 2.2 objects. The target sets DEMECH2_SOURCE_OPTIMIZATION, so the directory's /Od (vc22/
# CMakeLists.txt) leaves it out.
function(demech2_add_texus TARGET)
  set(dir "${CMAKE_CURRENT_FUNCTION_LIST_DIR}")
  add_library(${TARGET} STATIC
    "${dir}/texuslib.c"
    "${dir}/util.c"
    "${dir}/quantize.c"
    "${dir}/mipmap.c"
    "${dir}/clamp.c"
    "${dir}/resample.c"
    "${dir}/dequant.c"
    "${dir}/pal256.c"
    "${dir}/ncc.c"
    "${dir}/diffuse.c"
    "${dir}/nccnnet.c"
  )
  # The DLL's single-threaded CRT (2.2's LIBC, /ML); CMake has no MSVC_RUNTIME_LIBRARY value for it.
  set_target_properties(${TARGET} PROPERTIES
    MSVC_RUNTIME_LIBRARY ""
    DEMECH2_SOURCE_OPTIMIZATION TRUE
  )
  target_include_directories(${TARGET} PRIVATE "${dir}")
  target_include_directories(${TARGET} SYSTEM PRIVATE "${dir}/../glide")
  if (MSVC_FOR_DECOMP)
    target_compile_options(${TARGET} PRIVATE /ML /Ox /G5 /QIfdiv-)
  endif()
endfunction()
