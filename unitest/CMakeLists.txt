file(GLOB SRC_FILES *.cc)
add_executable(unitest ${SRC_FILES})
target_link_libraries(unitest PRIVATE FabCc)
add_test(NAME unitest COMMAND unitest)
