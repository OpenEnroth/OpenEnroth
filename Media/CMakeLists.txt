cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

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

target_link_libraries(media openal)
target_link_libraries(media ffmpeg)
