#pragma once

#include <vector>

#include "GUI/UI/UIHouses.h"

class GUIWindow_MagicGuild : public GUIWindow_House {
 public:
    explicit GUIWindow_MagicGuild(HouseId houseId) : GUIWindow_House(houseId) {}
    virtual ~GUIWindow_MagicGuild() {}

    virtual void houseDialogueOptionSelected(DialogueId option) override;
    virtual void houseSpecificDialogue() override;
    virtual std::vector<DialogueId> listDialogueOptions() override;
    virtual void houseScreenClick() override;

 protected:
    /**
     * @offset 0x4BC8D5
     */
    void generateSpellBooksForGuild();

    void mainDialogue();
    void buyBooksDialogue();
};
