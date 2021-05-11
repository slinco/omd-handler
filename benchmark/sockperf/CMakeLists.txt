set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall --param inline-unit-growth=200 -O3 -g")

file(GLOB sockperf_cpp ./*.cpp )
add_executable(sockperf ${sockperf_cpp})
target_link_libraries(sockperf dl)