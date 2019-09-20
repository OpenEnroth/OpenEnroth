if("${FFMPEG_VERSION}" STREQUAL "")
    set(FFMPEG_VERSION "4.0")
    DEBUG_PRINT("No FFMPEG_VERSION specified, defaulting to ${FFMPEG_VERSION}")
endif()


set(FFMPEG_ARTIFACTS    ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg-artifacts)
set(FFMPEG_INSTALL      ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg-install)
set(FFMPEG_SRC          ${FFMPEG_INSTALL}/src/ffmpeg_builder)
set(FFMPEG_INCLUDE      ${FFMPEG_INSTALL}/src/ffmpeg_builder/include)
set(FFMPEG_BIN          ${FFMPEG_INSTALL}/src/ffmpeg_binaries_builder/bin)
set(FFMPEG_LIB          ${FFMPEG_INSTALL}/src/ffmpeg_builder/lib)
set(FFMPEG_SHARE        ${FFMPEG_INSTALL}/share)
DEBUG_PRINT("FFMPEG_ARTIFACTS ${FFMPEG_ARTIFACTS}")
DEBUG_PRINT("FFMPEG_INSTALL   ${FFMPEG_INSTALL}")
DEBUG_PRINT("FFMPEG_SRC       ${FFMPEG_SRC}")
DEBUG_PRINT("FFMPEG_INCLUDE   ${FFMPEG_INCLUDE}")
DEBUG_PRINT("FFMPEG_BIN       ${FFMPEG_BIN}")
DEBUG_PRINT("FFMPEG_LIB       ${FFMPEG_LIB}")
DEBUG_PRINT("FFMPEG_SHARE     ${FFMPEG_SHARE}")


add_library(ffmpeg STATIC IMPORTED)
add_dependencies(ffmpeg ffmpeg_builder)
add_dependencies(ffmpeg ffmpeg_binaries_builder)
include_directories(${FFMPEG_INCLUDE})
    
set_property(
    TARGET ffmpeg PROPERTY
    IMPORTED_LOCATION "${FFMPEG_LIB}/avcodec.lib"
)
set_property(
    TARGET ffmpeg
    PROPERTY INTERFACE_LINK_LIBRARIES
        "${FFMPEG_LIB}/avdevice.lib"
        "${FFMPEG_LIB}/avfilter.lib"
        "${FFMPEG_LIB}/avformat.lib"
        "${FFMPEG_LIB}/avutil.lib"
        "${FFMPEG_LIB}/postproc.lib"
        "${FFMPEG_LIB}/swresample.lib"
        "${FFMPEG_LIB}/swscale.lib"
)

list(APPEND DEPENDENCIES "${FFMPEG_BIN}/avcodec-58.dll")
list(APPEND DEPENDENCIES "${FFMPEG_BIN}/avdevice-58.dll")
list(APPEND DEPENDENCIES "${FFMPEG_BIN}/avfilter-7.dll")
list(APPEND DEPENDENCIES "${FFMPEG_BIN}/avformat-58.dll")
list(APPEND DEPENDENCIES "${FFMPEG_BIN}/avutil-56.dll")
list(APPEND DEPENDENCIES "${FFMPEG_BIN}/postproc-55.dll")
list(APPEND DEPENDENCIES "${FFMPEG_BIN}/swresample-3.dll")
list(APPEND DEPENDENCIES "${FFMPEG_BIN}/swscale-5.dll")

NINJA_WORKAROUND_GET_BYPRODUCTS(ffmpeg)



# For win32 it's faster to download the pre-compiled development binaries.
if (WIN32)
    ExternalProject_Add(
        ffmpeg_builder
        PREFIX              "${FFMPEG_INSTALL}"
        DOWNLOAD_DIR        "${FFMPEG_ARTIFACTS}"
        URL                 "https://ffmpeg.zeranoe.com/builds/win32/dev/ffmpeg-${FFMPEG_VERSION}-${BUILD_WIN_PLATFORM}-dev.zip"
        INSTALL_DIR         "${FFMPEG_INSTALL}"
        CONFIGURE_COMMAND   ""
        BUILD_COMMAND       ""
        INSTALL_COMMAND     ""

        BUILD_BYPRODUCTS  ${FFMPEG_BYPRODUCTS}
    )


    
    ExternalProject_Add(
        ffmpeg_binaries_builder
        PREFIX              "${FFMPEG_INSTALL}"
        DOWNLOAD_DIR        "${FFMPEG_ARTIFACTS}"
        URL                 "https://ffmpeg.zeranoe.com/builds/${BUILD_WIN_PLATFORM}/shared/ffmpeg-${FFMPEG_VERSION}-${BUILD_WIN_PLATFORM}-shared.zip"
        INSTALL_DIR         "${FFMPEG_INSTALL}"
        CONFIGURE_COMMAND   ""
        BUILD_COMMAND       ""
        INSTALL_COMMAND     ""
        
        BUILD_BYPRODUCTS  "${FFMPEG_BIN}/avcodec.dll"
    )

 # build from source
else()
  message(FATAL_ERROR "FFmpeg: No non-win builds yet" )
endif()

