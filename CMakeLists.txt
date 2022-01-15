cmake_minimum_required(VERSION 3.1)

project(cebus VERSION 1.0 LANGUAGES C)
set(cebus_VERSION 1.0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# enable unit testing via "make test" once the code has been compiled.
include(CTest)

option(CEBUS_BUILD_TESTS "Build tests" OFF)
option(CEBUS_THREAD_C11 "Use C11 threading library support" ON)
option(CEBUS_THREAD_POSIX "Use posix threading library" OFF)

if (CEBUS_THREAD_C11 AND CEBUS_THREAD_POSIX)
    message(FATAL_ERROR "CEBUS_THREAD_C11 and CEBUS_THREAD_POSIX can not be both enabled at the same time")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")

set(DEPS_ROOT ${CMAKE_SOURCE_DIR}/deps)

add_subdirectory(cebus-protoc-gen)
add_subdirectory(src)

if(CEBUS_BUILD_TESTS)
    add_subdirectory(tests)
endif()

set(cebus_INCLUDE_DEST "include/cebus-${cebus_VERSION}")
set(cebus_LIB_DEST "lib/cebus-${cebus_VERSION}")

# 
# install(
#     # targets to install
#     TARGETS cebus 
#     # name of the CMake "export group" containing the targets we want to install
#     EXPORT cebusTargets
#     # Dynamic, static library and include destination locations after running
#     # "make install"
#     LIBRARY DESTINATION ${cebus_LIB_DEST}
#     ARCHIVE DESTINATION ${cebus_LIB_DEST} 
#     INCLUDES DESTINATION ${cebus_INCLUDE_DEST}
#     )
# 
# install(
#     # The export we want to save (matches name defined above containing the
#     # install targets)
#     EXPORT cebusTargets
#     # CMake file in which to store the export's information
#     FILE  cebusTargets.cmake
#     # Namespace prepends all targets in the export (when we import later, we
#     # will use cebus::cebus)
#     NAMESPACE cebus::
#     # where to place the resulting file (here, we're putting it with the library)
#     DESTINATION ${cebus_LIB_DEST}
#     )
# 
# install(FILES ${cebus_INC} DESTINATION ${cebus_INCLUDE_DEST})
# 
# include(CMakePackageConfigHelpers)
# write_basic_package_version_file(
#     cebusConfigVersion.cmake
#     VERSION ${cebus_VERSION}
#     COMPATIBILITY AnyNewerVersion
#     )
# 
# install(
#     FILES 
#         "cmake/cebus-config.cmake"
#         "${CMAKE_CURRENT_BINARY_DIR}/cebusConfigVersion.cmake"
#     DESTINATION ${cebus_LIB_DEST}
# )

add_executable(cebus-main main.c)
target_link_libraries(cebus-main PRIVATE cebus)
