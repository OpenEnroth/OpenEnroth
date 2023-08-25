#pragma once

#include <stdint.h>
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIHouseEnums.h"

enum DIALOGUE_TYPE : int32_t;

class GUIWindow_Jail : public GUIWindow_House {
 public:
    explicit GUIWindow_Jail(HOUSE_ID houseId) : GUIWindow_House(houseId) {}
    virtual ~GUIWindow_Jail() {}

    virtual void houseDialogueOptionSelected(DIALOGUE_TYPE option) override;
    virtual void houseSpecificDialogue() override;
};
