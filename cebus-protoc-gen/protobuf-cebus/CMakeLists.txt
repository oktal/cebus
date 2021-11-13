project(protobuf-cebus VERSION 1.0)

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)

set(Protobuf_INCLUDE_DIR ${DEPS_ROOT}/include)
set(Protobuf_PROTOC_EXECUTABLE ${DEPS_ROOT}/bin/protoc)
set(Protobuf_IMPORT_DIRS ${PARENT_DIR})

find_package(Protobuf REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS cebus.proto)

add_library(protobuf-cebus ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(protobuf-cebus
  PUBLIC
      ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_directories(protobuf-cebus
  PUBLIC
    ${DEPS_ROOT}/lib
)

target_link_libraries(protobuf-cebus
  PUBLIC
    protobuf
    protobuf-c
)
