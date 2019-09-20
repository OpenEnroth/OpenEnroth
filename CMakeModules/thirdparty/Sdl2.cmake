if("${SDL2_VERSION}" STREQUAL "")
    set(SDL2_VERSION "2.0.10")
    DEBUG_PRINT("No SDL2_VERSION specified, defaulting to ${SDL2_VERSION}")
endif()


set(SDL2_ARTIFACTS  ${CMAKE_CURRENT_BINARY_DIR}/sdl2-artifacts)
set(SDL2_INSTALL    ${CMAKE_CURRENT_BINARY_DIR}/sdl2-install)
set(SDL2_SRC        ${SDL2_INSTALL}/src/sdl2_builder)
set(SDL2_INCLUDE    ${SDL2_INSTALL}/src/sdl2_builder/include)
set(SDL2_BIN        ${SDL2_INSTALL}/src/sdl2_builder/lib/${BUILD_PLATFORM})
set(SDL2_LIB        ${SDL2_INSTALL}/src/sdl2_builder/lib/${BUILD_PLATFORM})
DEBUG_PRINT("SDL2_ARTIFACTS ${SDL2_ARTIFACTS}")
DEBUG_PRINT("SDL2_INSTALL   ${SDL2_INSTALL}")
DEBUG_PRINT("SDL2_SRC       ${SDL2_SRC}")
DEBUG_PRINT("SDL2_INCLUDE   ${SDL2_INCLUDE}")
DEBUG_PRINT("SDL2_BIN       ${SDL2_BIN}")
DEBUG_PRINT("SDL2_LIB       ${SDL2_LIB}")


add_library(sdl2 STATIC IMPORTED)
add_dependencies(sdl2 sdl2_builder)
include_directories(${SDL2_INCLUDE})

set_property(TARGET sdl2 PROPERTY IMPORTED_LOCATION "${SDL2_LIB}/SDL2.lib")
set_property(TARGET sdl2 PROPERTY INTERFACE_LINK_LIBRARIES "${SDL2_LIB}/SDL2main.lib")

list(APPEND DEPENDENCIES "${SDL2_BIN}/SDL2.dll")

NINJA_WORKAROUND_GET_BYPRODUCTS(sdl2)



if (WIN32)
    ExternalProject_Add(
        sdl2_builder
        PREFIX              "${SDL2_INSTALL}"
        DOWNLOAD_DIR        "${SDL2_ARTIFACTS}"
        #URL                 "https://www.libsdl.org/release/SDL2-${SDL2_VERSION}-win32-${BUILD_PLATFORM}.zip"
        URL                 "https://www.libsdl.org/release/SDL2-devel-${SDL2_VERSION}-VC.zip"
        INSTALL_DIR         "${SDL2_INSTALL}"
        CONFIGURE_COMMAND   ""
        BUILD_COMMAND       ""
        INSTALL_COMMAND     ""
        BUILD_BYPRODUCTS    ${SDL2_BYPRODUCTS}
    )

 # build from source
else()
  message(FATAL_ERROR "SDL2: No non-win builds yet" )
endif()