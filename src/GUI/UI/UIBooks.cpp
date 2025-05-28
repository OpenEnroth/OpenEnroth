#include <cstdlib>

#include "Engine/Localization.h"
#include "Engine/AssetsManager.h"

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"
#include "GUI/UI/UIBooks.h"

#include "Media/Audio/AudioPlayer.h"

void GUIWindow_Book::Release() {
    if (ui_book_map_frame) {
        ui_book_map_frame->Release();
    }
    if (ui_book_quest_div_bar) {
        ui_book_quest_div_bar->Release();
    }
    if (ui_book_button8_off) {
        ui_book_button8_off->Release();
    }
    if (ui_book_button8_on) {
        ui_book_button8_on->Release();
    }
    if (ui_book_button7_off) {
        ui_book_button7_off->Release();
    }
    if (ui_book_button7_on) {
        ui_book_button7_on->Release();
    }
    if (ui_book_button6_off) {
        ui_book_button6_off->Release();
    }
    if (ui_book_button6_on) {
        ui_book_button6_on->Release();
    }
    if (ui_book_button5_off) {
        ui_book_button5_off->Release();
    }
    if (ui_book_button5_on) {
        ui_book_button5_on->Release();
    }
    if (ui_book_button4_off) {
        ui_book_button4_off->Release();
    }
    if (ui_book_button4_on) {
        ui_book_button4_on->Release();
    }
    if (ui_book_button3_off) {
        ui_book_button3_off->Release();
    }
    if (ui_book_button3_on) {
        ui_book_button3_on->Release();
    }
    if (ui_book_button2_off) {
        ui_book_button2_off->Release();
    }
    if (ui_book_button2_on) {
        ui_book_button2_on->Release();
    }
    if (ui_book_button1_off) {
        ui_book_button1_off->Release();
    }
    if (ui_book_button1_on) {
        ui_book_button1_on->Release();
    }

    pAudioPlayer->playUISound(SOUND_closebook);

    if (pChildBooksOverlay) {
        pChildBooksOverlay->Release();
    }

    GUIWindow::Release();
}

GUIWindow_Book::GUIWindow_Book() : GUIWindow(WINDOW_Book, {0, 0}, render->GetRenderDimensions()) {
    initializeFonts();
    CreateButton({475, 445}, {158, 34}, 1, 0, UIMSG_Escape, 0, Io::InputAction::Invalid, localization->GetString(LSTR_EXIT_DIALOGUE));
    current_screen_type = SCREEN_BOOKS;
    pEventTimer->setPaused(true);
}

void GUIWindow_Book::initializeFonts() {
    pAudioPlayer->playUISound(SOUND_openbook);

    ui_book_map_frame = assets->getImage_Alpha("mapbordr");

    if (!assets->pFontBookCalendar)
        assets->pFontBookCalendar = GUIFont::LoadFont("book.fnt", "FONTPAL");
    if (!assets->pFontBookTitle)
        assets->pFontBookTitle = GUIFont::LoadFont("book2.fnt", "FONTPAL");
    if (!assets->pFontBookOnlyShadow)
        assets->pFontBookOnlyShadow = GUIFont::LoadFont("autonote.fnt", "FONTPAL");
    if (!assets->pFontBookLloyds)
        assets->pFontBookLloyds = GUIFont::LoadFont("spell.fnt", "FONTPAL");
}

void GUIWindow_Book::bookButtonClicked(BookButtonAction action) {
    _bookButtonClicked = true;
    _bookButtonAction = action;
}

GUIWindow_BooksButtonOverlay::GUIWindow_BooksButtonOverlay(Pointi position, Sizei dimensions, GUIButton *button, std::string_view hint) :
    GUIWindow(WINDOW_BooksButtonOverlay, position, dimensions, hint),
    _button(button)
{}

void GUIWindow_BooksButtonOverlay::Update() {
    render->DrawTextureNew(uFrameY / 640.0f, uFrameX / 480.0f, _button->vTextures[0]);
}
