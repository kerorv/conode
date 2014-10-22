cmake_minimum_required(VERSION 2.8)

project(tinyserver)
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/conode/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)
set(EXE_SRC tinyserver.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -g -O2 -pthread")
add_executable(tinyserver ${EXE_SRC})
target_link_libraries(tinyserver libconode.so)
