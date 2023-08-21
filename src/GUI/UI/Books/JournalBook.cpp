#include <string>
#include <array>
#include <memory>

#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Tables/StorylineTextTable.h"
#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/Books/JournalBook.h"
#include "Media/Audio/AudioPlayer.h"
#include "Engine/Time.h"
#include "Engine/mm7_data.h"
#include "GUI/GUIEnums.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/UIHouseEnums.h"
#include "Io/InputAction.h"
#include "Media/Audio/SoundEnums.h"
#include "Utility/Workaround/ToUnderlying.h"

GraphicsImage *ui_book_journal_background = nullptr;

GUIWindow_JournalBook::GUIWindow_JournalBook() : _currentIdx(0), GUIWindow_Book() {
    eWindowType = WINDOW_JournalBook;
    this->wData.val = WINDOW_JournalBook;

    pChildBooksOverlay = new GUIWindow_BooksButtonOverlay({600, 361}, {0, 0}, pBtn_History);
    bFlashHistoryBook = false;

    GUIWindow journal_window;

    ui_book_journal_background = assets->getImage_ColorKey("sbplayrnot");

    ui_book_button1_on = assets->getImage_Alpha("tab-an-6b");
    ui_book_button2_on = assets->getImage_Alpha("tab-an-7b");
    ui_book_button1_off = assets->getImage_Alpha("tab-an-6a");
    ui_book_button2_off = assets->getImage_Alpha("tab-an-7a");

    pBtn_Book_1 = CreateButton({pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 1}, ui_book_button1_on->size(), 1, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_PREV_PAGE), Io::InputAction::DialogLeft, localization->GetString(LSTR_SCROLL_UP), {ui_book_button1_on});
    pBtn_Book_2 = CreateButton({pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 38}, ui_book_button2_on->size(), 1, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NEXT_PAGE), Io::InputAction::DialogRight, localization->GetString(LSTR_SCROLL_DOWN), {ui_book_button2_on});

    journal_window.uFrameX = 48;
    journal_window.uFrameY = 70;
    journal_window.uFrameWidth = 360;
    journal_window.uFrameHeight = 264;
    journal_window.uFrameZ = 407;
    journal_window.uFrameHeight = (pFontBookOnlyShadow->GetHeight() - 3) * 264 / pFontBookOnlyShadow->GetHeight() - 3;
    journal_window.uFrameW = journal_window.uFrameHeight + 69;

    for (int i = 0; i < pParty->PartyTimes.HistoryEventTimes.size(); i++) {
        if (pParty->PartyTimes.HistoryEventTimes[i].Valid()) {
            if (!pStorylineText->StoreLine[i + 1].pText.empty()) {
                std::string str = BuildDialogueString(pStorylineText->StoreLine[i + 1].pText, 0, 0, HOUSE_INVALID, 0, &pParty->PartyTimes.HistoryEventTimes[i]);
                int pTextHeight = pFontBookOnlyShadow->CalcTextHeight(str, journal_window.uFrameWidth, 1);
                int pages = ((pTextHeight - (pFontBookOnlyShadow->GetHeight() - 3)) / (signed int)journal_window.uFrameHeight) + 1;
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

    render->DrawTextureNew(pViewport->uViewportTL_X / 640.0f, pViewport->uViewportTL_Y / 480.0f, ui_book_journal_background);
    if ((_bookButtonClicked && _bookButtonAction == BOOK_NEXT_PAGE) || !_currentIdx) {
        render->DrawTextureNew((pViewport->uViewportTL_X + 407) / 640.0f, (pViewport->uViewportTL_Y + 2) / 480.0f, ui_book_button1_off);
    } else {
        render->DrawTextureNew((pViewport->uViewportTL_X + 398) / 640.0f, (pViewport->uViewportTL_Y + 1) / 480.0f, ui_book_button1_on);
    }

    if ((_bookButtonClicked && _bookButtonAction == BOOK_PREV_PAGE) || (_currentIdx + 1) >= _journalIdx.size()) {
        render->DrawTextureNew((pViewport->uViewportTL_X + 407) / 640.0f, (pViewport->uViewportTL_Y + 38) / 480.0f, ui_book_button2_off);
    } else {
        render->DrawTextureNew((pViewport->uViewportTL_X + 398) / 640.0f, (pViewport->uViewportTL_Y + 38) / 480.0f, ui_book_button2_on);
    }

    if (_journalIdx.size() && !_journalEntryPage[_currentIdx]) {  // for title
        journal_window.uFrameWidth = game_viewport_width;
        journal_window.uFrameX = game_viewport_x;
        journal_window.uFrameY = game_viewport_y;
        journal_window.uFrameHeight = game_viewport_height;
        journal_window.uFrameZ = game_viewport_z;
        journal_window.uFrameW = game_viewport_w;
        if (!pStorylineText->StoreLine[_journalIdx[_currentIdx]].pPageTitle.empty()) {
            journal_window.DrawTitleText(pFontBookTitle, 0, 22, ui_book_journal_title_color, pStorylineText->StoreLine[_journalIdx[_currentIdx]].pPageTitle, 3);
        }
    }

    // for other text
    journal_window.uFrameX = 48;
    journal_window.uFrameY = 70;
    journal_window.uFrameWidth = 360;
    journal_window.uFrameHeight = 264;
    journal_window.uFrameZ = 407;
    journal_window.uFrameW = journal_window.uFrameHeight + 69;

    if (_bookButtonClicked && _bookButtonAction == BOOK_NEXT_PAGE && (_currentIdx + 1) < _journalIdx.size()) {
        pAudioPlayer->playUISound(SOUND_openbook);
        _currentIdx++;
    }
    if (_bookButtonClicked && _bookButtonAction == BOOK_PREV_PAGE && _currentIdx) {
        pAudioPlayer->playUISound(SOUND_openbook);
        _currentIdx--;
    }

    _bookButtonClicked = false;

    if (_journalIdx.size()) {
        std::string str = BuildDialogueString(pStorylineText->StoreLine[_journalIdx[_currentIdx]].pText,
                                              0, 0, HOUSE_INVALID, 0, &pParty->PartyTimes.HistoryEventTimes[_journalIdx[_currentIdx] - 1]);
        std::string pStringOnPage = pFontBookOnlyShadow->GetPageTop(str, &journal_window, 1, _journalEntryPage[_currentIdx]);
        journal_window.DrawText(pFontBookOnlyShadow, {1, 0}, ui_book_journal_text_color, pStringOnPage,
                                journal_window.uFrameY + journal_window.uFrameHeight, ui_book_journal_text_shadow);
    }
}
