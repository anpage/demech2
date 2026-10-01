# Build-time helper that extracts an icon group or a bitmap from an original binary,
# shared by the shell, the launcher and NetMech's lobby (tools/extract_resource). Included
# by the top-level project and by the VC++ 2.2 sub-project (vc22/), each building its own
# copy with its own compiler.
function(demech2_add_extract_resource)
  if (NOT TARGET extract_resource)
    add_executable(extract_resource "${DEMECH2_SOURCE_DIR}/tools/extract_resource/extract_resource.c")
    set_target_properties(extract_resource PROPERTIES MSVC_RUNTIME_LIBRARY MultiThreaded)
    target_link_libraries(extract_resource PRIVATE kernel32.lib)
  endif()
endfunction()
