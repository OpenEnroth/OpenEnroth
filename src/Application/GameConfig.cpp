#include "src/Application/GameConfig.h"

#include <algorithm>
#include <cctype>
#include <string>

#include "src/tools/DataPath.h"

#define MINI_CASE_SENSITIVE
#include "thirdparty/mini/src/mini/ini.h"

using Application::GameConfig;

mINI::INIStructure ini;

void LoadOption(std::string section, GameConfig::ConfigValue<bool> *val) {
    bool r = true;
    std::string v = ini[section].get(val->Name());

    if (v.empty()) {
        val->Reset();
        return;
    }

    if (v == "false" || v == "0")
        r = false;

    val->Set(r);
}

void LoadOption(std::string section, GameConfig::ConfigValue<float> *val) {
    std::string v = ini[section].get(val->Name());

    if (v.empty()) {
        val->Reset();
        return;
    }

    val->Set(std::stof(v));
}

void LoadOption(std::string section, GameConfig::ConfigValue<int> *val) {
    std::string v = ini[section].get(val->Name());

    if (v.empty()) {
        val->Reset();
        return;
    }

    val->Set(std::stoi(v));
}

void LoadOption(std::string section, GameConfig::ConfigValue<std::string> *val) {
    std::string v = ini[section].get(val->Name());

    if (v.empty()) {
        val->Reset();
        return;
    }

    val->Set(v);
}

void SaveOption(std::string section, GameConfig::ConfigValue<bool> *val) {
    std::string v = "false";
    if (val->Get())
        v = "true";

    ini[section].set(val->Name(), v);
}

void SaveOption(std::string section, GameConfig::ConfigValue<int> *val) {
    ini[section].set(val->Name(), std::to_string(val->Get()));
}

void SaveOption(std::string section, GameConfig::ConfigValue<float> *val) {
    ini[section].set(val->Name(), std::to_string(val->Get()));
}

void SaveOption(std::string section, GameConfig::ConfigValue<std::string> *val) {
    ini[section].set(val->Name(), val->Get());
}

void GameConfig::DefaultConfiguration() {
    debug.Default();
    gameplay.Default();
    graphics.Default();
    keybindings.Default();
    settings.Default();
    window.Default();
}

void GameConfig::LoadConfiguration() {
    std::string path = MakeDataPath(config_file);
    mINI::INIFile file(path);

    if (file.read(ini)) {
        debug.Load();
        gameplay.Load();
        graphics.Load();
        keybindings.Load();
        settings.Load();
        window.Load();

        printf("Configuration file '%s' loaded!\n", path.c_str());
    } else {
        DefaultConfiguration();
        printf("Cound not read configuration file '%s'! Loaded default configuration instead!\n", path.c_str());
    }
}

void GameConfig::SaveConfiguration() {
    mINI::INIFile file(MakeDataPath(config_file));

    debug.Save();
    gameplay.Save();
    graphics.Save();
    keybindings.Save();
    settings.Save();
    window.Save();

    file.write(ini, true);
}

void GameConfig::Startup() {
    LoadConfiguration();

    std::shared_ptr<std::string> value;
    if (command_line->TryFindKey("-nointro")) {
        debug.NoIntro.Set(true);
    }
    if (command_line->TryFindKey("-nologo")) {
        debug.NoLogo.Set(true);
    }
    if (command_line->TryFindKey("-nosound")) {
        debug.NoSound.Set(true);
    }
    if (command_line->TryFindKey("-novideo")) {
        debug.NoVideo.Set(true);
    }
    if (command_line->TryFindKey("-nomarg")) {
        debug.NoMargareth.Set(true);
    }
    if (command_line->TryFindKey("-verbose")) {
        debug.VerboseLogging.Set(true);
    }
    if (command_line->TryGetValue("render", &value)) {
        graphics.Renderer.Set(*value);
    }
    if (command_line->TryFindKey("-nowalksound")) {
        settings.WalkSound.Set(false);
    }
    if (command_line->TryFindKey("-nograb")) {
        window.MouseGrab.Set(false);
    }
    if (command_line->TryGetValue("display", &value)) {
        window.Display.Set(std::stoi(*value));
    }
    if (command_line->TryGetValue("window_width", &value)) {
        window.Width.Set(std::stoi(*value));
    }
    if (command_line->TryGetValue("window_height", &value)) {
        window.Height.Set(std::stoi(*value));
    }
    if (command_line->TryGetValue("window_x", &value)) {
        window.PositionX.Set(std::stoi(*value));
    }
    if (command_line->TryGetValue("window_y", &value)) {
        window.PositionY.Set(std::stoi(*value));
    }
    if (command_line->TryGetValue("fullscreen", &value)) {
        window.Fullscreen.Set(std::stoi(*value));
    }
    if (command_line->TryGetValue("borderless", &value)) {
        window.Borderless.Set(std::stoi(*value));
    }
    // minimal debug, w/o full magic etc.
    if (command_line->TryFindKey("-debug")) {
        debug.ShowFPS.Set(true);
        debug.ShowPickedFace.Set(true);
        debug.TownPortal.Set(true);
        debug.InfiniteFood.Set(true);
        debug.InfiniteGold.Set(true);
    }
}

GameConfig::GameConfig(const std::string &command_line_str) {
    command_line = std::make_shared<CommandLine>(command_line_str);
}

void GameConfig::Debug::Default() {
    AllMagic.Reset();
    InfiniteFood.Reset();
    InfiniteGold.Reset();
    LightmapDecals.Reset();
    NoActors.Reset();
    NoDamage.Reset();
    NoDecorations.Reset();
    NoIntro.Reset();
    NoLogo.Reset();
    NoMargareth.Reset();
    NoSound.Reset();
    NoVideo.Reset();
    PortalOutlines.Reset();
    ShowFPS.Reset();
    ShowPickedFace.Reset();
    Terrain.Reset();
    TownPortal.Reset();
    TurboSpeed.Reset();
    VerboseLogging.Reset();
    WizardEye.Reset();
}

void GameConfig::Debug::Load() {
    LoadOption(section_name, &AllMagic);
    LoadOption(section_name, &InfiniteFood);
    LoadOption(section_name, &InfiniteGold);
    LoadOption(section_name, &LightmapDecals);
    LoadOption(section_name, &NoActors);
    LoadOption(section_name, &NoDamage);
    LoadOption(section_name, &NoDecorations);
    LoadOption(section_name, &NoIntro);
    LoadOption(section_name, &NoLogo);
    LoadOption(section_name, &NoMargareth);
    LoadOption(section_name, &NoSound);
    LoadOption(section_name, &NoVideo);
    LoadOption(section_name, &PortalOutlines);
    LoadOption(section_name, &ShowFPS);
    LoadOption(section_name, &ShowPickedFace);
    LoadOption(section_name, &Terrain);
    LoadOption(section_name, &TownPortal);
    LoadOption(section_name, &TurboSpeed);
    LoadOption(section_name, &VerboseLogging);
    LoadOption(section_name, &WizardEye);
}

void GameConfig::Debug::Save() {
    SaveOption(section_name, &AllMagic);
    SaveOption(section_name, &InfiniteFood);
    SaveOption(section_name, &InfiniteGold);
    SaveOption(section_name, &LightmapDecals);
    SaveOption(section_name, &NoActors);
    SaveOption(section_name, &NoDamage);
    SaveOption(section_name, &NoDecorations);
    SaveOption(section_name, &NoIntro);
    SaveOption(section_name, &NoLogo);
    SaveOption(section_name, &NoMargareth);
    SaveOption(section_name, &NoSound);
    SaveOption(section_name, &NoVideo);
    SaveOption(section_name, &PortalOutlines);
    SaveOption(section_name, &ShowFPS);
    SaveOption(section_name, &ShowPickedFace);
    SaveOption(section_name, &Terrain);
    SaveOption(section_name, &TownPortal);
    SaveOption(section_name, &TurboSpeed);
    SaveOption(section_name, &VerboseLogging);
    SaveOption(section_name, &WizardEye);
}

void GameConfig::Gameplay::Default() {
    ArtifactLimit.Reset();
    FloorChecksEps.Reset();
    Gravity.Reset();
    KeyboardInteractionDepth.Reset();
    MaxFlightHeight.Reset();
    MouseInfoDepthIndoor.Reset();
    MouseInfoDepthOutdoor.Reset();
    MouseInteractionDepth.Reset();
    RangedAttackDepth.Reset();
    ShowUndentifiedItem.Reset();
}

void GameConfig::Gameplay::Load() {
    LoadOption(section_name, &ArtifactLimit);
    LoadOption(section_name, &FloorChecksEps);
    LoadOption(section_name, &Gravity);
    LoadOption(section_name, &KeyboardInteractionDepth);
    LoadOption(section_name, &MaxFlightHeight);
    LoadOption(section_name, &MouseInfoDepthIndoor);
    LoadOption(section_name, &MouseInfoDepthOutdoor);
    LoadOption(section_name, &MouseInteractionDepth);
    LoadOption(section_name, &RangedAttackDepth);
    LoadOption(section_name, &ShowUndentifiedItem);
}

void GameConfig::Gameplay::Save() {
    SaveOption(section_name, &ArtifactLimit);
    SaveOption(section_name, &FloorChecksEps);
    SaveOption(section_name, &Gravity);
    SaveOption(section_name, &KeyboardInteractionDepth);
    SaveOption(section_name, &MaxFlightHeight);
    SaveOption(section_name, &MouseInfoDepthIndoor);
    SaveOption(section_name, &MouseInfoDepthOutdoor);
    SaveOption(section_name, &MouseInteractionDepth);
    SaveOption(section_name, &RangedAttackDepth);
    SaveOption(section_name, &ShowUndentifiedItem);
}

void GameConfig::Graphics::Default() {
    AlternativePaletteMode.Reset();
    BloodSplats.Reset();
    BloodSplatsMultiplier.Reset();
    ClipFarDistance.Reset();
    ClipNearDistance.Reset();
    ColoredLights.Reset();
    D3DDevice.Reset();
    DynamicBrightness.Reset();
    Fog.Reset();
    Gamma.Reset();
    HWLBitmaps.Reset();
    HWLSprites.Reset();
    Renderer.Reset();
    SeasonsChange.Reset();
    Snow.Reset();
    SoftwareModeRules.Reset();
    Tinting.Reset();
    Torchlight.Reset();
}

void GameConfig::Graphics::Load() {
    LoadOption(section_name, &AlternativePaletteMode);
    LoadOption(section_name, &BloodSplats);
    LoadOption(section_name, &BloodSplatsMultiplier);
    LoadOption(section_name, &ClipFarDistance);
    LoadOption(section_name, &ClipNearDistance);
    LoadOption(section_name, &ColoredLights);
    LoadOption(section_name, &D3DDevice);
    LoadOption(section_name, &DynamicBrightness);
    LoadOption(section_name, &Fog);
    LoadOption(section_name, &Gamma);
    LoadOption(section_name, &HWLBitmaps);
    LoadOption(section_name, &HWLSprites);
    LoadOption(section_name, &Renderer);
    LoadOption(section_name, &SeasonsChange);
    LoadOption(section_name, &Snow);
    LoadOption(section_name, &SoftwareModeRules);
    LoadOption(section_name, &Tinting);
    LoadOption(section_name, &Torchlight);
}

void GameConfig::Graphics::Save() {
    SaveOption(section_name, &AlternativePaletteMode);
    SaveOption(section_name, &BloodSplats);
    SaveOption(section_name, &BloodSplatsMultiplier);
    SaveOption(section_name, &ClipFarDistance);
    SaveOption(section_name, &ClipNearDistance);
    SaveOption(section_name, &ColoredLights);
    SaveOption(section_name, &D3DDevice);
    SaveOption(section_name, &DynamicBrightness);
    SaveOption(section_name, &Fog);
    SaveOption(section_name, &Gamma);
    SaveOption(section_name, &HWLBitmaps);
    SaveOption(section_name, &HWLSprites);
    SaveOption(section_name, &Renderer);
    SaveOption(section_name, &SeasonsChange);
    SaveOption(section_name, &Snow);
    SaveOption(section_name, &SoftwareModeRules);
    SaveOption(section_name, &Tinting);
    SaveOption(section_name, &Torchlight);
}

void GameConfig::Keybindings::Default() {
    AlwaysRun.Reset();
    Attack.Reset();
    AutoNotes.Reset();
    Backward.Reset();
    CastReady.Reset();
    Cast.Reset();
    CenterView.Reset();
    CharCycle.Reset();
    Combat.Reset();
    EventTrigger.Reset();
    FlyDown.Reset();
    FlyUp.Reset();
    Forward.Reset();
    Jump.Reset();
    Land.Reset();
    Left.Reset();
    LookDown.Reset();
    LookUp.Reset();
    MapBook.Reset();
    Pass.Reset();
    Quest.Reset();
    QuickReference.Reset();
    Rest.Reset();
    Right.Reset();
    StepLeft.Reset();
    StepRight.Reset();
    TimeCalendar.Reset();
    Yell.Reset();
    ZoomIn.Reset();
    ZoomOut.Reset();
}

void GameConfig::Keybindings::Load() {
    LoadOption(section_name, &AlwaysRun);
    LoadOption(section_name, &Attack);
    LoadOption(section_name, &AutoNotes);
    LoadOption(section_name, &Backward);
    LoadOption(section_name, &Cast);
    LoadOption(section_name, &CastReady);
    LoadOption(section_name, &CenterView);
    LoadOption(section_name, &CharCycle);
    LoadOption(section_name, &Combat);
    LoadOption(section_name, &EventTrigger);
    LoadOption(section_name, &FlyDown);
    LoadOption(section_name, &FlyUp);
    LoadOption(section_name, &Forward);
    LoadOption(section_name, &Jump);
    LoadOption(section_name, &Land);
    LoadOption(section_name, &Left);
    LoadOption(section_name, &LookDown);
    LoadOption(section_name, &LookUp);
    LoadOption(section_name, &MapBook);
    LoadOption(section_name, &Pass);
    LoadOption(section_name, &Quest);
    LoadOption(section_name, &QuickReference);
    LoadOption(section_name, &Rest);
    LoadOption(section_name, &Right);
    LoadOption(section_name, &StepLeft);
    LoadOption(section_name, &StepRight);
    LoadOption(section_name, &TimeCalendar);
    LoadOption(section_name, &Yell);
    LoadOption(section_name, &ZoomIn);
    LoadOption(section_name, &ZoomOut);
}

void GameConfig::Keybindings::Save() {
    SaveOption(section_name, &AlwaysRun);
    SaveOption(section_name, &Attack);
    SaveOption(section_name, &AutoNotes);
    SaveOption(section_name, &Backward);
    SaveOption(section_name, &Cast);
    SaveOption(section_name, &CastReady);
    SaveOption(section_name, &CenterView);
    SaveOption(section_name, &CharCycle);
    SaveOption(section_name, &Combat);
    SaveOption(section_name, &EventTrigger);
    SaveOption(section_name, &FlyDown);
    SaveOption(section_name, &FlyUp);
    SaveOption(section_name, &Forward);
    SaveOption(section_name, &Jump);
    SaveOption(section_name, &Land);
    SaveOption(section_name, &Left);
    SaveOption(section_name, &LookDown);
    SaveOption(section_name, &LookUp);
    SaveOption(section_name, &MapBook);
    SaveOption(section_name, &Pass);
    SaveOption(section_name, &Quest);
    SaveOption(section_name, &QuickReference);
    SaveOption(section_name, &Rest);
    SaveOption(section_name, &Right);
    SaveOption(section_name, &StepLeft);
    SaveOption(section_name, &StepRight);
    SaveOption(section_name, &TimeCalendar);
    SaveOption(section_name, &Yell);
    SaveOption(section_name, &ZoomIn);
    SaveOption(section_name, &ZoomOut);
}

void GameConfig::Settings::Default() {
    AlwaysRun.Reset();
    FlipOnExit.Reset();
    MusicLevel.Reset();
    ShowHits.Reset();
    SoundLevel.Reset();
    TurnSpeed.Reset();
    VerticalTurnSpeed.Reset();
    VoiceLevel.Reset();
    WalkSound.Reset();
}

void GameConfig::Settings::Load() {
    LoadOption(section_name, &AlwaysRun);
    LoadOption(section_name, &FlipOnExit);
    LoadOption(section_name, &MusicLevel);
    LoadOption(section_name, &ShowHits);
    LoadOption(section_name, &SoundLevel);
    LoadOption(section_name, &TurnSpeed);
    LoadOption(section_name, &VerticalTurnSpeed);
    LoadOption(section_name, &VoiceLevel);
    LoadOption(section_name, &WalkSound);
}

void GameConfig::Settings::Save() {
    SaveOption(section_name, &AlwaysRun);
    SaveOption(section_name, &FlipOnExit);
    SaveOption(section_name, &MusicLevel);
    SaveOption(section_name, &ShowHits);
    SaveOption(section_name, &SoundLevel);
    SaveOption(section_name, &TurnSpeed);
    SaveOption(section_name, &VerticalTurnSpeed);
    SaveOption(section_name, &VoiceLevel);
    SaveOption(section_name, &WalkSound);
}

void GameConfig::Window::Default() {
    Borderless.Reset();
    Display.Reset();
    Fullscreen.Reset();
    Height.Reset();
    MouseGrab.Reset();
    PositionX.Reset();
    PositionY.Reset();
    Title.Reset();
    Width.Reset();
}

void GameConfig::Window::Load() {
    LoadOption(section_name, &Borderless);
    LoadOption(section_name, &Display);
    LoadOption(section_name, &Fullscreen);
    LoadOption(section_name, &Height);
    LoadOption(section_name, &MouseGrab);
    LoadOption(section_name, &PositionX);
    LoadOption(section_name, &PositionY);
    LoadOption(section_name, &Title);
    LoadOption(section_name, &Width);
}

void GameConfig::Window::Save() {
    SaveOption(section_name, &Borderless);
    SaveOption(section_name, &Display);
    SaveOption(section_name, &Fullscreen);
    SaveOption(section_name, &Height);
    SaveOption(section_name, &MouseGrab);
    SaveOption(section_name, &PositionX);
    SaveOption(section_name, &PositionY);
    SaveOption(section_name, &Title);
    SaveOption(section_name, &Width);
}

