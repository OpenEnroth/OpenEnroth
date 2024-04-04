cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(MEDIA_AUDIO_SOURCES
        AudioPlayer.cpp
        AudioSamplePool.cpp
        OpenALAudioDataSource.cpp
        OpenALSoundProvider.cpp
        OpenALTrack16.cpp
        OpenALSample16.cpp
        SoundList.cpp)

set(MEDIA_AUDIO_HEADERS
        AudioPlayer.h
        AudioSamplePool.h
        OpenALAudioDataSource.h
        OpenALSoundProvider.h
        OpenALTrack16.h
        OpenALSample16.h
        OpenALUpdateThread.h
        SoundEnums.h
        SoundInfo.h
        SoundList.h)

add_library(media_audio STATIC ${MEDIA_AUDIO_SOURCES} ${MEDIA_AUDIO_HEADERS})
target_check_style(media_audio)
target_link_libraries(media_audio
        PUBLIC
        utility
        library_snd
        application
        # PRIVATE # TODO(captainurist): should be private
        OpenAL::OpenAL)
