cmake_minimum_required(VERSION 2.8)
project(ctx LANGUAGES CXX)
# Build features
## Binary format
if(WIN32)
  set(CONTEXT_BINARY_FORMAT pe)
elseif(APPLE)
  set(CONTEXT_BINARY_FORMAT mach-o)
else()
  set(CONTEXT_BINARY_FORMAT elf)
endif()
## ABI
math(EXPR _bits "${CMAKE_SIZEOF_VOID_P}*8")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^[Aa][Rr][Mm]" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
  set(CONTEXT_ABI aapcs)
elseif(WIN32)
  set(CONTEXT_ABI ms)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^mips")
  if(_bits EQUAL 32)
    set(CONTEXT_ABI o32)
  else()
    set(CONTEXT_ABI n64)
  endif()
else()
  set(CONTEXT_ABI sysv)
endif()
## Arch-and-model
if(_bits EQUAL 32)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "^[Aa][Rr][Mm]")
    set(CONTEXT_ARCHITECTURE arm)
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^mips")
    set(CONTEXT_ARCHITECTURE mips32)
  else()
    set(CONTEXT_ARCHITECTURE i386)
  endif()
else()
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" OR
    CMAKE_SYSTEM_PROCESSOR MATCHES "^[Aa][Rr][Mm]") # armv8
    set(CONTEXT_ARCHITECTURE arm64)
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^mips")
    set(CONTEXT_ARCHITECTURE mips64)
  else()
    set(CONTEXT_ARCHITECTURE x86_64)
  endif()
endif()
unset(_bits)
## Assembler type
if(MSVC)
  if(CONTEXT_ARCHITECTURE STREQUAL arm64 OR CONTEXT_ARCHITECTURE STREQUAL arm)
    set(CONTEXT_ASSEMBLER armasm)
  else()
    set(CONTEXT_ASSEMBLER masm)
  endif()
else()
  set(CONTEXT_ASSEMBLER gas)
endif()
## Assembler source suffix
if(CONTEXT_BINARY_FORMAT STREQUAL pe)
  set(CONTEXT_ASM_SUFFIX .asm)
elseif(CONTEXT_ASSEMBLER STREQUAL gas)
  set(CONTEXT_ASM_SUFFIX .S)
else()
  set(CONTEXT_ASM_SUFFIX .asm)
endif()
## Implementation
## Assembler source suffix
  if(CONTEXT_ASSEMBLER STREQUAL gas)
    if(CMAKE_CXX_PLATFORM_ID MATCHES "Cygwin")
      enable_language(ASM-ATT)
    else()
      enable_language(ASM)
    endif()
  elseif(CONTEXT_ASSEMBLER STREQUAL armasm)
    enable_language(ASM_ARMASM)
  else()
    enable_language(ASM_MASM)
  endif()
# Choose .asm sources
if(CONTEXT_BINARY_FORMAT STREQUAL mach-o)
  set(CONTEXT_BINARY_FORMAT macho)
endif()
set(_asm_suffix ${CONTEXT_ARCHITECTURE}_${CONTEXT_ABI}_${CONTEXT_BINARY_FORMAT}_${CONTEXT_ASSEMBLER}${CONTEXT_ASM_SUFFIX})
set(ASM_SOURCES
  asm/make_${_asm_suffix}
  asm/jump_${_asm_suffix}
  asm/ontop_${_asm_suffix}
)
unset(_asm_suffix)
  set(IMPL_SOURCES ${ASM_SOURCES})
  if(CONTEXT_ASSEMBLER STREQUAL masm AND CONTEXT_ARCHITECTURE STREQUAL i386)
      set_source_files_properties(${ASM_SOURCES} PROPERTIES COMPILE_FLAGS "/safeseh")
  endif()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_property(SOURCE ${ASM_SOURCES} APPEND PROPERTY COMPILE_OPTIONS "-x" "assembler-with-cpp")
  endif()
message(STATUS "Boost.Context: "
  "architecture:: ${CONTEXT_ARCHITECTURE}, "
  "binary format:: ${CONTEXT_BINARY_FORMAT}, "
  "ABI:: ${CONTEXT_ABI}, "
  "assembler:: ${CONTEXT_ASSEMBLER}, "
  "suffix:: ${CONTEXT_ASM_SUFFIX}")

add_library(ctx fixedsize_stack.cc fcontext.hpp fixedsize_stack.hh ${ASM_SOURCES})
target_compile_definitions(ctx PUBLIC BOOST_CONTEXT_EXPORT)
