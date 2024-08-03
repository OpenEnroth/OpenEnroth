#pragma once

#include <array>
#include <vector>
#include <string>

#include "GUI/GUIWindow.h"

class GraphicsImage;

class GUIButton {
 public:
    GUIButton() {
        pParent = nullptr;
    }

    void DrawLabel(std::string_view text, GUIFont *font, Color color, Color shadowColor = colorTable.Black);
    bool Contains(unsigned int x, unsigned int y);
    void Release();

    std::string id = {}; // Button id, so that buttons can be referenced from tests.
    int uX = 0;
    int uY = 0;
    int uWidth = 0;
    int uHeight = 0;
    int uZ = 0; // = uX + uWidth
    int uW = 0; // = uY + uHeight
    int uButtonType = 0;
    int uData = 0;  // may be pMessageType
    UIMessageType msg = UIMSG_0;
    unsigned int  msg_param = 0;
    int field_28 = 0;
    bool field_2C_is_pushed = false;
    GUIWindow *pParent = nullptr;
    std::vector<GraphicsImage*> vTextures;
    Io::InputAction action = Io::InputAction::Invalid;
    std::string sLabel = ""; // TODO(Nik-RE-dev): rename properly. In most cases it is a hover hint for status bar.
    std::string field_75 = "";
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
extern GUIButton *pBtn_Scroll;

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

extern GUIButton *pBtn_ZoomOut;
extern GUIButton *pBtn_ZoomIn;

extern std::array<GUIButton*, 4> pCreationUI_BtnPressRight2;
extern std::array<GUIButton*, 4> pCreationUI_BtnPressLeft2;
extern std::array<GUIButton*, 4> pCreationUI_BtnPressLeft;
extern std::array<GUIButton*, 4> pCreationUI_BtnPressRight;
