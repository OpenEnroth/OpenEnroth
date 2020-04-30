#include "Engine/AssetsManager.h"
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
#include "GUI/UI/Books/QuestBook.h"

#include "Media/Audio/AudioPlayer.h"

Image *ui_book_quests_background = nullptr;

GUIWindow_QuestBook::GUIWindow_QuestBook() : GUIWindow_Book() {
    this->ptr_1C =
        (void *)WINDOW_QuestBook;  // inherited from GUIWindow::GUIWindow
    BasicBookInitialization();

    // --------------------------------
    // 004304E7 Game_EventLoop --- part
    pEventTimer->Pause();
    pAudioPlayer->StopChannels(-1, -1);
    pBooksButtonOverlay =
        new GUIWindow_BooksButtonOverlay(493u, 355u, 0, 0, pBtn_Quests);
    bFlashQuestBook = 0;

    // ----------------------------------------------
    // 00411BFC GUIWindow::InitializeBookView -- part
    ui_book_quests_background = assets->GetImage_Solid("sbquiknot");
    ui_book_quest_div_bar = assets->GetImage_Alpha("divbar");

    ui_book_button1_on = assets->GetImage_Alpha("tab-an-6b");
    ui_book_button2_on = assets->GetImage_Alpha("tab-an-7b");
    ui_book_button1_off = assets->GetImage_Alpha("tab-an-6a");
    ui_book_button2_off = assets->GetImage_Alpha("tab-an-7a");

    pBtn_Book_1 = CreateButton(
        pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 1,
        ui_book_button1_on->GetWidth(), ui_book_button1_on->GetWidth(), 1, 0,
        UIMSG_ClickBooksBtn, 0xBu, 0,
        localization->GetString(192),  // "Scroll Up"
        {{ui_book_button1_on}});
    pBtn_Book_2 = CreateButton(
        pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 38,
        ui_book_button2_on->GetWidth(), ui_book_button2_on->GetHeight(), 1, 0,
        UIMSG_ClickBooksBtn, 0xAu, 0,
        localization->GetString(193),  // "Scroll Down"
        {{ui_book_button2_on}});
    num_achieved_awards = 0;
    memset(achieved_awards.data(), 0, 4000);
    for (uint i = books_primary_item_per_page; i < 512; ++i) {
        if (_449B57_test_bit(pParty->_quest_bits, i) && pQuestTable[i]) {
            achieved_awards[num_achieved_awards] = (AwardType)i;
            ++num_achieved_awards;
        }
    }
    full_num_items_in_book = num_achieved_awards;
    num_achieved_awards = 0;
}

void GUIWindow_QuestBook::Update() {
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
    //     BookUI_Questbook_Draw();

    // ----- (00413126) --------------------------------------------------------
    // void BookUI_Questbook_Draw()
    // {
    int pTextHeight;             // eax@19
    GUIWindow questbook_window;  // [sp+Ch] [bp-54h]@9

    render->DrawTextureNew(pViewport->uViewportTL_X / 640.0f,
                           pViewport->uViewportTL_Y / 480.0f,
                           ui_book_quests_background);
    if (BtnUp_flag ||
        !books_primary_item_per_page)  // Bookmark Up(Закладка вверх)
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 407) / 640.0f,
                                    (pViewport->uViewportTL_Y + 2) / 480.0f,
                                    ui_book_button1_off);
    else
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 398) / 640.0f,
                                    (pViewport->uViewportTL_Y + 1) / 480.0f,
                                    ui_book_button1_on);

    if (BtnDown_flag ||
        books_primary_item_per_page + num_achieved_awards >=
            full_num_items_in_book)  // Bookmark Down(Закладка вниз)
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 407) / 640.0f,
                                    (pViewport->uViewportTL_Y + 38) / 480.0f,
                                    ui_book_button2_off);
    else
        render->DrawTextureAlphaNew((pViewport->uViewportTL_X + 398) / 640.0f,
                                    (pViewport->uViewportTL_Y + 38) / 480.0f,
                                    ui_book_button2_on);

    // for title
    questbook_window.uFrameWidth = game_viewport_width;
    questbook_window.uFrameHeight = game_viewport_height;
    questbook_window.uFrameX = game_viewport_x;
    questbook_window.uFrameY = game_viewport_y;
    questbook_window.uFrameZ = game_viewport_z;
    questbook_window.uFrameW = game_viewport_w;
    questbook_window.DrawTitleText(
        pBook2Font, 0, 22, ui_book_quests_title_color,
        localization->GetString(174), 3);  // "Current Quests"

    // for other text
    questbook_window.uFrameX = 48;
    questbook_window.uFrameY = 70;
    questbook_window.uFrameWidth = 360;
    questbook_window.uFrameHeight = 264;
    questbook_window.uFrameZ = 407;
    questbook_window.uFrameW = 333;
    if (BtnDown_flag &&
        books_primary_item_per_page + num_achieved_awards <
            full_num_items_in_book) {  // Click Bookmark Down(нажатие закладки
                                       // вниз)
        pAudioPlayer->PlaySound(SOUND_openbook, 0, 0, -1, 0, 0);
        books_primary_item_per_page += num_achieved_awards;
        books_num_items_per_page[books_page_number++] = num_achieved_awards;
    }
    if (BtnUp_flag &&
        books_page_number) {  // Click Bookmark Up(Нажатие закладки вверх)
        pAudioPlayer->PlaySound(SOUND_openbook, 0, 0, -1, 0, 0);
        --books_page_number;
        books_primary_item_per_page -=
            (unsigned __int8)books_num_items_per_page[books_page_number];
    }
    if (!num_achieved_awards || !books_primary_item_per_page) {
        books_page_number = 0;
        books_primary_item_per_page = 0;
    }
    BtnDown_flag = 0;
    BtnUp_flag = 0;
    num_achieved_awards = 0;
    for (uint i = books_primary_item_per_page; i < full_num_items_in_book;
         ++i) {
        ++num_achieved_awards;
        questbook_window.DrawText(pAutonoteFont, 1, 0,
                                  ui_book_quests_text_color,
                                  pQuestTable[achieved_awards[i]], 0, 0, 0);
        pTextHeight = pAutonoteFont->CalcTextHeight(
            pQuestTable[achieved_awards[i]], questbook_window.uFrameWidth, 1);
        if ((signed int)(questbook_window.uFrameY + pTextHeight) >
            (signed int)questbook_window.uFrameHeight)
            break;

        render->DrawTextureAlphaNew(
            100 / 640.0f,
            ((questbook_window.uFrameY + pTextHeight) + 12) / 480.0f,
            ui_book_quest_div_bar);

        questbook_window.uFrameY =
            (questbook_window.uFrameY + pTextHeight) + 24;
    }
}
