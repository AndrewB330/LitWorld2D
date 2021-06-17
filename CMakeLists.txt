cmake_minimum_required(VERSION 3.19)
project(LitWorld2D)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(glew GIT_REPOSITORY https://github.com/Perlmint/glew-cmake GIT_TAG glew-cmake-2.2.0)
FetchContent_MakeAvailable(glew)

FetchContent_Declare(sdl2 GIT_REPOSITORY https://github.com/libsdl-org/SDL GIT_TAG release-2.0.14)
FetchContent_MakeAvailable(sdl2)

FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm GIT_TAG 0.9.9.8)
FetchContent_MakeAvailable(glm)

file(
    GLOB SOURCES
    "src/application/*.hpp"
    "src/application/*.cpp"
    "src/physics/*.hpp"
    "src/physics/*.cpp"
    "src/*.hpp"
)

add_executable(LitWorld2D main.cpp ${SOURCES})
target_include_directories(LitWorld2D PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(LitWorld2D -lmingw32 -lopengl32 -mwindows SDL2main SDL2-static libglew_static glm)