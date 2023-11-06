#pragma once

#include "GUI/GUIWindow.h"

extern int pChestWidthsByType[8];
extern int pChestHeightsByType[8];
extern int pChestPixelOffsetX[8];
extern int pChestPixelOffsetY[8];

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
