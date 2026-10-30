#include <cstdlib>
#include <memory>

#include "Engine/Localization.h"
#include "Engine/AssetsManager.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Timer.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"
#include "GUI/UI/UIBooks.h"

#include "Media/Audio/AudioPlayer.h"

GUIWindow_Book::~GUIWindow_Book() {
    pAudioPlayer->playUISound(SOUND_closebook);

    pChildBooksOverlay = nullptr;
}

GUIWindow_Book::GUIWindow_Book() : GUIWindow(WINDOW_BOOK, {0, 0}, render->GetRenderDimensions()) {
    initializeFonts();
    CreateButton({475, 445}, {158, 34}, BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0, INPUT_ACTION_INVALID, localization->str(LSTR_EXIT_DIALOGUE));
    current_screen_type = SCREEN_BOOKS;
    gameTimer->setPaused(true);
}

void GUIWindow_Book::initializeFonts() {
    pAudioPlayer->playUISound(SOUND_openbook);

    ui_book_map_frame = assets->getIcon("mapbordr");

    if (!assets->pFontBookCalendar)
        assets->pFontBookCalendar = GUIFont::LoadFont("book.fnt");
    if (!assets->pFontBookTitle)
        assets->pFontBookTitle = GUIFont::LoadFont("book2.fnt");
    if (!assets->pFontBookOnlyShadow)
        assets->pFontBookOnlyShadow = GUIFont::LoadFont("autonote.fnt");
    if (!assets->pFontBookLloyds)
        assets->pFontBookLloyds = GUIFont::LoadFont("spell.fnt");
}

void GUIWindow_Book::bookButtonClicked(BookButtonAction action) {
    _bookButtonClicked = BOOK_BUTTON_PRESSED_FRAMES;
    _bookButtonAction = action;
}

GUIWindow_BooksButtonOverlay::GUIWindow_BooksButtonOverlay(Pointi position, Sizei dimensions, GUIButton *button, std::string_view hint) :
    GUIWindow(WINDOW_BOOKS_BUTTON_OVERLAY, position, dimensions, hint),
    _button(button)
{}

void GUIWindow_BooksButtonOverlay::Update() {
    render->DrawQuad2D(_button->vTextures[0], frameRect.topLeft());
}
