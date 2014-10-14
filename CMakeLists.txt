cmake_minimum_required(VERSION 2.8)

project(conode)
include_directories(/usr/include/lua5.2 ${CMAKE_SOURCE_DIR}/include)
link_directories(/usr/lib/x86_64-linux-gnu ${CMAKE_SOURCE_DIR}/lib)
set(LIB_SRC capi.cpp scheduler.cpp worker.cpp node.cpp idallocator.cpp timerservice.cpp cnodemodule.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -g -O2 -pthread")
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)
add_library(conode SHARED ${LIB_SRC})
target_link_libraries(conode liblua5.2.so dl)

project(tinyserver)
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)
set(EXE_SRC main.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -g -O2 -pthread")
add_executable(tinyserver ${EXE_SRC})
target_link_libraries(tinyserver libconode.so)

