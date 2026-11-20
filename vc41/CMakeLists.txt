# The VC++ 4.1 targets with optimized units: MW2_S3.DLL, the S3 ViRGE edition's simulator. They
# are built with the top-level project's compiler, but in a directory of their own: a source's
# optimization flags (DEMECH2_OPTIMIZATION, cmake/game_dll.cmake) are source properties, which
# CMake keeps per directory, so here they reach only this directory's targets and not MW2.DLL,
# which compiles the same sources at /Od. Added by the top-level CMakeLists.txt with VC++ 4.1.

# The S3 toolkit's import library, from its .def (cmake/import_library.cmake).
demech2_add_import_library(s3dtk DEF "${DEMECH2_SOURCE_DIR}/3rdparty/s3dtk/s3dtk.def")

include("${CMAKE_CURRENT_SOURCE_DIR}/targets.cmake")
demech2_add_mw2s3()
