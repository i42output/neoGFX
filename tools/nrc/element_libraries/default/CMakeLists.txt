# nrc's default element library: default.nrl
neogfx_add_plugin(nrc_default_elements default .nrl
    src/library.cpp
    src/plugin.cpp
    src/library.hpp)
if(WIN32)
    target_sources(nrc_default_elements PRIVATE src/dllmain.cpp)
endif()
target_include_directories(nrc_default_elements PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_compile_definitions(nrc_default_elements PRIVATE NEOLIB_HOSTED_ENVIRONMENT _SCL_SECURE_NO_WARNINGS DEFAULT_EXPORTS)
target_link_libraries(nrc_default_elements PRIVATE neolib::neolib Boost::program_options)
