# neogfxDependencies.cmake
# Finds the packages whose imported targets neoGFX's targets link to. Included by
# the neoGFX build and by the installed neogfxConfig.cmake, so that the targets
# referred to by the exported neoGFX targets exist for neoGFX's users too.

include_guard(DIRECTORY)
include(CMakeFindDependencyMacro)

macro(_neogfx_find_dependency)
    if(CMAKE_FIND_PACKAGE_NAME STREQUAL "neogfx")
        find_dependency(${ARGN})
    else()
        find_package(${ARGN} REQUIRED)
    endif()
endmacro()

# neolib (also finds Threads, Boost, OpenSSL and ZLIB)
_neogfx_find_dependency(neolib CONFIG)
if(NOT TARGET OpenSSL::Crypto)
    _neogfx_find_dependency(OpenSSL COMPONENTS Crypto)
endif()
if(NOT TARGET ZLIB::ZLIB)
    _neogfx_find_dependency(ZLIB)
endif()

# Boost.ProgramOptions (<neogfx/app/i_app.hpp> includes <boost/program_options.hpp>)
if(NOT TARGET Boost::program_options)
    if(WIN32)
        set(Boost_USE_STATIC_LIBS ON)
        set(Boost_USE_STATIC_RUNTIME ON)
    endif()
    _neogfx_find_dependency(Boost COMPONENTS program_options)
endif()

# FreeType: its CMake package if installed, else found by header and library
if(NOT TARGET Freetype::Freetype)
    find_package(freetype CONFIG QUIET)
    if(TARGET freetype)
        # (an installed package whose include directory is missing is not usable)
        set(_neogfx_freetype_usable TRUE)
        get_target_property(_neogfx_freetype_includes freetype INTERFACE_INCLUDE_DIRECTORIES)
        foreach(_dir IN LISTS _neogfx_freetype_includes)
            if(NOT IS_DIRECTORY "${_dir}")
                message(STATUS "neoGFX: ignoring FreeType package in ${freetype_DIR} (missing include directory ${_dir})")
                set(_neogfx_freetype_usable FALSE)
            endif()
        endforeach()
        if(_neogfx_freetype_usable)
            add_library(Freetype::Freetype INTERFACE IMPORTED)
            target_link_libraries(Freetype::Freetype INTERFACE freetype)
        endif()
    endif()
endif()
if(NOT TARGET Freetype::Freetype)
    find_path(NEOGFX_FREETYPE_INCLUDE_DIR ft2build.h PATH_SUFFIXES freetype2)
    find_library(NEOGFX_FREETYPE_LIBRARY NAMES freetype)
    if(NOT NEOGFX_FREETYPE_INCLUDE_DIR OR NOT NEOGFX_FREETYPE_LIBRARY)
        message(FATAL_ERROR "neoGFX: FreeType not found (set freetype_DIR, or NEOGFX_FREETYPE_INCLUDE_DIR and NEOGFX_FREETYPE_LIBRARY)")
    endif()
    add_library(Freetype::Freetype UNKNOWN IMPORTED)
    set_target_properties(Freetype::Freetype PROPERTIES
        IMPORTED_LOCATION "${NEOGFX_FREETYPE_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${NEOGFX_FREETYPE_INCLUDE_DIR}")
endif()

# HarfBuzz: its CMake package if installed, else found by header and library
if(NOT TARGET harfbuzz::harfbuzz)
    find_package(harfbuzz CONFIG QUIET)
endif()
if(NOT TARGET harfbuzz::harfbuzz)
    find_path(NEOGFX_HARFBUZZ_INCLUDE_DIR harfbuzz/hb.h)
    find_library(NEOGFX_HARFBUZZ_LIBRARY NAMES harfbuzz)
    if(NOT NEOGFX_HARFBUZZ_INCLUDE_DIR OR NOT NEOGFX_HARFBUZZ_LIBRARY)
        message(FATAL_ERROR "neoGFX: HarfBuzz not found (set harfbuzz_DIR, or NEOGFX_HARFBUZZ_INCLUDE_DIR and NEOGFX_HARFBUZZ_LIBRARY)")
    endif()
    add_library(harfbuzz::harfbuzz UNKNOWN IMPORTED)
    set_target_properties(harfbuzz::harfbuzz PROPERTIES
        IMPORTED_LOCATION "${NEOGFX_HARFBUZZ_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${NEOGFX_HARFBUZZ_INCLUDE_DIR};${NEOGFX_HARFBUZZ_INCLUDE_DIR}/harfbuzz")
endif()

# libjpeg (the static library, jpeg-static, in preference)
if(NOT TARGET JPEG::JPEG)
    if(NOT DEFINED JPEG_NAMES)
        set(JPEG_NAMES jpeg-static libjpeg-static)
    endif()
    _neogfx_find_dependency(JPEG)
endif()

# OpenGL
_neogfx_find_dependency(OpenGL)

# Vulkan
_neogfx_find_dependency(Vulkan)
