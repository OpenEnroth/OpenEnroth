#include "GUI/UI/Houses/Jail.h"

#include "GUI/GUIFont.h"

#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"

void GUIWindow_Jail::houseSpecificDialogue() {
    GUIWindow jail_dialogue_window = *this;
    jail_dialogue_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    jail_dialogue_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    jail_dialogue_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;

    int vertMargin = (310 - assets->pFontArrus->CalcTextHeight(localization->GetString(LSTR_ONE_YEAR_SENTENCE), jail_dialogue_window.uFrameWidth, 0)) / 2 + 18;
    jail_dialogue_window.DrawTitleText(assets->pFontArrus.get(), 0, vertMargin, colorTable.PaleCanary, localization->GetString(LSTR_ONE_YEAR_SENTENCE), 3);
}

void GUIWindow_Jail::houseDialogueOptionSelected(DIALOGUE_TYPE option) {
    _currentDialogue = option;
}
