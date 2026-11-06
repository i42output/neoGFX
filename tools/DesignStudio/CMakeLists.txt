# neoGFX Design Studio
add_subdirectory(element_libraries/default)
if(NEOGFX_BUILD_NEOS_ELEMENT_LIBRARY)
    add_subdirectory(element_libraries/neos)
endif()

add_executable(DesignStudio
    src/element_model.cpp
    src/main.cpp
    src/main_window.cpp
    src/new_project_dialog.cpp
    src/object_model.cpp
    src/project.cpp
    src/project_manager.cpp
    src/project_model.cpp
    src/settings.cpp
    src/toolbox_model.cpp
    src/widget_caddy.cpp
    src/workflow_model.cpp
    src/app.hpp
    src/element_model.hpp
    src/element_model.ipp
    src/main_window.hpp
    src/new_project_dialog.hpp
    src/object_model.hpp
    src/project_model.hpp
    src/property_model.hpp
    src/toolbox_model.hpp
    src/widget_caddy.hpp
    src/workflow_model.hpp)
if(WIN32)
    target_sources(DesignStudio PRIVATE src/resources/win32/DesignStudio.rc)
endif()
neogfx_gui_executable(DesignStudio)
neogfx_add_resources(DesignStudio src/DesignStudio.nrc)
target_link_libraries(DesignStudio PRIVATE neogfx::neogfx)
# Design Studio loads its element libraries (.nel) from its own folder
add_dependencies(DesignStudio DesignStudio_default_elements)
if(TARGET DesignStudio_neos_elements)
    add_dependencies(DesignStudio DesignStudio_neos_elements)
endif()
install(TARGETS DesignStudio RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
