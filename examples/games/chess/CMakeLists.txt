# chess example (uses the stockparrot headers, found on the include path)
add_executable(chess
    src/board.cpp
    src/ai.cpp
    src/default_player_factory.cpp
    src/human.cpp
    src/main.cpp
    src/mailbox.cpp
    src/move_validator.cpp
    src/new_game_dialog.cpp)
if(WIN32)
    target_sources(chess PRIVATE src/resources/win32/chess.rc)
endif()
neogfx_gui_executable(chess)
neogfx_add_resources(chess src/chess.nrc)
target_include_directories(chess PRIVATE include ../common/include)
target_link_libraries(chess PRIVATE neogfx::neogfx)
