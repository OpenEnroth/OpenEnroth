#include "Io/InputAction.h"

using namespace Io;

std::string GetDisplayName(InputAction action) {
    switch (action) {
    case InputAction::MoveForward: return "FORWARD";
    case InputAction::MoveBackwards: return "BACKWARD";
    case InputAction::TurnLeft: return "LEFT";
    case InputAction::TurnRight: return "RIGHT";
    case InputAction::Yell: return "YELL";
    case InputAction::Jump: return "JUMP";
    case InputAction::Combat: return "COMBAT";
    case InputAction::CastReady: return "CAST READY";
    case InputAction::Attack: return "ATTACK";
    case InputAction::EventTrigger: return "TRIGGER";
    case InputAction::Cast: return "CAST";
    case InputAction::Pass: return "PASS";
    case InputAction::CharCycle: return "CHAR CYCLE";
    case InputAction::Quest: return "QUEST";
    case InputAction::QuickRef: return "QUICK REF";
    case InputAction::Rest: return "REST";
    case InputAction::TimeCal: return "TIME/CAL";
    case InputAction::Autonotes: return "AUTONOTES";
    case InputAction::Mapbook: return "MAP BOOK";
    case InputAction::AlwaysRun: return "ALWAYS RUN";
    case InputAction::LookUp: return "LOOK UP";
    case InputAction::LookDown: return "LOOK DOWN";
    case InputAction::CenterView: return "CTR VIEW";
    case InputAction::ZoomIn: return "ZOOM IN";
    case InputAction::ZoomOut: return "ZOOM OUT";
    case InputAction::FlyUp: return "FLY UP";
    case InputAction::FlyDown: return "FLY DOWN";
    case InputAction::Land: return "LAND";
    }
    return "-INVALID-ACTION-";
}

const std::vector<InputAction> AllInputActions() {
    std::vector<InputAction> v;
    for (int i = 0; i < (int)InputAction::StrafeRight; ++i) {
        v.push_back((InputAction)i);
    }
    return v;
}

std::string ToString(InputAction action) {
#define ENUM_CASE(x) x: return #x;
    switch (action) {
        ENUM_CASE(MoveForward)
        ENUM_CASE(MoveBackwards)
        ENUM_CASE(TurnLeft)
        ENUM_CASE(TurnRight)
        ENUM_CASE(Yell)
        ENUM_CASE(Jump)
        ENUM_CASE(Combat)
        ENUM_CASE(CastReady)
        ENUM_CASE(Attack)
        ENUM_CASE(EventTrigger)
        ENUM_CASE(Cast)
        ENUM_CASE(Pass)
        ENUM_CASE(CharCycle)
        ENUM_CASE(Quest)
        ENUM_CASE(QuickRef)
        ENUM_CASE(Rest)
        ENUM_CASE(TimeCal)
        ENUM_CASE(Autonotes)
        ENUM_CASE(Mapbook)
        ENUM_CASE(AlwaysRun)
        ENUM_CASE(LookUp)
        ENUM_CASE(LookDown)
        ENUM_CASE(CenterView)
        ENUM_CASE(ZoomIn)
        ENUM_CASE(ZoomOut)
        ENUM_CASE(FlyUp)
        ENUM_CASE(FlyDown)
        ENUM_CASE(Land)
        ENUM_CASE(StrafeLeft)
        ENUM_CASE(StrafeRight)
    }
#undef ENUM_CASE
    return "-INVALID-VALUE-";
}