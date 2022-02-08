include(ProtobufCebus)

function(cebus_test test_name)
    set(TEST_EXECUTABLE ${test_name})
    set(TEST_SOURCE ${test_name}.c)

    add_executable(${TEST_EXECUTABLE} ${TEST_SOURCE} munit.c)
    target_include_directories(${TEST_EXECUTABLE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${TEST_EXECUTABLE} PRIVATE cebus)
    add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${TEST_EXECUTABLE}>)
endfunction()

function(cebus_test_proto test_name proto_name)
    set(TEST_EXECUTABLE ${test_name})
    set(TEST_SOURCE ${test_name}.c)

    set(TEST_PROTO ${proto_name}.proto)

    protobuf_generate_c(TEST_PROTO_SRCS TEST_PROTO_HDRS PROTOS ${TEST_PROTO})

    # message(FATAL_ERROR "${TEST_PROTO_SRCS}")

    add_executable(${TEST_EXECUTABLE} ${TEST_SOURCE} ${TEST_PROTO_SRCS} munit.c)
    target_include_directories(${TEST_EXECUTABLE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(${TEST_EXECUTABLE} PRIVATE cebus)
    add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${TEST_EXECUTABLE}>)
endfunction()

cebus_test_proto(binding_key_test test_routable_event)
cebus_test(hasher_test)
cebus_test(hash_map_test)
cebus_test(peer_id_test)
cebus_test(peer_subscription_tree_test)
cebus_test(string_test)
cebus_test(timespan_test)
cebus_test(time_test)
