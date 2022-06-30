cmake_minimum_required(VERSION 2.8)
project(FabCc)
set(CMAKE_BUILD_TYPE "Release")
if (MSVC)
	add_compile_options( /GA /O2 /Ob2 /utf-8 /std:c++17 )
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2 -fexec-charset=utf-8 -finput-charset=utf-8")
endif ()
include_directories(${PROJECT_SOURCE_DIR}/include)
aux_source_directory(${PROJECT_SOURCE_DIR}/src src_files)
add_library(Fab STATIC ${src_files})
set( FabCc_LIBRARIES
Fab
#${PROJECT_SOURCE_DIR}/lib/.lib
)
add_executable(FabCc w.cc
#${PROJECT_SOURCE_DIR}/src/lexical_cast.cc
)
target_link_libraries( FabCc ${FabCc_LIBRARIES})
