# Design Studio's default element library: default.nel
find_package(libssh CONFIG REQUIRED)

neogfx_add_plugin(DesignStudio_default_elements default .nel
    src/library.cpp
    src/plugin.cpp
    src/console_widget.hpp
    src/library.hpp
    src/node_widget.hpp
    src/script_graph.hpp
    src/script_widget.hpp
    src/ssh.hpp
    src/sticky_note.hpp
    src/telnet.hpp)
if(WIN32)
    target_sources(DesignStudio_default_elements PRIVATE src/dllmain.cpp)
endif()
neogfx_add_resources(DesignStudio_default_elements src/default_nel.nrc)
target_compile_definitions(DesignStudio_default_elements PRIVATE NEOGFX_PLUGIN _SCL_SECURE_NO_WARNINGS DEFAULT_EXPORTS)
target_link_libraries(DesignStudio_default_elements PRIVATE neogfx::neogfx ssh)
