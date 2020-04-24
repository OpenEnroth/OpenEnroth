# hack to be able to create a convenience library without sources
file(WRITE ${LIB_DIR}/ffmpeg.c "")
add_library(
    ffmpeg STATIC
    ${LIB_DIR}/ffmpeg.c
)

if (WIN32)
# https://ffmpeg.zeranoe.com/builds/

set(FFMPEG_DIR "${LIBRARY_DIR}/ffmpeg-4.2.2")
set(FFMPEG_BIN_DIR "${FFMPEG_DIR}/bin")
set(FFMPEG_LIB_DIR "${FFMPEG_DIR}/lib")
set(FFMPEG_INCLUDE_DIR "${FFMPEG_DIR}/include")

target_link_libraries(
    ffmpeg
    "${FFMPEG_LIB_DIR}/avcodec.lib"
    "${FFMPEG_LIB_DIR}/avdevice.lib"
    "${FFMPEG_LIB_DIR}/avfilter.lib"
    "${FFMPEG_LIB_DIR}/avformat.lib"
    "${FFMPEG_LIB_DIR}/avutil.lib"
    "${FFMPEG_LIB_DIR}/postproc.lib"
    "${FFMPEG_LIB_DIR}/swresample.lib"
    "${FFMPEG_LIB_DIR}/swscale.lib"
)

include_directories(
    "${FFMPEG_INCLUDE_DIR}"
)

ADD_GLOBAL_DEPENDENCY("${FFMPEG_BIN_DIR}/avcodec-58.dll")
ADD_GLOBAL_DEPENDENCY("${FFMPEG_BIN_DIR}/avdevice-58.dll")
ADD_GLOBAL_DEPENDENCY("${FFMPEG_BIN_DIR}/avfilter-7.dll")
ADD_GLOBAL_DEPENDENCY("${FFMPEG_BIN_DIR}/avformat-58.dll")
ADD_GLOBAL_DEPENDENCY("${FFMPEG_BIN_DIR}/avutil-56.dll")
ADD_GLOBAL_DEPENDENCY("${FFMPEG_BIN_DIR}/postproc-55.dll")
ADD_GLOBAL_DEPENDENCY("${FFMPEG_BIN_DIR}/swresample-3.dll")
ADD_GLOBAL_DEPENDENCY("${FFMPEG_BIN_DIR}/swscale-5.dll")

else()  # WIN32
find_package(FFmpeg COMPONENTS AVCODEC AVFORMAT SWSCALE REQUIRED)
target_link_libraries(ffmpeg "${FFMPEG_LIBRARIES}")

if (APPLE)
    include_directories(
            "/usr/local/include"
    )
    find_library(AVUTIL_LIBRARY avutil)
    find_library(SWRESAMPLE_LIBRARY swresample)
    target_link_libraries(ffmpeg ${AVUTIL_LIBRARY} ${SWRESAMPLE_LIBRARY})
endif()

endif()
