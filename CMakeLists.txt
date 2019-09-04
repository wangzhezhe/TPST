project(WFserver)
cmake_minimum_required(VERSION 3.2)

add_compile_options(-std=c++11)

# GRPC and Protocol Buffers libraries location
list(APPEND CMAKE_PREFIX_PATH "~/cworkspace/build/build_grpc" "~/cworkspace/build/build_protobuf")

# Cmake find modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)

set(PROTOS
    ${CMAKE_CURRENT_SOURCE_DIR}/proto/workflowserver.proto
)

set(PROTO_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto-src)
file(MAKE_DIRECTORY ${PROTO_SRC_DIR})
include_directories(${PROTO_SRC_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC_DIR} ${PROTOS})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC_DIR} ${PROTOS})


# Building server
add_executable(workflowserver 
src/server/workflowserver.cc 
src/server/pubsub.cc 
src/utils/dlm/dlm.cc
src/utils/dht/dht.cc
src/utils/groupManager/groupManager.cpp
src/utils/coordinator/coordinator.cpp
src/utils/split/split.cc
src/utils/strparse/strparse.cpp
src/publishclient/pubsubclient.cc
${PROTO_SRCS}
${GRPC_SRCS}
)
target_link_libraries(workflowserver
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)
target_include_directories(workflowserver PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps>
)
