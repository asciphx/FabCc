if( WIN32 )
SET(Pg_DIR "D:/C++/vcpkg/packages/libpq_x86-windows")
SET(PostgreSQL_LIBRARIES "${Pg_DIR}/lib/libpq.lib")
SET(PostgreSQL_INCLUDE_DIR "${Pg_DIR}/include")
else()#linux or mac
set( PostgreSQL_ROOT_DIR /home/asciphx/vcpkg/packages/libpq_x64-linux)
find_library(PostgreSQL_LIBRARY NO_DEFAULT_PATH
  NAMES "pq"
  PATHS ${HT_DEPENDENCY_LIB_DIR} /lib ${PostgreSQL_ROOT_DIR}/lib /usr/lib /usr/local/lib
)
if (PostgreSQL_LIBRARY)
  set( PostgreSQL_INCLUDE_DIR ${PostgreSQL_ROOT_DIR}/include)
  set( PostgreSQL_LIBRARIES ${PostgreSQL_LIBRARY}
  ${PostgreSQL_ROOT_DIR}/lib/libpgport.a
  ${PostgreSQL_ROOT_DIR}/lib/libpgcommon.a )
else ()
  set(PostgreSQL_FOUND FALSE)
  set( PostgreSQL_LIBRARIES )
endif ()
# "pgport" "pgcommon"
endif()
set( PostgreSQL_INCLUDE_DIRS ${PostgreSQL_INCLUDE_DIR} )
message( PostgreSQL_INCLUDE_DIRS ${PostgreSQL_INCLUDE_DIR} )
message( "pq: " ${PostgreSQL_LIBRARIES} "  " )
mark_as_advanced( PostgreSQL_INCLUDE_DIR PostgreSQL_LIBRARIES )