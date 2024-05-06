#pragma once

#include <Library/Fsm/FsmState.h>
#include <GUI/GUIEnums.h>

#include <string>
#include <memory>

class IMovie;

class VideoState : public FsmState {
 public:
    explicit VideoState(std::string_view videoFileName, std::string_view videoGroup);
    virtual FsmAction update() override;
    virtual FsmAction enter() override;
    virtual void exit() override;

    static const std::string_view LOGO_GROUP;
    static const std::string_view INTRO_GROUP;

 private:
    virtual bool mousePressEvent(const PlatformMouseEvent *event) override;
    virtual bool keyPressEvent(const PlatformKeyEvent *event) override;

    std::string _videoFileName;
    std::string _videoGroup;
    std::shared_ptr<IMovie> _movie;
    ScreenType _previousScreenType{};
    bool _isPaused{};
    bool _skipVideo{};
};
