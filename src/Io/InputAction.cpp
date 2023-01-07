#include "Io/InputAction.h"

using Io::InputAction;

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
    case InputAction::StrafeLeft: return "STRAFE LEFT";
    case InputAction::StrafeRight: return "STRAFE RIGHT";
    case InputAction::QuickSave: return "QUICK SAVE";
    case InputAction::QuickLoad: return "QUICK LOAD";
    case InputAction::History: return "HISTORY";
    case InputAction::Stats: return "STATS";
    case InputAction::Skills: return "SKILLS";
    case InputAction::Awards: return "INVENTORY";
    case InputAction::Inventory: return "AWARDS";
    case InputAction::NewGame: return "NEW GAME";
    case InputAction::SaveGame: return "SAVE GAME";
    case InputAction::LoadGame: return "Load GAME";
    case InputAction::ExitGame: return "EXIT GAME";
    case InputAction::ReturnToGame: return "RETURN TO GAME";
    case InputAction::Controls: return "CONTROLS";
    case InputAction::Options: return "OPTIONS";
    case InputAction::Credits: return "CREDITS";
    case InputAction::Clear: return "CLEAR";
    case InputAction::Return: return "RETURN";
    case InputAction::Minus: return "MINUS";
    case InputAction::Plus: return "PLUS";
    case InputAction::Yes: return "YES";
    case InputAction::No: return "NO";
    case InputAction::Rest8Hours: return "REST 8 HOURS";
    case InputAction::WaitTillDawn: return "WAIT TILL DAWN";
    case InputAction::WaitHour: return "WAIT 1 HOUR";
    case InputAction::Wait5Minutes: return "WAIT 5 MINUTES";
    case InputAction::Screenshot: return "SCREENSHOT";
    case InputAction::Console: return "CONSOLE";
    case InputAction::ToggleMouseGrab: return "TOGGLE MOUSE GRAB";
    case InputAction::ToggleBorderless: return "TOGGLE BORDERLESS";
    case InputAction::ToggleFullscreen: return "TOGGLE FULLSCREEN";
    case InputAction::ToggleResizable: return "TOGGLE RESIZABLE";
    case InputAction::CycleFilter: return "CYCLE FILTER";
    case InputAction::ReloadShaders: return "RELOAD SHADERS";
    case InputAction::SelectChar1: return "CHAR 1";
    case InputAction::SelectChar2: return "CHAR 2";
    case InputAction::SelectChar3: return "CHAR 3";
    case InputAction::SelectChar4: return "CHAR 4";
    case InputAction::SelectNPC1: return "NPC 1";
    case InputAction::SelectNPC2: return "NPC 2";
    case InputAction::DialogUp: return "DIALOG UP";
    case InputAction::DialogDown: return "DIALOG DOWN";
    case InputAction::DialogLeft: return "DIALOG LEFT";
    case InputAction::DialogRight: return "DIALOG RIGHT";
    case InputAction::DialogSelect: return "DIALOG SELECT";
    case InputAction::Escape: return "ESCAPE";
    case InputAction::Invalid:
    default:
        return "-INVALID-ACTION-";
    }
}

const std::vector<InputAction> AllInputActions() {
    std::vector<InputAction> v;
    for (int i = 0; i <= (int)InputAction::Escape; ++i) {
        v.push_back((InputAction)i);
    }
    return v;
}

const std::vector<InputAction> VanillaInputActions() {
    std::vector<InputAction> v;
    for (int i = 0; i <= (int)InputAction::Land; ++i) {
        v.push_back((InputAction)i);
    }
    return v;
}

std::string ToString(InputAction action) {
#define ENUM_CASE(x) case Io::InputAction::x: return #x;
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
        ENUM_CASE(QuickSave)
        ENUM_CASE(QuickLoad)
        ENUM_CASE(History)
        ENUM_CASE(Stats)
        ENUM_CASE(Skills)
        ENUM_CASE(Inventory)
        ENUM_CASE(Awards)
        ENUM_CASE(NewGame)
        ENUM_CASE(SaveGame)
        ENUM_CASE(LoadGame)
        ENUM_CASE(ExitGame)
        ENUM_CASE(ReturnToGame)
        ENUM_CASE(Controls)
        ENUM_CASE(Options)
        ENUM_CASE(Credits)
        ENUM_CASE(Clear)
        ENUM_CASE(Return)
        ENUM_CASE(Minus)
        ENUM_CASE(Plus)
        ENUM_CASE(Yes)
        ENUM_CASE(No)
        ENUM_CASE(Rest8Hours)
        ENUM_CASE(WaitTillDawn)
        ENUM_CASE(WaitHour)
        ENUM_CASE(Wait5Minutes)
        ENUM_CASE(Screenshot)
        ENUM_CASE(Console)
        ENUM_CASE(ToggleMouseGrab)
        ENUM_CASE(ToggleBorderless)
        ENUM_CASE(ToggleFullscreen)
        ENUM_CASE(ToggleResizable)
        ENUM_CASE(CycleFilter)
        ENUM_CASE(ReloadShaders)
        ENUM_CASE(SelectChar2)
        ENUM_CASE(SelectChar3)
        ENUM_CASE(SelectChar4)
        ENUM_CASE(SelectNPC1)
        ENUM_CASE(SelectNPC2)
        ENUM_CASE(DialogUp)
        ENUM_CASE(DialogDown)
        ENUM_CASE(DialogLeft)
        ENUM_CASE(DialogRight)
        ENUM_CASE(DialogSelect)
        ENUM_CASE(Escape)
#undef ENUM_CASE
    default:
        return "-INVALID-VALUE-";
    }
}
