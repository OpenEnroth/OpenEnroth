#pragma once

#include <Library/Fsm/FSM.h>
#include <Library/Platform/Filters/PlatformEventFilter.h>
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

    void stopVideo();

 private:
    std::string _videoFileName;
    std::shared_ptr<IMovie> _movie;
    ScreenType _previousScreenType{};
};
