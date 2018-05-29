cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project("World of Might and Magic")
set(PROJECT_VERSION "0.2.0")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Git)
if(GIT_FOUND)
  execute_process(COMMAND ${GIT_EXECUTABLE} describe --always --tags --dirty
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                  OUTPUT_VARIABLE WMM_VERSION
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(FIND ${WMM_VERSION} "." VERSION_FULL)
  if(VERSION_FULL STREQUAL "-1")
    string(REGEX REPLACE "^([0-9]+)\\..*" "\\1" PROJECT_VERSION_MAJOR "${PROJECT_VERSION}")
    string(REGEX REPLACE "^[0-9]+\\.([0-9]+).*" "\\1" PROJECT_VERSION_MINOR "${PROJECT_VERSION}")
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short=7 HEAD
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    OUTPUT_VARIABLE PROJECT_VERSION_PATCH
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(WMM_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
    set(PROJECT_VERSION "${WMM_VERSION}")
  else()
    string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" PROJECT_VERSION_MAJOR "${WMM_VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" PROJECT_VERSION_MINOR "${WMM_VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+.*-g([0-9a-f]+).*" "\\1" PROJECT_VERSION_PATCH "${WMM_VERSION}")
    set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
  endif()
else()
  set(WMM_VERSION "${PROJECT_VERSION}")
  string(REGEX REPLACE "^([0-9]+)\\..*" "\\1" PROJECT_VERSION_MAJOR "${PROJECT_VERSION}")
  string(REGEX REPLACE "^[0-9]+\\.([0-9]+).*" "\\1" PROJECT_VERSION_MINOR "${PROJECT_VERSION}")
  string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" PROJECT_VERSION_PATCH "${PROJECT_VERSION}")
endif()

message(STATUS "WMM_VERSION = ${WMM_VERSION}")
message(STATUS "PROJECT_VERSION = ${PROJECT_VERSION}")
message(STATUS "PROJECT_VERSION_MAJOR = ${PROJECT_VERSION_MAJOR}")
message(STATUS "PROJECT_VERSION_MINOR = ${PROJECT_VERSION_MINOR}")
message(STATUS "PROJECT_VERSION_PATCH = ${PROJECT_VERSION_PATCH}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "-std=gnu++11 ${CMAKE_CXX_FLAGS}")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ ${CMAKE_CXX_FLAGS}")
endif()

include_directories(${INCLUDE_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DPACKAGE_BUGREPORT="https://github.com/gp-alex/world-of-might-and-magic/issues")

find_package(FFmpeg REQUIRED COMPONENTS AVFORMAT AVUTIL AVCODEC SWSCALE SWRESAMPLE)
find_package(OpenAL REQUIRED)
set(ZLIB_ROOT "$ENV{ZLIBDIR}")
find_package(ZLIB REQUIRED)
find_package(SDL2 REQUIRED)

include_directories(${INCLUDE_DIRECTORIES} ${FFMPEG_INCLUDE_DIRS})
include_directories(${INCLUDE_DIRECTORIES} ${OPENAL_INCLUDE_DIR})
include_directories(${INCLUDE_DIRECTORIES} ${ZLIB_INCLUDE_DIR})

include(CppLint)
enable_check_style()

add_subdirectory(Arcomage)
add_subdirectory(Engine)
add_subdirectory(src)
add_subdirectory(GUI)
add_subdirectory(IO)
add_subdirectory(Launcher)
add_subdirectory(Media)
add_subdirectory(Platform)

SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")

add_executable(World_of_Might_and_Magic MACOSX_BUNDLE WIN32 ./Engine/Engine.cpp)
target_check_style(World_of_Might_and_Magic)

target_link_libraries(World_of_Might_and_Magic game arcomage engine gui io media platform objects spells tables graphics turnengine serialization)
target_link_libraries(World_of_Might_and_Magic ${FFMPEG_LIBRARIES})
target_link_libraries(World_of_Might_and_Magic ${OPENAL_LIBRARY})
target_link_libraries(World_of_Might_and_Magic ${ZLIB_LIBRARY_RELEASE})
target_link_libraries(World_of_Might_and_Magic ${SDL2_LIBRARY})
target_link_libraries(World_of_Might_and_Magic ${SDL2_MAIN_LIBRARY})
target_link_libraries(World_of_Might_and_Magic ddraw.lib dxguid.lib)

if(WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(VC_LIB_PATH_SUFFIX Win64)
    set(VC_LIB_PATH_SUFFIX2 x64)
  else()
    set(VC_LIB_PATH_SUFFIX Win32)
    set(VC_LIB_PATH_SUFFIX2 x86)
  endif()

  add_custom_command(TARGET World_of_Might_and_Magic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$ENV{ZLIBDIR}/zlib1.dll" $<TARGET_FILE_DIR:World_of_Might_and_Magic>)

  add_custom_command(TARGET World_of_Might_and_Magic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$ENV{OPENALDIR}/bin/${VC_LIB_PATH_SUFFIX}/soft_oal.dll" $<TARGET_FILE_DIR:World_of_Might_and_Magic>)

  add_custom_command(TARGET World_of_Might_and_Magic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$ENV{FFMPEGDIR}/bin/${VC_LIB_PATH_SUFFIX}/avcodec-58.dll" $<TARGET_FILE_DIR:World_of_Might_and_Magic>)
  add_custom_command(TARGET World_of_Might_and_Magic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$ENV{FFMPEGDIR}/bin/${VC_LIB_PATH_SUFFIX}/avformat-58.dll" $<TARGET_FILE_DIR:World_of_Might_and_Magic>)
  add_custom_command(TARGET World_of_Might_and_Magic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$ENV{FFMPEGDIR}/bin/${VC_LIB_PATH_SUFFIX}/avutil-56.dll" $<TARGET_FILE_DIR:World_of_Might_and_Magic>)
  add_custom_command(TARGET World_of_Might_and_Magic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$ENV{FFMPEGDIR}/bin/${VC_LIB_PATH_SUFFIX}/swresample-3.dll" $<TARGET_FILE_DIR:World_of_Might_and_Magic>)
  add_custom_command(TARGET World_of_Might_and_Magic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$ENV{FFMPEGDIR}/bin/${VC_LIB_PATH_SUFFIX}/swscale-5.dll" $<TARGET_FILE_DIR:World_of_Might_and_Magic>)
  add_custom_command(TARGET World_of_Might_and_Magic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$ENV{SDL2DIR}/lib/${VC_LIB_PATH_SUFFIX2}/sdl2.dll" $<TARGET_FILE_DIR:World_of_Might_and_Magic>)
endif()
