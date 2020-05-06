#pragma once

#include <string>
#include <vector>

namespace Io {
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
}

const std::vector<Io::InputAction> AllInputActions();
std::string ToString(Io::InputAction action);
std::string GetDisplayName(Io::InputAction action);
