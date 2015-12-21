cmake_minimum_required(VERSION 2.6)
project(Game)

SET (LIB_TYPE SHARED)

find_package(OpenGL)
# find_package(GLUT)

# Game as library
add_library(Game SHARED game.cpp )

# Windows executable
if (WIN32)
    add_executable(Windows WIN32 Win32.cpp ../dep/glew-1.13.0/src/glew.c)
    add_definitions(-DGLEW_BUILD)
    include_directories(${PROJECT_SOURCE_DIR}/../dep/glew-1.13.0/include)
    target_link_libraries(Windows user32.lib gdi32.lib winmm.lib xinput.lib ${OPENGL_gl_LIBRARY})
endif()


