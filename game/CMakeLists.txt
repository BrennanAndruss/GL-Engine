file(GLOB_RECURSE GAME_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(game ${GAME_SOURCES})

target_link_libraries(game PRIVATE 
    engine
)