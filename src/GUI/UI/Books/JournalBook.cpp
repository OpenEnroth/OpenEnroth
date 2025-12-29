#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Tables/HistoryTable.h"
#include "Engine/mm7_data.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/Books/JournalBook.h"

#include "Media/Audio/AudioPlayer.h"

GraphicsImage *ui_book_journal_background = nullptr;

GUIWindow_JournalBook::GUIWindow_JournalBook() {
    eWindowType = WINDOW_JournalBook;

    pChildBooksOverlay = new GUIWindow_BooksButtonOverlay({600, 361}, {0, 0}, pBtn_History);
    bFlashHistoryBook = false;

    GUIWindow journal_window;

    ui_book_journal_background = assets->getImage_ColorKey("sbplayrnot");

    ui_book_button1_on = assets->getImage_Alpha("tab-an-6b");
    ui_book_button2_on = assets->getImage_Alpha("tab-an-7b");
    ui_book_button1_off = assets->getImage_Alpha("tab-an-6a");
    ui_book_button2_off = assets->getImage_Alpha("tab-an-7a");

    pBtn_Book_1 = CreateButton(pViewport.topLeft() + Pointi(398, 1), ui_book_button1_on->size(), 1, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_PREV_PAGE), INPUT_ACTION_DIALOG_LEFT, localization->str(LSTR_SCROLL_UP), {ui_book_button1_on});
    pBtn_Book_2 = CreateButton(pViewport.topLeft() + Pointi(398, 38), ui_book_button2_on->size(), 1, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NEXT_PAGE), INPUT_ACTION_DIALOG_RIGHT, localization->str(LSTR_SCROLL_DOWN), {ui_book_button2_on});

    journal_window.frameRect = Recti(48, 70, 360, 264);
    journal_window.frameRect.h = (assets->pFontBookOnlyShadow->GetHeight() - 3) * 264 / assets->pFontBookOnlyShadow->GetHeight() - 3;

    for (int i = 0; i < pParty->PartyTimes.HistoryEventTimes.size(); i++) {
        if (pParty->PartyTimes.HistoryEventTimes[i].isValid()) {
            if (!pHistoryTable->historyLines[i + 1].pText.empty()) {
                NPCData dummyNpc;
                std::string str = BuildDialogueString(pHistoryTable->historyLines[i + 1].pText, 0, &dummyNpc, 0, HOUSE_INVALID, SHOP_SCREEN_INVALID, &pParty->PartyTimes.HistoryEventTimes[i]);
                int pTextHeight = assets->pFontBookOnlyShadow->CalcTextHeight(str, journal_window.frameRect.w, 1);
                int pages = ((pTextHeight - (assets->pFontBookOnlyShadow->GetHeight() - 3)) / (signed int)journal_window.frameRect.h) + 1;
                for (int j = 0; j < pages; ++j) {
                    _journalIdx.push_back(i + 1);
                    _journalEntryPage.push_back(j);
                }
            }
        }
    }
}

void GUIWindow_JournalBook::Update() {
    render->DrawQuad2D(ui_exit_cancel_button_background, {471, 445});

    GUIWindow journal_window;

    render->DrawQuad2D(ui_book_journal_background, pViewport.topLeft());
    if ((_bookButtonClicked && _bookButtonAction == BOOK_PREV_PAGE) || !_currentIdx) {
        render->DrawQuad2D(ui_book_button1_off, pViewport.topLeft() + Pointi(407, 2));
    } else {
        render->DrawQuad2D(ui_book_button1_on, pViewport.topLeft() + Pointi(398, 1));
    }

    if ((_bookButtonClicked && _bookButtonAction == BOOK_NEXT_PAGE) || (_currentIdx + 1) >= _journalIdx.size()) {
        render->DrawQuad2D(ui_book_button2_off, pViewport.topLeft() + Pointi(407, 38));
    } else {
        render->DrawQuad2D(ui_book_button2_on, pViewport.topLeft() + Pointi(398, 38));
    }

    if (_journalIdx.size() && !_journalEntryPage[_currentIdx]) {  // for title
        journal_window.frameRect = pViewport;

        if (!pHistoryTable->historyLines[_journalIdx[_currentIdx]].pPageTitle.empty()) {
            journal_window.DrawTitleText(assets->pFontBookTitle.get(), 0, 22, ui_book_journal_title_color, pHistoryTable->historyLines[_journalIdx[_currentIdx]].pPageTitle, 3);
        }
    }

    // for other text
    journal_window.frameRect = Recti(48, 70, 360, 264);

    if (_bookButtonClicked == 10 && _bookButtonAction == BOOK_NEXT_PAGE && (_currentIdx + 1) < _journalIdx.size()) {
        pAudioPlayer->playUISound(SOUND_openbook);
        _currentIdx++;
    }
    if (_bookButtonClicked == 10 && _bookButtonAction == BOOK_PREV_PAGE && _currentIdx) {
        pAudioPlayer->playUISound(SOUND_openbook);
        _currentIdx--;
    }

    if (_bookButtonClicked)
        _bookButtonClicked--;

    if (_journalIdx.size()) {
        NPCData dummyNpc;
        std::string str = BuildDialogueString(pHistoryTable->historyLines[_journalIdx[_currentIdx]].pText, 0, &dummyNpc, 0, HOUSE_INVALID,
                                              SHOP_SCREEN_INVALID, &pParty->PartyTimes.HistoryEventTimes[_journalIdx[_currentIdx] - 1]);
        std::string pStringOnPage = assets->pFontBookOnlyShadow->GetPageText(str, {journal_window.frameRect.w, journal_window.frameRect.h}, 1, _journalEntryPage[_currentIdx]);
        journal_window.DrawText(assets->pFontBookOnlyShadow.get(), {1, 0}, ui_book_journal_text_color, pStringOnPage,
                                journal_window.frameRect.y + journal_window.frameRect.h, ui_book_journal_text_shadow);
    }
}
