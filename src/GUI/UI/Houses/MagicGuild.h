#pragma once

#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIHouseEnums.h"

#include "Utility/IndexedArray.h"

class GUIWindow_MagicGuild : public GUIWindow_House {
 public:
    GUIWindow_MagicGuild(HOUSE_ID houseId) : GUIWindow_House(houseId) {}
    virtual ~GUIWindow_MagicGuild() {}

    virtual void houseSpecificDialogue() override;

    /**
     * @offset 0x4BC8D5
     */
    void generateSpellBooksForGuild();
 protected:
    void mainDialogue();
    void buyBooksDialogue();
};

extern IndexedArray<int, HOUSE_FIRE_GUILD_INITIATE_EMERALD_ISLE, HOUSE_DARK_GUILD_PARAMOUNT_PIT> guildMembershipFlags;

void generateSpellBooksForGuild();
