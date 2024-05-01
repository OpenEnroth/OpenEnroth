#include "VideoState.h"

#include <Media/MediaPlayer.h>

VideoState::VideoState(std::string_view videoFileName) : _videoFileName(videoFileName) {
}

void VideoState::enter() {
    _movie = pMediaPlayer->loadMovie(_videoFileName.c_str());
}

void VideoState::update() {
    if (!pMediaPlayer->drawMovie(_movie)) {
        executeTransition("videoEnd");
    }
}

void VideoState::exit() {
}
