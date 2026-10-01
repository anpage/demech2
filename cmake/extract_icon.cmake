# Build-time helper that extracts an icon group from an original binary, shared by the
# shell and the launcher (tools/extract_icon). Included by the top-level project and by
# the VC++ 2.2 sub-project (vc22/), each building its own copy with its own compiler.
function(demech2_add_extract_icon)
  if (NOT TARGET extract_icon)
    add_executable(extract_icon "${DEMECH2_SOURCE_DIR}/tools/extract_icon/extract_icon.c")
    set_target_properties(extract_icon PROPERTIES MSVC_RUNTIME_LIBRARY MultiThreaded)
    target_link_libraries(extract_icon PRIVATE kernel32.lib)
  endif()
endfunction()
