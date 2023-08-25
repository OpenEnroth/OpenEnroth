#pragma once

#include <stdint.h>
#include <vector>
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIHouseEnums.h"
#include "Utility/IndexedArray.h"

enum DIALOGUE_TYPE : int32_t;

void TrainingDialog(const char *s);

class GUIWindow_Training : public GUIWindow_House {
 public:
    explicit GUIWindow_Training(HOUSE_ID houseId);
    virtual ~GUIWindow_Training() {}

    virtual void houseDialogueOptionSelected(DIALOGUE_TYPE option) override;
    virtual void houseSpecificDialogue() override;
    virtual std::vector<DIALOGUE_TYPE> listDialogueOptions() override;
    virtual void updateDialogueOnEscape() override;

 protected:
    void mainDialogue();
    void trainDialogue();

 private:
    std::vector<int> _charactersTrainedLevels;
};
