#include "CommonTapeRecorder.h"

#include <cassert>
#include <ranges>
#include <string>

#include "Engine/Objects/Character.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/NPC.h"
#include "Engine/mm7_data.h"
#include "Engine/Party.h"
#include "Engine/Engine.h"

#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIDialogue.h"
#include "GUI/UI/UIStatusBar.h"

#include "Utility/String/Ascii.h"

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
            for (const ItemGen &item : character.pInventoryItemList)
                result += item.uItemID != ITEM_NULL;
        result += pParty->pPickedItem.uItemID != ITEM_NULL;
        return result;
    });
}

TestTape<int> CommonTapeRecorder::totalHirelings() {
    return custom([] {
        FlatHirelings hirelings;
        hirelings.Prepare();
        return static_cast<int>(hirelings.Size());
    });
}

TestTape<bool> CommonTapeRecorder::hasItem(ItemId itemId) {
    return custom([itemId] { return pParty->hasItem(itemId); });
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

TestTape<MapId> CommonTapeRecorder::map() {
    return custom([] { return engine->_currentLoadedMapId; });
}

TestTape<ScreenType> CommonTapeRecorder::screen() {
    return custom([] { return current_screen_type; });
}

TestTape<std::string> CommonTapeRecorder::statusBar() {
    return custom([] { return engine->_statusBar->get(); });
}

TestTape<DialogueId> CommonTapeRecorder::dialogueType() {
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

TestTape<Time> CommonTapeRecorder::time() {
    return custom([] { return pParty->GetPlayingTime(); });
}

TestTape<bool> CommonTapeRecorder::turnBasedMode() {
    return custom([] { return pParty->bTurnBasedModeOn; });
}

TestTape<int> CommonTapeRecorder::mapItemCount() {
    return custom([] {
        return static_cast<int>(std::ranges::count_if(pSpriteObjects, [] (const SpriteObject &object) {
            return object.uObjectDescID != 0 && object.containing_item.uItemID != ITEM_NULL;
        }));
    });
}

TestTape<int> CommonTapeRecorder::mapItemCount(ItemId itemId) {
    return custom([itemId] {
        return static_cast<int>(std::ranges::count_if(pSpriteObjects, [itemId] (const SpriteObject &object) {
            return object.uObjectDescID != 0 && object.containing_item.uItemID == itemId;
        }));
    });
}

TestTape<HouseId> CommonTapeRecorder::house() {
    return custom([] {
        return window_SpeakInHouse ? window_SpeakInHouse->houseId() : HOUSE_INVALID;
    });
}

TestMultiTape<SpriteId> CommonTapeRecorder::sprites() {
    return custom([] {
        AccessibleVector<SpriteId> result;
        for (const SpriteObject &sprite : pSpriteObjects)
            result.push_back(sprite.uType);
        return result;
    });
}

TestTape<int> CommonTapeRecorder::activeCharacterIndex() {
    return custom([] { return pParty->hasActiveCharacter() ? pParty->activeCharacterIndex() : 0; });
}

TestMultiTape<SoundId> CommonTapeRecorder::sounds() {
    return _controller->recordFunctionTape<SoundId>(CALL_PLAY_SOUND);
}

TestMultiTape<std::string> CommonTapeRecorder::hudTextures() {
    return _controller->recordFunctionTape<std::string>(CALL_DRAW_2D_TEXTURE);
}

TestMultiTape<std::string> CommonTapeRecorder::messageBoxes() {
    return _controller->recordFunctionTape<std::string>(CALL_DRAW_MESSAGE_BOX);
}

TestMultiTape<std::string> CommonTapeRecorder::allGUIWindowsText() {
    return _controller->recordFunctionTape<std::string>(CALL_GUIWINDOW_DRAWTEXT);
}
