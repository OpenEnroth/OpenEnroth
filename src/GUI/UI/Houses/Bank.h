#pragma once

#include <vector>

#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIHouseEnums.h"

class GUIWindow_Bank : public GUIWindow_House {
 public:
    explicit GUIWindow_Bank(HOUSE_ID houseId) : GUIWindow_House(houseId) {}
    virtual ~GUIWindow_Bank() {}

    virtual void houseDialogueOptionSelected(DIALOGUE_TYPE option) override;
    virtual void houseSpecificDialogue() override;
    virtual std::vector<DIALOGUE_TYPE> listDialogueOptions() override;
    virtual void playHouseGoodbyeSpeech() override;

 protected:
    void mainDialogue();
    void putGoldDialogue();
    void getGoldDialogue();
};
