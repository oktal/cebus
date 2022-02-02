include(ProtobufCebus)

# Generate protobuf files

set(cebus_PROTOS
    bcl.proto
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
    atomic_c11.c
    binding_key.c
    bus.c
    hash_map.c
    hasher.c
    log.c
    message_context.c
    message_id.c
    message_type_id.c
    originator_info.c
    peer.c
    peer_id.c
    random.c
    string.c
    time.c
    timespan.c
    threading.c
    transport_message.c
    transport.c
    uuid.c

    zmq_inbound_socket.c
    zmq_outbound_socket.c
    zmq_socket_options.c
    zmq_transport.c

    $<$<PLATFORM_ID:Linux>:time_unix.c>

    threading.c
    $<$<BOOL:${CEBUS_THREAD_C11}>:threading_c11.c>
    $<$<BOOL:${CEBUS_THREAD_POSIX}>:threading_posix.c>
)


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

if (CEBUS_THREAD_C11)
    message(STATUS "Enabling C11 threading support")
    target_compile_definitions(cebus PUBLIC CEBUS_THREAD_C11)
elseif (CEBUS_THREAD_POSIX)
    message(STATUS "Enabling POSIX threading support")
    target_compile_definitions(cebus PUBLIC CEBUS_THREAD_POSIX)
else()
    message(STATUS "Enabling C11 threading support")
    target_compile_definitions(cebus PUBLIC CEBUS_THREAD_C11)
endif()

target_link_directories(cebus PUBLIC ${DEPS_ROOT}/lib)
target_link_libraries(cebus PUBLIC protobuf-cebus czmq zmq pthread uuid)
