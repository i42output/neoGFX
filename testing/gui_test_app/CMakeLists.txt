# neoGFX GUI test app (test.exe)
add_executable(gui_test_app
    src/main.cpp
    src/game.cpp
    src/test.hpp)
set_target_properties(gui_test_app PROPERTIES OUTPUT_NAME test)
if(WIN32)
    target_sources(gui_test_app PRIVATE src/resources/win32/test.rc)
endif()
neogfx_gui_executable(gui_test_app)
neogfx_add_resources(gui_test_app src/test.nrc)
target_link_libraries(gui_test_app PRIVATE neogfx::neogfx)
install(TARGETS gui_test_app RUNTIME DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/test")
