
function(download_prebuilt_dependencies TAG FILE_NAME TARGET_DIR)
    set(SOURCE_URL "https://github.com/OpenEnroth/OpenEnroth_Dependencies/releases/download/${TAG}/${FILE_NAME}")
    set(TARGET_PATH "${TARGET_DIR}/${FILE_NAME}")
    message(STATUS "Downloading ${SOURCE_URL}...")
    file(DOWNLOAD
            "${SOURCE_URL}"
            "${TARGET_PATH}"
            SHOW_PROGRESS
            STATUS DOWNLOAD_STATUS
            TIMEOUT 60)  # seconds

    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)
    if(${STATUS_CODE} EQUAL 0)
        message(STATUS "Downloaded ${SOURCE_URL}!")
    else()
        file(REMOVE "${TARGET_PATH}")
        message(FATAL_ERROR "Could not download ${SOURCE_URL}: ${ERROR_MESSAGE}")
    endif()

    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf "${TARGET_PATH}"
            WORKING_DIRECTORY ${TARGET_DIR}
            RESULT_VARIABLE UNPACK_STATUS)

    if (UNPACK_STATUS)
        message(FATAL_ERROR "Could not unpack ${TARGET_PATH}: ${UNPACK_STATUS}")
    endif()
endfunction()

#TODO: all prebuilt dependency artifacts should be built and packaged in the same unified way.
#      so all code below could be drastically simplified and we wouldn't have per-platform blocks.

macro(resolve_dependencies) # Intentionally a macro - we want set() to work in parent scope.
    if(BUILD_PLATFORM STREQUAL "android")
        # NOTE: ${CMAKE_SOURCE_DIR} is pointing to OpenEnroth_Android/openenroth/jni directory in this case
        add_library(ffmpeg::avcodec SHARED IMPORTED)
        set_target_properties(ffmpeg::avcodec PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/FFmpeg/android/${ANDROID_ABI}/include
                IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/FFmpeg/android/${ANDROID_ABI}/lib/libavcodec-58.so)
        set(AVCODEC_LIBRARIES "ffmpeg::avcodec")
        add_library(ffmpeg::avformat SHARED IMPORTED)
        set_target_properties(ffmpeg::avformat PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/FFmpeg/android/${ANDROID_ABI}/include
                IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/FFmpeg/android/${ANDROID_ABI}/lib/libavformat-58.so)
        set(AVFORMAT_LIBRARIES "ffmpeg::avformat")
        add_library(ffmpeg::avfilter SHARED IMPORTED)
        set_target_properties(ffmpeg::avfilter PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/FFmpeg/android/${ANDROID_ABI}/include
                IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/FFmpeg/android/${ANDROID_ABI}/lib/libavfilter-7.so)
        set(AVFILTER_LIBRARIES "ffmpeg::avfilter")
        add_library(ffmpeg::avutil SHARED IMPORTED)
        set_target_properties(ffmpeg::avutil PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/FFmpeg/android/${ANDROID_ABI}/include
                IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/FFmpeg/android/${ANDROID_ABI}/lib/libavutil-56.so)
        set(AVUTIL_LIBRARIES "ffmpeg::avutil")
        add_library(ffmpeg::swscale SHARED IMPORTED)
        set_target_properties(ffmpeg::swscale PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/FFmpeg/android/${ANDROID_ABI}/include
                IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/FFmpeg/android/${ANDROID_ABI}/lib/libswscale-5.so)
        set(SWRESAMPLE_LIBRARIES "ffmpeg::swresample")
        add_library(ffmpeg::swresample SHARED IMPORTED)
        set_target_properties(ffmpeg::swresample PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/FFmpeg/android/${ANDROID_ABI}/include
                IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/FFmpeg/android/${ANDROID_ABI}/lib/libswresample-3.so)
        set(SWSCALE_LIBRARIES "ffmpeg::swscale")
        find_package(ZLIB REQUIRED)
    elseif(OE_USE_PREBUILT_DEPENDENCIES)
        set(PREBUILT_DEPS_TAG "deps_r1")
        set(PREBUILT_DEPS_FILENAME "${BUILD_PLATFORM}_${CMAKE_BUILD_TYPE}_${BUILD_ARCHITECTURE}.zip")
        set(PREBUILT_DEPS_DIR "${CMAKE_CURRENT_BINARY_DIR}/dependencies")
        if (NOT EXISTS "${PREBUILT_DEPS_DIR}/${PREBUILT_DEPS_FILENAME}")
            download_prebuilt_dependencies("${PREBUILT_DEPS_TAG}" "${PREBUILT_DEPS_FILENAME}" "${PREBUILT_DEPS_DIR}")
        endif()

        list(APPEND CMAKE_MODULE_PATH "${PREBUILT_DEPS_DIR}")
        list(APPEND CMAKE_PREFIX_PATH "${PREBUILT_DEPS_DIR}")

        set(ZLIB_USE_STATIC_LIBS ON)
        find_package(ZLIB REQUIRED)

        find_package(SDL2 CONFIG REQUIRED GLOBAL)
        add_library(SDL2OE INTERFACE)
        target_link_libraries(SDL2OE INTERFACE SDL2::SDL2)
        if(TARGET SDL2::SDL2main) # Not all platforms have SDL2main.
            target_link_libraries(SDL2OE INTERFACE SDL2::SDL2main)
        endif()
        add_library(SDL2::SDL2OE ALIAS SDL2OE)

        find_package(OpenAL CONFIG REQUIRED GLOBAL)

        find_package(FFmpeg REQUIRED)
    else()
        find_package(FFmpeg REQUIRED)
        find_package(OpenGL REQUIRED)
        find_package(ZLIB REQUIRED)
    endif()
endmacro()
