cmake_minimum_required(VERSION 3.7)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(gen_painter)

find_package(SDL2 REQUIRED)

file(GLOB_RECURSE SRC_FILES "src/**/*.c")
add_compile_options("-Wall" "-Wextra" "-Werror")
add_executable(gen_painter src/sdl_main.c ${SRC_FILES})
target_link_libraries(gen_painter m ${SDL2_LIBRARIES})
include_directories(include ${SDL2_INCLUDE_DIRS})
