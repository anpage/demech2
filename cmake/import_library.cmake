# Import libraries for the DLLs the game links against but we don't build. Each is
# generated from a .def file with the build's LINK (/lib). LIB 3.10 can't express a .def
# export whose import name differs from its symbol (DirectDrawCreate vs.
# _DirectDrawCreate@12), so .def names are resolved against a stub object that carries the
# __stdcall decorations. The stub object is never linked into a game DLL.
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
