cmake_minimum_required(VERSION 3.10)
project(omd-handler VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall --param inline-unit-growth=200 -O3 -g")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)
message("EXECUTABLE_OUTPUT_PATH=" ${EXECUTABLE_OUTPUT_PATH})

include_directories(./include)

set(COMMON_SRC include/utils/ini.c include/utils/INIReader.cpp)
set(EASYLOGGING_SRC include/utils/easylogging++.cc OMDCPrinter.hpp)

configure_file(conf/omd.xml ${EXECUTABLE_OUTPUT_PATH}/omd.xml)
configure_file(conf/omd_dev.xml ${EXECUTABLE_OUTPUT_PATH}/omd_dev.xml)
file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/log)

#udpfeed
configure_file(conf/log_udpfeed.conf ${EXECUTABLE_OUTPUT_PATH}/conf/log_udpfeed.conf)
configure_file(conf/udpfeed.ini ${EXECUTABLE_OUTPUT_PATH}/conf/udpfeed.ini)

add_subdirectory(gtest)
add_subdirectory(test-throughput)
add_subdirectory(test-lowlatency)
#add_subdirectory(tools)
add_subdirectory(benchmark)

add_executable(udpfeed udpfeed.cpp ${COMMON_SRC} ${EASYLOGGING_SRC})

set(BOOST_ROOT "~/Projects/boost_1_65_0")
find_package(Boost COMPONENTS system thread)
if(Boost_FOUND)
    #    message("Boost_INCLUDE_DIRS=" ${Boost_INCLUDE_DIRS})
    #    message("Boost_LIBRARIES=" ${Boost_LIBRARIES})

    add_definitions(-DBOOST_LOG_DYN_LINK=1)
    add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)

    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(main main.cpp ${COMMON_SRC})
    target_link_libraries(main ${Boost_LIBRARIES})
    #
    #    add_executable(LockFree LockFree.cpp )
    #    target_link_libraries(LockFree ${Boost_LIBRARIES} )
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/hkex.out DESTINATION ${EXECUTABLE_OUTPUT_PATH})