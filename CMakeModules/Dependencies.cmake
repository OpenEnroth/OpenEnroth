if(NOT BUILD_PLATFORM STREQUAL "windows")
  include_directories("/usr/local/include")
  link_directories("/usr/local/lib")
endif()

function(PREBUILT_DEPENDENCIES_RESOLVE targetName)
endfunction()

#TODO: all prebuilt dependency artifacts should be built and packaged in the same unified way.
#      so all code below could be drastically simplified and we wouldn't have per-platform blocks.

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
    set(OPENAL_LIBRARY "OpenAL")
    find_library(SDL2 SDL2)
    set(SDL2_LIBRARIES "SDL2")
    find_package(ZLIB REQUIRED)
elseif(PREBUILT_DEPENDENCIES)
  message(STATUS "Prebuilt dependencies have been enabled")

  function(PREBUILT_DEPENDENCIES_ADD)
    foreach(filename ${ARGV})
      if (NOT EXISTS ${filename})
        message(FATAL_ERROR "Prebuilt dependency does not exist: '${filename}' ")
      else()
        list(APPEND PREBUILT_DEPENDENCIES_LIST "${filename}")
        set (PREBUILT_DEPENDENCIES_LIST ${PREBUILT_DEPENDENCIES_LIST} PARENT_SCOPE)
      endif()
    endforeach()
  endfunction()

  function(PREBUILT_DEPENDENCIES_RESOLVE targetName)
    foreach(dep ${PREBUILT_DEPENDENCIES_LIST})
      message(STATUS "Copying binary dependency ${dep} to $<TARGET_FILE_DIR:${targetName}>")
      add_custom_command(TARGET ${targetName} POST_BUILD
                         COMMAND ${CMAKE_COMMAND} -E copy "${dep}" $<TARGET_FILE_DIR:${targetName}>)
    endforeach()
  endfunction()

  #TODO: remove that workaround once dependencies archives are updated
  set(DEP_PLATFORM ${BUILD_PLATFORM})
  if (BUILD_PLATFORM STREQUAL "windows")
    set(DEP_PLATFORM "win32")
  endif()
  
  set(LIB_DIR "${CMAKE_SOURCE_DIR}/lib")
  set(LIBRARY_DIR "${LIB_DIR}/${DEP_PLATFORM}/${BUILD_TYPE}")

  set(DEPS_ZIP_FILENAME "all_deps_${DEP_PLATFORM}_${BUILD_TYPE}.zip")
  set(DEPS_ZIP_FULL_PATH "${LIB_DIR}/${DEPS_ZIP_FILENAME}")
  if(BUILD_PLATFORM STREQUAL "windows" AND BUILD_TYPE STREQUAL "x64")
    set(DEPS_ZIP_MD5_CHECKSUM "02cbd9b53a221f6014528305c1d1f728")
  elseif(BUILD_PLATFORM STREQUAL "windows" AND BUILD_TYPE STREQUAL "x86")
    set(DEPS_ZIP_MD5_CHECKSUM "1f6c8a2eb0394cb2eb72670db1af7432")
  else()
    message(FATAL_ERROR "Prebuilt dependencies for ${BUILD_PLATFORM}/${BUILD_TYPE} are not available!")
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
        TIMEOUT 60  # seconds
      )

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
                    RESULT_VARIABLE UNPACK_STATUS
    )

    if (UNPACK_STATUS)
      message(FATAL_ERROR "Error occurred during unpack of dependencies for ${BUILD_PLATFORM}/${BUILD_TYPE}: ${UNPACK_STATUS}")
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
    PREBUILT_DEPENDENCIES_ADD("${FFMPEG_DIR}/bin/avcodec-58.dll"
                              "${FFMPEG_BIN_DIR}/avdevice-58.dll"
                              "${FFMPEG_BIN_DIR}/avfilter-7.dll"
                              "${FFMPEG_BIN_DIR}/avformat-58.dll"
                              "${FFMPEG_BIN_DIR}/avutil-56.dll"
                              "${FFMPEG_BIN_DIR}/postproc-55.dll"
                              "${FFMPEG_BIN_DIR}/swresample-3.dll"
                              "${FFMPEG_BIN_DIR}/swscale-5.dll")

    set(OPENAL_DIR "${LIBRARY_DIR}/openal-1.1")
    set(OPENAL_INCLUDE_DIRS "${OPENAL_DIR}/include")
    set(OPENAL_BIN_DIR "${OPENAL_DIR}/bin")
    set(OPENAL_LIB_DIR "${OPENAL_DIR}/lib")
    set(OPENAL_LIBRARY "${OPENAL_LIB_DIR}/OpenAL32.lib")
    PREBUILT_DEPENDENCIES_ADD("${OPENAL_BIN_DIR}/OpenAL32.dll"
                              "${OPENAL_BIN_DIR}/soft_oal.dll")

    set(SDL2_DIR "${LIBRARY_DIR}/sdl2-2.0.12")
    set(SDL2_INCLUDE_DIRS "${SDL2_DIR}/include")
    set(SDL2_BIN_DIR "${SDL2_DIR}/bin")
    set(SDL2_LIB_DIR "${SDL2_DIR}/lib")
    set(SDL2_LIBRARIES "${SDL2_LIB_DIR}/SDL2.lib"
                       "${SDL2_LIB_DIR}/SDL2main.lib")
    PREBUILT_DEPENDENCIES_ADD("${SDL2_BIN_DIR}/SDL2.dll")

    set(ZLIB_DIR "${LIBRARY_DIR}/zlib-1.2.11")
    set(ZLIB_INCLUDE_DIRS "${ZLIB_DIR}/include")
    set(ZLIB_BIN_DIR "${ZLIB_DIR}/bin")
    set(ZLIB_LIB_DIR "${ZLIB_DIR}/lib")
    set(ZLIB_LIBRARIES "${ZLIB_LIB_DIR}/zlibstatic.lib")
  else()
    message(FATAL_ERROR "Prebuilt dependencies for ${BUILD_PLATFORM} are unknown!")
  endif()
else()
  find_package(FFmpeg REQUIRED)
  find_package(OpenGL REQUIRED)
  find_package(SDL2 CONFIG REQUIRED)
  find_package(ZLIB REQUIRED)
endif()
