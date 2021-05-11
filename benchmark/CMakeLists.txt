set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall --param inline-unit-growth=200 -O3 -g")

set(BENCHMARK_DIR "~/Projects/benchmark")
include_directories("~/Projects/libvma")

include(FindBenchmark.cmake)
include_directories(${Benchmark_INCLUDE_DIR})

message("Benchmark_INCLUDE_DIR=" ${Benchmark_INCLUDE_DIR})
message("Benchmark_LIBRARIES=" ${Benchmark_LIBRARIES})

add_executable(benchtest benchtest.cpp)
target_link_libraries(benchtest "${Benchmark_LIBRARIES}")

include_directories("./sockperf")
file(GLOB sockperf_lib sockperf/*.cpp)

add_executable(UdpLatencyTest UdpLatencyTest.cpp ${sockperf_lib})
target_link_libraries(UdpLatencyTest dl)

add_executable(sockperf sockperf.cpp ${sockperf_lib})
target_link_libraries(sockperf dl)

add_executable(udpbench udpbench.cpp ${sockperf_lib})
target_link_libraries(udpbench "${Benchmark_LIBRARIES}")
target_link_libraries(udpbench dl)