#include <memory>

#include "Engine/AssetsManager.h"
#include "Engine/GameType.h"
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

std::shared_ptr<GraphicsImage> ui_book_quests_background;

// MM8's Close button position, measured from the original game's screen.
static const Pointi kMm8QuestCloseButtonPos = {555, 445};

GUIWindow_QuestBook::GUIWindow_QuestBook() {
    this->eWindowType = WINDOW_QUEST_BOOK;

    pChildBooksOverlay = std::make_unique<GUIWindow_BooksButtonOverlay>(Pointi{493, 355}, Sizei{0, 0}, pBtn_Quests);
    bFlashQuestBook = false;

    // MM8's equivalents of "sbquiknot" and "tab-an-6/7". "irbgrnd" is the shared book background, "irt01"/
    // "irt02" are the prev/next-page book-glyph ribbons ("r"=red/on, "b"=blue/off).
    if (gameType() == GAME_TYPE_MM8) {
        ui_book_quests_background = assets->getImage_ColorKey("irbgrnd");
        ui_book_button1_on = assets->getImage_Alpha("irt01r");
        ui_book_button2_on = assets->getImage_Alpha("irt02r");
        ui_book_button1_off = assets->getImage_Alpha("irt01b");
        ui_book_button2_off = assets->getImage_Alpha("irt02b");
    } else {
        ui_book_quests_background = assets->getImage_Solid("sbquiknot");
        ui_book_button1_on = assets->getImage_Alpha("tab-an-6b");
        ui_book_button2_on = assets->getImage_Alpha("tab-an-7b");
        ui_book_button1_off = assets->getImage_Alpha("tab-an-6a");
        ui_book_button2_off = assets->getImage_Alpha("tab-an-7a");
    }
    ui_book_quest_div_bar = assets->getImage_Alpha("divbar");

    bool isMm8 = gameType() == GAME_TYPE_MM8;
    Sizei book1Size = isMm8 ? Sizei{50, 34} : ui_book_button1_on->size();
    Sizei book2Size = isMm8 ? Sizei{50, 34} : ui_book_button2_on->size();
    pBtn_Book_1 = CreateButton(pViewport.topLeft() + Pointi(398, 1), book1Size, BUTTON_TYPE_NORMAL, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_PREV_PAGE), INPUT_ACTION_DIALOG_LEFT, localization->str(LSTR_SCROLL_UP), {ui_book_button1_on});
    pBtn_Book_2 = CreateButton(pViewport.topLeft() + Pointi(398, 38), book2Size, BUTTON_TYPE_NORMAL, 0,
                               UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NEXT_PAGE), INPUT_ACTION_DIALOG_RIGHT, localization->str(LSTR_SCROLL_DOWN), {ui_book_button2_on});

    if (isMm8) {
        CreateMm8BookTabColumn();
        // Real MM8 Close button position/size (see kMm8QuestCloseButtonPos above) - re-positions
        // the clickable rect the base ctor already created at MM7's {471,445}-ish slot.
        for (GUIButton *button : vButtons) {
            if (button->id == "Book_Close") {
                button->rect = Recti(kMm8QuestCloseButtonPos, ui_exit_cancel_button_background->size());
                break;
            }
        }
    }

    for (auto i : pQuestTable.indices())
        if (pParty->_questBits[i] && !pQuestTable[i].empty())
            _activeQuestsIdx.push_back(i);
}

void GUIWindow_QuestBook::Update() {
    bool isMm8 = gameType() == GAME_TYPE_MM8;
    if (!isMm8)
        render->DrawQuad2D(ui_exit_cancel_button_background, {471, 445});

    int pTextHeight;
    render->DrawQuad2D(ui_book_quests_background, pViewport.topLeft());
    if (isMm8)
        DrawMm8BookTabColumn();

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
    DrawTitleText(assets->pFontBookTitle.get(), 0, 22, ui_book_quests_title_color, localization->str(LSTR_CURRENT_QUESTS), 3, pViewport);

    // for other text
    Recti questbook_window(48, 70, 360, 264);

    if (_bookButtonClicked == BOOK_BUTTON_PRESSED_FRAMES && _bookButtonAction == BOOK_NEXT_PAGE && (_startingQuestIdx + _currentPageQuests) < _activeQuestsIdx.size()) {
        pAudioPlayer->playUISound(SOUND_openbook);
        _startingQuestIdx += _currentPageQuests;
        _questsPerPage[_currentPage] = _currentPageQuests;
        _currentPage++;
    }

    if (_bookButtonClicked == BOOK_BUTTON_PRESSED_FRAMES && _bookButtonAction == BOOK_PREV_PAGE && _startingQuestIdx) {
        pAudioPlayer->playUISound(SOUND_openbook);
        _currentPage--;
        _startingQuestIdx -= _questsPerPage[_currentPage];
    }

    if (_bookButtonClicked)
        _bookButtonClicked--;

    _currentPageQuests = 0;

    for (int i = _startingQuestIdx; i < _activeQuestsIdx.size(); ++i) {
        _currentPageQuests++;

        DrawText(assets->pFontBookOnlyShadow.get(), {1, 0}, ui_book_quests_text_color, pQuestTable[_activeQuestsIdx[i]], questbook_window);
        pTextHeight = assets->pFontBookOnlyShadow->CalcTextHeight(pQuestTable[_activeQuestsIdx[i]], questbook_window.w, 1);
        if ((questbook_window.y + pTextHeight) > questbook_window.h)
            break;

        render->DrawQuad2D(ui_book_quest_div_bar, {100, (questbook_window.y + pTextHeight) + 12});
        questbook_window.y = (questbook_window.y + pTextHeight) + 24;
    }

    // Drawn last (not at the top of Update() like MM7's line above) so the background fill two
    // lines up doesn't paint over it - see kMm8QuestCloseButtonPos's own comment.
    if (isMm8)
        render->DrawQuad2D(ui_exit_cancel_button_background, kMm8QuestCloseButtonPos);
}
