# example apps (NEOGFX_BUILD_EXAMPLES)
add_subdirectory(games/video_poker)
add_subdirectory(games/chess)
add_subdirectory(mod_tracker)

# the card game headers the example games share, for apps built with an installed neoGFX that use them too
install(DIRECTORY games/common/include/card_games DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
