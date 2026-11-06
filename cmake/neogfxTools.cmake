# neogfxTools.cmake
# Build helpers for neoGFX and for projects that use an installed neoGFX:
#
#   neogfx_add_resources(<target> <file.nrc>...)
#     Runs nrc (the neoGFX resource compiler) on each .nrc file and adds the
#     generated <stem>.res.cpp and <stem>.ui.hpp to <target>; the directory
#     they are generated in is added to <target>'s include path.
#
#   neogfx_add_source_strings(<target> NAMESPACE <ns> SOURCES <file> <symbol> [<file> <symbol>]...)
#     Runs src2string on each file, generating <file name>.hpp, which defines
#     the file's text as "const char* const <symbol>" in namespace <ns>.

include_guard(GLOBAL)

function(_neogfx_tool OUT_VAR NAME)
    if(TARGET neogfx::${NAME})
        set(${OUT_VAR} neogfx::${NAME} PARENT_SCOPE)
    elseif(TARGET ${NAME})
        set(${OUT_VAR} ${NAME} PARENT_SCOPE)
    else()
        message(FATAL_ERROR "neoGFX: the ${NAME} tool is not available (build it, or find_package(neogfx))")
    endif()
endfunction()

function(_neogfx_generated_dir OUT_VAR TARGET)
    set(${OUT_VAR} "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_GeneratedFiles" PARENT_SCOPE)
endfunction()

# The files a .nrc embeds ("file: <path>" and "files: [ <path>... ]"), relative to
# the .nrc file's directory, so that editing one of them re-runs nrc.
function(_neogfx_nrc_inputs NRC_FILE OUT_VAR)
    file(READ "${NRC_FILE}" _text)
    get_filename_component(_dir "${NRC_FILE}" DIRECTORY)
    set(_paths)
    string(REGEX MATCHALL "(^|[\r\n\t {])file[ \t]*:[^\r\n]*" _single "${_text}")
    foreach(_item IN LISTS _single)
        string(REGEX REPLACE "^[\r\n\t {]*file[ \t]*:[ \t]*" "" _item "${_item}")
        list(APPEND _paths "${_item}")
    endforeach()
    string(REGEX MATCHALL "files[ \t]*:[ \t]*\\[[^]]*\\]" _multi "${_text}")
    foreach(_item IN LISTS _multi)
        string(REGEX REPLACE "^files[ \t]*:[ \t]*\\[" "" _item "${_item}")
        string(REGEX REPLACE "\\]$" "" _item "${_item}")
        string(REGEX REPLACE "[\r\n]+" ";" _item "${_item}")
        list(APPEND _paths ${_item})
    endforeach()
    set(_inputs)
    foreach(_path IN LISTS _paths)
        string(STRIP "${_path}" _path)
        string(REGEX REPLACE "^\"(.*)\"$" "\\1" _path "${_path}")
        # (without any query parameters for the resource, e.g. "?themable=true")
        string(REGEX REPLACE "\\?.*$" "" _path "${_path}")
        if(_path STREQUAL "")
            continue()
        endif()
        if(NOT IS_ABSOLUTE "${_path}")
            set(_path "${_dir}/${_path}")
        endif()
        if(EXISTS "${_path}")
            list(APPEND _inputs "${_path}")
        endif()
    endforeach()
    set(${OUT_VAR} ${_inputs} PARENT_SCOPE)
endfunction()

function(neogfx_add_resources TARGET)
    _neogfx_tool(_nrc nrc)
    _neogfx_generated_dir(_out_dir ${TARGET})
    foreach(_nrc_file IN LISTS ARGN)
        get_filename_component(_input "${_nrc_file}" ABSOLUTE)
        get_filename_component(_input_dir "${_input}" DIRECTORY)
        get_filename_component(_stem "${_input}" NAME_WLE)
        set(_res "${_out_dir}/${_stem}.res.cpp")
        set(_ui "${_out_dir}/${_stem}.ui.hpp")
        _neogfx_nrc_inputs("${_input}" _embedded)
        # re-scan the embedded file list when the .nrc changes
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_input}")
        # nrc writes <stem>.res.cpp only if the .nrc has a resource section and
        # <stem>.ui.hpp only if it has a ui section; the touch creates whichever
        # is missing (empty) so that both outputs always exist.
        add_custom_command(
            OUTPUT "${_res}" "${_ui}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${_out_dir}"
            COMMAND ${_nrc} "${_input}" "${_out_dir}"
            COMMAND "${CMAKE_COMMAND}" -E touch "${_res}" "${_ui}"
            DEPENDS ${_nrc} "${_input}" ${_embedded}
            WORKING_DIRECTORY "${_input_dir}"
            COMMENT "nrc ${_nrc_file}"
            VERBATIM)
        target_sources(${TARGET} PRIVATE "${_input}" "${_res}" "${_ui}")
    endforeach()
    target_include_directories(${TARGET} PRIVATE "${_out_dir}")
endfunction()

function(neogfx_add_source_strings TARGET)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "NAMESPACE" "SOURCES")
    if(NOT arg_NAMESPACE OR NOT arg_SOURCES)
        message(FATAL_ERROR "neogfx_add_source_strings: NAMESPACE and SOURCES are required")
    endif()
    list(LENGTH arg_SOURCES _count)
    math(EXPR _odd "${_count} % 2")
    if(NOT _odd EQUAL 0)
        message(FATAL_ERROR "neogfx_add_source_strings: SOURCES must be <file> <symbol> pairs")
    endif()
    _neogfx_tool(_src2string src2string)
    _neogfx_generated_dir(_out_dir ${TARGET})
    math(EXPR _last "${_count} - 1")
    foreach(_index RANGE 0 ${_last} 2)
        math(EXPR _symbol_index "${_index} + 1")
        list(GET arg_SOURCES ${_index} _source)
        list(GET arg_SOURCES ${_symbol_index} _symbol)
        get_filename_component(_input "${_source}" ABSOLUTE)
        get_filename_component(_name "${_input}" NAME)
        set(_output "${_out_dir}/${_name}.hpp")
        add_custom_command(
            OUTPUT "${_output}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${_out_dir}"
            COMMAND ${_src2string} "${_input}" "${_output}" "${arg_NAMESPACE}" "${_symbol}"
            DEPENDS ${_src2string} "${_input}"
            COMMENT "src2string ${_source}"
            VERBATIM)
        target_sources(${TARGET} PRIVATE "${_input}" "${_output}")
    endforeach()
    target_include_directories(${TARGET} PRIVATE "${_out_dir}")
endfunction()
