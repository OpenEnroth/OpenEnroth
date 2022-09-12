set(MSVC_PREBUILT_DEPS ON CACHE BOOL "Use prebuilt dependencies for MSVC")

function(RESOLVE_DEPENDENCIES targetName)
  if(MSVC_PREBUILT_DEPS AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    foreach(dep ${GLOBAL_DEPENDENCIES})
      message(STATUS "Copying binary dependency ${dep} to $<TARGET_FILE_DIR:${targetName}>")
      add_custom_command(TARGET ${targetName} POST_BUILD
                         COMMAND ${CMAKE_COMMAND} -E copy "${dep}" $<TARGET_FILE_DIR:${targetName}>)
    endforeach()
  endif()
endfunction()

if(MSVC_PREBUILT_DEPS AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  function(ADD_GLOBAL_DEPENDENCY filename)
    if (NOT EXISTS ${filename})
      message(STATUS "ADD_GLOBAL_DEPENDENCY(${filename}): file does not exist")
    else()
      list(APPEND GLOBAL_DEPENDENCIES "${filename}")
      set (GLOBAL_DEPENDENCIES ${GLOBAL_DEPENDENCIES} PARENT_SCOPE)
    endif()
  endfunction()

  set(LIB_DIR "${CMAKE_SOURCE_DIR}/lib")
  set(LIBRARY_DIR "${LIB_DIR}/${BUILD_PLATFORM}/${BUILD_TYPE}")

  set(DEPS_ZIP_FILENAME "all_deps_${BUILD_PLATFORM}_${BUILD_TYPE}.zip")
  set(DEPS_ZIP_FULL_PATH "${LIB_DIR}/${DEPS_ZIP_FILENAME}")

  # resolve 3d party libs
  if(NOT EXISTS "${LIBRARY_DIR}")
    if(NOT EXISTS "${DEPS_ZIP_FULL_PATH}")
      MESSAGE(STATUS "Downloading dependencies: ${DEPS_ZIP_FILENAME}")
      file(DOWNLOAD
        "https://github.com/gp-alex/world-of-might-and-magic-deps/raw/e1f68f71952c7736e119c8f226d177aade6dd202/${DEPS_ZIP_FILENAME}"
        "${DEPS_ZIP_FULL_PATH}"
        SHOW_PROGRESS
        TIMEOUT 60  # seconds
      )
    endif()

    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf "${DEPS_ZIP_FULL_PATH}"
                    WORKING_DIRECTORY ${LIB_DIR}
    )
  endif()

  set(FFMPEG_DIR "${LIBRARY_DIR}/ffmpeg-4.2.2")
  set(FFMPEG_INCLUDE_DIRS "${FFMPEG_DIR}/include")
  set(FFMPEG_LIB_DIR "${FFMPEG_DIR}/lib")
  set(AVCODEC_LIBRARIES "${FFMPEG_LIB_DIR}/avcodec.lib")
  set(AVDEVICE_LIBRARIES "${FFMPEG_LIB_DIR}/avdevice.lib")
  set(AVFILTER_LIBRARIES "${FFMPEG_LIB_DIR}/avfilter.lib")
  set(AVFORMAT_LIBRARIES "${FFMPEG_LIB_DIR}/avformat.lib")
  set(AVUTIL_LIBRARIES "${FFMPEG_LIB_DIR}/avutil.lib")
  set(POSTPROC_LIBRARIES "${FFMPEG_LIB_DIR}/postproc.lib")
  set(SWRESAMPLE_LIBRARIES "${FFMPEG_LIB_DIR}/swresample.lib")
  set(SWSCALE_LIBRARIES "${FFMPEG_LIB_DIR}/swscale.lib")
  ADD_GLOBAL_DEPENDENCY("${FFMPEG_DIR}/bin/avcodec-58.dll")
  ADD_GLOBAL_DEPENDENCY("${FFMPEG_DIR}/bin/avdevice-58.dll")
  ADD_GLOBAL_DEPENDENCY("${FFMPEG_DIR}/bin/avfilter-7.dll")
  ADD_GLOBAL_DEPENDENCY("${FFMPEG_DIR}/bin/avformat-58.dll")
  ADD_GLOBAL_DEPENDENCY("${FFMPEG_DIR}/bin/avutil-56.dll")
  ADD_GLOBAL_DEPENDENCY("${FFMPEG_DIR}/bin/postproc-55.dll")
  ADD_GLOBAL_DEPENDENCY("${FFMPEG_DIR}/bin/swresample-3.dll")
  ADD_GLOBAL_DEPENDENCY("${FFMPEG_DIR}/bin/swscale-5.dll")

  set(OPENAL_DIR "${LIBRARY_DIR}/openal-1.1")
  set(OPENAL_INCLUDE_DIRS "${OPENAL_DIR}/include")
  set(OPENAL_LIB_DIR "${OPENAL_DIR}/lib")
  set(OPENAL_LIBRARY "${OPENAL_LIB_DIR}/OpenAL32.lib")
  ADD_GLOBAL_DEPENDENCY("${OPENAL_DIR}/bin/OpenAL32.dll")
  
  set(SDL2_DIR "${LIBRARY_DIR}/sdl2-2.0.12")
  set(SDL2_INCLUDE_DIRS "${SDL2_DIR}/include")
  set(SDL2_LIB_DIR "${SDL2_DIR}/lib")
  set(SDL2_LIBRARIES "${SDL2_DIR}/lib/SDL2.lib"
                     "${SDL2_DIR}/lib/SDL2main.lib")
  ADD_GLOBAL_DEPENDENCY("${SDL2_DIR}/bin/SDL2.dll")

  set(ZLIB_DIR "${LIBRARY_DIR}/zlib-1.2.11")
  set(ZLIB_INCLUDE_DIRS "${ZLIB_DIR}/include")
  set(ZLIB_LIB_DIR "${ZLIB_DIR}/lib")
  set(ZLIB_LIBRARIES "${ZLIB_LIB_DIR}/zlibstatic.lib")
  
else()
  find_package(FFmpeg REQUIRED)
  find_package(OpenGL REQUIRED)
  find_package(OpenAL REQUIRED)
  find_package(SDL2 CONFIG REQUIRED)
  find_package(ZLIB REQUIRED)
endif()
