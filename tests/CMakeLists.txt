add_executable(benchmark benchmark.cc)
target_link_libraries(benchmark PRIVATE ${FabCc_LIBRARIES})

add_executable(UrlResolution UrlResolution.cc)
target_link_libraries(UrlResolution)
