#pragma once

#include <string>
#include <vector>

enum class InputAction : int {
    MoveForward, // = 0
    MoveBackwards,
    TurnLeft,
    TurnRight,
    Yell,
    Jump,
    Combat,
    CastReady,
    Attack,
    EventTrigger,
    Cast,
    Pass,
    CharCycle,
    Quest,
    QuickRef,
    Rest,
    TimeCal,
    Autonotes,
    Mapbook,
    AlwaysRun,
    LookUp,
    LookDown,
    CenterView,
    ZoomIn,
    ZoomOut,
    FlyUp,
    FlyDown,
    Land,
    StrafeLeft,
    StrafeRight,

    Invalid
};

const std::vector<InputAction> AllInputActions();
std::string ToString(InputAction action);
std::string GetDisplayName(InputAction action);
