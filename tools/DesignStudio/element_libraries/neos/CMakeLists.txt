# Design Studio's neos element library: neos.nel (requires neos)
find_package(neos CONFIG QUIET)
if(NOT TARGET neos::neos)
    find_path(NEOGFX_NEOS_INCLUDE_DIR neos/context.hpp)
    find_library(NEOGFX_NEOS_LIBRARY_RELEASE NAMES neos)
    find_library(NEOGFX_NEOS_LIBRARY_DEBUG NAMES neosd)
    if(NOT NEOGFX_NEOS_INCLUDE_DIR OR NOT NEOGFX_NEOS_LIBRARY_RELEASE)
        message(FATAL_ERROR "neoGFX: neos not found (set neos_DIR, or NEOGFX_NEOS_INCLUDE_DIR and NEOGFX_NEOS_LIBRARY_RELEASE)")
    endif()
    add_library(neos::neos UNKNOWN IMPORTED)
    set_target_properties(neos::neos PROPERTIES
        IMPORTED_LOCATION "${NEOGFX_NEOS_LIBRARY_RELEASE}"
        INTERFACE_INCLUDE_DIRECTORIES "${NEOGFX_NEOS_INCLUDE_DIR}")
    if(NEOGFX_NEOS_LIBRARY_DEBUG)
        set_target_properties(neos::neos PROPERTIES IMPORTED_LOCATION_DEBUG "${NEOGFX_NEOS_LIBRARY_DEBUG}")
    endif()
endif()

neogfx_add_plugin(DesignStudio_neos_elements neos .nel
    src/library.cpp
    src/plugin.cpp
    src/library.hpp)
if(WIN32)
    target_sources(DesignStudio_neos_elements PRIVATE src/dllmain.cpp)
endif()
neogfx_add_resources(DesignStudio_neos_elements src/neos_nel.nrc)
target_compile_definitions(DesignStudio_neos_elements PRIVATE NEOGFX_PLUGIN _SCL_SECURE_NO_WARNINGS DEFAULT_EXPORTS)
target_link_libraries(DesignStudio_neos_elements PRIVATE neogfx::neogfx neos::neos)
