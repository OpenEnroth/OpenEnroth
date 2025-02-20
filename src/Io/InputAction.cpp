#include "InputAction.h"

#include <string>
#include <vector>

#include "Library/Serialization/EnumSerialization.h"

using Io::InputAction;

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(InputAction, CASE_INSENSITIVE, {
    {InputAction::MoveForward, "FORWARD"},
    {InputAction::MoveBackwards, "BACKWARD"},
    {InputAction::TurnLeft, "LEFT"},
    {InputAction::TurnRight, "RIGHT"},
    {InputAction::Yell, "YELL"},
    {InputAction::Jump, "JUMP"},
    {InputAction::Combat, "COMBAT"},
    {InputAction::CastReady, "CAST READY"},
    {InputAction::Attack, "ATTACK"},
    {InputAction::EventTrigger, "TRIGGER"},
    {InputAction::Cast, "CAST"},
    {InputAction::Pass, "PASS"},
    {InputAction::CharCycle, "CHAR CYCLE"},
    {InputAction::Quest, "QUEST"},
    {InputAction::QuickRef, "QUICK REF"},
    {InputAction::Rest, "REST"},
    {InputAction::TimeCal, "TIME/CAL"},
    {InputAction::Autonotes, "AUTONOTES"},
    {InputAction::Mapbook, "MAP BOOK"},
    {InputAction::AlwaysRun, "ALWAYS RUN"},
    {InputAction::LookUp, "LOOK UP"},
    {InputAction::LookDown, "LOOK DOWN"},
    {InputAction::CenterView, "CTR VIEW"},
    {InputAction::ZoomIn, "ZOOM IN"},
    {InputAction::ZoomOut, "ZOOM OUT"},
    {InputAction::FlyUp, "FLY UP"},
    {InputAction::FlyDown, "FLY DOWN"},
    {InputAction::Land, "LAND"},
    {InputAction::StrafeLeft, "STRAFE LEFT"},
    {InputAction::StrafeRight, "STRAFE RIGHT"},
    {InputAction::QuickSave, "QUICK SAVE"},
    {InputAction::QuickLoad, "QUICK LOAD"},
    {InputAction::History, "HISTORY"},
    {InputAction::Stats, "STATS"},
    {InputAction::Skills, "SKILLS"},
    {InputAction::Awards, "INVENTORY"},
    {InputAction::Inventory, "AWARDS"},
    {InputAction::NewGame, "NEW GAME"},
    {InputAction::SaveGame, "SAVE GAME"},
    {InputAction::LoadGame, "Load GAME"},
    {InputAction::ExitGame, "EXIT GAME"},
    {InputAction::ReturnToGame, "RETURN TO GAME"},
    {InputAction::Controls, "CONTROLS"},
    {InputAction::Options, "OPTIONS"},
    {InputAction::Credits, "CREDITS"},
    {InputAction::Clear, "CLEAR"},
    {InputAction::Return, "RETURN"},
    {InputAction::Minus, "MINUS"},
    {InputAction::Plus, "PLUS"},
    {InputAction::Yes, "YES"},
    {InputAction::No, "NO"},
    {InputAction::Rest8Hours, "REST 8 HOURS"},
    {InputAction::WaitTillDawn, "WAIT TILL DAWN"},
    {InputAction::WaitHour, "WAIT 1 HOUR"},
    {InputAction::Wait5Minutes, "WAIT 5 MINUTES"},
    {InputAction::Screenshot, "SCREENSHOT"},
    {InputAction::Console, "CONSOLE"},
    {InputAction::ToggleMouseGrab, "TOGGLE MOUSE GRAB"},
    {InputAction::ToggleBorderless, "TOGGLE BORDERLESS"},
    {InputAction::ToggleFullscreen, "TOGGLE FULLSCREEN"},
    {InputAction::ToggleResizable, "TOGGLE RESIZABLE"},
    {InputAction::CycleFilter, "CYCLE FILTER"},
    {InputAction::ReloadShaders, "RELOAD SHADERS"},
    {InputAction::SelectChar1, "CHAR 1"},
    {InputAction::SelectChar2, "CHAR 2"},
    {InputAction::SelectChar3, "CHAR 3"},
    {InputAction::SelectChar4, "CHAR 4"},
    {InputAction::SelectNPC1, "NPC 1"},
    {InputAction::SelectNPC2, "NPC 2"},
    {InputAction::DialogUp, "DIALOG UP"},
    {InputAction::DialogDown, "DIALOG DOWN"},
    {InputAction::DialogLeft, "DIALOG LEFT"},
    {InputAction::DialogRight, "DIALOG RIGHT"},
    {InputAction::DialogSelect, "DIALOG SELECT"},
    {InputAction::Escape, "ESCAPE"},
    {InputAction::ToggleMouseLook, "TOGGLE MOUSE LOOK"}
});

std::string GetDisplayName(InputAction action) {
    std::string result;
    if (trySerialize(action, &result))
        return result;
    return "-INVALID-ACTION-";
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
