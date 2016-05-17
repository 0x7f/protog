include(ExternalProject)

# required to find generated protobuf source files
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# TODO: find yajl dependency

# gtest dependency
externalproject_add(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    CMAKE_ARGS -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG:PATH=DebugLibs
               -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE:PATH=ReleaseLibs
               -DCMAKE_CXX_FLAGS=${MSVC_COMPILER_DEFS}
               -Dgtest_force_shared_crt=ON
               -DBUILD_GTEST=ON
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
    UPDATE_COMMAND "" # disable further updates
    INSTALL_COMMAND "" # disables install step
)

externalproject_get_property(googletest source_dir)
include_directories(${source_dir}/googletest/include)

externalproject_get_property(googletest binary_dir)
set(GTEST_LIB_DIR ${binary_dir}/googlemock/gtest)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${binary_dir}/googlemock/gtest)

macro(ADD_PROTO PROTO_FILE)
    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE}.proto)
    list(APPEND TEST_SRC_FILES ${PROTO_SRCS} ${PROTO_HDRS})
endmacro()

macro(ADD_PARSER PROTO_FILE PROTO_MSG)
    string(TOLOWER ${PROTO_MSG} PROTO_MSG_LOW)
    add_custom_command(
            OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_MSG_LOW}_parser.pb.cc
            ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_MSG_LOW}_parser.pb.h
            COMMAND
            ${CMAKE_BINARY_DIR}/protog
            -p ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_FILE}.proto
            -i ${PROTO_FILE}.pb.h
            -m protog.test.${PROTO_MSG}
            -o .
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS protog
    )
    list(APPEND TEST_SRC_FILES
            ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_MSG_LOW}_parser.pb.cc
            ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_MSG_LOW}_parser.pb.h)
endmacro()

file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/test_*.cpp)

add_proto(messages)
add_parser(messages SimpleMessage)
add_parser(messages NestedMessage)

add_executable(protog_test ${TEST_SRC_FILES})
target_link_libraries(protog_test
    yajl
    ${PROTOBUF_LIBRARIES}
    ${GTEST_LIB_DIR}/libgtest.a
    ${GTEST_LIB_DIR}/libgtest_main.a
    m pthread)
