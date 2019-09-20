include("${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/Common.cmake")


SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")

# Support both 32 and 64 bit builds
if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
  set(BUILD_PLATFORM "x64")
  set(BUILD_WIN_PLATFORM "win64")
else ()
  set(BUILD_PLATFORM "x86")
  set(BUILD_WIN_PLATFORM "win32")
endif ()

DEBUG_PRINT("BUILD_PLATFORM     ${BUILD_PLATFORM}")
DEBUG_PRINT("BUILD_WIN_PLATFORM ${BUILD_WIN_PLATFORM}")


# required for unit testing using CMake's ctest command
include(CTest)
enable_testing()

# required modules for our task
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckIncludeFiles)
include(ExternalProject)

include("${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/thirdparty/zlib.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/thirdparty/OpenAL.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/thirdparty/SDL2.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/thirdparty/ffmpeg.cmake")

# we add the sub-directories that we want CMake to scan
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)

