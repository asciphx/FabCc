cmake_minimum_required(VERSION 2.8)
project(Uv)
if (MSVC)
	add_compile_options( /O2 /GA /Ob2 )#只有是Release才会是O2優化，否則會被覆写
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
	add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)
endif ()
include_directories(${PROJECT_SOURCE_DIR})
set(uv_sources
    fs-poll.c
    idna.c
    inet.c
    random.c
    strscpy.c
    strtok.c
    threadpool.c
    uv-timer.c
    uv-common.c
    uv-data-getter-setters.c
    version.c)
if(WIN32)
  if(EXISTS "${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/Uv.lib")
	  message(STATUS "lib: ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/Uv.lib")
  else()
    list(APPEND uv_defines WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0602)
    list(APPEND uv_sources
           win/async.c
           win/core.c
           win/detect-wakeup.c
           win/dl.c
           win/error.c
           win/fs.c
           win/fs-event.c
           win/getaddrinfo.c
           win/getnameinfo.c
           win/handle.c
           win/loop-watcher.c
           win/pipe.c
           win/thread.c
           win/poll.c
           win/process.c
           win/process-stdio.c
           win/signal.c
           win/snprintf.c
           win/stream.c
           win/tcp.c
           win/tty.c
           win/udp.c
           win/util.c
           win/winapi.c
           win/winsock.c)
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
else ()
  list(APPEND uv_defines _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE)
  if(NOT CMAKE_SYSTEM_NAME MATCHES "Android|OS390|QNX")
    list(APPEND uv_libraries pthread)
    message(STATUS "NAME: ${CMAKE_SYSTEM_NAME}")
  endif()
  list(APPEND uv_sources
         unix/async.c
         unix/core.c
         unix/dl.c
         unix/fs.c
         unix/getaddrinfo.c
         unix/getnameinfo.c
         unix/loop-watcher.c
         unix/loop.c
         unix/pipe.c
         unix/poll.c
         unix/process.c
         unix/random-devurandom.c
         unix/signal.c
         unix/stream.c
         unix/tcp.c
         unix/thread.c
         unix/tty.c
         unix/udp.c)
  if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "Android|Linux")
    list(APPEND uv_sources unix/proctitle.c)
  endif()
  if(CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD")
    list(APPEND uv_sources unix/freebsd.c)
  endif()
  if(CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD|NetBSD|OpenBSD")
    list(APPEND uv_sources unix/posix-hrtime.c unix/bsd-proctitle.c)
  endif()
  if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD|NetBSD|OpenBSD")
    list(APPEND uv_sources unix/bsd-ifaddrs.c unix/kqueue.c)
  endif()
  if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    list(APPEND uv_sources unix/random-getrandom.c)
  endif()
  if(APPLE OR CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
    list(APPEND uv_sources unix/random-getentropy.c)
  endif()
  if(APPLE)
    list(APPEND uv_defines _DARWIN_UNLIMITED_SELECT=1 _DARWIN_USE_64_BIT_INODE=1)
    list(APPEND uv_sources
         unix/darwin-proctitle.c
         unix/darwin.c
         unix/fsevents.c)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "GNU")
    list(APPEND uv_libraries dl)
    list(APPEND uv_sources
         unix/bsd-ifaddrs.c
         unix/no-fsevents.c
         unix/no-proctitle.c
         unix/posix-hrtime.c
         unix/posix-poll.c
         unix/hurd.c)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "kFreeBSD")
    list(APPEND uv_defines _GNU_SOURCE)
    list(APPEND uv_libraries dl freebsd-glue)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND uv_defines _GNU_SOURCE _POSIX_C_SOURCE=200112)
    list(APPEND uv_libraries dl rt)
    list(APPEND uv_sources
         unix/linux-core.c
         unix/linux-inotify.c
         unix/linux-syscalls.c
         unix/procfs-exepath.c
         unix/random-getrandom.c
         unix/random-sysctl-linux.c
         unix/epoll.c)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
    list(APPEND uv_sources unix/netbsd.c)
    list(APPEND uv_libraries kvm)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
    list(APPEND uv_sources unix/openbsd.c)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "OS400")
    list(APPEND uv_defines
         _ALL_SOURCE
         _LINUX_SOURCE_COMPAT
         _THREAD_SAFE
         _XOPEN_SOURCE=500)
    list(APPEND uv_sources
      unix/aix-common.c
      unix/ibmi.c
      unix/no-fsevents.c
      unix/posix-poll.c)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
    list(APPEND uv_defines __EXTENSIONS__ _XOPEN_SOURCE=500 _REENTRANT)
    list(APPEND uv_libraries kstat nsl sendfile socket)
    list(APPEND uv_sources
         unix/no-proctitle.c
         unix/sunos.c)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "Haiku")
    list(APPEND uv_defines _BSD_SOURCE)
    list(APPEND uv_libraries bsd network)
    list(APPEND uv_sources
	    unix/haiku.c
	    unix/bsd-ifaddrs.c
	    unix/no-fsevents.c
	    unix/no-proctitle.c
	    unix/posix-hrtime.c
	    unix/posix-poll.c)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "QNX")
    list(APPEND uv_sources
      unix/posix-hrtime.c
      unix/posix-poll.c
      unix/qnx.c
      unix/bsd-ifaddrs.c
      unix/no-proctitle.c
      unix/no-fsevents.c)
    list(APPEND uv_libraries socket)
  endif()
  if(APPLE OR CMAKE_SYSTEM_NAME MATCHES "DragonFly|FreeBSD|Linux|NetBSD|OpenBSD")
    list(APPEND uv_test_libraries util)
  endif()
  add_library(Uv STATIC ${uv_sources} ${uv_c_files})
  target_compile_definitions(Uv PRIVATE ${uv_defines})
  target_compile_options(Uv PRIVATE ${uv_cflags})
endif ()
message(STATUS "LIBRARIES: ${uv_libraries}")
target_link_libraries( Uv ${uv_libraries})

