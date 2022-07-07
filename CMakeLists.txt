cmake_minimum_required(VERSION 2.8)
project(FabCc)
set(CMAKE_BUILD_TYPE "Release")#如果是Debug，那么_DEBUG宏才會生效
if (MSVC)
	add_compile_options( /GA /O2 /Ob2 /utf-8 /std:c++17 )#只有是Release才会是O2優化，否則會被覆写
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2 -fexec-charset=utf-8 -finput-charset=utf-8")
endif ()
include_directories(${PROJECT_SOURCE_DIR}/include)#添加掃描的頭文件目錄
aux_source_directory(${PROJECT_SOURCE_DIR}/src cc_files)#添加需要生成庫的源文件目錄

if (MSVC)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/c c_files)
add_library(Fab STATIC ${cc_files} ${c_files})
include_directories(${PROJECT_SOURCE_DIR}/hh)
set( FabCc_LIBRARIES
Fab
${PROJECT_SOURCE_DIR}/lib/AdvAPI32.lib
${PROJECT_SOURCE_DIR}/lib/iphlpapi.lib
${PROJECT_SOURCE_DIR}/lib/kernel32.lib
${PROJECT_SOURCE_DIR}/lib/Psapi.lib
${PROJECT_SOURCE_DIR}/lib/Secur32.lib
${PROJECT_SOURCE_DIR}/lib/User32.lib
${PROJECT_SOURCE_DIR}/lib/UserEnv.lib
${PROJECT_SOURCE_DIR}/lib/uv.lib
${PROJECT_SOURCE_DIR}/lib/WS2_32.lib
${PROJECT_SOURCE_DIR}/lib/WSock32.lib
)
add_executable(FabCc tcpServer.cc
)#添加庫文件，可以利用bat脚本，在當前目錄運行一鍵生成上面的複製過來
else ()
#Linux后续即将支持网络库
add_library(Fab STATIC ${cc_files})#如果是動態庫那么SHARED
set( FabCc_LIBRARIES
Fab
)
add_executable(FabCc w.cc
#${PROJECT_SOURCE_DIR}/src/lexical_cast.cc
)
endif ()
target_link_libraries( FabCc ${FabCc_LIBRARIES})
