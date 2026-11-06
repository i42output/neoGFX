# video_poker example
add_executable(video_poker
    src/card_space.cpp
    src/main.cpp
    src/table.cpp)
if(WIN32)
    target_sources(video_poker PRIVATE src/resources/win32/video_poker.rc)
endif()
neogfx_gui_executable(video_poker)
neogfx_add_resources(video_poker src/video_poker.nrc)
target_include_directories(video_poker PRIVATE include ../common/include)
target_link_libraries(video_poker PRIVATE neogfx::neogfx)
install(TARGETS video_poker RUNTIME DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/examples")
