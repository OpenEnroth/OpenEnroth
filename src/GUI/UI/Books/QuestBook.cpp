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

    pBtn_Book_1 = CreateButton(pViewport.topLeft() + Pointi(398, 1), ui_book_button1_on->size(), 1, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_PREV_PAGE), INPUT_ACTION_DIALOG_LEFT, localization->str(LSTR_SCROLL_UP), {ui_book_button1_on});
    pBtn_Book_2 = CreateButton(pViewport.topLeft() + Pointi(398, 38), ui_book_button2_on->size(), 1, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NEXT_PAGE), INPUT_ACTION_DIALOG_RIGHT, localization->str(LSTR_SCROLL_DOWN), {ui_book_button2_on});

    for (auto i : pQuestTable.indices()) {
        if (pParty->_questBits[i] && !pQuestTable[i].empty()) {
            _activeQuestsIdx.push_back(i);
        }
    }
}

void GUIWindow_QuestBook::Update() {
    render->DrawQuad2D(ui_exit_cancel_button_background, {471, 445});

    int pTextHeight;
    GUIWindow questbook_window;

    render->DrawQuad2D(ui_book_quests_background, pViewport.topLeft());

    if ((_bookButtonClicked && _bookButtonAction == BOOK_PREV_PAGE) || !_startingQuestIdx) {
        render->DrawQuad2D(ui_book_button1_off, pViewport.topLeft() + Pointi(407, 2));
    } else {
        render->DrawQuad2D(ui_book_button1_on, pViewport.topLeft() + Pointi(398, 1));
    }

    if ((_bookButtonClicked && _bookButtonAction == BOOK_NEXT_PAGE) || (_startingQuestIdx + _currentPageQuests) >= _activeQuestsIdx.size()) {
        render->DrawQuad2D(ui_book_button2_off, pViewport.topLeft() + Pointi(407, 38));
    } else {
        render->DrawQuad2D(ui_book_button2_on, pViewport.topLeft() + Pointi(398, 38));
    }

    // for title
    questbook_window.frameRect = pViewport;
    questbook_window.DrawTitleText(assets->pFontBookTitle.get(), 0, 22, ui_book_quests_title_color, localization->str(LSTR_CURRENT_QUESTS), 3);

    // for other text
    questbook_window.frameRect = Recti(48, 70, 360, 264);

    if (_bookButtonClicked == 10 && _bookButtonAction == BOOK_NEXT_PAGE && (_startingQuestIdx + _currentPageQuests) < _activeQuestsIdx.size()) {
        pAudioPlayer->playUISound(SOUND_openbook);
        _startingQuestIdx += _currentPageQuests;
        _questsPerPage[_currentPage] = _currentPageQuests;
        _currentPage++;
    }

    if (_bookButtonClicked == 10 && _bookButtonAction == BOOK_PREV_PAGE && _startingQuestIdx) {
        pAudioPlayer->playUISound(SOUND_openbook);
        _currentPage--;
        _startingQuestIdx -= _questsPerPage[_currentPage];
    }

    if (_bookButtonClicked)
        _bookButtonClicked--;

    _currentPageQuests = 0;

    for (int i = _startingQuestIdx; i < _activeQuestsIdx.size(); ++i) {
        _currentPageQuests++;

        questbook_window.DrawText(assets->pFontBookOnlyShadow.get(), {1, 0}, ui_book_quests_text_color, pQuestTable[_activeQuestsIdx[i]]);
        pTextHeight = assets->pFontBookOnlyShadow->CalcTextHeight(pQuestTable[_activeQuestsIdx[i]], questbook_window.frameRect.w, 1);
        if ((questbook_window.frameRect.y + pTextHeight) > questbook_window.frameRect.h) {
            break;
        }

        render->DrawQuad2D(ui_book_quest_div_bar, {100, (questbook_window.frameRect.y + pTextHeight) + 12});
        questbook_window.frameRect.y = (questbook_window.frameRect.y + pTextHeight) + 24;
    }
}
