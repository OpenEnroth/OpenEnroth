#pragma once

#include <Library/Fsm/FsmState.h>
#include <GUI/GUIEnums.h>
#include <Media/Movie.h>

#include <string>
#include <memory>

class VideoState : public FsmState {
 public:
    enum class Type {
        VIDEO_LOGO,
        VIDEO_INTRO
    };
    using enum Type;

    explicit VideoState(Type type, std::string_view videoFileName);
    virtual FsmAction update() override;
    virtual FsmAction enter() override;
    virtual void exit() override;

 private:
    virtual bool mousePressEvent(const PlatformMouseEvent *event) override;
    virtual bool keyPressEvent(const PlatformKeyEvent *event) override;

    Type _type;
    std::string _videoFileName;
    std::unique_ptr<IMovie> _movie;
    ScreenType _previousScreenType{};
    bool _isPaused{};
    bool _skipVideo{};
};
