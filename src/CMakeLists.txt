include(Protobuf)

# Generate protobuf files

set(cebus_PROTOS
    binding_key.proto
    message_type_id.proto
    peer_descriptor.proto
    peer_id.proto
    peer.proto
    subscription.proto
)

protobuf_generate_c(PROTO_SRCS PROTO_HDRS PROTOS ${cebus_PROTOS})

set(cebus_SRC
    binding_key.c
    peer_id.c
)

set(DEPS_ROOT ${CMAKE_SOURCE_DIR}/deps)

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)

add_library(cebus STATIC ${cebus_SRC} ${PROTO_SRCS} ${PROTO_HDRS})
target_include_directories(cebus
  PUBLIC
    $<BUILD_INTERFACE:${PARENT_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>

    ${CMAKE_CURRENT_BINARY_DIR}
    ${DEPS_ROOT}/include

  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cebus PUBLIC)
