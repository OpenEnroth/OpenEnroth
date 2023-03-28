#pragma once

#include "GUI/GUIWindow.h"
#include "Engine/Time.h"

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
    virtual void Update();

 private:
    int hourglassLoopTimer;
};

// This window is unused.
#if 0
class GUIWindow_RestWindow : public GUIWindow {
 public:
    GUIWindow_RestWindow(Pointi position, Sizei dimensions, WindowData data, const char *hint) :
        GUIWindow(WINDOW_Rest, position, dimensions, data, hint)
    {}
    virtual ~GUIWindow_RestWindow() {}

    virtual void Update();
};
#endif

enum class REST_TYPE {
    REST_NONE = 0,
    REST_WAIT = 1,
    REST_HEAL = 2
};
using enum REST_TYPE;

extern class Image *rest_ui_sky_frame_current;
extern class Image *rest_ui_hourglass_frame_current;

extern int foodRequiredToRest;
extern GameTime remainingRestTime;
extern REST_TYPE currentRestType;
