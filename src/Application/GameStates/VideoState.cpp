#include "VideoState.h"

#include <Engine/Engine.h>
#include <Engine/EngineGlobals.h>
#include <Engine/Time/Timer.h>
#include <Library/Platform/Application/PlatformApplication.h>
#include <Media/Audio/AudioPlayer.h>
#include <Media/MediaPlayer.h>

#include <GUI/GUIWindow.h>

class VideoStateInputHandler: public PlatformEventFilter {
 public:
    VideoStateInputHandler(VideoState &videoState)
        : PlatformEventFilter({EVENT_MOUSE_BUTTON_PRESS, EVENT_KEY_PRESS})
        , _videoState(videoState) {
    }

    virtual bool mousePressEvent(const PlatformMouseEvent *event) override {
        if (event->button == BUTTON_LEFT) {
            _videoState.stopVideo();
            return true;
        }
        return false;
    }

    virtual bool keyPressEvent(const PlatformKeyEvent *event) override {
        if (!event->isAutoRepeat) {
            _videoState.stopVideo();
            return true;
        }
        return false;
    }

 private:
    VideoState &_videoState;
};

VideoState::VideoState(std::string_view videoFileName) : _videoFileName(videoFileName) {
}

void VideoState::enter() {
    ::application->installComponent(std::make_unique<VideoStateInputHandler>(*this));

    if (engine->config->debug.NoVideo.value()) {
        return;
    }

    _movie = pMediaPlayer->loadFullScreenMovie(_videoFileName.c_str());
    if (!_movie) {
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
    if (_movie) {
        bool isOver = _movie->renderFrame();
        if (isOver) {
            executeTransition("videoEnd");
        }
    } else {
        executeTransition("videoEnd");
    }
}

void VideoState::stopVideo() {
    executeTransition("videoEnd");
}

void VideoState::exit() {
    ::application->removeComponent<VideoStateInputHandler>();
    _movie = nullptr;
    // restore the screen type that was set before the video started
    current_screen_type = _previousScreenType;
    platform->setCursorShown(true);
}
