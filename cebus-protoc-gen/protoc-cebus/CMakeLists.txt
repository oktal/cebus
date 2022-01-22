find_package(Threads REQUIRED)

set(PROTOC_CEBUS_SRC
    field_generator.cc
    file_generator.cc
    message_generator.cc
    main.cc
)

add_executable(protoc-cebus-gen ${PROTOC_CEBUS_SRC})
target_compile_features(protoc-cebus-gen PRIVATE cxx_std_17)

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)
target_include_directories(protoc-cebus-gen
    PUBLIC ${DEPS_ROOT}/include
    PRIVATE ${PARENT_DIR}
)

target_link_directories(protoc-cebus-gen PUBLIC ${DEPS_ROOT}/lib)
target_link_libraries(protoc-cebus-gen
  PRIVATE
    protoc
    protoc-c
    protobuf-cebus
    Threads::Threads
)
