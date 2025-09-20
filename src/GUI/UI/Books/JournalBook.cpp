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

    pBtn_Book_1 = CreateButton({pViewport->viewportTL_X + 398, pViewport->viewportTL_Y + 1}, ui_book_button1_on->size(), 1, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_PREV_PAGE), INPUT_ACTION_DIALOG_LEFT, localization->GetString(LSTR_SCROLL_UP), {ui_book_button1_on});
    pBtn_Book_2 = CreateButton({pViewport->viewportTL_X + 398, pViewport->viewportTL_Y + 38}, ui_book_button2_on->size(), 1, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NEXT_PAGE), INPUT_ACTION_DIALOG_RIGHT, localization->GetString(LSTR_SCROLL_DOWN), {ui_book_button2_on});

    journal_window.uFrameX = 48;
    journal_window.uFrameY = 70;
    journal_window.uFrameWidth = 360;
    journal_window.uFrameHeight = 264;
    journal_window.uFrameZ = 407;
    journal_window.uFrameHeight = (assets->pFontBookOnlyShadow->GetHeight() - 3) * 264 / assets->pFontBookOnlyShadow->GetHeight() - 3;
    journal_window.uFrameW = journal_window.uFrameHeight + 69;

    for (int i = 0; i < pParty->PartyTimes.HistoryEventTimes.size(); i++) {
        if (pParty->PartyTimes.HistoryEventTimes[i].isValid()) {
            if (!pHistoryTable->historyLines[i + 1].pText.empty()) {
                NPCData dummyNpc;
                std::string str = BuildDialogueString(pHistoryTable->historyLines[i + 1].pText, 0, &dummyNpc, 0, HOUSE_INVALID, SHOP_SCREEN_INVALID, &pParty->PartyTimes.HistoryEventTimes[i]);
                int pTextHeight = assets->pFontBookOnlyShadow->CalcTextHeight(str, journal_window.uFrameWidth, 1);
                int pages = ((pTextHeight - (assets->pFontBookOnlyShadow->GetHeight() - 3)) / (signed int)journal_window.uFrameHeight) + 1;
                for (int j = 0; j < pages; ++j) {
                    _journalIdx.push_back(i + 1);
                    _journalEntryPage.push_back(j);
                }
            }
        }
    }
}

void GUIWindow_JournalBook::Update() {
    render->DrawTextureNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);

    GUIWindow journal_window;

    render->DrawTextureNew(pViewport->viewportTL_X / 640.0f, pViewport->viewportTL_Y / 480.0f, ui_book_journal_background);
    if ((_bookButtonClicked && _bookButtonAction == BOOK_PREV_PAGE) || !_currentIdx) {
        render->DrawTextureNew((pViewport->viewportTL_X + 407) / 640.0f, (pViewport->viewportTL_Y + 2) / 480.0f, ui_book_button1_off);
    } else {
        render->DrawTextureNew((pViewport->viewportTL_X + 398) / 640.0f, (pViewport->viewportTL_Y + 1) / 480.0f, ui_book_button1_on);
    }

    if ((_bookButtonClicked && _bookButtonAction == BOOK_NEXT_PAGE) || (_currentIdx + 1) >= _journalIdx.size()) {
        render->DrawTextureNew((pViewport->viewportTL_X + 407) / 640.0f, (pViewport->viewportTL_Y + 38) / 480.0f, ui_book_button2_off);
    } else {
        render->DrawTextureNew((pViewport->viewportTL_X + 398) / 640.0f, (pViewport->viewportTL_Y + 38) / 480.0f, ui_book_button2_on);
    }

    if (_journalIdx.size() && !_journalEntryPage[_currentIdx]) {  // for title
        journal_window.uFrameWidth = pViewport->viewportWidth;
        journal_window.uFrameHeight = pViewport->viewportHeight;
        journal_window.uFrameX = pViewport->viewportTL_X;
        journal_window.uFrameY = pViewport->viewportTL_Y;
        journal_window.uFrameZ = pViewport->viewportBR_X;
        journal_window.uFrameW = pViewport->viewportBR_Y;

        if (!pHistoryTable->historyLines[_journalIdx[_currentIdx]].pPageTitle.empty()) {
            journal_window.DrawTitleText(assets->pFontBookTitle.get(), 0, 22, ui_book_journal_title_color, pHistoryTable->historyLines[_journalIdx[_currentIdx]].pPageTitle, 3);
        }
    }

    // for other text
    journal_window.uFrameX = 48;
    journal_window.uFrameY = 70;
    journal_window.uFrameWidth = 360;
    journal_window.uFrameHeight = 264;
    journal_window.uFrameZ = 407;
    journal_window.uFrameW = journal_window.uFrameHeight + 69;

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
        std::string pStringOnPage = assets->pFontBookOnlyShadow->GetPageText(str, {journal_window.uFrameWidth, journal_window.uFrameHeight}, 1, _journalEntryPage[_currentIdx]);
        journal_window.DrawText(assets->pFontBookOnlyShadow.get(), {1, 0}, ui_book_journal_text_color, pStringOnPage,
                                journal_window.uFrameY + journal_window.uFrameHeight, ui_book_journal_text_shadow);
    }
}
