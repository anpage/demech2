# The game DLLs' helpers, for the top-level project and the nested VC++ builds (vc40/).
# Expect DEMECH2_SOURCE_DIR and MSVC_FOR_DECOMP; the top-level project registers each DLL
# with reccmp.

# Every VC++ 4.x DLL links from copies of its objects (see "Compiler and linker flags"). The
# objects, in link order, for cmake/link_objects.cmake: it copies them to o/<name>/ under their
# base names and writes <name>.objects.rsp, which CMAKE_<LANG>_CREATE_SHARED_LIBRARY reads
# instead of <OBJECTS>.
function(demech2_link_object_copies TARGET)
  if (MSVC_FOR_DECOMP)
    set(dir "$<TARGET_FILE_DIR:${TARGET}>")
    set(name "$<TARGET_FILE_BASE_NAME:${TARGET}>")
    file(GENERATE OUTPUT "${dir}/${name}.objects.list"
      CONTENT "$<JOIN:$<TARGET_OBJECTS:${TARGET}>,\n>\n")
    add_custom_command(TARGET ${TARGET} PRE_LINK
      COMMAND "${CMAKE_COMMAND}"
        "-DLIST=${dir}/${name}.objects.list" "-DDEST=${dir}/o/${name}"
        "-DRSP=${dir}/${name}.objects.rsp" "-DPREFIX=o\\${name}"
        -P "${DEMECH2_SOURCE_DIR}/cmake/link_objects.cmake"
      COMMENT "Copying ${TARGET} objects to o/${name}"
      VERBATIM
    )
  endif()
endfunction()

# Adds a game DLL and registers it with reccmp. SOURCES must follow the original link
# order: object order determines function addresses. SOURCE_DIR is the directory whose include/
# the target uses (default: ID). LIBRARIES are the import
# libraries, in the order of the original's .idata descriptors (library processing
# order decides it). RUNTIME is the MSVC_RUNTIME_LIBRARY value, COMPILE_OPTIONS and
# LINK_OPTIONS the target's own codegen and link flags.
function(demech2_add_dll TARGET)
  cmake_parse_arguments(ARGS "" "ID;SOURCE_DIR;OUTPUT_NAME;RUNTIME"
    "SOURCES;LIBRARIES;COMPILE_OPTIONS;LINK_OPTIONS" ${ARGN})
  if (NOT ARGS_SOURCE_DIR)
    set(ARGS_SOURCE_DIR "${ARGS_ID}")
  endif()
  add_library(${TARGET} SHARED ${ARGS_SOURCES} "${DEMECH2_SOURCE_DIR}/util/decomp.cpp")
  set_target_properties(${TARGET} PROPERTIES
    OUTPUT_NAME "${ARGS_OUTPUT_NAME}"
    MSVC_RUNTIME_LIBRARY "${ARGS_RUNTIME}"
  )
  target_include_directories(${TARGET} PRIVATE
    "${DEMECH2_SOURCE_DIR}/util"
    "${DEMECH2_SOURCE_DIR}/${ARGS_SOURCE_DIR}/include"
    "${DEMECH2_SOURCE_DIR}/common/include"
  )
  target_include_directories(${TARGET} SYSTEM PRIVATE
    "${DEMECH2_SOURCE_DIR}/3rdparty/dispdib"
    "${DEMECH2_SOURCE_DIR}/3rdparty/dx2/INC"
    "${DEMECH2_SOURCE_DIR}/3rdparty/mss"
    "${DEMECH2_SOURCE_DIR}/3rdparty/smacker"
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
  demech2_link_object_copies(${TARGET})
  if (COMMAND reccmp_add_target)
    reccmp_add_target(${TARGET} ID ${ARGS_ID})
  endif()
endfunction()
