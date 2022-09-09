cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

if(APPLE)
  add_definitions(-DFF_API_NEXT)
endif()

set(MEDIA_SOURCES Media.cpp
                  MediaPlayer.cpp
                  Audio/AudioPlayer.cpp
                  Audio/OpenALSoundProvider.cpp)

set(MEDIA_HEADERS Media.h
                  MediaPlayer.h
                  Audio/AudioPlayer.h
                  Audio/OpenALSoundProvider.h)

if(WIN32)
else()
endif()

add_library(media STATIC ${MEDIA_SOURCES} ${MEDIA_HEADERS})
target_check_style(media)

if(WIN32)
target_link_libraries(media openal)
else()
target_link_libraries(media ${OPENAL_LIBRARY})
endif()
target_link_libraries(media ffmpeg)
