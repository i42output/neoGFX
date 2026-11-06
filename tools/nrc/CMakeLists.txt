# nrc: the neoGFX resource compiler
# (nrc and its element library must not link neoGFX, so that they can be built before it;
# they only include neoGFX headers)
add_subdirectory(element_libraries/default)

add_executable(nrc
    src/nrc.cpp
    src/resource_parser.cpp
    src/ui_parser.cpp
    src/resource_parser.hpp
    src/ui_parser.hpp)
target_include_directories(nrc PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_compile_definitions(nrc PRIVATE IGNORE_UNKNOWN_ELEMENTS NEOLIB_HOSTED_ENVIRONMENT _SCL_SECURE_NO_WARNINGS)
target_link_libraries(nrc PRIVATE neolib::neolib Boost::program_options)
# nrc loads its element libraries (.nrl) from its own folder
add_dependencies(nrc nrc_default_elements)
