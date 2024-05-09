#pragma once

#include <string>

#include "Engine/MapEnums.h"

#include "GUI/GUIWindow.h"
#include "GUI/UI/UIHouseEnums.h"

class GUIWindow_Travel : public GUIWindow {
 public:
    GUIWindow_Travel();
    virtual ~GUIWindow_Travel() {}

    virtual void Update() override;
    virtual void Release() override;
};

class GUIWindow_Transition : public GUIWindow {
 public:
    GUIWindow_Transition(HouseId transitionHouse, uint32_t exit_pic_id, Vec3f pos, int yaw, int pitch, int zspeed, std::string_view locationName);
    virtual ~GUIWindow_Transition() {}

    virtual void Update() override;
    virtual void Release() override;

    MapId mapid = MAP_INVALID;
    std::string _mapName{};
};

extern std::string transition_button_label;
