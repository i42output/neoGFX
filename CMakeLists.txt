cmake_minimum_required(VERSION 3.24)
cmake_policy(VERSION 3.24)

# meta
project(neogfx VERSION 0.0.1 LANGUAGES C CXX)

# deployment
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Default install prefix" FORCE)
endif()

include(GNUInstallDirs)
set(CMAKE_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

# options
option(NEOGFX_BUILD_DESIGN_STUDIO "Build neoGFX Design Studio and its default element library (requires libssh)" ON)
option(NEOGFX_BUILD_NEOS_ELEMENT_LIBRARY "Build Design Studio's neos element library (requires neos)" OFF)
option(NEOGFX_BUILD_WEB_VIEW_PLUGIN "Build the web_view plugin and neogfx_cef (requires CEF)" OFF)
option(NEOGFX_BUILD_TESTS "Build the neoGFX GUI test app" ON)
option(NEOGFX_BUILD_EXAMPLES "Build the example apps (video_poker, chess, mod_tracker)" OFF)
option(NEOGFX_DEBUG "Define NEOGFX_DEBUG (the Visual Studio projects define it in every configuration)" ON)
set(NEOGFX_BUILD_CORES "" CACHE STRING "Number of cores to use for concurrent compilation (empty or 0: all cores)")
if(NOT NEOGFX_BUILD_CORES STREQUAL "" AND NOT NEOGFX_BUILD_CORES MATCHES "^[0-9]+$")
    message(FATAL_ERROR "NEOGFX_BUILD_CORES must be a whole number (empty or 0: all cores)")
endif()
if(NEOGFX_BUILD_CORES EQUAL 0)
    set(_neogfx_build_cores "")
else()
    set(_neogfx_build_cores "${NEOGFX_BUILD_CORES}")
endif()

# compiler support
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

if(MSVC)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/std:c++latest> /bigobj)
    if(CMAKE_GENERATOR MATCHES "Visual Studio")
        add_compile_options(/MP${_neogfx_build_cores})
    endif()
    add_compile_definitions(UNICODE _UNICODE)
endif()
if(_neogfx_build_cores AND CMAKE_GENERATOR MATCHES "Ninja")
    set_property(GLOBAL APPEND PROPERTY JOB_POOLS neogfx_compile=${_neogfx_build_cores})
    set(CMAKE_JOB_POOL_COMPILE neogfx_compile)
endif()

# static C runtime (as neolib)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# build tree layout: programs and the plugins they load (.nrl, .nel, .plg) side by side
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

# helpers
include(cmake/neogfxTools.cmake)

# plugin (module) libraries: <name><suffix>, no "lib" prefix or debug postfix, installed with the programs
function(neogfx_add_plugin TARGET OUTPUT_NAME SUFFIX)
    add_library(${TARGET} MODULE ${ARGN})
    set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME "${OUTPUT_NAME}" PREFIX "" SUFFIX "${SUFFIX}")
    install(TARGETS ${TARGET} LIBRARY DESTINATION "${CMAKE_INSTALL_BINDIR}" RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endfunction()

# programs that use main() but are GUI (no console) apps on Windows
function(neogfx_gui_executable TARGET)
    if(WIN32)
        set_property(TARGET ${TARGET} PROPERTY WIN32_EXECUTABLE ON)
        if(MSVC)
            target_link_options(${TARGET} PRIVATE /ENTRY:mainCRTStartup)
        endif()
    endif()
endfunction()

# dependencies
include(cmake/neogfxDependencies.cmake)

set(NEOGFX_3RDPARTY_LIBDIR "${PROJECT_SOURCE_DIR}/3rdparty/lib")

# libraries found by name (prebuilt in 3rdparty/lib on Windows), linked by path
function(neogfx_find_library VAR)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "NAMES;DEBUG_NAMES;HINTS")
    find_library(${VAR}_RELEASE NAMES ${arg_NAMES} HINTS ${arg_HINTS})
    if(arg_DEBUG_NAMES)
        find_library(${VAR}_DEBUG NAMES ${arg_DEBUG_NAMES} HINTS ${arg_HINTS})
    endif()
    if(NOT ${VAR}_RELEASE)
        message(FATAL_ERROR "neoGFX: library not found (${arg_NAMES}); set ${VAR}_RELEASE")
    endif()
    if(${VAR}_DEBUG)
        set(${VAR} "$<$<CONFIG:Debug>:${${VAR}_DEBUG}>" "$<$<NOT:$<CONFIG:Debug>>:${${VAR}_RELEASE}>" PARENT_SCOPE)
    else()
        set(${VAR} "${${VAR}_RELEASE}" PARENT_SCOPE)
    endif()
endfunction()

# libpng (included as <libpng/png.h>)
find_path(NEOGFX_PNG_INCLUDE_DIR libpng/png.h
    HINTS "${PROJECT_SOURCE_DIR}/3rdparty/libpng/libpng-1.6.21/include")
neogfx_find_library(NEOGFX_PNG_LIBRARY
    NAMES libpng16_static png16_static libpng16 png16 png
    DEBUG_NAMES libpng16_staticd png16_staticd libpng16d png16d
    HINTS "${NEOGFX_3RDPARTY_LIBDIR}")

# GLEW (static)
find_path(NEOGFX_GLEW_INCLUDE_DIR GL/glew.h HINTS "${PROJECT_SOURCE_DIR}/3rdparty/glew/glew-2.1.0/include")
neogfx_find_library(NEOGFX_GLEW_LIBRARY
    NAMES libglew32 glew32s GLEW glew32 glew
    DEBUG_NAMES libglew32d glew32sd
    HINTS "${NEOGFX_3RDPARTY_LIBDIR}")

# shaderc (shared, since neoGFX uses the static C runtime)
get_filename_component(_neogfx_vulkan_libdir "${Vulkan_LIBRARY}" DIRECTORY)
neogfx_find_library(NEOGFX_SHADERC_LIBRARY
    NAMES shaderc_shared
    HINTS "${_neogfx_vulkan_libdir}" "$ENV{VULKAN_SDK}/Lib" "$ENV{VULKAN_SDK}/lib")

foreach(_dir IN ITEMS NEOGFX_PNG_INCLUDE_DIR NEOGFX_GLEW_INCLUDE_DIR)
    if(NOT ${_dir})
        message(FATAL_ERROR "neoGFX: ${_dir} not found")
    endif()
endforeach()

# tools needed to build neoGFX (they do not link neoGFX)
add_subdirectory(tools/src2string)
add_subdirectory(tools/nrc)

# neogfx
set(NEOGFX_SOURCE_FILES
    src/app/action.cpp
    src/app/app.cpp
    src/app/clipboard.cpp
    src/app/drag_drop.cpp
    src/app/event_processing_context.cpp
    src/app/file_dialog.cpp
    src/app/i18n.cpp
    src/app/module_resource.cpp
    src/app/native/3rdparty/tinyfiledialogs.c
    src/app/palette.cpp
    src/app/resource.cpp
    src/app/settings.cpp
    src/app/style.cpp
    src/audio/3rdparty/smb/smbPitchShift.cpp
    src/audio/audio.cpp
    src/audio/audio_bitstream.cpp
    src/audio/audio_clip.cpp
    src/audio/audio_device.cpp
    src/audio/audio_instrument.cpp
    src/audio/audio_instrument_atlas.cpp
    src/audio/audio_oscillator.cpp
    src/audio/audio_sample.cpp
    src/audio/audio_waveform.cpp
    src/core/async_task.cpp
    src/core/async_thread.cpp
    src/core/units.cpp
    src/game/animator.cpp
    src/game/canvas.cpp
    src/game/collision_detector.cpp
    src/game/ecs.cpp
    src/game/game_world.cpp
    src/game/rectangle.cpp
    src/game/renderable_entity_archetype.cpp
    src/game/scene_graph_canvas.cpp
    src/game/sequencer.cpp
    src/game/simple_physics.cpp
    src/game/text_mesh.cpp
    src/gfx/color.cpp
    src/gfx/fragment_shader.cpp
    src/gfx/gltf.cpp
    src/gfx/gradient.cpp
    src/gfx/graphics_context.cpp
    src/gfx/graphics_operations.cpp
    src/gfx/hsl_color.cpp
    src/gfx/hsv_color.cpp
    src/gfx/image.cpp
    src/gfx/native/native_buffer.cpp
    src/gfx/native/native_renderer.cpp
    src/gfx/native/native_rendering_context.cpp
    src/gfx/native/native_texture.cpp
    src/gfx/native/native_triangle_renderer.cpp
    src/gfx/native/native_vertex.cpp
    src/gfx/native/opengl/opengl_error.cpp
    src/gfx/native/opengl/opengl_graphics_backend.cpp
    src/gfx/native/opengl/opengl_shader_program.cpp
    src/gfx/native/opengl/opengl_surface.cpp
    src/gfx/native/opengl/opengl_texture.cpp
    src/gfx/native/opengl/opengl_texture_manager.cpp
    src/gfx/native/vulkan/vulkan_error.cpp
    src/gfx/native/vulkan/vulkan_glsl.cpp
    src/gfx/native/vulkan/vulkan_graphics_backend.cpp
    src/gfx/native/vulkan/vulkan_shader_program.cpp
    src/gfx/native/vulkan/vulkan_surface.cpp
    src/gfx/native/vulkan/vulkan_texture.cpp
    src/gfx/native/vulkan/vulkan_texture_manager.cpp
    src/gfx/rect_pack.cpp
    src/gfx/render_target.cpp
    src/gfx/scene_graph.cpp
    src/gfx/standard_shader_program.cpp
    src/gfx/sub_texture.cpp
    src/gfx/text/emoji_atlas.cpp
    src/gfx/text/font.cpp
    src/gfx/text/font_manager.cpp
    src/gfx/text/glyph.cpp
    src/gfx/text/glyph_text.cpp
    src/gfx/text/math_layout.cpp
    src/gfx/text/native/native_font.cpp
    src/gfx/text/native/native_font_face.cpp
    src/gfx/texture.cpp
    src/gfx/texture_atlas.cpp
    src/gfx/texture_manager.cpp
    src/gfx/utility.cpp
    src/gfx/vertex_shader.cpp
    src/gui/widget/native_widget.cpp
    src/gui/widget/timer.cpp
    src/gui/window/native/native_surface.cpp
    src/gui/window/native/native_window.cpp
    src/gui/window/native/virtual_surface.cpp
    src/gui/window/native/virtual_window.cpp
    src/hid/display.cpp
    src/hid/game_controller.cpp
    src/hid/game_controllers.cpp
    src/hid/hid_devices.cpp
    src/hid/keyboard.cpp
    src/hid/mouse.cpp
    src/hid/nest.cpp
    src/hid/surface_manager.cpp
    src/hid/surface_window.cpp
    src/neogfx.cpp)
if(WIN32)
    list(APPEND NEOGFX_SOURCE_FILES
        src/app/native/windows_accessibility.cpp
        src/app/native/windows_basic_services.cpp
        src/app/native/windows_drag_drop.cpp
        src/app/native/windows_services.cpp
        src/gfx/native/windows_renderer.cpp
        src/gui/widget/native/windows_skin.cpp
        src/gui/window/native/windows_window.cpp
        src/hid/native/windows_directinput_controller.cpp
        src/hid/native/windows_display.cpp
        src/hid/native/windows_game_controllers.cpp
        src/hid/native/windows_hid_devices.cpp
        src/hid/native/windows_keyboard.cpp
        src/hid/native/windows_mouse.cpp
        src/hid/native/windows_window_manager.cpp
        src/hid/native/windows_xinput_controller.cpp)
endif()

set(NEOGFX_WIDGET_SOURCE_FILES
    src/app/resource_manager.cpp
    src/core/html.cpp
    src/core/style_sheet.cpp
    src/core/transition_animator.cpp
    src/gfx/gradient_manager.cpp
    src/gfx/shapes.cpp
    src/gfx/text/emoticon_translator.cpp
    src/gfx/view.cpp
    src/gui/dialog/color_dialog.cpp
    src/gui/dialog/dialog.cpp
    src/gui/dialog/dialog_button_box.cpp
    src/gui/dialog/font_dialog.cpp
    src/gui/dialog/game_controller_dialog.cpp
    src/gui/dialog/gradient_dialog.cpp
    src/gui/dialog/language_dialog.cpp
    src/gui/dialog/message_box.cpp
    src/gui/dialog/paragraph_dialog.cpp
    src/gui/dialog/settings_dialog.cpp
    src/gui/layout/async_layout.cpp
    src/gui/layout/border_layout.cpp
    src/gui/layout/flow_layout.cpp
    src/gui/layout/grid_layout.cpp
    src/gui/layout/horizontal_layout.cpp
    src/gui/layout/layout.cpp
    src/gui/layout/spacer.cpp
    src/gui/layout/stack_layout.cpp
    src/gui/layout/vertical_layout.cpp
    src/gui/mvc/controller.cpp
    src/gui/mvc/view.cpp
    src/gui/mvc/view_container.cpp
    src/gui/widget/button.cpp
    src/gui/widget/check_box.cpp
    src/gui/widget/color_widget.cpp
    src/gui/widget/cursor.cpp
    src/gui/widget/default_skin.cpp
    src/gui/widget/dock.cpp
    src/gui/widget/dockable.cpp
    src/gui/widget/drop_list.cpp
    src/gui/widget/font_widget.cpp
    src/gui/widget/framed_widget.cpp
    src/gui/widget/gradient_widget.cpp
    src/gui/widget/group_box.cpp
    src/gui/widget/header_view.cpp
    src/gui/widget/image_widget.cpp
    src/gui/widget/item_view.cpp
    src/gui/widget/label.cpp
    src/gui/widget/line_edit.cpp
    src/gui/widget/list_view.cpp
    src/gui/widget/menu.cpp
    src/gui/widget/menu_bar.cpp
    src/gui/widget/menu_item.cpp
    src/gui/widget/menu_item_widget.cpp
    src/gui/widget/neotif_skin.cpp
    src/gui/widget/normal_title_bar.cpp
    src/gui/widget/progress_bar.cpp
    src/gui/widget/push_button.cpp
    src/gui/widget/radio_button.cpp
    src/gui/widget/rich_text_parser.cpp
    src/gui/widget/scrollable_widget.cpp
    src/gui/widget/scrollbar.cpp
    src/gui/widget/skin_manager.cpp
    src/gui/widget/slider.cpp
    src/gui/widget/spin_box.cpp
    src/gui/widget/splitter.cpp
    src/gui/widget/status_bar.cpp
    src/gui/widget/tab_bar.cpp
    src/gui/widget/tab_button.cpp
    src/gui/widget/tab_page.cpp
    src/gui/widget/tab_page_container.cpp
    src/gui/widget/table_view.cpp
    src/gui/widget/terminal.cpp
    src/gui/widget/text_edit.cpp
    src/gui/widget/text_field.cpp
    src/gui/widget/text_widget.cpp
    src/gui/widget/tool_title_bar.cpp
    src/gui/widget/toolbar.cpp
    src/gui/widget/toolbar_button.cpp
    src/gui/widget/tree_view.cpp
    src/gui/widget/web_view.cpp
    src/gui/widget/widget.cpp
    src/gui/window/context_menu.cpp
    src/gui/window/popup_menu.cpp
    src/gui/window/tooltip.cpp
    src/gui/window/window.cpp
    src/hid/window_manager.cpp)

# headers (for IDEs)
file(GLOB_RECURSE NEOGFX_HEADER_FILES CONFIGURE_DEPENDS include/neogfx/*.hpp include/neogfx/*.ipp include/neogfx/*.inl)
file(GLOB_RECURSE NEOGFX_PRIVATE_HEADER_FILES CONFIGURE_DEPENDS src/*.hpp src/*.ipp src/*.h)
list(FILTER NEOGFX_PRIVATE_HEADER_FILES EXCLUDE REGEX "/src/plugin/")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES
    ${NEOGFX_SOURCE_FILES} ${NEOGFX_WIDGET_SOURCE_FILES} ${NEOGFX_HEADER_FILES} ${NEOGFX_PRIVATE_HEADER_FILES})

add_library(neogfx STATIC ${NEOGFX_SOURCE_FILES} ${NEOGFX_HEADER_FILES} ${NEOGFX_PRIVATE_HEADER_FILES})
add_library(neogfx::neogfx ALIAS neogfx)
add_library(neogfx_widget STATIC ${NEOGFX_WIDGET_SOURCE_FILES})
add_library(neogfx::neogfx_widget ALIAS neogfx_widget)

# HarfBuzz is included as <harfbuzz/hb.h>
set(_neogfx_harfbuzz_parents)
get_target_property(_neogfx_harfbuzz_includes harfbuzz::harfbuzz INTERFACE_INCLUDE_DIRECTORIES)
foreach(_dir IN LISTS _neogfx_harfbuzz_includes)
    get_filename_component(_parent "${_dir}" DIRECTORY)
    list(APPEND _neogfx_harfbuzz_parents "${_parent}")
endforeach()

# (both libraries are compiled as the Visual Studio projects compile them)
foreach(_target IN ITEMS neogfx neogfx_widget)
    set_target_properties(${_target} PROPERTIES
        DEBUG_POSTFIX "d"
        POSITION_INDEPENDENT_CODE ON
        EXPORT_NAME ${_target})
    # (neoGFX's own headers first: an installed neoGFX may be in the dependencies' include directories)
    target_include_directories(${_target} BEFORE PRIVATE "${PROJECT_SOURCE_DIR}/include")
    target_include_directories(${_target} PRIVATE
        "${NEOGFX_PNG_INCLUDE_DIR}" "${NEOGFX_GLEW_INCLUDE_DIR}" ${_neogfx_harfbuzz_parents})
    target_compile_definitions(${_target} PRIVATE GLEW_STATIC $<$<CONFIG:Debug>:DEBUG_HID>)
    target_compile_options(${_target} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4>)
endforeach()

target_include_directories(neogfx PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_definitions(neogfx PUBLIC NEOLIB_HOSTED_ENVIRONMENT $<$<BOOL:${NEOGFX_DEBUG}>:NEOGFX_DEBUG>)

target_link_libraries(neogfx PUBLIC neolib::neolib Boost::program_options)
# every neoGFX app (executable) reserves a 100MB stack
target_link_options(neogfx INTERFACE
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>>:/STACK:104857600>)
target_link_libraries(neogfx PRIVATE
    Freetype::Freetype
    harfbuzz::harfbuzz
    JPEG::JPEG
    ${NEOGFX_PNG_LIBRARY}
    ZLIB::ZLIB
    OpenSSL::Crypto
    ${NEOGFX_GLEW_LIBRARY}
    OpenGL::GL
    Vulkan::Vulkan
    ${NEOGFX_SHADERC_LIBRARY})
if(WIN32)
    target_link_libraries(neogfx PRIVATE crypt32 imm32 version winmm d2d1 shcore uxtheme)
endif()
# neogfx and neogfx_widget use each other
target_link_libraries(neogfx INTERFACE neogfx_widget)
target_link_libraries(neogfx_widget PUBLIC neogfx)

# generated sources: GLSL as C++ strings and the embedded resources
neogfx_add_source_strings(neogfx NAMESPACE neogfx::glsl SOURCES
    src/gfx/native/shader.glsl Shader
    src/gfx/native/standard-texture.vert StandardTextureVertexShader
    src/gfx/native/standard.vert StandardVertexShader
    src/gfx/native/standard-filter.frag StandardFilterFragmentShader
    src/gfx/native/standard-glyph.frag StandardGlyphFragmentShader
    src/gfx/native/standard-gradient.frag StandardGradientFragmentShader
    src/gfx/native/standard-shape.frag StandardShapeFragmentShader
    src/gfx/native/standard-stipple.frag StandardStippleFragmentShader
    src/gfx/native/standard-pbr.frag StandardPbrFragmentShader
    src/gfx/native/standard-texture.frag StandardTextureFragmentShader
    src/gfx/native/standard.frag StandardFragmentShader)
neogfx_add_source_strings(neogfx NAMESPACE neogfx::glsl::gles SOURCES
    src/gfx/native/shader.glsl-gles Shader)
neogfx_add_resources(neogfx src/resources.nrc src/icons.nrc)

# tools, plugins and apps built with neoGFX
if(NEOGFX_BUILD_DESIGN_STUDIO)
    add_subdirectory(tools/DesignStudio)
endif()
if(NEOGFX_BUILD_WEB_VIEW_PLUGIN)
    add_subdirectory(src/plugin/web_view)
endif()
if(NEOGFX_BUILD_TESTS)
    add_subdirectory(testing/gui_test_app)
endif()
if(NEOGFX_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# install
include(CMakePackageConfigHelpers)
configure_package_config_file(neogfxConfig.cmake.in neogfxConfig.cmake INSTALL_DESTINATION "${CMAKE_INSTALL_CMAKEDIR}")
write_basic_package_version_file(neogfxConfigVersion.cmake COMPATIBILITY AnyNewerVersion)

install(TARGETS neogfx neogfx_widget EXPORT neogfx
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(TARGETS nrc src2string EXPORT neogfx
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(DIRECTORY include/neogfx DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(EXPORT neogfx DESTINATION "${CMAKE_INSTALL_CMAKEDIR}" FILE neogfxTargets.cmake NAMESPACE neogfx::)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/neogfxConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/neogfxConfigVersion.cmake"
    cmake/neogfxDependencies.cmake
    cmake/neogfxTools.cmake
    DESTINATION "${CMAKE_INSTALL_CMAKEDIR}")
install(FILES LICENSE LICENSES DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/${PROJECT_NAME}")
