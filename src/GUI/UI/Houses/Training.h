#pragma once

#include <vector>

#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIHouseEnums.h"

#include "Utility/IndexedArray.h"

/**
 * Level cap of a training hall, the highest level its trainer will train a member to.
 *
 * @param hall                          Training hall house id.
 * @return                              Highest trainable level.
 */
int trainingHallMaxLevel(HouseId hall);

class GUIWindow_Training : public GUIWindow_House {
 public:
    explicit GUIWindow_Training(HouseId houseId);
    virtual ~GUIWindow_Training() {}

    virtual void houseDialogueOptionSelected(DialogueId option) override;
    virtual void houseSpecificDialogue() override;
    virtual std::vector<DialogueId> listDialogueOptions() override;
    virtual void updateDialogueOnEscape() override;

 protected:
    void mainDialogue();
    void trainDialogue();

 private:
    std::vector<int> _charactersTrainedLevels;
};
