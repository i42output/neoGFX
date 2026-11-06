# mod_tracker example (links FFTS if it is found)
add_executable(mod_tracker
    src/main.cpp
    src/icons.cpp
    src/load_dbm.cpp
    src/load_it.cpp
    src/load_mo3.cpp
    src/load_mod.cpp
    src/load_s3m.cpp
    src/load_xm.cpp
    src/mixer.cpp
    src/module.cpp
    src/module_bitstream.cpp
    src/oscilloscope_view.cpp
    src/pattern_view.cpp
    src/player.cpp
    src/replayer.cpp
    src/sample_view.cpp
    src/spectrum_view.cpp
    src/timeline.cpp
    src/tracker.cpp
    src/vorbis.cpp
    src/loader.hpp
    src/tables.hpp)
set_source_files_properties(src/vorbis.cpp PROPERTIES COMPILE_OPTIONS $<IF:$<CXX_COMPILER_ID:MSVC>,/w,-w>)
neogfx_gui_executable(mod_tracker)
target_include_directories(mod_tracker PRIVATE include "${PROJECT_SOURCE_DIR}/3rdparty")
find_library(NEOGFX_FFTS_LIBRARY NAMES ffts_static ffts HINTS "${NEOGFX_3RDPARTY_LIBDIR}")
if(NEOGFX_FFTS_LIBRARY)
    target_link_libraries(mod_tracker PRIVATE "${NEOGFX_FFTS_LIBRARY}")
endif()
target_link_libraries(mod_tracker PRIVATE neogfx::neogfx)
install(TARGETS mod_tracker RUNTIME DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/examples")
