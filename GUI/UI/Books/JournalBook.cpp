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
#include "GUI/UI/Books/JournalBook.h"

#include "Media/Audio/AudioPlayer.h"

Image *ui_book_journal_background = nullptr;

GUIWindow_JournalBook::GUIWindow_JournalBook() : GUIWindow_Book() {
    eWindowType = WINDOW_JournalBook;
    this->ptr_1C = (void *)WINDOW_JournalBook;  // inherited from GUIWindow::GUIWindow
    BasicBookInitialization();

    pEventTimer->Pause();
    pAudioPlayer->StopChannels(-1, -1);
    pBooksButtonOverlay = new GUIWindow_BooksButtonOverlay(0x258u, 0x169u, 0, 0,
                                                           (GUIButton *)pBtn_History);
    bFlashHistoryBook = 0;

    int pTextHeight;           // eax@12
    unsigned int page_count;   // esi@12
    GUIWindow journal_window;  // [sp+18h] [bp-54h]@8

    ui_book_journal_background = assets->GetImage_ColorKey("sbplayrnot", 0x7FF);

    ui_book_button1_on = assets->GetImage_Alpha("tab-an-6b");
    ui_book_button2_on = assets->GetImage_Alpha("tab-an-7b");
    ui_book_button1_off = assets->GetImage_Alpha("tab-an-6a");
    ui_book_button2_off = assets->GetImage_Alpha("tab-an-7a");

    pBtn_Book_1 = this->CreateButton(
        pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 1,
        ui_book_button1_on->GetWidth(), ui_book_button1_on->GetHeight(), 1, 0,
        UIMSG_ClickBooksBtn, 11, 0, localization->GetString(192),
        {{ui_book_button1_on}});
    pBtn_Book_2 = this->CreateButton(
        pViewport->uViewportTL_X + 398, pViewport->uViewportTL_Y + 38,
        ui_book_button2_on->GetWidth(), ui_book_button2_on->GetHeight(), 1, 0,
        UIMSG_ClickBooksBtn, 10, 0, localization->GetString(193),
        {{ui_book_button2_on}});

    num_achieved_awards = 0;
    journal_window.uFrameX = 48;
    journal_window.uFrameY = 70;
    journal_window.uFrameWidth = 360;
    journal_window.uFrameHeight = 264;
    journal_window.uFrameZ = 407;
    journal_window.uFrameHeight =
        (pAutonoteFont->GetHeight() - 3) * 264 / pAutonoteFont->GetHeight() - 3;
    journal_window.uFrameW = journal_window.uFrameHeight + 69;
    memset(&achieved_awards, 0, 4000);
    memset(Journal_limitation_factor.data(), 0, 100);
    if (books_primary_item_per_page < 29) {
        for (int i = books_primary_item_per_page;
             i < books_primary_item_per_page + 29; i++) {
            if (pParty->PartyTimes.HistoryEventTimes[i].Valid()) {
                if (pStorylineText->StoreLine[i + 1].pText) {
                    auto str = BuildDialogueString(
                        pStorylineText->StoreLine[i + 1].pText,
                        uActiveCharacter - 1, 0, 0, 0,
                        &pParty->PartyTimes.HistoryEventTimes[i]);
                    pTextHeight = pAutonoteFont->CalcTextHeight(
                        str, journal_window.uFrameWidth, 1);
                    page_count =
                        ((pTextHeight - (pAutonoteFont->GetHeight() - 3)) /
                         (signed int)journal_window.uFrameHeight) +
                        1;
                    memset32((char *)&achieved_awards[num_achieved_awards],
                             i + 1, page_count);
                    for (uint j = 0; j <= page_count - 1; ++j)
                        Journal_limitation_factor[num_achieved_awards++] = j;
                }
            }
        }
    }
    full_num_items_in_book = num_achieved_awards;
    num_achieved_awards = 0;
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
    render->DrawTextureAlphaNew(471 / 640.0f, 445 / 480.0f,
                                ui_exit_cancel_button_background);
    //     BookUI_Journal_Draw();

    // ----- (00412E85) --------------------------------------------------------
    // void BookUI_Journal_Draw()
    // {
    // char *pDialogueString; // eax@21
    GUIWindow journal_window;  // [sp+8h] [bp-54h]@10

    render->DrawTextureAlphaNew(pViewport->uViewportTL_X / 640.0f,
                                pViewport->uViewportTL_Y / 480.0f,
                                ui_book_journal_background);
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

    if (!Journal_limitation_factor[books_primary_item_per_page]) {  // for title
        journal_window.uFrameWidth = game_viewport_width;
        journal_window.uFrameX = game_viewport_x;
        journal_window.uFrameY = game_viewport_y;
        journal_window.uFrameHeight = game_viewport_height;
        journal_window.uFrameZ = game_viewport_z;
        journal_window.uFrameW = game_viewport_w;
        if (pStorylineText
                ->StoreLine[achieved_awards[books_primary_item_per_page]]
                .pPageTitle)
            journal_window.DrawTitleText(
                pBook2Font, 0, 22, ui_book_journal_title_color,
                pStorylineText
                    ->StoreLine[achieved_awards[books_primary_item_per_page]]
                    .pPageTitle,
                3);
    }

    // for other text
    journal_window.uFrameX = 48;
    journal_window.uFrameY = 70;
    journal_window.uFrameWidth = 360;
    journal_window.uFrameHeight = 264;
    journal_window.uFrameZ = 407;
    journal_window.uFrameW = journal_window.uFrameHeight + 69;
    if (BtnDown_flag && books_primary_item_per_page + num_achieved_awards <
                            full_num_items_in_book) {  // Press bookmark next page
        pAudioPlayer->PlaySound(SOUND_openbook, 0, 0, -1, 0, 0);
        books_primary_item_per_page += num_achieved_awards;
        books_num_items_per_page[books_page_number++] = num_achieved_awards;
    }
    if (BtnUp_flag && books_page_number) {  // Press bookmark previous page
        pAudioPlayer->PlaySound(SOUND_openbook, 0, 0, -1, 0, 0);
        --books_page_number;
        books_primary_item_per_page -=
            (unsigned __int8)books_num_items_per_page[books_page_number];
    }
    if (!num_achieved_awards || books_primary_item_per_page < 1) {
        books_primary_item_per_page = 0;
        books_page_number = 0;
    }
    BtnDown_flag = 0;
    BtnUp_flag = 0;
    num_achieved_awards = 0;
    if (achieved_awards[books_primary_item_per_page]) {
        int index = ((int)achieved_awards[books_primary_item_per_page] - 1);
        auto str = BuildDialogueString(
            pStorylineText
                ->StoreLine[achieved_awards[books_primary_item_per_page]]
                .pText,
            uActiveCharacter - 1, 0, 0, 0,
            &pParty->PartyTimes.HistoryEventTimes[index]);
        String pStringOnPage = pAutonoteFont->GetPageTop(
            str.c_str(), &journal_window, 1,
            (unsigned __int8)
                Journal_limitation_factor[books_primary_item_per_page]);
        journal_window.DrawText(
            pAutonoteFont, 1, 0, ui_book_journal_text_color, pStringOnPage, 0,
            journal_window.uFrameY + journal_window.uFrameHeight,
            ui_book_journal_text_shadow);
        ++num_achieved_awards;
    }
}
