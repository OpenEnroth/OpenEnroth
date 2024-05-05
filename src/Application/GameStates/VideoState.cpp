#include "VideoState.h"

#include <Engine/Engine.h>
#include <Engine/EngineGlobals.h>
#include <Engine/Time/Timer.h>
#include <Media/Audio/AudioPlayer.h>
#include <Media/MediaPlayer.h>

#include <GUI/GUIWindow.h>

VideoState::VideoState(std::string_view videoFileName) : _videoFileName(videoFileName) {
}

void VideoState::enter() {
    _skipVideo = false;
    if (engine->config->debug.NoVideo.value()) {
        _skipVideo = true;
        return;
    }

    _movie = pMediaPlayer->loadFullScreenMovie(_videoFileName.c_str());
    if (!_movie) {
        _skipVideo = true;
        return;
    }

    // Stop the event timer and audio before playing a video
    pEventTimer->setPaused(true);
    pAudioPlayer->pauseLooping();
    pAudioPlayer->MusicPause();

    // Also hide the mouse cursor
    platform->setCursorShown(false);

    // Wish we could get rid of this type of screen states
    _previousScreenType = current_screen_type;
    current_screen_type = SCREEN_VIDEO;

    // Actually, calling Play() does not play something but just setup some internal flags.
    _movie->Play();
}

void VideoState::update() {
    if (!_movie || _skipVideo)
        _scheduleTransition("videoEnd");

    bool isOver = _movie->renderFrame();
    if (isOver)
        _scheduleTransition("videoEnd");
}

void VideoState::exit() {
    _movie = nullptr;
    // restore the screen type that was set before the video started
    current_screen_type = _previousScreenType;
    platform->setCursorShown(true);
}

bool VideoState::mousePressEvent(const PlatformMouseEvent *event) {
    // We skip the video if we press any mouse button
    _skipVideo = true;
    return true;
}

bool VideoState::keyPressEvent(const PlatformKeyEvent *event) {
    // We skip the video if we press any key button
    if (!event->isAutoRepeat) {
        _skipVideo = true;
        return true;
    }
    return false;
}
