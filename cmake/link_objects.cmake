# Copies a DLL's objects to a flat directory under their base names and writes the
# response file the link reads them from, in link order (see CMakeLists.txt, "Compiler
# and linker flags"). Run at PRE_LINK with -DLIST=<file listing the objects, one per
# line> -DDEST=<flat directory> -DRSP=<response file> -DPREFIX=<the directory as the
# response file names it>.
cmake_minimum_required(VERSION 3.21)  # file(COPY_FILE), if(IN_LIST)
file(STRINGS "${LIST}" objects)
file(MAKE_DIRECTORY "${DEST}")
set(names "")
set(lines "")
foreach(object IN LISTS objects)
  get_filename_component(name "${object}" NAME)
  if (name IN_LIST names)
    message(FATAL_ERROR "Two objects share the base name ${name}; the flat link directory needs unique names")
  endif()
  list(APPEND names "${name}")
  file(COPY_FILE "${object}" "${DEST}/${name}" ONLY_IF_DIFFERENT)
  string(APPEND lines "${PREFIX}\\${name}\n")
endforeach()
file(WRITE "${RSP}" "${lines}")
