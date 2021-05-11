
add_executable(udpreceiver1 udpreceiver1.c net.c)
add_executable(udpsender udpsender.c net.c)

find_package(Threads REQUIRED)
if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(udpreceiver1  "${CMAKE_THREAD_LIBS_INIT}")
    target_link_libraries(udpsender  "${CMAKE_THREAD_LIBS_INIT}")
endif()