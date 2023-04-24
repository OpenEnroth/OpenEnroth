#include <stdlib.h>

#include "Engine/Awards.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"

#include "Engine/Graphics/IRender.h"

#include "GUI/GUIFont.h"
#include "GUI/UI/UIBooks.h"

#include "Media/Audio/AudioPlayer.h"

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

bool bookButtonClicked;
BOOK_BUTTON_ACTION bookButtonAction;

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

    pAudioPlayer->playUISound(SOUND_closebook);

    if (pChildBooksOverlay) {
        pChildBooksOverlay->Release();
    }

    GUIWindow::Release();
}

GUIWindow_Book::GUIWindow_Book() : GUIWindow(WINDOW_Book, {0, 0}, render->GetRenderDimensions(), 0) {
    InitializeFonts();
    CreateButton({475, 445}, {158, 34}, 1, 0, UIMSG_Escape, 0, InputAction::Invalid, localization->GetString(LSTR_DIALOGUE_EXIT));
    current_screen_type = CURRENT_SCREEN::SCREEN_BOOKS;
    bookButtonClicked = false;
    pEventTimer->Pause();
}

//----- (00411AAA) --------------------------------------------------------
void GUIWindow_Book::InitializeFonts() {
    pAudioPlayer->playUISound(SOUND_openbook);

    ui_book_map_frame = assets->GetImage_Alpha("mapbordr");

    pBookFont = GUIFont::LoadFont("book.fnt", "FONTPAL");
    pBook2Font = GUIFont::LoadFont("book2.fnt", "FONTPAL");
    pAutonoteFont = GUIFont::LoadFont("autonote.fnt", "FONTPAL");
    pSpellFont = GUIFont::LoadFont("spell.fnt", "FONTPAL");
}
