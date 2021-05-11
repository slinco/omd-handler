add_executable(udpclient udpclient.c net.c)
add_executable(udpserver udpserver.c net.c)

find_package(Threads REQUIRED)
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(udpclient  "${CMAKE_THREAD_LIBS_INIT}")
  target_link_libraries(udpclient  m)

  target_link_libraries(udpserver  "${CMAKE_THREAD_LIBS_INIT}")
endif()