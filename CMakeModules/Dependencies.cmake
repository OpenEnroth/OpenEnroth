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

        #TODO: remove that workaround once dependencies archives are updated
        set(DEP_PLATFORM ${BUILD_PLATFORM})
        if (DEP_PLATFORM STREQUAL "windows")
            set(DEP_PLATFORM "win32")
        endif()

        set(DEP_ARCHITECTURE ${BUILD_ARCHITECTURE})
        if (DEP_ARCHITECTURE STREQUAL "x86_64")
            set(DEP_ARCHITECTURE "x64")
        endif()

        set(LIB_DIR "${CMAKE_SOURCE_DIR}/lib")
        set(LIBRARY_DIR "${LIB_DIR}/${DEP_PLATFORM}/${DEP_ARCHITECTURE}")

        set(DEPS_ZIP_FILENAME "all_deps_${DEP_PLATFORM}_${DEP_ARCHITECTURE}.zip")
        set(DEPS_ZIP_FULL_PATH "${LIB_DIR}/${DEPS_ZIP_FILENAME}")
        if(BUILD_PLATFORM STREQUAL "windows" AND BUILD_ARCHITECTURE STREQUAL "x86_64")
            set(DEPS_ZIP_MD5_CHECKSUM "02cbd9b53a221f6014528305c1d1f728")
        elseif(BUILD_PLATFORM STREQUAL "windows" AND BUILD_ARCHITECTURE STREQUAL "x86")
            set(DEPS_ZIP_MD5_CHECKSUM "1f6c8a2eb0394cb2eb72670db1af7432")
        else()
            message(FATAL_ERROR "Prebuilt dependencies for ${BUILD_PLATFORM}/${BUILD_ARCHITECTURE} are not available!")
        endif()

        # resolve 3d party libs
        if(NOT EXISTS "${LIBRARY_DIR}")
            if(NOT EXISTS "${DEPS_ZIP_FULL_PATH}")
                MESSAGE(STATUS "Downloading dependencies: ${DEPS_ZIP_FILENAME}")
                file(DOWNLOAD
                        "https://github.com/gp-alex/world-of-might-and-magic-deps/raw/e1f68f71952c7736e119c8f226d177aade6dd202/${DEPS_ZIP_FILENAME}"
                        "${DEPS_ZIP_FULL_PATH}"
                        SHOW_PROGRESS
                        STATUS DOWNLOAD_STATUS
                        EXPECTED_MD5 ${DEPS_ZIP_MD5_CHECKSUM}
                        TIMEOUT 60)  # seconds

                list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
                list(GET DOWNLOAD_STATUS 1 ERROR_MESSAGE)
                if(${STATUS_CODE} EQUAL 0)
                    message(STATUS "Prebuilt dependencies for ${BUILD_PLATFORM}/${BUILD_ARCHITECTURE} successfully downloaded!")
                else()
                    file(REMOVE "${DEPS_ZIP_FULL_PATH}")
                    message(FATAL_ERROR "Error occurred during download of dependencies for ${BUILD_PLATFORM}/${BUILD_ARCHITECTURE}: ${ERROR_MESSAGE}")
                endif()
            endif()

            execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf "${DEPS_ZIP_FULL_PATH}"
                    WORKING_DIRECTORY ${LIB_DIR}
                    RESULT_VARIABLE UNPACK_STATUS)

            if (UNPACK_STATUS)
                message(FATAL_ERROR "Error occurred during unpack of dependencies for ${BUILD_PLATFORM}/${BUILD_ARCHITECTURE}: ${UNPACK_STATUS}")
            endif()
        endif()

        if (BUILD_PLATFORM STREQUAL "windows")
            set(FFMPEG_DIR "${LIBRARY_DIR}/ffmpeg-4.2.2")
            set(FFMPEG_INCLUDE_DIRS "${FFMPEG_DIR}/include")
            set(FFMPEG_BIN_DIR "${FFMPEG_DIR}/bin")
            set(FFMPEG_LIB_DIR "${FFMPEG_DIR}/lib")
            set(AVCODEC_LIBRARIES "${FFMPEG_LIB_DIR}/avcodec.lib")
            set(AVDEVICE_LIBRARIES "${FFMPEG_LIB_DIR}/avdevice.lib")
            set(AVFILTER_LIBRARIES "${FFMPEG_LIB_DIR}/avfilter.lib")
            set(AVFORMAT_LIBRARIES "${FFMPEG_LIB_DIR}/avformat.lib")
            set(AVUTIL_LIBRARIES "${FFMPEG_LIB_DIR}/avutil.lib")
            set(POSTPROC_LIBRARIES "${FFMPEG_LIB_DIR}/postproc.lib")
            set(SWRESAMPLE_LIBRARIES "${FFMPEG_LIB_DIR}/swresample.lib")
            set(SWSCALE_LIBRARIES "${FFMPEG_LIB_DIR}/swscale.lib")
            prebuilt_dependencies_add("${FFMPEG_DIR}/bin/avcodec-58.dll"
                    "${FFMPEG_BIN_DIR}/avdevice-58.dll"
                    "${FFMPEG_BIN_DIR}/avfilter-7.dll"
                    "${FFMPEG_BIN_DIR}/avformat-58.dll"
                    "${FFMPEG_BIN_DIR}/avutil-56.dll"
                    "${FFMPEG_BIN_DIR}/postproc-55.dll"
                    "${FFMPEG_BIN_DIR}/swresample-3.dll"
                    "${FFMPEG_BIN_DIR}/swscale-5.dll")

            set(ZLIB_DIR "${LIBRARY_DIR}/zlib-1.2.11")
            set(ZLIB_INCLUDE_DIRS "${ZLIB_DIR}/include")
            set(ZLIB_BIN_DIR "${ZLIB_DIR}/bin")
            set(ZLIB_LIB_DIR "${ZLIB_DIR}/lib")
            set(ZLIB_LIBRARIES "${ZLIB_LIB_DIR}/zlibstatic.lib")

            add_library(ZLIB INTERFACE)
            add_library(ZLIB::ZLIB ALIAS ZLIB)
            target_link_libraries(ZLIB INTERFACE ${ZLIB_LIBRARIES})
            target_include_directories(ZLIB INTERFACE ${ZLIB_INCLUDE_DIRS})
        else()
            message(FATAL_ERROR "Prebuilt dependencies for ${BUILD_PLATFORM} are unknown!")
        endif()
    else()
        find_package(FFmpeg REQUIRED)
        find_package(OpenGL REQUIRED)
        find_package(ZLIB REQUIRED)
    endif()
endmacro()
