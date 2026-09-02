cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

project(glad)

set(GLAD_GL_SOURCES src/gl.c
                    src/egl.c)

set(GLAD_GL_HEADERS include/glad/gl.h
                    include/glad/egl.h
                    include/EGL/eglplatform.h
                    include/KHR/khrplatform.h)

add_library(glad STATIC ${GLAD_GL_SOURCES} ${GLAD_GL_HEADERS})
target_include_directories(glad PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

source_check_style(glad "${GLAD_GL_SOURCES}")
