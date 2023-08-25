#pragma once

#include <stdint.h>
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIHouseEnums.h"

enum DIALOGUE_TYPE : int32_t;

void MercenaryGuildDialog();

class GUIWindow_MercenaryGuild : public GUIWindow_House {
 public:
    explicit GUIWindow_MercenaryGuild(HOUSE_ID houseId) : GUIWindow_House(houseId) {}
    virtual ~GUIWindow_MercenaryGuild() {}

    /**
     * @offset 0x4B6478
     *
     * @brief Mercenary guild in MM6.
     */
    virtual void houseDialogueOptionSelected(DIALOGUE_TYPE option) override;
    virtual void houseSpecificDialogue() override;
};
