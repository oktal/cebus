find_program(
    PROTOC_BIN

    NAMES protoc
    PATHS ${DEPS_ROOT}
    PATH_SUFFIXES bin
)

function(protobuf_generate_c SRCS HDRS)
  set(_options)
  set(_singleargs PROTOC_OUT_DIR)
  set(_multiargs PROTOS IMPORT_DIRS)

  cmake_parse_arguments(protobuf_generate_c "${_options}" "${_singleargs}" "${_multiargs}" "${ARGN}")

  if(NOT protobuf_generate_c_PROTOS)
    message(SEND_ERROR "Error: protobuf_generate_c called without any source files")
    return()
  endif()

  if(NOT protobuf_generate_c_PROTOC_OUT_DIR)
    set(protobuf_generate_c_PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  set(protobuf_generate_c_GENERATE_EXTENSIONS .pb-c.h .pb-c.c)

  set(_protobuf_include_path -I${CMAKE_CURRENT_SOURCE_DIR})

  set(_generated_srcs_all)
  foreach(_proto ${protobuf_generate_c_PROTOS})
    get_filename_component(_abs_file ${_proto} ABSOLUTE)
    get_filename_component(_abs_dir ${_abs_file} DIRECTORY)
    get_filename_component(_basename ${_proto} NAME_WLE)
    file(RELATIVE_PATH _rel_dir ${CMAKE_CURRENT_SOURCE_DIR} ${_abs_dir})

    set(_possible_rel_dir)
    if (NOT protobuf_generate_c_APPEND_PATH)
        set(_possible_rel_dir ${_rel_dir}/)
    endif()

    set(_generated_srcs)
    foreach(_ext ${protobuf_generate_c_GENERATE_EXTENSIONS})
      list(APPEND _generated_srcs "${protobuf_generate_c_PROTOC_OUT_DIR}/${_possible_rel_dir}${_basename}${_ext}")
    endforeach()

    list(APPEND _generated_srcs_all ${_generated_srcs})

    add_custom_command(
      OUTPUT ${_generated_srcs}
      COMMAND  ${PROTOC_BIN}
               --proto_path=${CMAKE_SOURCE_DIR}/cebus-protoc-gen/protobuf-c
               --proto_path=${CMAKE_SOURCE_DIR}/cebus-protoc-gen/protobuf-cebus
               --plugin=protoc-gen-cebus=$<TARGET_FILE:cebus-protoc-gen>
               --cebus_out=${protobuf_generate_c_PROTOC_OUT_DIR}
               ${_protobuf_include_path} ${_abs_file}
      DEPENDS ${_abs_file} ${PROTOC_BIN} $<TARGET_FILE:cebus-protoc-gen>
      COMMENT "Running C protocol buffer compiler on ${_proto}"
      VERBATIM )
  endforeach()

  set_source_files_properties(${_generated_srcs_all} PROPERTIES GENERATED TRUE)
  set(${SRCS})
  set(${HDRS})

  foreach(_file ${_generated_srcs_all})
    if(_file MATCHES "c$")
      list(APPEND ${SRCS} ${_file})
    else()
      list(APPEND ${HDRS} ${_file})
    endif()
  endforeach()
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()
