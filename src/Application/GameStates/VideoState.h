#pragma once

#include <Library/Fsm/FSM.h>

class VideoState : public FSMState {
 public:
    VideoState(std::string_view videoFileName);
    virtual void update() override;
    virtual void enter() override;
    virtual void exit() override;

 private:
    std::string _videoFileName;
};
