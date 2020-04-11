#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>

#include "Engine/Awards.h"
#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"

#include "Engine/Graphics/IRender.h"

#include "GUI/GUIFont.h"
#include "GUI/UI/UIBooks.h"

#include "Media/Audio/AudioPlayer.h"

#include "Platform/Api.h"

Image *ui_book_button8_off = nullptr;
Image *ui_book_button8_on = nullptr;
Image *ui_book_button7_off = nullptr;
Image *ui_book_button7_on = nullptr;
Image *ui_book_button6_off = nullptr;
Image *ui_book_button6_on = nullptr;
Image *ui_book_button5_off = nullptr;
Image *ui_book_button5_on = nullptr;
Image *ui_book_button4_off = nullptr;
Image *ui_book_button4_on = nullptr;
Image *ui_book_button3_off = nullptr;
Image *ui_book_button3_on = nullptr;
Image *ui_book_button2_off = nullptr;
Image *ui_book_button2_on = nullptr;
Image *ui_book_button1_off = nullptr;
Image *ui_book_button1_on = nullptr;

Image *ui_book_map_frame = nullptr;

Image *ui_book_quest_div_bar = nullptr;

//----- (00411597) --------------------------------------------------------
void GUIWindow_Book::Release() {
    // -----------------------------------------
    // 0041C26A void GUIWindow::Release --- part
    free(pSpellFont);
    pSpellFont = nullptr;
    free(pBookFont);
    pBookFont = nullptr;
    free(pBook2Font);
    pBook2Font = nullptr;
    free(pAutonoteFont);
    pAutonoteFont = nullptr;

    if (ui_book_map_frame) {
        ui_book_map_frame->Release();
        ui_book_map_frame = nullptr;
    }

    pAudioPlayer->PlaySound(SOUND_closebook, 0, 0, -1, 0, 0);
    MapBookOpen = 0;

    GUIWindow::Release();
}

GUIWindow_Book::GUIWindow_Book()
    : GUIWindow(WINDOW_Book, 0, 0, window->GetWidth(), window->GetHeight(), 0) {}

void GUIWindow_Book::BasicBookInitialization() {
    pAudioPlayer->StopChannels(-1, -1);
    InitializeFonts();
    CreateButton(475, 445, 158, 34, 1, 0, UIMSG_Escape, 0, 0,
                 localization->GetString(79));  // Close
    current_screen_type = CURRENT_SCREEN::SCREEN_BOOKS;
    full_num_items_in_book = 0;
    books_primary_item_per_page = 0;
    books_page_number = 0;
    num_achieved_awards = 0;
}

//----- (00411AAA) --------------------------------------------------------
void GUIWindow_Book::InitializeFonts() {
    pAudioPlayer->StopChannels(-1, -1);
    pAudioPlayer->PlaySound(SOUND_openbook, 0, 0, -1, 0, 0);

    ui_book_map_frame = assets->GetImage_Alpha("mapbordr");

    pBookFont = GUIFont::LoadFont("book.fnt", "FONTPAL", NULL);
    pBook2Font = GUIFont::LoadFont("book2.fnt", "FONTPAL", NULL);
    pAutonoteFont = GUIFont::LoadFont("autonote.fnt", "FONTPAL", NULL);
    pSpellFont = GUIFont::LoadFont("spell.fnt", "FONTPAL", NULL);
}
