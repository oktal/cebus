find_package(Threads REQUIRED)

set(cebus_PROTOC_SRC
    c_bytes_field.cc
    c_enum.cc
    c_enum_field.cc
    c_extension.cc
    c_field.cc
    c_file.cc
    c_generator.cc
    c_helpers.cc
    c_message.cc
    c_message_field.cc
    c_primitive_field.cc
    c_service.cc
    c_string_field.cc
    main.cc
)

add_executable(cebus-protoc-gen ${cebus_PROTOC_SRC})

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)
target_include_directories(cebus-protoc-gen
    PUBLIC ${DEPS_ROOT}/include
    PRIVATE ${PARENT_DIR}
)

target_link_directories(cebus-protoc-gen PUBLIC ${DEPS_ROOT}/lib)
target_link_libraries(cebus-protoc-gen
  PRIVATE
    protobuf-cebus
    protoc

    Threads::Threads
)
