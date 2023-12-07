find_library(SQLite3_LIBRARY NO_DEFAULT_PATH
  NAMES sqlite3
  PATHS ${HT_DEPENDENCY_LIB_DIR} /lib /home/asciphx/vcpkg/packages/sqlite3_x64-linux/lib /usr/lib /usr/local/lib
)
if (SQLite3_LIBRARY)
  set( SQLite3_INCLUDE_DIR /home/asciphx/vcpkg/packages/sqlite3_x64-linux/include)
  set(SQLite3_FOUND TRUE)
endif ()

# Create the imported target
if(SQLite3_FOUND)
    set(SQLite3_INCLUDE_DIRS ${SQLite3_INCLUDE_DIR})
    set(SQLite3_LIBRARIES ${SQLite3_LIBRARY})
    if(NOT TARGET SQLite::SQLite3)
        add_library(SQLite::SQLite3 UNKNOWN IMPORTED)
        set_target_properties(SQLite::SQLite3 PROPERTIES
            IMPORTED_LOCATION             "${SQLite3_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SQLite3_INCLUDE_DIR}")
    endif()
endif()
