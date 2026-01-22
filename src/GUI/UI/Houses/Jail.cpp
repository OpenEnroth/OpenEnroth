#include "GUI/UI/Houses/Jail.h"

#include "GUI/GUIFont.h"

#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"

void GUIWindow_Jail::houseSpecificDialogue() {
    Recti jail_dialogue_window = this->frameRect;
    jail_dialogue_window.x = SIDE_TEXT_BOX_POS_X;
    jail_dialogue_window.w = SIDE_TEXT_BOX_WIDTH;

    int vertMargin = (310 - assets->pFontArrus->CalcTextHeight(localization->str(LSTR_FOR_YOUR_NUMEROUS_CRIMES_AND_EVIL_DEEDS), jail_dialogue_window.w, 0)) / 2 + 18;
    DrawTitleText(assets->pFontArrus.get(), 0, vertMargin, colorTable.PaleCanary, localization->str(LSTR_FOR_YOUR_NUMEROUS_CRIMES_AND_EVIL_DEEDS), 3, jail_dialogue_window);
}

void GUIWindow_Jail::houseDialogueOptionSelected(DialogueId option) {
    _currentDialogue = option;
}
