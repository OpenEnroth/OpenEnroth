#pragma once

#include <Library/Fsm/FSM.h>
#include <GUI/GUIEnums.h>

#include <string>
#include <memory>

class IMovie;

class VideoState : public FSMState {
 public:
    explicit VideoState(std::string_view videoFileName);
    virtual void update() override;
    virtual void enter() override;
    virtual void exit() override;

 private:
    virtual bool mousePressEvent(const PlatformMouseEvent *event) override;
    virtual bool keyPressEvent(const PlatformKeyEvent *event) override;
    void _skipVideo();

    std::string _videoFileName;
    std::shared_ptr<IMovie> _movie;
    ScreenType _previousScreenType{};
    bool _isPaused{};
};
