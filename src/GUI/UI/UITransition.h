#pragma once

#include <string>

#include "Engine/MapEnums.h"

#include "GUI/GUIWindow.h"
#include "GUI/UI/UIHouseEnums.h"

class GUIWindow_Transition : public GUIWindow {
 public:
    GUIWindow_Transition(WindowType windowType, ScreenType screenType);
    virtual ~GUIWindow_Transition() {}

    virtual void Release() override;

 protected:
    void createButtons(const std::string &okHint, const std::string &cancelHint, UIMessageType confirmMsg, UIMessageType cancelMsg);
};

class GUIWindow_Travel : public GUIWindow_Transition {
 public:
    GUIWindow_Travel();
    virtual ~GUIWindow_Travel() {}

    virtual void Update() override;
};

class GUIWindow_IndoorEntryExit : public GUIWindow_Transition {
 public:
    GUIWindow_IndoorEntryExit(HouseId transitionHouse, uint32_t exit_pic_id, Vec3f pos, int yaw, int pitch, int zspeed, std::string_view locationName);
    virtual ~GUIWindow_IndoorEntryExit() {}

    virtual void Update() override;

    std::string _mapName = "";
    int _transitionStringId = 0;
};
