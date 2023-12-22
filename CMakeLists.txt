cmake_minimum_required(VERSION 2.8)
project(Nod)
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release") # 如果是Debug，那么_DEBUG宏生效
endif()
message(STATUS "CXX_COMPILER is ${CMAKE_CXX_COMPILER_ID}")
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
# cmake --build ./ -- /p:Configuration=Release  ||  cmake --build ./ --config Release
include(CheckIncludeFileCXX)
check_include_file_cxx(coroutine HAS_COROUTINE)
check_include_file_cxx(any HAS_ANY)
CHECK_INCLUDE_FILE_CXX(shared_mutex HAS_SHARED_MUTEX)
if(HAS_COROUTINE)
  set(CMAKE_CXX_STANDARD 20)
elseif(HAS_ANY)
  set(CMAKE_CXX_STANDARD 17)
elseif(HAS_SHARED_MUTEX)
  set(CMAKE_CXX_STANDARD 14)
else()
  set(CMAKE_CXX_STANDARD 11)
endif()
add_subdirectory(third-party/ctx)
add_subdirectory(third-party/zlib)
if(MSVC)
  cmake_host_system_information (RESULT CPU_NUMBER QUERY NUMBER_OF_LOGICAL_CORES)
  add_compile_options(/GA /utf-8 /MP${CPU_NUMBER} /Zc:__cplusplus)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  message(STATUS "Build CORES : ${CPU_NUMBER}")
else()
  set(CMAKE_CXX_FLAGS -std=c++${CMAKE_CXX_STANDARD})
  add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)
endif()
message(STATUS "Build STANDARD: c++${CMAKE_CXX_STANDARD}"
)

find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
message(STATUS "Build OpenSSL: ON")
endif()
include_directories(fc/include ${OPENSSL_INCLUDE_DIR})
aux_source_directory(fc/src cc_files)
aux_source_directory(fc/src/c c_files)
aux_source_directory(fc/src/cc cc_files)
add_library(Nod STATIC ${cc_files} ${c_files})
if(APPLE)
  set(libraries
    z
    iconv)
elseif(WIN32)
  set(libraries
    PSAPI
    wsock32
    ws2_32)
else()
  set(libraries
    pthread)
endif()
# ${PROJECT_SOURCE_DIR}/
# 添加第三方的库, 而第三方库的头文件集中复制到include/tp下使用
set(Nod_LIBRARIES
  Nod ${libraries}
  ctx
  zlibstatic
)
if(WIN32)
  set(SUF .exe)
  if(MSVC)
  set(DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
  else()
  set(DIR ${CMAKE_BINARY_DIR})
  endif()
else()
  set(DIR ${CMAKE_CURRENT_BINARY_DIR})
endif()
message(STATUS "to: ${PROJECT_SOURCE_DIR}/HttpWeb")
message(STATUS "selected to ${CMAKE_BUILD_TYPE}")
if(CMAKE_CXX_STANDARD STREQUAL 11)
  add_executable(HttpWeb webC11.cc)
else()
  add_executable(HttpWeb web.cc)
endif()
add_custom_command(TARGET HttpWeb
  COMMAND ${CMAKE_COMMAND} -E copy ${DIR}/HttpWeb${SUF} ${PROJECT_SOURCE_DIR}/HttpWeb${SUF}
)
target_link_libraries(HttpWeb ${Nod_LIBRARIES})

add_executable(test tests.cc)
add_custom_command(TARGET test
  COMMAND ${CMAKE_COMMAND} -E copy ${DIR}/test${SUF} ${PROJECT_SOURCE_DIR}/test${SUF}
)
target_link_libraries(test PUBLIC Nod ctx)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
message(STATUS "OUTPUT_DIRECTORY=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
add_subdirectory(tests)

# cmake .. -DBUILD_UNITEST=ON 来打开
option(BUILD_UNITEST "Build UNITEST" OFF)
if(BUILD_UNITEST)
  message(STATUS "Build UNITEST: ${BUILD_UNITEST}")
  enable_testing()
  add_subdirectory(unitest)
endif()