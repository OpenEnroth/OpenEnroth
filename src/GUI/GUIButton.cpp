#include "GUI/GUIButton.h"

#include <vector>

#include "Engine/Graphics/IRender.h"

#include "GUI/GUIFont.h"


GUIButton *pBtn_CloseBook;
GUIButton *pBtn_InstallRemoveSpell;
GUIButton *pBtn_Autonotes_Instructors;
GUIButton *pBtn_Autonotes_Misc;
GUIButton *pBtn_Book_6;
GUIButton *pBtn_Book_5;
GUIButton *pBtn_Book_4;
GUIButton *pBtn_Book_3;
GUIButton *pBtn_Book_2;
GUIButton *pBtn_Book_1;

GUIButton *pPlayerCreationUI_BtnReset;
GUIButton *pPlayerCreationUI_BtnOK;
GUIButton *pBtn_ExitCancel;
GUIButton *pBtn_YES;
GUIButton *pPlayerCreationUI_BtnPlus;
GUIButton *pPlayerCreationUI_BtnMinus;

GUIButton *pButton_RestUI_Main;
GUIButton *pButton_RestUI_Exit;
GUIButton *pButton_RestUI_Wait5Minutes;
GUIButton *pButton_RestUI_WaitUntilDawn;
GUIButton *pButton_RestUI_Wait1Hour;

GUIButton *pCharacterScreen_ExitBtn;
GUIButton *pCharacterScreen_AwardsBtn;
GUIButton *pCharacterScreen_InventoryBtn;
GUIButton *pCharacterScreen_SkillsBtn;
GUIButton *pCharacterScreen_StatsBtn;
GUIButton *pCharacterScreen_DollBtn;
GUIButton *pCharacterScreen_DetalizBtn;

GUIButton *pBtn_NPCRight;
GUIButton *pBtn_NPCLeft;
GUIButton *pBtn_GameSettings;
GUIButton *pBtn_QuickReference;
GUIButton *pBtn_CastSpell;
GUIButton *pBtn_Rest;
GUIButton *pBtn_History;
GUIButton *pBtn_Calendar;
GUIButton *pBtn_Maps;
GUIButton *pBtn_Autonotes;
GUIButton *pBtn_Quests;

GUIButton *pMMT_MainMenu_BtnMM6;
GUIButton *pMMT_MainMenu_BtnMM7;
GUIButton *pMMT_MainMenu_BtnMM8;
GUIButton *pMMT_MainMenu_BtnContinue;
GUIButton *pMMT_MainMenu_BtnExit;

GUIButton *pBtn_Up;
GUIButton *pBtn_Down;
GUIButton *pBtn_Scroll;

GUIButton *pBtn_Resume;
GUIButton *pBtn_QuitGame;
GUIButton *pBtn_GameControls;
GUIButton *pBtn_LoadGame;
GUIButton *pBtn_SaveGame;
GUIButton *pBtn_NewGame;

GUIButton *pBtn_SliderRight;
GUIButton *pBtn_SliderLeft;

GUIButton *pBtn_ZoomOut;
GUIButton *pBtn_ZoomIn;

GUIButton *pBtnDownArrow;
GUIButton *pBtnArrowUp;
GUIButton *pBtnCancel;
GUIButton *pBtnLoadSlot;

std::array<GUIButton *, 4> pCreationUI_BtnPressRight2;
std::array<GUIButton *, 4> pCreationUI_BtnPressLeft2;
std::array<GUIButton *, 4> pCreationUI_BtnPressLeft;
std::array<GUIButton *, 4> pCreationUI_BtnPressRight;

void GUIButton::Release() {
    std::vector<GUIButton *>::iterator it =
        std::find(pParent->vButtons.begin(), pParent->vButtons.end(), this);
    if (it != pParent->vButtons.end()) {
        pParent->vButtons.erase(it);
    }
    delete this;
}

void GUIButton::DrawLabel(const std::string &text, GUIFont *font, Color color, Color shadowColor) {
    return pParent->DrawText(font,
                             {this->uX + (int)(this->uWidth - font->GetLineWidth(text)) / 2, this->uY + (int)(this->uHeight - font->GetHeight()) / 2},
                             color, text, 0, shadowColor);
}

bool GUIButton::Contains(unsigned int x, unsigned int y) {
    return (x >= uX && x <= uZ && y >= uY && y <= uW);
}
