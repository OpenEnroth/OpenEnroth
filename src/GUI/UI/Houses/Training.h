#pragma once

#include <vector>

#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIHouseEnums.h"

#include "Utility/IndexedArray.h"

void TrainingDialog(const char *s);

class GUIWindow_Training : public GUIWindow_House {
 public:
    explicit GUIWindow_Training(HOUSE_ID houseId);
    virtual ~GUIWindow_Training() {}

    virtual void houseDialogueOptionSelected(DIALOGUE_TYPE option) override;
    virtual void houseSpecificDialogue() override;

 protected:
    void mainDialogue();
    void trainDialogue();
    void learnSkillsDialogue();

 private:
    std::vector<int> _charactersTrainedLevels;
};
