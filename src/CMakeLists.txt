cmake_minimum_required(VERSION 2.6)
project(Game)

SET (LIB_TYPE SHARED)

# Game as library
add_library(Game game.cpp)

# Windows executable
add_executable(Windows WIN32 Win32.cpp)
target_link_libraries(Windows user32.lib gdi32.lib winmm.lib xinput.lib)

