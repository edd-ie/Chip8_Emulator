cmake_minimum_required(VERSION 3.28)
project(Chip8_Emulator)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake-modules)
set(SDL2_PATH "X:/Code/C++/SDL2_mingw/SDL2-2.30.4/x86_64-w64-mingw32")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

add_executable(Chip8_Emulator main.cpp)

target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})