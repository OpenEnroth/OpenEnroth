#include "CommonTapeRecorder.h"

#include "Engine/Objects/Character.h"
#include "Engine/mm7_data.h"
#include "Engine/Party.h"
#include "Engine/Engine.h"

#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/UIStatusBar.h"

#include "Utility/String.h"

CommonTapeRecorder::CommonTapeRecorder(TestController *controller) : _controller(controller) {
    assert(controller);
}

TestTape<int64_t> CommonTapeRecorder::totalExperience() {
    return custom([] {
        int64_t result = 0;
        for (const Character &character : pParty->pCharacters)
            result += character.experience;
        return result;
    });
}

TestTape<int> CommonTapeRecorder::totalHp() {
    return custom([] {
        int result = 0;
        for (const Character &character : pParty->pCharacters)
            result += character.health;
        return result;
    });
}

TestTape<int> CommonTapeRecorder::totalItemCount() {
    return custom([] {
        int result = 0;
        for (const Character &character : pParty->pCharacters)
            for (const ItemGen &item : character.pOwnItems)
                result += item.uItemID != ITEM_NULL;
        result += pParty->pPickedItem.uItemID != ITEM_NULL;
        return result;
    });
}

TestTape<bool> CommonTapeRecorder::hasItem(ItemId item) {
    return custom([item] { return pParty->hasItem(item); });
}

TestTape<int> CommonTapeRecorder::gold() {
    return custom([] { return pParty->GetGold(); });
}

TestTape<int> CommonTapeRecorder::food() {
    return custom([] { return pParty->GetFood(); });
}

TestTape<int> CommonTapeRecorder::deaths() {
    return custom([] { return pParty->uNumDeaths; });
}

TestTape<std::string> CommonTapeRecorder::map() {
    return custom([] { return toLower(pCurrentMapName); });
}

TestTape<ScreenType> CommonTapeRecorder::screen() {
    return custom([] { return current_screen_type; });
}

TestTape<std::string> CommonTapeRecorder::statusBar() {
    return custom([] { return engine->_statusBar->get(); });
}

TestTape<DIALOGUE_TYPE> CommonTapeRecorder::dialogueType() {
    return custom([] {
        if (GUIWindow_Dialogue *dlg = dynamic_cast<GUIWindow_Dialogue*>(pDialogueWindow)) {
            return dlg->getDisplayedDialogueType();
        } else if (GUIWindow_House *dlg = dynamic_cast<GUIWindow_House*>(window_SpeakInHouse)) {
            return dlg->getCurrentDialogue();
        } else {
            return DIALOGUE_NULL;
        }
    });
}

TestTape<GameTime> CommonTapeRecorder::time() {
    return custom([] { return pParty->GetPlayingTime(); });
}

TestTape<bool> CommonTapeRecorder::turnBasedMode() {
    return custom([] { return pParty->bTurnBasedModeOn; });
}
