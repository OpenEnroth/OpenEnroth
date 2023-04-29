#include "Engine/AssetsManager.h"
#include "Engine/Awards.h"
#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/Books/QuestBook.h"

#include "Media/Audio/AudioPlayer.h"

Image *ui_book_quests_background = nullptr;

GUIWindow_QuestBook::GUIWindow_QuestBook() : _startingQuestIdx(0), _currentPage(0), _currentPageQuests(0), GUIWindow_Book() {
    this->wData.val = WINDOW_QuestBook;  // inherited from GUIWindow::GUIWindow
    this->eWindowType = WindowType::WINDOW_QuestBook;

    // --------------------------------
    // 004304E7 Game_EventLoop --- part
    pChildBooksOverlay = new GUIWindow_BooksButtonOverlay({493, 355}, {0, 0}, pBtn_Quests);
    bFlashQuestBook = false;

    // ----------------------------------------------
    // 00411BFC GUIWindow::InitializeBookView -- part
    ui_book_quests_background = assets->GetImage_Solid("sbquiknot");
    ui_book_quest_div_bar = assets->GetImage_Alpha("divbar");

    ui_book_button1_on = assets->GetImage_Alpha("tab-an-6b");
    ui_book_button2_on = assets->GetImage_Alpha("tab-an-7b");
    ui_book_button1_off = assets->GetImage_Alpha("tab-an-6a");
    ui_book_button2_off = assets->GetImage_Alpha("tab-an-7a");

    pBtn_Book_1 = CreateButton({pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 1}, {ui_book_button1_on->GetWidth(), ui_book_button1_on->GetHeight()}, 1, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_PREV_PAGE), InputAction::Invalid, localization->GetString(LSTR_SCROLL_UP), {ui_book_button1_on});
    pBtn_Book_2 = CreateButton({pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 38}, {ui_book_button2_on->GetWidth(), ui_book_button2_on->GetHeight()}, 1, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NEXT_PAGE), InputAction::Invalid, localization->GetString(LSTR_SCROLL_DOWN), {ui_book_button2_on});

    for (int i = 1; i < pQuestTable.size(); ++i) {
        if (pParty->_questBits[i] && pQuestTable[i]) {
            _activeQuestsIdx.push_back(i);
        }
    }
}

void GUIWindow_QuestBook::Update() {
    // -----------------------------------
    // 004156F0 GUI_UpdateWindows --- part
    // {
    //     BookUI_Draw((WindowType)(int)ptr_1C);
    // }

    // ----- (00413CC6) --------------------------------------------------------
    // void BookUI_Draw(WindowType book) --- part
    // {
    render->DrawTextureNew(471 / 640.0f, 445 / 480.0f,
                                ui_exit_cancel_button_background);
    //     BookUI_Questbook_Draw();

    // ----- (00413126) --------------------------------------------------------
    // void BookUI_Questbook_Draw()
    // {
    int pTextHeight;             // eax@19
    GUIWindow questbook_window;  // [sp+Ch] [bp-54h]@9

    render->DrawTextureNew(pViewport->uViewportTL_X / 640.0f, pViewport->uViewportTL_Y / 480.0f, ui_book_quests_background);

    if ((bookButtonClicked && bookButtonAction == BOOK_PREV_PAGE) || !_startingQuestIdx) {
        render->DrawTextureNew((pViewport->uViewportTL_X + 407) / 640.0f, (pViewport->uViewportTL_Y + 2) / 480.0f, ui_book_button1_off);
    } else {
        render->DrawTextureNew((pViewport->uViewportTL_X + 398) / 640.0f, (pViewport->uViewportTL_Y + 1) / 480.0f, ui_book_button1_on);
    }

    if ((bookButtonClicked && bookButtonAction == BOOK_NEXT_PAGE) || (_startingQuestIdx + _currentPageQuests) >= _activeQuestsIdx.size()) {
        render->DrawTextureNew((pViewport->uViewportTL_X + 407) / 640.0f, (pViewport->uViewportTL_Y + 38) / 480.0f, ui_book_button2_off);
    } else {
        render->DrawTextureNew((pViewport->uViewportTL_X + 398) / 640.0f, (pViewport->uViewportTL_Y + 38) / 480.0f, ui_book_button2_on);
    }

    // for title
    questbook_window.uFrameWidth = game_viewport_width;
    questbook_window.uFrameHeight = game_viewport_height;
    questbook_window.uFrameX = game_viewport_x;
    questbook_window.uFrameY = game_viewport_y;
    questbook_window.uFrameZ = game_viewport_z;
    questbook_window.uFrameW = game_viewport_w;
    questbook_window.DrawTitleText(pBook2Font, 0, 22, ui_book_quests_title_color, localization->GetString(LSTR_CURRENT_QUESTS), 3);

    // for other text
    questbook_window.uFrameX = 48;
    questbook_window.uFrameY = 70;
    questbook_window.uFrameWidth = 360;
    questbook_window.uFrameHeight = 264;
    questbook_window.uFrameZ = 407;
    questbook_window.uFrameW = 333;

    if (bookButtonClicked && bookButtonAction == BOOK_NEXT_PAGE && (_startingQuestIdx + _currentPageQuests) < _activeQuestsIdx.size()) {
        pAudioPlayer->playUISound(SOUND_openbook);
        _startingQuestIdx += _currentPageQuests;
        _questsPerPage[_currentPage] = _currentPageQuests;
        _currentPage++;
    }

    if (bookButtonClicked && bookButtonAction == BOOK_PREV_PAGE && _startingQuestIdx) {
        pAudioPlayer->playUISound(SOUND_openbook);
        _currentPage--;
        _startingQuestIdx -= _questsPerPage[_currentPage];
    }

    bookButtonClicked = false;
    _currentPageQuests = 0;

    for (int i = _startingQuestIdx; i < _activeQuestsIdx.size(); ++i) {
        _currentPageQuests++;

        questbook_window.DrawText(pAutonoteFont, {1, 0}, ui_book_quests_text_color, pQuestTable[_activeQuestsIdx[i]], 0, 0, 0);
        pTextHeight = pAutonoteFont->CalcTextHeight(pQuestTable[_activeQuestsIdx[i]], questbook_window.uFrameWidth, 1);
        if ((questbook_window.uFrameY + pTextHeight) > questbook_window.uFrameHeight) {
            break;
        }

        render->DrawTextureNew(100 / 640.0f, ((questbook_window.uFrameY + pTextHeight) + 12) / 480.0f, ui_book_quest_div_bar);
        questbook_window.uFrameY = (questbook_window.uFrameY + pTextHeight) + 24;
    }
}
