cmake_minimum_required(VERSION 3.0)
project(protog C CXX)

# protobuf
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

# we'd like to have c++14. but do we really need it?
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

add_executable(protog src/protog.cpp)
target_link_libraries(protog ${PROTOBUF_LIBRARIES})
