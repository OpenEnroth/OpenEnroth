#pragma once

#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIHouseEnums.h"

#include <string>

class GUIWindow_TownHall : public GUIWindow_House {
 public:
    explicit GUIWindow_TownHall(HOUSE_ID houseId) : GUIWindow_House(houseId) {}
    virtual ~GUIWindow_TownHall() {}

    virtual void houseDialogueOptionSelected(DIALOGUE_TYPE option) override;
    virtual void houseSpecificDialogue() override;

    std::string bountyHuntingText();

 protected:
    void mainDialogue();
    void bountyHuntDialogue();
    void payFineDialogue();

 private:
    int randomMonsterForHunting(HOUSE_ID townhall);
    void bountyHuntingDialogueOptionClicked();

    std::string _bountyHuntText = "";
    int _bountyHuntMonsterId = 0;
};
