cmake_minimum_required(VERSION 3.10)
project(GL-Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Dependencies
add_subdirectory(external/glfw)
add_subdirectory(external/glm)

find_package(OpenGL REQUIRED)

# Engine library
add_subdirectory(engine)

# Game executable
add_subdirectory(game)