#pragma once

#include <string>
#include <vector>

#include "Engine/Objects/MonsterEnums.h"

#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIHouseEnums.h"

class GUIWindow_TownHall : public GUIWindow_House {
 public:
    explicit GUIWindow_TownHall(HouseId houseId) : GUIWindow_House(houseId) {}
    virtual ~GUIWindow_TownHall() {}

    virtual void houseDialogueOptionSelected(DIALOGUE_TYPE option) override;
    virtual void houseSpecificDialogue() override;
    virtual std::vector<DIALOGUE_TYPE> listDialogueOptions() override;

    /**
     * @return   Text to show after the player has clicked on the "Bounty Hunt" dialogue option.
     */
    std::string bountyHuntingText();

 protected:
    void mainDialogue();
    void bountyHuntDialogue();
    void payFineDialogue();

 private:
    MONSTER_TYPE randomMonsterForHunting(HouseId townhall);

    /**
     * Handler for the "Bounty Hunt" dialogue option in a town hall.
     *
     * Regenerates bounty if needed, gives gold for a completed bounty hunt, and updates the current reply message to
     * be retrieved later with a call to `bountyHuntingText`.
     */
    void bountyHuntingDialogueOptionClicked();

    std::string _bountyHuntText = "";
    MONSTER_TYPE _bountyHuntMonsterId = MONSTER_INVALID;
};
