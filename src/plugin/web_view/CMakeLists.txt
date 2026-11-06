# web_view plugin (web_view.plg) and its CEF subprocess (neogfx_cef), using CEF
find_path(NEOGFX_CEF_INCLUDE_DIR include/cef_app.h PATH_SUFFIXES cef)
find_library(NEOGFX_CEF_LIBRARY NAMES libcef cef)
find_library(NEOGFX_CEF_WRAPPER_LIBRARY NAMES libcef_dll_wrapper cef_dll_wrapper)
if(NOT NEOGFX_CEF_INCLUDE_DIR OR NOT NEOGFX_CEF_LIBRARY OR NOT NEOGFX_CEF_WRAPPER_LIBRARY)
    message(FATAL_ERROR "neoGFX: CEF not found (set NEOGFX_CEF_INCLUDE_DIR, NEOGFX_CEF_LIBRARY and NEOGFX_CEF_WRAPPER_LIBRARY)")
endif()
# CEF is included both as <cef/include/...> and (by CEF's own headers) as "include/..."
get_filename_component(_neogfx_cef_parent_dir "${NEOGFX_CEF_INCLUDE_DIR}" DIRECTORY)
set(_neogfx_cef_include_dirs "${_neogfx_cef_parent_dir}" "${NEOGFX_CEF_INCLUDE_DIR}")

neogfx_add_plugin(web_view web_view .plg
    src/plugin.cpp
    src/web_view_canvas.cpp
    src/web_view_plugin.cpp
    hdr/web_view_canvas.hpp
    hdr/web_view_plugin.hpp)
if(WIN32)
    target_sources(web_view PRIVATE src/dllmain.cpp)
endif()
target_include_directories(web_view PRIVATE hdr ${_neogfx_cef_include_dirs})
target_compile_definitions(web_view PRIVATE NEOGFX_PLUGIN _SCL_SECURE_NO_WARNINGS DEFAULT_EXPORTS)
target_link_libraries(web_view PRIVATE neogfx::neogfx "${NEOGFX_CEF_LIBRARY}" "${NEOGFX_CEF_WRAPPER_LIBRARY}")

add_executable(neogfx_cef src/neogfx_cef.cpp)
neogfx_gui_executable(neogfx_cef)
target_include_directories(neogfx_cef PRIVATE ${_neogfx_cef_include_dirs})
target_link_libraries(neogfx_cef PRIVATE Boost::program_options "${NEOGFX_CEF_LIBRARY}" "${NEOGFX_CEF_WRAPPER_LIBRARY}")
add_dependencies(web_view neogfx_cef)
install(TARGETS neogfx_cef RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
