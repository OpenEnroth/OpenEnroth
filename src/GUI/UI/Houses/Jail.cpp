#include "GUI/UI/Houses/Jail.h"

#include "GUI/GUIFont.h"

#include "Engine/Localization.h"

void JailDialog() {
    GUIWindow jail_dialogue_window = *window_SpeakInHouse;
    jail_dialogue_window.uFrameX = SIDE_TEXT_BOX_POS_X;
    jail_dialogue_window.uFrameWidth = SIDE_TEXT_BOX_WIDTH;
    jail_dialogue_window.uFrameZ = SIDE_TEXT_BOX_POS_Z;
    jail_dialogue_window.DrawTitleText(
        pFontArrus, 0,
        (310 - pFontArrus->CalcTextHeight(localization->GetString(LSTR_ONE_YEAR_SENTENCE),
            jail_dialogue_window.uFrameWidth,
            0)) / 2 + 18, colorTable.PaleCanary, localization->GetString(LSTR_ONE_YEAR_SENTENCE), 3);
}
