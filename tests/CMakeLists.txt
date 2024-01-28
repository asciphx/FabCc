message(STATUS "Build libraries : ${libraries}")
add_executable(benchmark benchmark.cc)
target_link_libraries(benchmark PRIVATE ${FabCc_LIBRARIES})

add_executable(media_server media_server.cc)
target_link_libraries(media_server PRIVATE ${FabCc_LIBRARIES})

if(NOT CMAKE_CXX_STANDARD STREQUAL 11)
  add_executable(test_server test_server.cc)
  set_target_properties(test_server PROPERTIES CXX_STANDARD ${CMAKE_CXX_STANDARD})
  target_link_libraries(test_server PRIVATE ${FabCc_LIBRARIES})
endif()

add_executable(UrlResolution UrlResolution.cc)
target_link_libraries(UrlResolution)

add_executable(client client.cc)
target_link_libraries(client ${FabCc_LIBRARIES})

if(${OPENSSL})
add_executable(https https.cc)
target_link_libraries(https ${FabCc_LIBRARIES})
endif()