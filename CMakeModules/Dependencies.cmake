set(PREBUILT_DEPENDENCIES_LIST)

function(prebuilt_dependencies_add)
    if(PREBUILT_DEPENDENCIES)
        foreach(filename ${ARGV})
            if (NOT EXISTS ${filename})
                message(FATAL_ERROR "Prebuilt dependency does not exist: '${filename}' ")
            else()
                list(APPEND PREBUILT_DEPENDENCIES_LIST "${filename}")
                set (PREBUILT_DEPENDENCIES_LIST ${PREBUILT_DEPENDENCIES_LIST} PARENT_SCOPE)
            endif()
        endforeach()
    endif()
endfunction()

function(target_resolve_prebuilt_dependencies targetName)
    if(PREBUILT_DEPENDENCIES)
        foreach(dep ${PREBUILT_DEPENDENCIES_LIST})
            add_custom_command(
                    TARGET ${targetName}
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy "${dep}" $<TARGET_FILE_DIR:${targetName}>)
            # Note that we cannot use message() here because it cannot expand a generator expressions.
            add_custom_command(
                    TARGET ${targetName}
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E echo "Copying binary dependency ${dep} to $<TARGET_FILE_DIR:${targetName}>")
        endforeach()
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
    elseif(PREBUILT_DEPENDENCIES)
        message(STATUS "Prebuilt dependencies have been enabled")

        set(DEP_PLATFORM ${BUILD_PLATFORM})
        if (WIN32)
            set(DEP_PLATFORM "win32")
        elseif(APPLE)
            set(DEP_PLATFORM "macos")
        elseif(Linux)
            set(DEP_PLATFORM "linux")
        else()
            MESSAGE(FATAL_ERROR "Unknown platform for prebuilt dependencies.")
        endif()

        set(LIB_DIR "${CMAKE_SOURCE_DIR}/lib")
        set(LIBRARY_DIR "${LIB_DIR}/${DEP_PLATFORM}/${BUILD_TYPE}")

        set(DEPS_ZIP_FILENAME "${DEP_PLATFORM}_${CMAKE_BUILD_TYPE}_${BUILD_TYPE}.zip")
        set(DEPS_ZIP_FULL_PATH "${LIB_DIR}/${DEPS_ZIP_FILENAME}")
        
        # resolve 3d party libs
        if(NOT EXISTS "${LIBRARY_DIR}")
            if(NOT EXISTS "${DEPS_ZIP_FULL_PATH}")
                MESSAGE(STATUS "Downloading dependencies: ${DEPS_ZIP_FILENAME}")
                file(DOWNLOAD
                        "https://github.com/botanicvelious/OpenEnroth_Dependencies/releases/download/nightly/${DEPS_ZIP_FILENAME}"
                        "${DEPS_ZIP_FULL_PATH}"
                        SHOW_PROGRESS
                        STATUS DOWNLOAD_STATUS
                        TIMEOUT 60)  # seconds

                list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
                list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)
                if(${STATUS_CODE} EQUAL 0)
                    message(STATUS "Prebuilt dependencies for ${BUILD_PLATFORM}/${BUILD_TYPE} successfully downloaded!")
                else()
                    file(REMOVE "${DEPS_ZIP_FULL_PATH}")
                    message(FATAL_ERROR "Error occurred during download of dependencies for ${BUILD_PLATFORM}/${BUILD_TYPE}: ${ERROR_MESSAGE}")
                endif()
            endif()

            execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf "${DEPS_ZIP_FULL_PATH}"
                    WORKING_DIRECTORY ${LIB_DIR}
                    RESULT_VARIABLE UNPACK_STATUS)

            if (UNPACK_STATUS)
                message(FATAL_ERROR "Error occurred during unpack of dependencies for ${BUILD_PLATFORM}/${BUILD_TYPE}: ${UNPACK_STATUS}")
            endif()

            list(APPEND CMAKE_FIND_ROOT_PATH ${LIBRARY_DIR})
            list(APPEND CMAKE_PREFIX_PATH ${LIBRARY_DIR})
        endif()

        if (BUILD_PLATFORM STREQUAL "windows")
            message(STATUS "Libs dir: ${LIBRARY_DIR}")
            
            find_package(ZLIB REQUIRED)
            find_package(FFmpeg REQUIRED)
            find_package(OpenAL REQUIRED)
            find_package(SDL2 REQUIRED)
        else()
            message(FATAL_ERROR "Prebuilt dependencies for ${BUILD_PLATFORM} are unknown!")
        endif()
    else()
        find_package(FFmpeg REQUIRED)
        find_package(OpenGL REQUIRED)
        find_package(ZLIB REQUIRED)
    endif()
endmacro()
