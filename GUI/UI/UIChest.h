#pragma once

#include "GUI/GUIWindow.h"

class GUIWindow_Chest : public GUIWindow {
 public:
    explicit GUIWindow_Chest(unsigned int chest_id);
    virtual ~GUIWindow_Chest() {}

    virtual void Update();
};
