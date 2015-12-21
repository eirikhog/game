cmake_minimum_required(VERSION 2.6)
project(Game)

SET (LIB_TYPE SHARED)

find_package(OpenGL)
# find_package(GLUT)

# Game as library
file(GLOB GameSources *.cpp *.h)
add_library(Game SHARED ${GameSources} ../dep/glew-1.13.0/src/glew.c)
target_link_libraries(Game ${OPENGL_gl_LIBRARY})

# Windows executable
if (WIN32)
    file(GLOB WindowsSources win32/*.cpp win32/*.h)
    add_executable(Windows WIN32 ${WindowsSources} ../dep/glew-1.13.0/src/glew.c)
    add_definitions(-DGLEW_BUILD)
    include_directories(${PROJECT_SOURCE_DIR}/../dep/glew-1.13.0/include)
    target_link_libraries(Windows user32.lib gdi32.lib winmm.lib xinput.lib ${OPENGL_gl_LIBRARY})
endif()


