# Compiler options with hardening flags

if(MSVC)
  list(APPEND compiler_options
    $<$<CONFIG:RELEASE>:/O2 /Ob2>
    $<$<CONFIG:DEBUG>:/Zi /Ob0 /Od /RTC1>)
else(MSVC)
  list(APPEND compiler_options
    -g
    -m64
    -pg
    -fexec-charset=utf-8
    -finput-charset=utf-8
    $<$<CONFIG:RELEASE>:-O2>
    $<$<CONFIG:DEBUG>:-p>)
endif()
