# The compiler and linker flags of a VC++ 4.x build (see "Compiler and linker flags" in the
# top-level CMakeLists.txt). Included by the top-level project with VC++ 4.1 and by vc40/.

foreach(lang C CXX)
  set(CMAKE_${lang}_FLAGS "/DWIN32 /D_WINDOWS /W3 /Od /Oi")
  set(CMAKE_${lang}_FLAGS_DEBUG "/Zi")
  set(CMAKE_${lang}_FLAGS_RELEASE "/DNDEBUG")
  set(CMAKE_${lang}_FLAGS_RELWITHDEBINFO "/Zi /DNDEBUG")
  set(CMAKE_${lang}_FLAGS_MINSIZEREL "/DNDEBUG")
  set(CMAKE_${lang}_STANDARD_LIBRARIES "")
endforeach()
# LINK 3.10 crashes on Windows ("Internal error during Pass1", at a varying object) once
# the object names it is given total about 4K: MW2's 100 objects as CMakeFiles\mw2.dir\...
# paths did (4226 bytes; 91 linked at 3843), and so did MW2SHELL's 60 as absolute paths
# (5016 bytes; 3309 as relative ones linked). Response-file lines are not the limit (the
# linker allows 16K per line), and Wine's LINK, which runs on Wine's C runtime instead of
# the system's MSVCRT40, links them all. So the DLLs are linked from copies of their
# objects in a flat directory under their base names (o\MW2\overlay.c.obj), which
# demech2_add_dll copies before each link, and the response file lists those.
foreach(lang C CXX)
  string(REPLACE "<OBJECTS>" "@<TARGET_BASE>.objects.rsp"
    CMAKE_${lang}_CREATE_SHARED_LIBRARY "${CMAKE_${lang}_CREATE_SHARED_LIBRARY}")
endforeach()
foreach(kind SHARED EXE)
  set(CMAKE_${kind}_LINKER_FLAGS "/machine:I386")
  foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_${kind}_LINKER_FLAGS_${config} "")
  endforeach()
endforeach()
