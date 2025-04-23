#pragma once

#include "GUI/GUIWindow.h"

class GUIWindow_Chest : public GUIWindow {
 public:
    explicit GUIWindow_Chest(int chestId);
    virtual ~GUIWindow_Chest() {}

    virtual void Update() override;

    int chestId() const {
        return _chestId;
    }

 private:
    int _chestId = 0;
};
