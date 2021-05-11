include_directories(../include)

include(FindPCAP.cmake)

add_executable(runUnitTests gtest.cpp omdcparsertest.cpp)
target_link_libraries(runUnitTests gtest pthread ${PCAP_LIBRARY})

add_executable(binarytest binarytest.cpp)
