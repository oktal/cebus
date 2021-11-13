project(protobuf-c VERSION 1.0)

set(Protobuf_INCLUDE_DIR ${DEPS_ROOT}/include)
set(Protobuf_PROTOC_EXECUTABLE ${DEPS_ROOT}/bin/protoc)
find_package(Protobuf REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS protobuf-c.proto)

add_library(protobuf-c ${PROTO_SRCS} ${PROTO_HDRS})

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)

target_include_directories(protobuf-c
  PUBLIC
      ${PARENT_DIR}
      ${Protobuf_INCLUDE_DIRS}
      ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_directories(protobuf-c
  PUBLIC
    ${DEPS_ROOT}/lib
)

target_link_libraries(protobuf-c
  PUBLIC
    protobuf
)
