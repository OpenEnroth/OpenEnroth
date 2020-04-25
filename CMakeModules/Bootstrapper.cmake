include("${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/Common.cmake")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(APPLE TRUE)
endif()

# Support both 32 and 64 bit builds
if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
  set(BUILD_TYPE "x64")
  set(BUILD_PLATFORM "win32")
else ()
  set(BUILD_TYPE "x86")
  set(BUILD_PLATFORM "win32")
endif ()

DEBUG_PRINT("BUILD_TYPE     ${BUILD_TYPE}")
DEBUG_PRINT("BUILD_PLATFORM ${BUILD_PLATFORM}")

if( BUILD_TYPE STREQUAL "x64" )
  message(STATUS  "==============================================================================" )
  message(STATUS  " " )
  message(WARNING "Plase note that currently x64 builds could be unstable or fail to build at all" )
  message(STATUS  " " )
  message(STATUS  "==============================================================================" )
endif()


# required for unit testing using CMake's ctest command
include(CTest)
enable_testing()

# required modules for our task
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckIncludeFiles)

set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

if (WIN32)
# actual library dir for current build configuration
set(LIBRARY_DIR "${LIB_DIR}/${BUILD_PLATFORM}/${BUILD_TYPE}")

set(DEPS_ZIP_FILENAME "all_deps_${BUILD_PLATFORM}_${BUILD_TYPE}.zip")
set(DEPS_ZIP_FULL_PATH "${LIB_DIR}/${DEPS_ZIP_FILENAME}")


# resolve 3d party libs
if (NOT EXISTS "${LIBRARY_DIR}")
    if (NOT EXISTS "${DEPS_ZIP_FULL_PATH}")
        MESSAGE(STATUS "Downloading dependencies: ${DEPS_ZIP_FILENAME}")
        file(DOWNLOAD
            "https://github.com/gp-alex/world-of-might-and-magic-deps/raw/e1f68f71952c7736e119c8f226d177aade6dd202/${DEPS_ZIP_FILENAME}"
            "${DEPS_ZIP_FULL_PATH}"
            SHOW_PROGRESS
            TIMEOUT 60  # seconds
        )
    endif()

    execute_process(COMMAND ${CMAKE_COMMAND}
        -E tar xzf "${DEPS_ZIP_FULL_PATH}"
        WORKING_DIRECTORY ${LIB_DIR}
    )
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/thirdparty/OpenAL.cmake")

else()  # WIN32
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/find)
find_package(OpenGL REQUIRED)
#find_package(OPENAL REQUIRED)
if(APPLE)
    link_directories("/usr/local/lib")
    link_directories("/usr/local/opt/openal-soft/lib")
endif()
endif()
include("${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/thirdparty/zlib.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/thirdparty/sdl2.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/thirdparty/ffmpeg.cmake")


# we add the sub-directories that we want CMake to scan
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
