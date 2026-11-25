#include <memory>
#include <string>

#include "Engine/AssetsManager.h"
#include "Engine/GameType.h"
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

std::shared_ptr<GraphicsImage> ui_book_journal_background;

// MM8's Close button position, measured from the original game's screen.
static const Pointi kMm8JournalCloseButtonPos = {555, 445};

GUIWindow_JournalBook::GUIWindow_JournalBook() {
    eWindowType = WINDOW_JOURNAL_BOOK;

    pChildBooksOverlay = std::make_unique<GUIWindow_BooksButtonOverlay>(Pointi{600, 361}, Sizei{0, 0}, pBtn_History);
    bFlashHistoryBook = false;


    // MM8's equivalents of "sbplayrnot" and "tab-an-6/7".
    if (gameType() == GAME_TYPE_MM8) {
        ui_book_journal_background = assets->getImage_ColorKey("irbgrnd");
        ui_book_button1_on = assets->getImage_Alpha("irt01r");
        ui_book_button2_on = assets->getImage_Alpha("irt02r");
        ui_book_button1_off = assets->getImage_Alpha("irt01b");
        ui_book_button2_off = assets->getImage_Alpha("irt02b");
    } else {
        ui_book_journal_background = assets->getImage_ColorKey("sbplayrnot");
        ui_book_button1_on = assets->getImage_Alpha("tab-an-6b");
        ui_book_button2_on = assets->getImage_Alpha("tab-an-7b");
        ui_book_button1_off = assets->getImage_Alpha("tab-an-6a");
        ui_book_button2_off = assets->getImage_Alpha("tab-an-7a");
    }

    bool isMm8 = gameType() == GAME_TYPE_MM8;
    Sizei book1Size = isMm8 ? Sizei{50, 34} : ui_book_button1_on->size();
    Sizei book2Size = isMm8 ? Sizei{50, 34} : ui_book_button2_on->size();
    pBtn_Book_1 = CreateButton(pViewport.topLeft() + Pointi(398, 1), book1Size, BUTTON_TYPE_NORMAL, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_PREV_PAGE), INPUT_ACTION_DIALOG_LEFT, localization->str(LSTR_SCROLL_UP), {ui_book_button1_on});
    pBtn_Book_2 = CreateButton(pViewport.topLeft() + Pointi(398, 38), book2Size, BUTTON_TYPE_NORMAL, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NEXT_PAGE), INPUT_ACTION_DIALOG_RIGHT, localization->str(LSTR_SCROLL_DOWN), {ui_book_button2_on});

    if (isMm8) {
        CreateMm8BookTabColumn();
        // Real MM8 Close button position/size (see kMm8JournalCloseButtonPos above) -
        // re-positions the clickable rect the base ctor already created at MM7's slot.
        for (GUIButton *button : vButtons) {
            if (button->id == "Book_Close") {
                button->rect = Recti(kMm8JournalCloseButtonPos, ui_exit_cancel_button_background->size());
                break;
            }
        }
    }

    Recti journal_window(48, 70, 360, 264);
    journal_window.h = (assets->pFontBookOnlyShadow->GetHeight() - 3) * 264 / assets->pFontBookOnlyShadow->GetHeight() - 3;

    for (int i = 0; i < pParty->PartyTimes.HistoryEventTimes.size(); i++) {
        if (pParty->PartyTimes.HistoryEventTimes[i].isValid()) {
            if (!pHistoryTable->historyLines[i + 1].pText.empty()) {
                NPCData dummyNpc;
                std::string str = BuildDialogueString(pHistoryTable->historyLines[i + 1].pText, 0, &dummyNpc, 0, HOUSE_INVALID, SHOP_SCREEN_INVALID, &pParty->PartyTimes.HistoryEventTimes[i]);
                int pTextHeight = assets->pFontBookOnlyShadow->CalcTextHeight(str, journal_window.w, 1);
                int pages = ((pTextHeight - (assets->pFontBookOnlyShadow->GetHeight() - 3)) / (signed int)journal_window.h) + 1;
                for (int j = 0; j < pages; ++j) {
                    _journalIdx.push_back(i + 1);
                    _journalEntryPage.push_back(j);
                }
            }
        }
    }
}

void GUIWindow_JournalBook::Update() {
    bool isMm8 = gameType() == GAME_TYPE_MM8;
    if (!isMm8)
        render->DrawQuad2D(ui_exit_cancel_button_background, {471, 445});

    render->DrawQuad2D(ui_book_journal_background, pViewport.topLeft());
    if (isMm8)
        DrawMm8BookTabColumn();
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

    if (_journalIdx.size() && !_journalEntryPage[_currentIdx] && !pHistoryTable->historyLines[_journalIdx[_currentIdx]].pPageTitle.empty())  // for title
        DrawTitleText(assets->pFontBookTitle.get(), 0, 22, ui_book_journal_title_color, pHistoryTable->historyLines[_journalIdx[_currentIdx]].pPageTitle, 3, pViewport);

    // for other text
    Recti journal_window(48, 70, 360, 264);

    if (_bookButtonClicked == BOOK_BUTTON_PRESSED_FRAMES && _bookButtonAction == BOOK_NEXT_PAGE && (_currentIdx + 1) < _journalIdx.size()) {
        pAudioPlayer->playUISound(SOUND_openbook);
        _currentIdx++;
    }
    if (_bookButtonClicked == BOOK_BUTTON_PRESSED_FRAMES && _bookButtonAction == BOOK_PREV_PAGE && _currentIdx) {
        pAudioPlayer->playUISound(SOUND_openbook);
        _currentIdx--;
    }

    if (_bookButtonClicked)
        _bookButtonClicked--;

    if (_journalIdx.size()) {
        NPCData dummyNpc;
        std::string str = BuildDialogueString(pHistoryTable->historyLines[_journalIdx[_currentIdx]].pText, 0, &dummyNpc, 0, HOUSE_INVALID,
                                              SHOP_SCREEN_INVALID, &pParty->PartyTimes.HistoryEventTimes[_journalIdx[_currentIdx] - 1]);
        std::string pStringOnPage = assets->pFontBookOnlyShadow->GetPageText(str, {journal_window.w, journal_window.h}, 1, _journalEntryPage[_currentIdx]);
        DrawText(assets->pFontBookOnlyShadow.get(), {1, 0}, ui_book_journal_text_color, pStringOnPage, journal_window,
                                journal_window.y + journal_window.h, ui_book_journal_text_shadow);
    }

    // Drawn last (not at the top of Update() like MM7's line above) so the background fill above
    // doesn't paint over it - see kMm8JournalCloseButtonPos's own comment.
    if (isMm8)
        render->DrawQuad2D(ui_exit_cancel_button_background, kMm8JournalCloseButtonPos);
}
