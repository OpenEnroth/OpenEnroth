# hack to be able to create a convenience library without sources
file(WRITE ${LIB_DIR}/sdl2.c "")
add_library(
    sdl2 STATIC
    ${LIB_DIR}/sdl2.c
)

if (WIN32)
set(SDL2_DIR "${LIBRARY_DIR}/sdl2-2.0.12")
set(SDL2_BIN_DIR "${SDL2_DIR}/bin")
set(SDL2_LIB_DIR "${SDL2_DIR}/lib")
set(SDL2_INCLUDE_DIR "${SDL2_DIR}/include")

set_target_properties(
    sdl2 PROPERTIES
    PUBLIC_HEADER "${SDL2_INCLUDE_DIR}/SDL.h"
)

target_link_libraries(
    sdl2
    "${SDL2_LIB_DIR}/SDL2.lib"
    "${SDL2_LIB_DIR}/SDL2main.lib"
)

include_directories(
    "${SDL2_INCLUDE_DIR}"
)

ADD_GLOBAL_DEPENDENCY("${SDL2_BIN_DIR}/SDL2.dll")

else()  # WIN32
find_package(SDL2 REQUIRED)
target_link_libraries(sdl2 SDL2)
include_directories("${SDL_INCLUDE_DIR}")
endif()
