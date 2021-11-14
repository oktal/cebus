include(Protobuf)

# Generate protobuf files

set(cebus_PROTOS
    binding_key.proto
    message_id.proto
    message_type_id.proto
    originator_info.proto
    peer_descriptor.proto
    peer_id.proto
    peer.proto
    register_peer_command.proto
    subscription.proto
    transport_message.proto
    uuid.proto
)

protobuf_generate_c(PROTO_SRCS PROTO_HDRS PROTOS ${cebus_PROTOS})

set(cebus_SRC
    alloc.c
    binding_key.c
    message_context.c
    message_id.c
    message_type_id.c
    originator_info.c
    peer.c
    peer_id.c
    timespan.c
    transport_message.c
    zmq_outbound_socket.c
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

target_link_directories(cebus PUBLIC ${DEPS_ROOT}/lib)
target_link_libraries(cebus PUBLIC czmq protobuf-c)
