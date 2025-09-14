#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Image.h"
#include "Engine/mm7_data.h"

#include "Engine/Tables/QuestTable.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/Books/QuestBook.h"

#include "Media/Audio/AudioPlayer.h"

GraphicsImage *ui_book_quests_background = nullptr;

GUIWindow_QuestBook::GUIWindow_QuestBook() {
    this->eWindowType = WindowType::WINDOW_QuestBook;

    pChildBooksOverlay = new GUIWindow_BooksButtonOverlay({493, 355}, {0, 0}, pBtn_Quests);
    bFlashQuestBook = false;

    ui_book_quests_background = assets->getImage_Solid("sbquiknot");
    ui_book_quest_div_bar = assets->getImage_Alpha("divbar");

    ui_book_button1_on = assets->getImage_Alpha("tab-an-6b");
    ui_book_button2_on = assets->getImage_Alpha("tab-an-7b");
    ui_book_button1_off = assets->getImage_Alpha("tab-an-6a");
    ui_book_button2_off = assets->getImage_Alpha("tab-an-7a");

    pBtn_Book_1 = CreateButton({pViewport->viewportTL_X + 398, pViewport->viewportTL_Y + 1}, ui_book_button1_on->size(), 1, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_PREV_PAGE), Io::InputAction::DialogLeft, localization->GetString(LSTR_SCROLL_UP), {ui_book_button1_on});
    pBtn_Book_2 = CreateButton({pViewport->viewportTL_X + 398, pViewport->viewportTL_Y + 38}, ui_book_button2_on->size(), 1, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NEXT_PAGE), Io::InputAction::DialogRight, localization->GetString(LSTR_SCROLL_DOWN), {ui_book_button2_on});

    for (auto i : pQuestTable.indices()) {
        if (pParty->_questBits[i] && !pQuestTable[i].empty()) {
            _activeQuestsIdx.push_back(i);
        }
    }
}

void GUIWindow_QuestBook::Update() {
    render->DrawTextureNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);

    int pTextHeight;
    GUIWindow questbook_window;

    render->DrawTextureNew(pViewport->viewportTL_X / 640.0f, pViewport->viewportTL_Y / 480.0f, ui_book_quests_background);

    if ((_bookButtonClicked && _bookButtonAction == BOOK_PREV_PAGE) || !_startingQuestIdx) {
        render->DrawTextureNew((pViewport->viewportTL_X + 407) / 640.0f, (pViewport->viewportTL_Y + 2) / 480.0f, ui_book_button1_off);
    } else {
        render->DrawTextureNew((pViewport->viewportTL_X + 398) / 640.0f, (pViewport->viewportTL_Y + 1) / 480.0f, ui_book_button1_on);
    }

    if ((_bookButtonClicked && _bookButtonAction == BOOK_NEXT_PAGE) || (_startingQuestIdx + _currentPageQuests) >= _activeQuestsIdx.size()) {
        render->DrawTextureNew((pViewport->viewportTL_X + 407) / 640.0f, (pViewport->viewportTL_Y + 38) / 480.0f, ui_book_button2_off);
    } else {
        render->DrawTextureNew((pViewport->viewportTL_X + 398) / 640.0f, (pViewport->viewportTL_Y + 38) / 480.0f, ui_book_button2_on);
    }

    // for title
    questbook_window.uFrameWidth = pViewport->viewportWidth;
    questbook_window.uFrameHeight = pViewport->viewportHeight;
    questbook_window.uFrameX = pViewport->viewportTL_X;
    questbook_window.uFrameY = pViewport->viewportTL_Y;
    questbook_window.uFrameZ = pViewport->viewportBR_X;
    questbook_window.uFrameW = pViewport->viewportBR_Y;
    questbook_window.DrawTitleText(assets->pFontBookTitle.get(), 0, 22, ui_book_quests_title_color, localization->GetString(LSTR_CURRENT_QUESTS), 3);

    // for other text
    questbook_window.uFrameX = 48;
    questbook_window.uFrameY = 70;
    questbook_window.uFrameWidth = 360;
    questbook_window.uFrameHeight = 264;
    questbook_window.uFrameZ = 407;
    questbook_window.uFrameW = 333;

    if (_bookButtonClicked && _bookButtonAction == BOOK_NEXT_PAGE && (_startingQuestIdx + _currentPageQuests) < _activeQuestsIdx.size()) {
        pAudioPlayer->playUISound(SOUND_openbook);
        _startingQuestIdx += _currentPageQuests;
        _questsPerPage[_currentPage] = _currentPageQuests;
        _currentPage++;
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_PREV_PAGE && _startingQuestIdx) {
        pAudioPlayer->playUISound(SOUND_openbook);
        _currentPage--;
        _startingQuestIdx -= _questsPerPage[_currentPage];
    }

    _bookButtonClicked = false;
    _currentPageQuests = 0;

    for (int i = _startingQuestIdx; i < _activeQuestsIdx.size(); ++i) {
        _currentPageQuests++;

        questbook_window.DrawText(assets->pFontBookOnlyShadow.get(), {1, 0}, ui_book_quests_text_color, pQuestTable[_activeQuestsIdx[i]]);
        pTextHeight = assets->pFontBookOnlyShadow->CalcTextHeight(pQuestTable[_activeQuestsIdx[i]], questbook_window.uFrameWidth, 1);
        if ((questbook_window.uFrameY + pTextHeight) > questbook_window.uFrameHeight) {
            break;
        }

        render->DrawTextureNew(100 / 640.0f, ((questbook_window.uFrameY + pTextHeight) + 12) / 480.0f, ui_book_quest_div_bar);
        questbook_window.uFrameY = (questbook_window.uFrameY + pTextHeight) + 24;
    }
}
