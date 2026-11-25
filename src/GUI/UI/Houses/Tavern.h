#pragma once

#include <vector>

#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIHouseEnums.h"

class GUIButton;

/// Arcomage "Play" gate. MM7 needs the deck item (651); MM8 has none (651 is the Ground Wyvern Horn, the house deck is implied), so always true there.
bool partyCanPlayArcomage();

class GUIWindow_Tavern : public GUIWindow_House {
 public:
    explicit GUIWindow_Tavern(HouseId houseId) : GUIWindow_House(houseId) {}
    virtual ~GUIWindow_Tavern() {}

    virtual void houseDialogueOptionSelected(DialogueId option) override;
    virtual void houseSpecificDialogue() override;
    virtual std::vector<DialogueId> listDialogueOptions() override;
    virtual void updateDialogueOnEscape() override;

 protected:
    void mainDialogue();
    void arcomageMainDialogue();
    void arcomageRulesDialogue();
    void arcomageVictoryCondDialogue();
    void arcomageResultDialogue();
    void restDialogue();
    void buyFoodDialogue();
};

/**
 * MM8's Adventurer's Inn (screen id 29 in the original, a `HOUSE_TYPE_TAVERN` house that opens
 * directly, with no tavern dialogue options). Lists the Inn bench and
 * the active party (skipping the champion, slot 0) as clickable roster portraits. See Tavern.cpp
 * for the click/swap/dismiss logic.
 */
class GUIWindow_AdventurersInn : public GUIWindow_House {
 public:
    explicit GUIWindow_AdventurersInn(HouseId houseId) : GUIWindow_House(houseId) {}
    virtual ~GUIWindow_AdventurersInn() {}

    virtual void houseSpecificDialogue() override;
    virtual std::vector<DialogueId> listDialogueOptions() override;

    virtual void innClickBenchPortrait(int rosterId) override;
    virtual void innClickActivePortrait(int rosterId) override;
    virtual void innClickDismiss(int rosterId) override;

 protected:
    void rebuildButtonsIfNeeded();
    void releaseInnButtons();

    bool _layoutDirty = true;
    int _pendingSwapBenchRosterId = 0;
    int _pendingDismissRosterId = 0;
    std::vector<GUIButton *> _innButtons;
};
