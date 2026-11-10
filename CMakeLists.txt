cmake_minimum_required(VERSION 3.21)
project(Raycaster C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

add_executable(raycaster
    src/main.c
    src/map.c
    src/player.c
    src/renderer.c)
target_link_libraries(raycaster PRIVATE SDL3::SDL3)