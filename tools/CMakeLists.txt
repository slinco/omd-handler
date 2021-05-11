set(EXECUTABLE_OUTPUT_PATH ../build/bin)

include_directories(../include)
include(FindPCAP.cmake)

configure_file(rt-omdc-ft.conf ${EXECUTABLE_OUTPUT_PATH}/rt-omdc-ft.conf)

add_definitions(-DBOOST_LOG_DYN_LINK=1)
add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)
find_package(Boost REQUIRED system thread timer chrono log log_setup filesystem)
if(Boost_FOUND)

    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(openomd_tools openomd_tools.cpp)
    target_link_libraries(openomd_tools pthread ${PCAP_LIBRARY} ${Boost_INCLUDE_DIRS})

    set_property(TARGET openomd_tools PROPERTY CXX_STANDARD 14)
endif()