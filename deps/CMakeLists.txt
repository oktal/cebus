cmake_minimum_required(VERSION 3.1)

project(cebus-deps)

include(ExternalProject)

set(CZMQ_VERSION 4.2.1)
set(PROTOBUF_VERSION 3.12.4)
set(PROTOBUF_C_VERSION 1.4.0)

find_program(MAKE_BIN NAMES make gmake nmake)

ExternalProject_Add(
    czmq

    URL "https://github.com/zeromq/czmq/releases/download/v${CZMQ_VERSION}/czmq-${CZMQ_VERSION}.zip"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/czmq
    CMAKE_ARGS -DCZMQ_BUILD_STATIC=ON -DCMAKE_INSTALL_PREFIX=${CMAKE_SOURCE_DIR}
)

ExternalProject_Add(
    protobuf

    URL "https://github.com/protocolbuffers/protobuf/releases/download/v${PROTOBUF_VERSION}/protobuf-cpp-${PROTOBUF_VERSION}.zip"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/protobuf

    CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/protobuf/configure --prefix ${CMAKE_SOURCE_DIR}
    BUILD_COMMAND ${MAKE_BIN}
    INSTALL_COMMAND ${MAKE_BIN} install
)

ExternalProject_Add(
    protobuf-c

    URL "https://github.com/protobuf-c/protobuf-c/releases/download/v${PROTOBUF_C_VERSION}/protobuf-c-${PROTOBUF_C_VERSION}.tar.gz"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/protobuf-c


    CONFIGURE_COMMAND
        ${CMAKE_COMMAND} -E env CXXFLAGS=-I${CMAKE_SOURCE_DIR}/include
        ${CMAKE_COMMAND} -E env LDFLAGS=-L${CMAKE_SOURCE_DIR}/lib
        ${CMAKE_BINARY_DIR}/protobuf-c/configure --prefix ${CMAKE_SOURCE_DIR}
    BUILD_COMMAND ${MAKE_BIN}
    INSTALL_COMMAND ${MAKE_BIN} install

    DEPENDS protobuf
)
