cmake_minimum_required(VERSION 2.8)
project(FabCc)
if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected, default to Release")
	set(CMAKE_BUILD_TYPE "Release")#如果是Debug，那么_DEBUG宏才會生效
else()
	message(STATUS "selected to ${CMAKE_BUILD_TYPE}")
endif()
#cmake --build ./ -- /p:Configuration=Release  ||  cmake --build ./ --config Release
include(CheckIncludeFileCXX)
check_include_file_cxx(any HAS_ANY)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (HAS_ANY)
  set(CMAKE_CXX_STANDARD 17)
else ()
  set(CMAKE_CXX_STANDARD 11)
endif ()
message(STATUS "Build STANDARD: c++${CMAKE_CXX_STANDARD}")
if (MSVC)
	add_compile_options(/O2 /GA /Ob2 /utf-8 )#只有是Release才会是O2優化，否則會被覆写
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -fexec-charset=utf-8 -finput-charset=utf-8")
	add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)
endif ()
include_directories(fc/include)#添加掃描的頭文件目錄
aux_source_directory(fc/src cc_files)#添加需要生成庫的源cc文件目錄
aux_source_directory(fc/src/c c_files)#添加需要生成庫的源c文件目錄
add_library(Fab STATIC ${cc_files} ${c_files})
if (APPLE)
  set(libraries
	  z
	  iconv)
elseif (WIN32)
  set(libraries
	  wsock32
	  ws2_32)
else ()
  set(libraries
	  pthread)
endif()
#${PROJECT_SOURCE_DIR}/
if(WIN32)
  #windows下扫描的很慢, 但是如果已经编译过的话, 这种方式就可以跳过扫描, 第一次编译需要两次运行
  if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/third-party/zlib/${CMAKE_BUILD_TYPE}/zlibstatic.lib")
	  message(STATUS "lib: ${CMAKE_CURRENT_BINARY_DIR}/third-party/zlib/${CMAKE_BUILD_TYPE}/zlibstatic.lib")
  else()
	  add_subdirectory(third-party/zlib)
  endif()
  if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/third-party/ctx/${CMAKE_BUILD_TYPE}/ctx.lib")
	  message(STATUS "lib: ${CMAKE_CURRENT_BINARY_DIR}/third-party/ctx/${CMAKE_BUILD_TYPE}/ctx.lib")
  else()
	  add_subdirectory(third-party/ctx)
  endif()
  set( FabCc_LIBRARIES
	Fab ${libraries} ${CMAKE_CURRENT_BINARY_DIR}/third-party/ctx/${CMAKE_BUILD_TYPE}/ctx.lib
	${CMAKE_CURRENT_BINARY_DIR}/third-party/zlib/${CMAKE_BUILD_TYPE}/zlibstatic.lib
  )
  add_executable(HttpWeb web.cc)#添加庫文件，可以利用bat脚本，在當前目錄運行一鍵生成上面的複製過來
  message(STATUS "to: ${PROJECT_SOURCE_DIR}/HttpWeb.exe")
  add_custom_command(TARGET HttpWeb
	  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/HttpWeb.exe ${PROJECT_SOURCE_DIR}/HttpWeb.exe
#COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/third-party/zlib/${CMAKE_BUILD_TYPE}/zlib.dll ${PROJECT_SOURCE_DIR}/zlib.dll
  )
  
  add_executable(test tests.cc)
  target_link_libraries(test PUBLIC Fab
	${CMAKE_CURRENT_BINARY_DIR}/third-party/ctx/${CMAKE_BUILD_TYPE}/ctx.lib)
  add_custom_command(TARGET test
	  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/test.exe ${PROJECT_SOURCE_DIR}/test.exe
  )
else()
  #添加第三方的库, 而第三方库的头文件集中复制到include/tp下使用
  add_subdirectory(third-party/ctx)
  add_subdirectory(third-party/zlib)
  add_executable(HttpWeb web.cc)#添加庫文件，可以利用bat脚本，在當前目錄運行一鍵生成上面的複製過來
  message(STATUS "to: ${PROJECT_SOURCE_DIR}/HttpWeb")
  set( FabCc_LIBRARIES
	Fab ${libraries}
	ctx
	zlibstatic
  )
  add_custom_command(TARGET HttpWeb
	  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/HttpWeb ${PROJECT_SOURCE_DIR}/HttpWeb
  )#将生成的文件，拷贝到最外层目录，与static文件夹同级，那么就可以方便执行

  add_executable(test tests.cc)
  target_link_libraries(test PUBLIC Fab ctx)
  add_custom_command(TARGET test
	  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/test ${PROJECT_SOURCE_DIR}/test
  )
endif()
target_link_libraries( HttpWeb ${FabCc_LIBRARIES})
#cmake .. -DBUILD_UNITEST=ON 来打开
option(BUILD_UNITEST "Build UNITEST" OFF)
if(BUILD_UNITEST)
  message(STATUS "Build UNITEST: ${BUILD_UNITEST}")
  enable_testing()
  add_subdirectory(unitest)
endif()
