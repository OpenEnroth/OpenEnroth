cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(MEDIA_SOURCES
        Audio/AudioPlayer.cpp
        Audio/OpenALSoundProvider.cpp
        Media.cpp
        MediaLogger.cpp
        MediaPlayer.cpp)

set(MEDIA_HEADERS
        Audio/AudioPlayer.h
        Audio/OpenALSoundProvider.h
        Audio/SoundEnums.h
        Audio/SoundInfo.h
        Media.h
        MediaLogger.h
        MediaPlayer.h)

add_library(media STATIC ${MEDIA_SOURCES} ${MEDIA_HEADERS})
target_link_libraries(media utility application)
target_compile_definitions(media PRIVATE FF_API_NEXT)

include_directories(${INCLUDE_DIRECTORIES} ${OPENAL_INCLUDE_DIRS} ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(media ${OPENAL_LIBRARY} ${AVCODEC_LIBRARIES} ${AVFORMAT_LIBRARIES} ${AVUTIL_LIBRARIES} ${SWSCALE_LIBRARIES} ${SWRESAMPLE_LIBRARIES})

target_check_style(media)

message(VERBOSE "FFMPEG_LIBRARIES: ${FFMPEG_LIBRARIES}")
message(VERBOSE "OPENAL_LIBRARY: ${OPENAL_LIBRARY}")
