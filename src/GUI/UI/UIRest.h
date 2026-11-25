#pragma once

#include <memory>

#include "GUI/GUIWindow.h"

#include "Core/Time/Time.h"

class GUIWindow_Rest : public GUIWindow {
 public:
    /**
     * @offset 0x41F6C1
     */
    GUIWindow_Rest();
    virtual ~GUIWindow_Rest() {}

    /**
     * @offset 0x41FA01
     */
    virtual void Update() override;

 private:
    Duration hourglassLoopTimer;
};

enum class RestType {
    REST_NONE = 0,
    REST_WAIT = 1,
    REST_HEAL = 2
};
using enum RestType;

extern std::shared_ptr<GraphicsImage> rest_ui_sky_frame_current;

// MM8 rest-screen art, all from EnglishD.lod (see GUIWindow_Rest's ctor for the list).
extern std::shared_ptr<GraphicsImage> rest_ui_restmain;         // "restmain" - the stone panel
extern std::shared_ptr<GraphicsImage> rest_ui_mm8_plate_1;      // "rdawnu" - Wait Until Dawn plate
extern std::shared_ptr<GraphicsImage> rest_ui_mm8_plate_2;      // "r1hu" - Wait One Hour plate
extern std::shared_ptr<GraphicsImage> rest_ui_mm8_plate_3;      // "r5mu" - Wait 5 Minutes plate
extern std::shared_ptr<GraphicsImage> rest_ui_mm8_plate_4;      // "r8hu" - Rest & Heal 8 Hours plate
extern std::shared_ptr<GraphicsImage> rest_ui_mm8_plate_exit;   // "rexitu" - Exit Rest plate

extern int foodRequiredToRest;
extern Duration remainingRestTime;
extern RestType currentRestType;
