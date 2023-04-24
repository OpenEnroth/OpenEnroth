#include <string>
#include <vector>

#include "Engine/AssetsManager.h"
#include "Engine/Awards.h"
#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"

#include "Engine/Tables/StorylineTextTable.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/Books/JournalBook.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/Memory/MemSet.h"

Image *ui_book_journal_background = nullptr;

static int currentIdx;
static std::vector<int> journalIdx;
static std::vector<int> journalEntryPage;

GUIWindow_JournalBook::GUIWindow_JournalBook() : GUIWindow_Book() {
    eWindowType = WINDOW_JournalBook;
    this->wData.val = WINDOW_JournalBook;  // inherited from GUIWindow::GUIWindow

    pChildBooksOverlay = new GUIWindow_BooksButtonOverlay({600, 361}, {0, 0}, pBtn_History);
    bFlashHistoryBook = false;

    GUIWindow journal_window;

    ui_book_journal_background = assets->GetImage_ColorKey("sbplayrnot");

    ui_book_button1_on = assets->GetImage_Alpha("tab-an-6b");
    ui_book_button2_on = assets->GetImage_Alpha("tab-an-7b");
    ui_book_button1_off = assets->GetImage_Alpha("tab-an-6a");
    ui_book_button2_off = assets->GetImage_Alpha("tab-an-7a");

    pBtn_Book_1 = CreateButton({pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 1}, {ui_book_button1_on->GetWidth(), ui_book_button1_on->GetHeight()}, 1, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_PREV_PAGE), InputAction::Invalid, localization->GetString(LSTR_SCROLL_UP), {ui_book_button1_on});
    pBtn_Book_2 = CreateButton({pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 38}, {ui_book_button2_on->GetWidth(), ui_book_button2_on->GetHeight()}, 1, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NEXT_PAGE), InputAction::Invalid, localization->GetString(LSTR_SCROLL_DOWN), {ui_book_button2_on});

    journal_window.uFrameX = 48;
    journal_window.uFrameY = 70;
    journal_window.uFrameWidth = 360;
    journal_window.uFrameHeight = 264;
    journal_window.uFrameZ = 407;
    journal_window.uFrameHeight = (pAutonoteFont->GetHeight() - 3) * 264 / pAutonoteFont->GetHeight() - 3;
    journal_window.uFrameW = journal_window.uFrameHeight + 69;

    currentIdx = 0;
    journalIdx.clear();
    journalEntryPage.clear();
    for (int i = 0; i < pParty->PartyTimes.HistoryEventTimes.size(); i++) {
        if (pParty->PartyTimes.HistoryEventTimes[i].Valid()) {
            if (pStorylineText->StoreLine[i + 1].pText) {
                std::string str = BuildDialogueString(pStorylineText->StoreLine[i + 1].pText, 0, 0, 0, 0, &pParty->PartyTimes.HistoryEventTimes[i]);
                int pTextHeight = pAutonoteFont->CalcTextHeight(str, journal_window.uFrameWidth, 1);
                int pages = ((pTextHeight - (pAutonoteFont->GetHeight() - 3)) / (signed int)journal_window.uFrameHeight) + 1;
                for (int j = 0; j < pages; ++j) {
                    journalIdx.push_back(i + 1);
                    journalEntryPage.push_back(j);
                }
            }
        }
    }
}

void GUIWindow_JournalBook::Update() {
    // -----------------------------------
    // 004156F0 GUI_UpdateWindows --- part
    // {
    //     BookUI_Draw((WindowType)(int)ptr_1C);
    // }

    // ----- (00413CC6) --------------------------------------------------------
    // void BookUI_Draw(WindowType book) --- part
    // {
    render->DrawTextureNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);
    //     BookUI_Journal_Draw();

    // ----- (00412E85) --------------------------------------------------------
    // void BookUI_Journal_Draw()
    // {
    // char *pDialogueString; // eax@21
    GUIWindow journal_window;

    render->DrawTextureNew(pViewport->uViewportTL_X / 640.0f, pViewport->uViewportTL_Y / 480.0f, ui_book_journal_background);
    if ((bookButtonClicked && bookButtonAction == BOOK_NEXT_PAGE) || !currentIdx) {
        render->DrawTextureNew((pViewport->uViewportTL_X + 407) / 640.0f, (pViewport->uViewportTL_Y + 2) / 480.0f, ui_book_button1_off);
    } else {
        render->DrawTextureNew((pViewport->uViewportTL_X + 398) / 640.0f, (pViewport->uViewportTL_Y + 1) / 480.0f, ui_book_button1_on);
    }

    if (BtnDown_flag || (currentIdx + 1) >= journalIdx.size()) {
        render->DrawTextureNew((pViewport->uViewportTL_X + 407) / 640.0f, (pViewport->uViewportTL_Y + 38) / 480.0f, ui_book_button2_off);
    } else {
        render->DrawTextureNew((pViewport->uViewportTL_X + 398) / 640.0f, (pViewport->uViewportTL_Y + 38) / 480.0f, ui_book_button2_on);
    }

    if (journalIdx.size() && !journalEntryPage[currentIdx]) {  // for title
        journal_window.uFrameWidth = game_viewport_width;
        journal_window.uFrameX = game_viewport_x;
        journal_window.uFrameY = game_viewport_y;
        journal_window.uFrameHeight = game_viewport_height;
        journal_window.uFrameZ = game_viewport_z;
        journal_window.uFrameW = game_viewport_w;
        if (pStorylineText->StoreLine[journalIdx[currentIdx]].pPageTitle) {
            journal_window.DrawTitleText(pBook2Font, 0, 22, ui_book_journal_title_color, pStorylineText->StoreLine[journalIdx[currentIdx]].pPageTitle, 3);
        }
    }

    // for other text
    journal_window.uFrameX = 48;
    journal_window.uFrameY = 70;
    journal_window.uFrameWidth = 360;
    journal_window.uFrameHeight = 264;
    journal_window.uFrameZ = 407;
    journal_window.uFrameW = journal_window.uFrameHeight + 69;

    if (bookButtonClicked && bookButtonAction == BOOK_NEXT_PAGE && (currentIdx + 1) < journalIdx.size()) {
        pAudioPlayer->playUISound(SOUND_openbook);
        currentIdx++;
    }
    if (bookButtonClicked && bookButtonAction == BOOK_PREV_PAGE && currentIdx) {
        pAudioPlayer->playUISound(SOUND_openbook);
        currentIdx--;
    }

    bookButtonClicked = false;

    if (journalIdx.size()) {
        std::string str = BuildDialogueString(pStorylineText->StoreLine[journalIdx[currentIdx]].pText,
                                              0, 0, 0, 0, &pParty->PartyTimes.HistoryEventTimes[journalIdx[currentIdx] - 1]);
        std::string pStringOnPage = pAutonoteFont->GetPageTop(str.c_str(), &journal_window, 1, journalEntryPage[currentIdx]);
        journal_window.DrawText(pAutonoteFont, {1, 0}, ui_book_journal_text_color, pStringOnPage, 0,
                                journal_window.uFrameY + journal_window.uFrameHeight, ui_book_journal_text_shadow);
    }
}
