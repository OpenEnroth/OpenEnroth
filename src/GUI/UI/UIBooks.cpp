#include <cstdlib>
#include <memory>

#include "Engine/Localization.h"
#include "Engine/AssetsManager.h"
#include "Engine/GameType.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Timer.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIButton.h"
#include "GUI/UI/UIBooks.h"

#include "Media/Audio/AudioPlayer.h"

namespace {

// MM8's book tab icons ("ira-N_01", icons.lod, frame 01 of an idle animation) at (556, y), y in {24,123,222,321}:
// medallion = Maps, dagger = Quests, hourglass = History, quill = Autonotes.
std::shared_ptr<GraphicsImage> ui_book_tab_maps;
std::shared_ptr<GraphicsImage> ui_book_tab_quests;
std::shared_ptr<GraphicsImage> ui_book_tab_history;
std::shared_ptr<GraphicsImage> ui_book_tab_autonotes;

constexpr Pointi kMm8BookTabMapsPos = {556, 24};
constexpr Pointi kMm8BookTabQuestsPos = {556, 123};
constexpr Pointi kMm8BookTabHistoryPos = {556, 222};
constexpr Pointi kMm8BookTabAutonotesPos = {556, 321};

} // namespace

GUIWindow_Book::~GUIWindow_Book() {
    pAudioPlayer->playUISound(SOUND_closebook);

    pChildBooksOverlay = nullptr;
}

GUIWindow_Book::GUIWindow_Book() : GUIWindow(WINDOW_BOOK, {0, 0}, render->GetRenderDimensions()) {
    initializeFonts();
    CreateButton("Book_Close", {475, 445}, {158, 34}, BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0, INPUT_ACTION_INVALID, localization->str(LSTR_EXIT_DIALOGUE));
    current_screen_type = SCREEN_BOOKS;
    gameTimer->setPaused(true);
}

void GUIWindow_Book::initializeFonts() {
    pAudioPlayer->playUISound(SOUND_openbook);

    // MM8 ships no "mapbordr" asset (icons.lod/icons.UI.lod/EnglishD.lod/bitmaps.lod). The
    // one draw site (MapBook.cpp) is a no-op on the null texture (DrawQuad2D
    // null-checks its texture argument).
    if (gameType() != GAME_TYPE_MM8) {
        ui_book_map_frame = assets->getImage_Alpha("mapbordr");
    }

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
    // This overlay redraws the HUD tome icon over the book's own corner button
    // while a book screen is open. MM8 has no HUD tome row (its books are reached from the map
    // parchment's tab column, CreateMm8BookTabColumn/DrawMm8BookTabColumn below) -
    // drawing the MM7-positioned icon here would stamp it onto the parchment, so skip. The null
    // guard covers a game type that has no tome buttons at all.
    if (gameType() == GAME_TYPE_MM8 || !_button || _button->vTextures.empty())
        return;
    render->DrawQuad2D(_button->vTextures[0], frameRect.topLeft());
}

void GUIWindow_Book::CreateMm8BookTabColumn() {
    ui_book_tab_maps = assets->getImage_Alpha("ira-1_01");
    ui_book_tab_quests = assets->getImage_Alpha("ira-2_01");
    ui_book_tab_history = assets->getImage_Alpha("ira-3_01");
    ui_book_tab_autonotes = assets->getImage_Alpha("ira-4_01");

    CreateButton("Book_TabMaps", kMm8BookTabMapsPos, ui_book_tab_maps->size(), BUTTON_TYPE_NORMAL, 0,
        UIMSG_OpenMapBook, 0, INPUT_ACTION_OPEN_MAP, localization->str(LSTR_MAPS), {ui_book_tab_maps});
    CreateButton("Book_TabQuests", kMm8BookTabQuestsPos, ui_book_tab_quests->size(), BUTTON_TYPE_NORMAL, 0,
        UIMSG_OpenQuestBook, 0, INPUT_ACTION_OPEN_QUESTS, localization->str(LSTR_CURRENT_QUESTS), {ui_book_tab_quests});
    CreateButton("Book_TabHistory", kMm8BookTabHistoryPos, ui_book_tab_history->size(), BUTTON_TYPE_NORMAL, 0,
        UIMSG_OpenHistoryBook, 0, INPUT_ACTION_OPEN_HISTORY, localization->str(LSTR_HISTORY), {ui_book_tab_history});
    CreateButton("Book_TabAutonotes", kMm8BookTabAutonotesPos, ui_book_tab_autonotes->size(), BUTTON_TYPE_NORMAL, 0,
        UIMSG_OpenAutonotes, 0, INPUT_ACTION_OPEN_AUTONOTES, localization->str(LSTR_AUTO_NOTES), {ui_book_tab_autonotes});
}

void GUIWindow_Book::DrawMm8BookTabColumn() {
    render->DrawQuad2D(ui_book_tab_maps, kMm8BookTabMapsPos);
    render->DrawQuad2D(ui_book_tab_quests, kMm8BookTabQuestsPos);
    render->DrawQuad2D(ui_book_tab_history, kMm8BookTabHistoryPos);
    render->DrawQuad2D(ui_book_tab_autonotes, kMm8BookTabAutonotesPos);
}
