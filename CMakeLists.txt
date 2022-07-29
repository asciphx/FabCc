cmake_minimum_required(VERSION 2.8)
project(FabCc)
if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected, default to Release")
	set(CMAKE_BUILD_TYPE "Release")#如果是Debug，那么_DEBUG宏才會生效
else()
	message(STATUS "selected to ${CMAKE_BUILD_TYPE}")
endif()
#cmake --build ./ -- /p:Configuration=Release  ||  cmake --build ./ --config Release
if (MSVC)
	add_compile_options( /O2 /GA /Ob2 /utf-8 /std:c++17 )#只有是Release才会是O2優化，否則會被覆写
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2 -fexec-charset=utf-8 -finput-charset=utf-8")
	add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)
endif ()
include_directories(fc/include)#添加掃描的頭文件目錄
aux_source_directory(fc/src cc_files)#添加需要生成庫的源cc文件目錄
aux_source_directory(fc/src/c c_files)#添加需要生成庫的源c文件目錄
add_library(Fab STATIC ${cc_files} ${c_files})
#${PROJECT_SOURCE_DIR}/
set(SRC third-party/libuv)
include_directories(${SRC})
set(uv_sources
    ${SRC}/fs-poll.c
    ${SRC}/idna.c
    ${SRC}/inet.c
    ${SRC}/random.c
    ${SRC}/strscpy.c
    ${SRC}/strtok.c
    ${SRC}/threadpool.c
    ${SRC}/uv-timer.c
    ${SRC}/uv-common.c
    ${SRC}/uv-data-getter-setters.c
    ${SRC}/version.c)
if(WIN32)
  if(EXISTS "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/Uv.lib")
	  message(STATUS "lib: ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/Uv.lib")
  else()
    list(APPEND uv_defines WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0602)
  list(APPEND uv_sources
         ${SRC}/win/async.c
         ${SRC}/win/core.c
         ${SRC}/win/detect-wakeup.c
         ${SRC}/win/dl.c
         ${SRC}/win/error.c
         ${SRC}/win/fs.c
         ${SRC}/win/fs-event.c
         ${SRC}/win/getaddrinfo.c
         ${SRC}/win/getnameinfo.c
         ${SRC}/win/handle.c
         ${SRC}/win/loop-watcher.c
         ${SRC}/win/pipe.c
         ${SRC}/win/thread.c
         ${SRC}/win/poll.c
         ${SRC}/win/process.c
         ${SRC}/win/process-stdio.c
         ${SRC}/win/signal.c
         ${SRC}/win/snprintf.c
         ${SRC}/win/stream.c
         ${SRC}/win/tcp.c
         ${SRC}/win/tty.c
         ${SRC}/win/udp.c
         ${SRC}/win/util.c
         ${SRC}/win/winapi.c
         ${SRC}/win/winsock.c)
  add_library(Uv STATIC ${uv_sources} ${uv_c_files})
  target_compile_definitions(Uv PRIVATE ${uv_defines})
  target_compile_options(Uv PRIVATE ${uv_cflags})
  #find_package(unofficial-libuv CONFIG REQUIRED) #unofficial::libuv::libuv
  endif()
  list(APPEND uv_libraries
        psapi
        user32
        advapi32
        iphlpapi
        userenv
        ws2_32)
  set( FabCc_LIBRARIES
  Fab
  ${uv_libraries}
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/Uv.lib
  )
  add_executable(FabCc tcpServer.cc)#添加庫文件，可以利用bat脚本，在當前目錄運行一鍵生成上面的複製過來
  message(STATUS "to: ${PROJECT_SOURCE_DIR}/FabCc.exe")
  add_custom_command(TARGET FabCc
	  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/FabCc.exe ${PROJECT_SOURCE_DIR}/FabCc.exe
  )#将生成的文件，拷贝到最外层目录，与static文件夹同级，那么就可以方便执行
  add_executable(W w.cc
  fc/src/lexical_cast.cc
  )
else ()
  list(APPEND uv_sources
         ${SRC}/unix/async.c
         ${SRC}/unix/core.c
         ${SRC}/unix/dl.c
         ${SRC}/unix/fs.c
         ${SRC}/unix/getaddrinfo.c
         ${SRC}/unix/getnameinfo.c
         ${SRC}/unix/loop-watcher.c
         ${SRC}/unix/loop.c
         ${SRC}/unix/pipe.c
         ${SRC}/unix/poll.c
         ${SRC}/unix/process.c
         ${SRC}/unix/random-devurandom.c
         ${SRC}/unix/signal.c
         ${SRC}/unix/stream.c
         ${SRC}/unix/tcp.c
         ${SRC}/unix/thread.c
         ${SRC}/unix/tty.c
         ${SRC}/unix/udp.c)
  if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "Android|Linux")
    list(APPEND uv_sources ${SRC}/unix/proctitle.c)
  endif()
  if(CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD")
    list(APPEND uv_sources ${SRC}/unix/freebsd.c)
  endif()
  if(CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD|NetBSD|OpenBSD")
    list(APPEND uv_sources ${SRC}/unix/posix-hrtime.c ${SRC}/unix/bsd-proctitle.c)
  endif()
  if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD|NetBSD|OpenBSD")
    list(APPEND uv_sources ${SRC}/unix/bsd-ifaddrs.c ${SRC}/unix/kqueue.c)
  endif()
  if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    list(APPEND uv_sources ${SRC}/unix/random-getrandom.c)
  endif()
  if(APPLE OR CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
    list(APPEND uv_sources ${SRC}/unix/random-getentropy.c)
  endif()
  if(APPLE)
    list(APPEND uv_defines _DARWIN_UNLIMITED_SELECT=1 _DARWIN_USE_64_BIT_INODE=1)
    list(APPEND uv_sources
         ${SRC}/unix/darwin-proctitle.c
         ${SRC}/unix/darwin.c
         ${SRC}/unix/fsevents.c)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "GNU")
    list(APPEND uv_libraries dl)
    list(APPEND uv_sources
         ${SRC}/unix/bsd-ifaddrs.c
         ${SRC}/unix/no-fsevents.c
         ${SRC}/unix/no-proctitle.c
         ${SRC}/unix/posix-hrtime.c
         ${SRC}/unix/posix-poll.c
         ${SRC}/unix/hurd.c)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD")
    list(APPEND uv_defines _GNU_SOURCE)
    list(APPEND uv_libraries dl freebsd-glue)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND uv_defines _GNU_SOURCE _POSIX_C_SOURCE=200112)
    list(APPEND uv_libraries dl rt)
    list(APPEND uv_sources
         ${SRC}/unix/linux-core.c
         ${SRC}/unix/linux-inotify.c
         ${SRC}/unix/linux-syscalls.c
         ${SRC}/unix/procfs-exepath.c
         ${SRC}/unix/random-getrandom.c
         ${SRC}/unix/random-sysctl-linux.c
         ${SRC}/unix/epoll.c)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
    list(APPEND uv_sources ${SRC}/unix/netbsd.c)
    list(APPEND uv_libraries kvm)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
    list(APPEND uv_sources ${SRC}/unix/openbsd.c)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "OS400")
    list(APPEND uv_defines
         _ALL_SOURCE
         _LINUX_SOURCE_COMPAT
         _THREAD_SAFE
         _XOPEN_SOURCE=500)
    list(APPEND uv_sources
      ${SRC}/unix/aix-common.c
      ${SRC}/unix/ibmi.c
      ${SRC}/unix/no-fsevents.c
      ${SRC}/unix/posix-poll.c)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
    list(APPEND uv_defines __EXTENSIONS__ _XOPEN_SOURCE=500 _REENTRANT)
    list(APPEND uv_libraries kstat nsl sendfile socket)
    list(APPEND uv_sources
         ${SRC}/unix/no-proctitle.c
         ${SRC}/unix/sunos.c)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "Haiku")
    list(APPEND uv_defines _BSD_SOURCE)
    list(APPEND uv_libraries bsd network)
    list(APPEND uv_sources
	    ${SRC}/unix/haiku.c
	    ${SRC}/unix/bsd-ifaddrs.c
	    ${SRC}/unix/no-fsevents.c
	    ${SRC}/unix/no-proctitle.c
	    ${SRC}/unix/posix-hrtime.c
	    ${SRC}/unix/posix-poll.c)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "QNX")
    list(APPEND uv_sources
      ${SRC}/unix/posix-hrtime.c
      ${SRC}/unix/posix-poll.c
      ${SRC}/unix/qnx.c
      ${SRC}/unix/bsd-ifaddrs.c
      ${SRC}/unix/no-proctitle.c
      ${SRC}/unix/no-fsevents.c)
    list(APPEND uv_libraries socket)
  endif()
  if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD|Linux|NetBSD|OpenBSD")
    list(APPEND uv_test_libraries util)
  endif()
  add_library(Uv STATIC ${uv_sources} ${uv_c_files})
  target_compile_definitions(Uv PRIVATE ${uv_defines})
  target_compile_options(Uv PRIVATE ${uv_cflags})
  set( FabCc_LIBRARIES
  Fab
  ${uv_libraries}
  Uv
  )
  add_executable(FabCc tcpServer.cc)#添加庫文件，可以利用bat脚本，在當前目錄運行一鍵生成上面的複製過來
  message(STATUS "to: ${PROJECT_SOURCE_DIR}/FabCc")
  add_custom_command(TARGET FabCc
	  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/FabCc ${PROJECT_SOURCE_DIR}/FabCc
  )#将生成的文件，拷贝到最外层目录，与static文件夹同级，那么就可以方便执行
  add_executable(W w.cc
  fc/src/lexical_cast.cc
  )
endif ()
target_link_libraries( FabCc ${FabCc_LIBRARIES})
