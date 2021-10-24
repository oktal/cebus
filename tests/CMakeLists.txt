function(cebus_test test_name)
    set(TEST_EXECUTABLE test_${test_name})
    set(TEST_SOURCE ${test_name}.c)

    add_executable(${TEST_EXECUTABLE} ${TEST_SOURCE} munit.c)
    target_include_directories(${TEST_EXECUTABLE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${TEST_EXECUTABLE} PRIVATE cebus)
    add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${TEST_EXECUTABLE}>)
endfunction()

cebus_test(binding_key_test)
cebus_test(peer_id_test)
