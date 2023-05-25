#pragma once

#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIHouseEnums.h"

class GUIWindow_Temple : public GUIWindow_House {
 public:
    explicit GUIWindow_Temple(HOUSE_ID houseId) : GUIWindow_House(houseId) {}
    virtual ~GUIWindow_Temple() {}

    virtual void houseDialogueOptionSelected(DIALOGUE_TYPE option) override;
    virtual void houseSpecificDialogue() override;

 protected:
    void mainDialogue();
    void healDialogue();
    void donateDialogue();
    void learnSkillsDialogue();
};
