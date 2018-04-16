#pragma once

#include <array>
#include <vector>

#include "Engine/Strings.h"
#include "GUI/GUIWindow.h"

class Image;

class GUIButton {
 public:
    GUIButton() {
        pParent = nullptr;
    }

    void DrawLabel(const String &label_text, GUIFont *pFont, int a5, int uFontShadowColor);
    bool Contains(unsigned int x, unsigned int y);
    void Release();

    unsigned int uX;
    unsigned int uY;
    unsigned int uWidth;
    unsigned int uHeight;
    unsigned int uZ;
    unsigned int uW;
    int uButtonType;
    int field_1C;  // may be pMessageType
    UIMessageType msg;
    unsigned int  msg_param;
    int field_28;
    bool field_2C_is_pushed;
    GUIWindow *pParent;
    std::vector<Image*> vTextures;
    uint8_t uHotkey;
    String sLabel;
    String field_75;
};

extern GUIButton *pBtn_CloseBook;
extern GUIButton *pBtn_InstallRemoveSpell;
extern GUIButton *pBtn_Autonotes_Instructors;
extern GUIButton *pBtn_Autonotes_Misc;
extern GUIButton *pBtn_Book_6;
extern GUIButton *pBtn_Book_5;
extern GUIButton *pBtn_Book_4;
extern GUIButton *pBtn_Book_3;
extern GUIButton *pBtn_Book_2;
extern GUIButton *pBtn_Book_1;


extern GUIButton *pPlayerCreationUI_BtnReset;
extern GUIButton *pPlayerCreationUI_BtnOK;
extern GUIButton *pBtn_ExitCancel;
extern GUIButton *pBtn_YES;
extern GUIButton *pPlayerCreationUI_BtnPlus;
extern GUIButton *pPlayerCreationUI_BtnMinus;


extern GUIButton *pButton_RestUI_Main;
extern GUIButton *pButton_RestUI_Exit;
extern GUIButton *pButton_RestUI_Wait5Minutes;
extern GUIButton *pButton_RestUI_WaitUntilDawn;
extern GUIButton *pButton_RestUI_Wait1Hour;


extern GUIButton *pCharacterScreen_ExitBtn;
extern GUIButton *pCharacterScreen_AwardsBtn;
extern GUIButton *pCharacterScreen_InventoryBtn;
extern GUIButton *pCharacterScreen_SkillsBtn;
extern GUIButton *pCharacterScreen_StatsBtn;
extern GUIButton *pCharacterScreen_DollBtn;
extern GUIButton *pCharacterScreen_DetalizBtn;


extern GUIButton *pBtn_NPCRight;
extern GUIButton *pBtn_NPCLeft;
extern GUIButton *pBtn_GameSettings;
extern GUIButton *pBtn_QuickReference;
extern GUIButton *pBtn_CastSpell;
extern GUIButton *pBtn_Rest;
extern GUIButton *pBtn_History;
extern GUIButton *pBtn_Calendar;
extern GUIButton *pBtn_Maps;
extern GUIButton *pBtn_Autonotes;
extern GUIButton *pBtn_Quests;


extern GUIButton *pMMT_MainMenu_BtnMM6;
extern GUIButton *pMMT_MainMenu_BtnMM7;
extern GUIButton *pMMT_MainMenu_BtnMM8;
extern GUIButton *pMMT_MainMenu_BtnContinue;
extern GUIButton *pMMT_MainMenu_BtnExit;


extern GUIButton *pBtn_Up;
extern GUIButton *pBtn_Down;
extern GUIButton *ptr_507BA4;

extern GUIButton *pBtn_Resume;
extern GUIButton *pBtn_QuitGame;
extern GUIButton *pBtn_GameControls;
extern GUIButton *pBtn_LoadGame;
extern GUIButton *pBtn_SaveGame;
extern GUIButton *pBtn_NewGame;

extern GUIButton *pBtn_SliderRight;
extern GUIButton *pBtn_SliderLeft;


extern GUIButton *pBtnDownArrow;
extern GUIButton *pBtnArrowUp;
extern GUIButton *pBtnCancel;
extern GUIButton *pBtnLoadSlot;


extern std::array<GUIButton*, 4> pCreationUI_BtnPressRight2;
extern std::array<GUIButton*, 4> pCreationUI_BtnPressLeft2;
extern std::array<GUIButton*, 4> pCreationUI_BtnPressLeft;
extern std::array<GUIButton*, 4> pCreationUI_BtnPressRight;

void CreateButtonInColumn(int a1, unsigned int a2);
void UI_CreateEndConversationButton();
