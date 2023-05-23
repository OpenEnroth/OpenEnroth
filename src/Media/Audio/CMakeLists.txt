cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(MEDIA_AUDIO_SOURCES
        AudioPlayer.cpp
        OpenALSoundProvider.cpp)

set(MEDIA_AUDIO_HEADERS
        AudioPlayer.h
        OpenALSoundProvider.h
        SoundEnums.h
        SoundInfo.h)

add_library(media_audio STATIC ${MEDIA_AUDIO_SOURCES} ${MEDIA_AUDIO_HEADERS})
target_link_libraries(media_audio utility application)

target_check_style(media_audio)
