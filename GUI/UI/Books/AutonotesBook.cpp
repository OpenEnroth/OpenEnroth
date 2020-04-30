#include "Engine/AssetsManager.h"
#include "Engine/Autonotes.h"
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
#include "GUI/UI/Books/AutonotesBook.h"

#include "Media/Audio/AudioPlayer.h"

Image *ui_book_autonotes_background = nullptr;

GUIWindow_AutonotesBook::GUIWindow_AutonotesBook() : GUIWindow_Book() {
    this->ptr_1C =
        (void *)WINDOW_AutonotesBook;  // inherited from GUIWindow::GUIWindow
    BasicBookInitialization();

    // --------------------------------
    // 004304E7 Game_EventLoop --- part
    pEventTimer->Pause();
    pAudioPlayer->StopChannels(-1, -1);
    pBooksButtonOverlay =
        new GUIWindow_BooksButtonOverlay(527, 353, 0, 0, pBtn_Autonotes);
    bFlashAutonotesBook = 0;

    // ----------------------------------------------
    // 00411BFC GUIWindow::InitializeBookView -- part
    ui_book_autonotes_background = assets->GetImage_ColorKey("sbautnot", 0x7FF);
    ui_book_quest_div_bar = assets->GetImage_Alpha("divbar");

    ui_book_button1_on = assets->GetImage_Alpha("tab-an-6b");
    ui_book_button2_on = assets->GetImage_Alpha("tab-an-7b");
    ui_book_button3_on = assets->GetImage_Alpha("tab-an-1b");
    ui_book_button4_on = assets->GetImage_Alpha("tab-an-2b");
    ui_book_button5_on = assets->GetImage_Alpha("tab-an-3b");
    ui_book_button6_on = assets->GetImage_Alpha("tab-an-5b");
    ui_book_button7_on = assets->GetImage_Alpha("tab-an-4b");
    ui_book_button8_on = assets->GetImage_Alpha("tab-an-8b");
    ui_book_button1_off = assets->GetImage_Alpha("tab-an-6a");
    ui_book_button2_off = assets->GetImage_Alpha("tab-an-7a");
    ui_book_button3_off = assets->GetImage_Alpha("tab-an-1a");
    ui_book_button4_off = assets->GetImage_Alpha("tab-an-2a");
    ui_book_button5_off = assets->GetImage_Alpha("tab-an-3a");
    ui_book_button6_off = assets->GetImage_Alpha("tab-an-5a");
    ui_book_button7_off = assets->GetImage_Alpha("tab-an-4a");
    ui_book_button8_off = assets->GetImage_Alpha("tab-an-8a");

    pBtn_Book_1 = CreateButton(
        pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 1, 50, 34, 1,
        0, UIMSG_ClickBooksBtn, 11, 0, localization->GetString(193),
        {{ui_book_button1_on}});
    pBtn_Book_2 = CreateButton(
        pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 38, 50, 34,
        1, 0, UIMSG_ClickBooksBtn, 10, 0, localization->GetString(192),
        {{ui_book_button2_on}});
    pBtn_Book_3 = CreateButton(
        pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 113, 50, 34,
        1, 0, UIMSG_ClickBooksBtn, 2, 0, localization->GetString(85),
        {{ui_book_button3_on}});  // "Potion Notes"
    pBtn_Book_4 = CreateButton(
        pViewport->uViewportTL_X + 399, pViewport->uViewportTL_Y + 150, 50, 34,
        1, 0, UIMSG_ClickBooksBtn, 3, 0, localization->GetString(137),
        {{ui_book_button4_on}});  // "Fountain Notes"
    pBtn_Book_5 = CreateButton(
        pViewport->uViewportTL_X + 397, pViewport->uViewportTL_Y + 188, 50, 34,
        1, 0, UIMSG_ClickBooksBtn, 4, 0, localization->GetString(8),
        {{ui_book_button5_on}});  // "Obelisk Notes"
    pBtn_Book_6 = CreateButton(
        pViewport->uViewportTL_X + 397, pViewport->uViewportTL_Y + 226, 50, 34,
        1, 0, UIMSG_ClickBooksBtn, 5, 0, localization->GetString(141),
        {{ui_book_button6_on}});  // "Seer Notes"
    pBtn_Autonotes_Misc = CreateButton(
        pViewport->uViewportTL_X + 397, pViewport->uViewportTL_Y + 264, 50, 34,
        1, 0, UIMSG_ClickBooksBtn, 6, 0, localization->GetString(123),
        {{ui_book_button7_on}});  // "Miscellaneous Notes"
    pBtn_Autonotes_Instructors = CreateButton(
        pViewport->uViewportTL_X + 397, pViewport->uViewportTL_Y + 302, 50, 34,
        1, 0, UIMSG_ClickBooksBtn, 7, 0, localization->GetString(662),
        {{ui_book_button8_on}});  // "Instructors"

    int num_achieved_awards = 0;
    for (uint i = books_primary_item_per_page; i < 196; ++i) {
        if (_506568_autonote_type ==
            pAutonoteTxt[i].eType) {  // dword_72371C[2 * v10] )
            if (i) {
                if (_449B57_test_bit(pParty->_autonote_bits, i) &&
                    pAutonoteTxt[i].pText) {
                    achieved_awards[num_achieved_awards] = (AwardType)i;
                    ++num_achieved_awards;
                }
            }
        }
    }
    full_num_items_in_book = num_achieved_awards;
    num_achieved_awards = 0;
}

void GUIWindow_AutonotesBook::Update() {
    // -----------------------------------
    // 004156F0 GUI_UpdateWindows --- part
    // {
    //     BookUI_Draw((WindowType)(int)ptr_1C);
    // }

    // ----- (00413CC6) --------------------------------------------------------
    // void BookUI_Draw(WindowType book) --- part
    // {
    render->DrawTextureAlphaNew(471 / 640.0f, 445 / 480.0f,
                                ui_exit_cancel_button_background);
    //     BookUI_Autonotes_Draw();

    // ----- (0041338E) --------------------------------------------------------
    // void BookUI_Autonotes_Draw()
    // {
    int pTextHeight;             // eax@65
    bool change_flag;            // [sp+10h] [bp-58h]@1
    GUIWindow autonotes_window;  // [sp+14h] [bp-54h]@46

    change_flag = false;
    render->DrawTextureAlphaNew(pViewport->uViewportTL_X / 640.0f,
                                pViewport->uViewportTL_Y / 480.0f,
                                ui_book_autonotes_background);
    if (BtnUp_flag || !books_primary_item_per_page)
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 407) / 640.0f,
                                    (pViewport->uViewportTL_Y + 2) / 480.0f,
                                    ui_book_button1_off);
    else
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 398) / 640.0f,
                                    (pViewport->uViewportTL_Y + 1) / 480.0f,
                                    ui_book_button1_on);

    if (BtnDown_flag || books_primary_item_per_page + num_achieved_awards >=
                            full_num_items_in_book)
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 407) / 640.0f,
                                    (pViewport->uViewportTL_Y + 38) / 480.0f,
                                    ui_book_button2_off);
    else
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 398) / 640.0f,
                                    (pViewport->uViewportTL_Y + 38) / 480.0f,
                                    ui_book_button2_on);

    if (Book_PageBtn3_flag) {  // Potions_page_flag
        if (_506568_autonote_type ==
            AUTONOTE_POTION_RECEPIE) {  // press again(повторное нажатие)
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 398) / 640.0f,
                (pViewport->uViewportTL_Y + 113) / 480.0f, ui_book_button3_on);
        } else {  // press(нажатие)
            change_flag = true;
            pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0,
                                    0);
            _506568_autonote_type = AUTONOTE_POTION_RECEPIE;
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 398) / 640.0f,
                (pViewport->uViewportTL_Y + 113) / 480.0f, ui_book_button3_on);
        }
    } else {
        if (_506568_autonote_type ==
            AUTONOTE_POTION_RECEPIE)  // default(по умолчанию при запуске окна)
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 398) / 640.0f,
                (pViewport->uViewportTL_Y + 113) / 480.0f, ui_book_button3_on);
        else  // Potions_page not active(вкладка снадобья не активна)
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 408) / 640.0f,
                (pViewport->uViewportTL_Y + 113) / 480.0f, ui_book_button3_off);
    }

    if (Book_PageBtn4_flag) {  // Fontains_page_flag
        if (_506568_autonote_type == AUTONOTE_STAT_HINT) {
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 399) / 640.0f,
                (pViewport->uViewportTL_Y + 150) / 480.0f, ui_book_button4_on);
        } else {
            change_flag = true;
            pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0,
                                    0);
            _506568_autonote_type = AUTONOTE_STAT_HINT;
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 399) / 640.0f,
                (pViewport->uViewportTL_Y + 150) / 480.0f, ui_book_button4_on);
        }
    } else {
        if (_506568_autonote_type == AUTONOTE_STAT_HINT)
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 399) / 640.0f,
                (pViewport->uViewportTL_Y + 150) / 480.0f, ui_book_button4_on);
        else
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 408) / 640.0f,
                (pViewport->uViewportTL_Y + 150) / 480.0f, ui_book_button4_off);
    }

    if (Book_PageBtn5_flag) {  // Autonotes_Obelisks_page_flag
        if (_506568_autonote_type == AUTONOTE_OBELISK) {
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 397) / 640.0f,
                (pViewport->uViewportTL_Y + 188) / 480.0f, ui_book_button5_on);
        } else {
            change_flag = true;
            pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0,
                                    0);
            _506568_autonote_type = AUTONOTE_OBELISK;
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 397) / 640.0f,
                (pViewport->uViewportTL_Y + 188) / 480.0f, ui_book_button5_on);
        }
    } else {
        if (_506568_autonote_type == AUTONOTE_OBELISK)
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 397) / 640.0f,
                (pViewport->uViewportTL_Y + 188) / 480.0f, ui_book_button5_on);
        else
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 408) / 640.0f,
                (pViewport->uViewportTL_Y + 188) / 480.0f, ui_book_button5_off);
    }

    if (Book_PageBtn6_flag) {  // Autonotes_Seer_page_flag
        if (_506568_autonote_type == AUTONOTE_SEER) {
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 397) / 640.0f,
                (pViewport->uViewportTL_Y + 226) / 480.0f, ui_book_button6_on);
        } else {
            change_flag = true;
            pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0,
                                    0);
            _506568_autonote_type = AUTONOTE_SEER;
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 397) / 640.0f,
                (pViewport->uViewportTL_Y + 226) / 480.0f, ui_book_button6_on);
        }
    } else {
        if (_506568_autonote_type == AUTONOTE_SEER)
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 397) / 640.0f,
                (pViewport->uViewportTL_Y + 226) / 480.0f, ui_book_button6_on);
        else
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 408) / 640.0f,
                (pViewport->uViewportTL_Y + 226) / 480.0f, ui_book_button6_off);
    }

    if (Autonotes_Misc_page_flag) {
        if (_506568_autonote_type == AUTONOTE_MISC) {
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 397) / 640.0f,
                (pViewport->uViewportTL_Y + 264) / 480.0f, ui_book_button7_on);
        } else {
            change_flag = true;
            pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0,
                                    0);
            _506568_autonote_type = AUTONOTE_MISC;
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 397) / 640.0f,
                (pViewport->uViewportTL_Y + 264) / 480.0f, ui_book_button7_on);
        }
    } else {
        if (_506568_autonote_type == AUTONOTE_MISC)
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 397) / 640.0f,
                (pViewport->uViewportTL_Y + 264) / 480.0f, ui_book_button7_on);
        else
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 408) / 640.0f,
                (pViewport->uViewportTL_Y + 263) / 480.0f, ui_book_button7_off);
    }

    if (Autonotes_Instructors_page_flag) {
        if (_506568_autonote_type == AUTONOTE_TEACHER) {
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 397) / 640.0f,
                (pViewport->uViewportTL_Y + 302) / 480.0f, ui_book_button8_on);
        } else {
            change_flag = true;
            pAudioPlayer->PlaySound(SOUND_StartMainChoice02, 0, 0, -1, 0,
                                    0);
            _506568_autonote_type = AUTONOTE_TEACHER;
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 397) / 640.0f,
                (pViewport->uViewportTL_Y + 302) / 480.0f, ui_book_button8_on);
        }
    } else {
        if (_506568_autonote_type == AUTONOTE_TEACHER)
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 397) / 640.0f,
                (pViewport->uViewportTL_Y + 302) / 480.0f, ui_book_button8_on);
        else
            render->DrawTextureAlphaNew(
                (pViewport->uViewportTL_X + 408) / 640.0f,
                (pViewport->uViewportTL_Y + 302) / 480.0f, ui_book_button8_off);
    }

    // for title
    autonotes_window.uFrameWidth = game_viewport_width;
    autonotes_window.uFrameHeight = game_viewport_height;
    autonotes_window.uFrameX = game_viewport_x;
    autonotes_window.uFrameY = game_viewport_y;
    autonotes_window.uFrameZ = game_viewport_z;
    autonotes_window.uFrameW = game_viewport_w;
    autonotes_window.DrawTitleText(
        pBook2Font, 0, 22, ui_book_autonotes_title_color,
        localization->GetString(154), 3);  // "Auto notes"

    // for other text
    autonotes_window.uFrameX = 48;
    autonotes_window.uFrameY = 70;
    autonotes_window.uFrameWidth = 360;
    autonotes_window.uFrameHeight = 264;
    autonotes_window.uFrameZ = 407;
    autonotes_window.uFrameW = 333;
    if (change_flag) {  // change bookmark(смена закладки)
        full_num_items_in_book = 0;
        books_primary_item_per_page = 0;
        books_page_number = 0;
        num_achieved_awards = 0;
        for (uint i = 1; i <= 195; ++i) {
            if (pAutonoteTxt[i].eType == _506568_autonote_type) {
                if ((unsigned __int16)_449B57_test_bit(pParty->_autonote_bits,
                                                       i) &&
                    (char *)pAutonoteTxt[i].pText)
                    achieved_awards[num_achieved_awards++] = (AwardType)i;
            }
        }
        full_num_items_in_book = num_achieved_awards;
    } else {  // not change bookmark(не меняется закладка)
        if (BtnDown_flag) {  // press Down bookmark(нажатие закладки пролистать
                             // дальше)
            if (num_achieved_awards + books_primary_item_per_page <
                full_num_items_in_book) {
                books_num_items_per_page[books_page_number++] =
                    num_achieved_awards;
                books_primary_item_per_page =
                    num_achieved_awards + books_primary_item_per_page;
                pAudioPlayer->PlaySound(SOUND_openbook, 0, 0, -1, 0, 0);
            }
        }
        if (BtnUp_flag && books_page_number) {  // press Up bookmark(нажатие
                                                // закладки пролистать назад)
            --books_page_number;
            books_primary_item_per_page -=
                (unsigned __int8)books_num_items_per_page[books_page_number];
            pAudioPlayer->PlaySound(SOUND_openbook, 0, 0, -1, 0, 0);
        }
        if (!num_achieved_awards ||
            !books_primary_item_per_page) {  // количество записей 0 или номер
                                             // первой страницы 0
            books_primary_item_per_page = 0;
            books_page_number = 0;
        }
    }
    BtnUp_flag = 0;
    BtnDown_flag = 0;
    Book_PageBtn3_flag = 0;  // Potions_page_flag
    Book_PageBtn4_flag = 0;  // Fontains_page_flag
    Book_PageBtn5_flag = 0;  // Autonotes_Obelisks_page_flag
    Book_PageBtn6_flag = 0;  // Autonotes_Seer_page_flag
    Autonotes_Misc_page_flag = 0;
    Autonotes_Instructors_page_flag = 0;
    num_achieved_awards = 0;
    for (uint i = books_primary_item_per_page; i < full_num_items_in_book;
         ++i) {
        ++num_achieved_awards;
        autonotes_window.DrawText(
            pAutonoteFont, 1, 0, ui_book_autonotes_text_color,
            pAutonoteTxt[achieved_awards[i]].pText, 0, 0, 0);
        pTextHeight = pAutonoteFont->CalcTextHeight(
            pAutonoteTxt[achieved_awards[i]].pText,
            autonotes_window.uFrameWidth, 1);
        if ((signed int)(autonotes_window.uFrameY + pTextHeight) >
            (signed int)autonotes_window.uFrameHeight)
            break;

        render->DrawTextureAlphaNew(
            100 / 640.0f,
            ((autonotes_window.uFrameY + pTextHeight) + 12) / 480.0f,
            ui_book_quest_div_bar);

        autonotes_window.uFrameY =
            (autonotes_window.uFrameY + pTextHeight) + 24;
    }
}
