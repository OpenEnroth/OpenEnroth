cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(MEDIA_AUDIO_SOURCES
        AudioPlayer.cpp
        OpenALSoundProvider.cpp
        SoundList.cpp)

set(MEDIA_AUDIO_HEADERS
        AudioPlayer.h
        OpenALSoundProvider.h
        SoundEnums.h
        SoundInfo.h
        SoundList.h)

add_library(media_audio STATIC ${MEDIA_AUDIO_SOURCES} ${MEDIA_AUDIO_HEADERS})
target_check_style(media_audio)
target_link_libraries(media_audio
        PUBLIC
        utility
        application
        # PRIVATE # TODO(captainurist): should be private
        OpenAL::OpenAL)
