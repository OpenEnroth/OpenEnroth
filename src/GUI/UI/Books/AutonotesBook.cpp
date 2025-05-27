#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Viewport.h"
#include "Engine/Tables/AutonoteTable.h"
#include "Engine/mm7_data.h"

#include "GUI/GUIButton.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/Books/AutonotesBook.h"

#include "Media/Audio/AudioPlayer.h"

GraphicsImage *ui_book_autonotes_background = nullptr;

AutonoteType autonoteBookDisplayType;

void GUIWindow_AutonotesBook::recalculateCurrentNotesTypePages() {
    _startingNotesIdx = 0;
    _currentPage = 0;
    _currentPageNotes = 0;
    _activeNotesIdx.clear();
    for (int i = 1; i < pAutonoteTxt.size(); ++i) {
        if (autonoteBookDisplayType == pAutonoteTxt[i].eType) {
            if (pParty->_autonoteBits[i] && !pAutonoteTxt[i].pText.empty()) {
                _activeNotesIdx.push_back(i);
            }
        }
    }
}

GUIWindow_AutonotesBook::GUIWindow_AutonotesBook() : GUIWindow_Book() {
    this->eWindowType = WindowType::WINDOW_AutonotesBook;

    pChildBooksOverlay = new GUIWindow_BooksButtonOverlay({527, 353}, {0, 0}, pBtn_Autonotes);
    bFlashAutonotesBook = false;

    ui_book_autonotes_background = assets->getImage_ColorKey("sbautnot");
    ui_book_quest_div_bar = assets->getImage_Alpha("divbar");

    ui_book_button1_on = assets->getImage_Alpha("tab-an-6b");
    ui_book_button2_on = assets->getImage_Alpha("tab-an-7b");
    ui_book_button3_on = assets->getImage_Alpha("tab-an-1b");
    ui_book_button4_on = assets->getImage_Alpha("tab-an-2b");
    ui_book_button5_on = assets->getImage_Alpha("tab-an-3b");
    ui_book_button6_on = assets->getImage_Alpha("tab-an-5b");
    ui_book_button7_on = assets->getImage_Alpha("tab-an-4b");
    ui_book_button8_on = assets->getImage_Alpha("tab-an-8b");
    ui_book_button1_off = assets->getImage_Alpha("tab-an-6a");
    ui_book_button2_off = assets->getImage_Alpha("tab-an-7a");
    ui_book_button3_off = assets->getImage_Alpha("tab-an-1a");
    ui_book_button4_off = assets->getImage_Alpha("tab-an-2a");
    ui_book_button5_off = assets->getImage_Alpha("tab-an-3a");
    ui_book_button6_off = assets->getImage_Alpha("tab-an-5a");
    ui_book_button7_off = assets->getImage_Alpha("tab-an-4a");
    ui_book_button8_off = assets->getImage_Alpha("tab-an-8a");

    pBtn_Book_1 = CreateButton({pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 1}, {50, 34}, 1, 0,
        UIMSG_ClickBooksBtn, std::to_underlying(BOOK_PREV_PAGE), Io::InputAction::DialogLeft, localization->GetString(LSTR_SCROLL_DOWN), {ui_book_button1_on});
    pBtn_Book_2 = CreateButton({pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 38}, {50, 34}, 1, 0,
        UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NEXT_PAGE), Io::InputAction::DialogRight, localization->GetString(LSTR_SCROLL_UP), {ui_book_button2_on});
    pBtn_Book_3 = CreateButton({pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 113}, {50, 34}, 1, 0,
        UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NOTES_POTION), Io::InputAction::Invalid, localization->GetString(LSTR_POTION_NOTES), {ui_book_button3_on});
    pBtn_Book_4 = CreateButton({pViewport->uViewportTL_X + 399, pViewport->uViewportTL_Y + 150}, {50, 34}, 1, 0,
        UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NOTES_FOUNTAIN), Io::InputAction::Invalid, localization->GetString(LSTR_FOUNTAIN_NOTES), {ui_book_button4_on});
    pBtn_Book_5 = CreateButton({pViewport->uViewportTL_X + 397, pViewport->uViewportTL_Y + 188}, {50, 34}, 1, 0,
        UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NOTES_OBELISK), Io::InputAction::Invalid, localization->GetString(LSTR_OBELISK_NOTES), {ui_book_button5_on});
    pBtn_Book_6 = CreateButton({pViewport->uViewportTL_X + 397, pViewport->uViewportTL_Y + 226}, {50, 34}, 1, 0,
        UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NOTES_SEER), Io::InputAction::Invalid, localization->GetString(LSTR_SEER_NOTES), {ui_book_button6_on});
    pBtn_Autonotes_Misc = CreateButton({pViewport->uViewportTL_X + 397, pViewport->uViewportTL_Y + 264}, {50, 34}, 1, 0,
        UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NOTES_MISC), Io::InputAction::Invalid, localization->GetString(LSTR_MISCELLANEOUS_NOTES), {ui_book_button7_on});
    pBtn_Autonotes_Instructors = CreateButton({pViewport->uViewportTL_X + 397, pViewport->uViewportTL_Y + 302}, {50, 34}, 1, 0,
        UIMSG_ClickBooksBtn, std::to_underlying(BOOK_NOTES_INSTRUCTORS), Io::InputAction::Invalid, localization->GetString(LSTR_INSTRUCTORS), {ui_book_button8_on});

    recalculateCurrentNotesTypePages();
}

void GUIWindow_AutonotesBook::Update() {
    render->DrawTextureNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);

    int pTextHeight;
    bool noteTypeChanged = false;
    GUIWindow autonotes_window;

    render->DrawTextureNew(pViewport->uViewportTL_X / 640.0f, pViewport->uViewportTL_Y / 480.0f, ui_book_autonotes_background);
    if ((_bookButtonClicked && _bookButtonAction == BOOK_PREV_PAGE) || !_startingNotesIdx) {
        render->DrawTextureNew((pViewport->uViewportTL_X + 407) / 640.0f, (pViewport->uViewportTL_Y + 2) / 480.0f, ui_book_button1_off);
    } else {
        render->DrawTextureNew((pViewport->uViewportTL_X + 398) / 640.0f, (pViewport->uViewportTL_Y + 1) / 480.0f, ui_book_button1_on);
    }

    if ((_bookButtonClicked && _bookButtonAction == BOOK_NEXT_PAGE) || (_startingNotesIdx + _currentPageNotes) >= _activeNotesIdx.size()) {
        render->DrawTextureNew((pViewport->uViewportTL_X + 407) / 640.0f, (pViewport->uViewportTL_Y + 38) / 480.0f, ui_book_button2_off);
    } else {
        render->DrawTextureNew((pViewport->uViewportTL_X + 398) / 640.0f, (pViewport->uViewportTL_Y + 38) / 480.0f, ui_book_button2_on);
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_NOTES_POTION) {
        if (autonoteBookDisplayType == AUTONOTE_POTION_RECIPE) {
            render->DrawTextureNew((pViewport->uViewportTL_X + 398) / 640.0f, (pViewport->uViewportTL_Y + 113) / 480.0f, ui_book_button3_on);
        } else {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            autonoteBookDisplayType = AUTONOTE_POTION_RECIPE;
            noteTypeChanged = true;
            render->DrawTextureNew((pViewport->uViewportTL_X + 398) / 640.0f, (pViewport->uViewportTL_Y + 113) / 480.0f, ui_book_button3_on);
        }
    } else {
        if (autonoteBookDisplayType == AUTONOTE_POTION_RECIPE) {
            render->DrawTextureNew((pViewport->uViewportTL_X + 398) / 640.0f, (pViewport->uViewportTL_Y + 113) / 480.0f, ui_book_button3_on);
        } else {
            render->DrawTextureNew((pViewport->uViewportTL_X + 408) / 640.0f, (pViewport->uViewportTL_Y + 113) / 480.0f, ui_book_button3_off);
        }
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_NOTES_FOUNTAIN) {
        if (autonoteBookDisplayType == AUTONOTE_STAT_HINT) {
            render->DrawTextureNew((pViewport->uViewportTL_X + 399) / 640.0f, (pViewport->uViewportTL_Y + 150) / 480.0f, ui_book_button4_on);
        } else {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            autonoteBookDisplayType = AUTONOTE_STAT_HINT;
            noteTypeChanged = true;
            render->DrawTextureNew( (pViewport->uViewportTL_X + 399) / 640.0f, (pViewport->uViewportTL_Y + 150) / 480.0f, ui_book_button4_on);
        }
    } else {
        if (autonoteBookDisplayType == AUTONOTE_STAT_HINT) {
            render->DrawTextureNew((pViewport->uViewportTL_X + 399) / 640.0f, (pViewport->uViewportTL_Y + 150) / 480.0f, ui_book_button4_on);
        } else {
            render->DrawTextureNew((pViewport->uViewportTL_X + 408) / 640.0f, (pViewport->uViewportTL_Y + 150) / 480.0f, ui_book_button4_off);
        }
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_NOTES_OBELISK) {
        if (autonoteBookDisplayType == AUTONOTE_OBELISK) {
            render->DrawTextureNew((pViewport->uViewportTL_X + 397) / 640.0f, (pViewport->uViewportTL_Y + 188) / 480.0f, ui_book_button5_on);
        } else {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            autonoteBookDisplayType = AUTONOTE_OBELISK;
            noteTypeChanged = true;
            render->DrawTextureNew((pViewport->uViewportTL_X + 397) / 640.0f, (pViewport->uViewportTL_Y + 188) / 480.0f, ui_book_button5_on);
        }
    } else {
        if (autonoteBookDisplayType == AUTONOTE_OBELISK) {
            render->DrawTextureNew((pViewport->uViewportTL_X + 397) / 640.0f, (pViewport->uViewportTL_Y + 188) / 480.0f, ui_book_button5_on);
        } else {
            render->DrawTextureNew( (pViewport->uViewportTL_X + 408) / 640.0f, (pViewport->uViewportTL_Y + 188) / 480.0f, ui_book_button5_off);
        }
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_NOTES_SEER) {
        if (autonoteBookDisplayType == AUTONOTE_SEER) {
            render->DrawTextureNew((pViewport->uViewportTL_X + 397) / 640.0f, (pViewport->uViewportTL_Y + 226) / 480.0f, ui_book_button6_on);
        } else {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            autonoteBookDisplayType = AUTONOTE_SEER;
            noteTypeChanged = true;
            render->DrawTextureNew((pViewport->uViewportTL_X + 397) / 640.0f, (pViewport->uViewportTL_Y + 226) / 480.0f, ui_book_button6_on);
        }
    } else {
        if (autonoteBookDisplayType == AUTONOTE_SEER) {
            render->DrawTextureNew((pViewport->uViewportTL_X + 397) / 640.0f, (pViewport->uViewportTL_Y + 226) / 480.0f, ui_book_button6_on);
        } else {
            render->DrawTextureNew((pViewport->uViewportTL_X + 408) / 640.0f, (pViewport->uViewportTL_Y + 226) / 480.0f, ui_book_button6_off);
        }
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_NOTES_MISC) {
        if (autonoteBookDisplayType == AUTONOTE_MISC) {
            render->DrawTextureNew((pViewport->uViewportTL_X + 397) / 640.0f, (pViewport->uViewportTL_Y + 264) / 480.0f, ui_book_button7_on);
        } else {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            autonoteBookDisplayType = AUTONOTE_MISC;
            noteTypeChanged = true;
            render->DrawTextureNew((pViewport->uViewportTL_X + 397) / 640.0f, (pViewport->uViewportTL_Y + 264) / 480.0f, ui_book_button7_on);
        }
    } else {
        if (autonoteBookDisplayType == AUTONOTE_MISC) {
            render->DrawTextureNew((pViewport->uViewportTL_X + 397) / 640.0f, (pViewport->uViewportTL_Y + 264) / 480.0f, ui_book_button7_on);
        } else {
            render->DrawTextureNew((pViewport->uViewportTL_X + 408) / 640.0f, (pViewport->uViewportTL_Y + 263) / 480.0f, ui_book_button7_off);
        }
    }

    if (_bookButtonClicked && _bookButtonAction == BOOK_NOTES_INSTRUCTORS) {
        if (autonoteBookDisplayType == AUTONOTE_TEACHER) {
            render->DrawTextureNew((pViewport->uViewportTL_X + 397) / 640.0f, (pViewport->uViewportTL_Y + 302) / 480.0f, ui_book_button8_on);
        } else {
            pAudioPlayer->playUISound(SOUND_StartMainChoice02);
            autonoteBookDisplayType = AUTONOTE_TEACHER;
            noteTypeChanged = true;
            render->DrawTextureNew((pViewport->uViewportTL_X + 397) / 640.0f, (pViewport->uViewportTL_Y + 302) / 480.0f, ui_book_button8_on);
        }
    } else {
        if (autonoteBookDisplayType == AUTONOTE_TEACHER) {
            render->DrawTextureNew((pViewport->uViewportTL_X + 397) / 640.0f, (pViewport->uViewportTL_Y + 302) / 480.0f, ui_book_button8_on);
        } else {
            render->DrawTextureNew((pViewport->uViewportTL_X + 408) / 640.0f, (pViewport->uViewportTL_Y + 302) / 480.0f, ui_book_button8_off);
        }
    }

    // for title
    autonotes_window.uFrameWidth = game_viewport_width;
    autonotes_window.uFrameHeight = game_viewport_height;
    autonotes_window.uFrameX = game_viewport_x;
    autonotes_window.uFrameY = game_viewport_y;
    autonotes_window.uFrameZ = game_viewport_z;
    autonotes_window.uFrameW = game_viewport_w;
    autonotes_window.DrawTitleText(assets->pFontBookTitle.get(), 0, 22, ui_book_autonotes_title_color, localization->GetString(LSTR_AUTO_NOTES), 3);

    // for other text
    autonotes_window.uFrameX = 48;
    autonotes_window.uFrameY = 70;
    autonotes_window.uFrameWidth = 360;
    autonotes_window.uFrameHeight = 264;
    autonotes_window.uFrameZ = 407;
    autonotes_window.uFrameW = 333;

    if (_bookButtonClicked) {
        if (_bookButtonAction >= BOOK_NOTES_POTION && _bookButtonAction <= BOOK_NOTES_INSTRUCTORS) {
            if (noteTypeChanged) {
                recalculateCurrentNotesTypePages();
            }
        } else {
            if (_bookButtonAction == BOOK_NEXT_PAGE && (_startingNotesIdx + _currentPageNotes) < _activeNotesIdx.size()) {
                pAudioPlayer->playUISound(SOUND_openbook);
                _startingNotesIdx += _currentPageNotes;
                _notesPerPage[_currentPage] = _currentPageNotes;
                _currentPage++;
            }
            if (_bookButtonAction == BOOK_PREV_PAGE && _startingNotesIdx) {
                pAudioPlayer->playUISound(SOUND_openbook);
                _currentPage--;
                _startingNotesIdx -= _notesPerPage[_currentPage];
            }
        }
    }

    _bookButtonClicked = false;
    _currentPageNotes = 0;

    for (int i = _startingNotesIdx; i < _activeNotesIdx.size(); ++i) {
        _currentPageNotes++;

        autonotes_window.DrawText(assets->pFontBookOnlyShadow.get(), {1, 0}, ui_book_autonotes_text_color, pAutonoteTxt[_activeNotesIdx[i]].pText);
        pTextHeight = assets->pFontBookOnlyShadow->CalcTextHeight(pAutonoteTxt[_activeNotesIdx[i]].pText, autonotes_window.uFrameWidth, 1);
        if ((autonotes_window.uFrameY + pTextHeight) > autonotes_window.uFrameHeight) {
            break;
        }

        render->DrawTextureNew(100 / 640.0f, ((autonotes_window.uFrameY + pTextHeight) + 12) / 480.0f, ui_book_quest_div_bar);
        autonotes_window.uFrameY = (autonotes_window.uFrameY + pTextHeight) + 24;
    }
}
