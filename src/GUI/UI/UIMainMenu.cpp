#include "GUI/UI/UIMainMenu.h"

#include <memory>
#include <utility>

#include "Engine/EngineGlobals.h"
#include "Engine/GameType.h"
#include "Engine/Localization.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Image.h"
#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIButtonFrames.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"

#include "Io/Mouse.h"

GUIWindow_MainMenu::GUIWindow_MainMenu() :
    GUIWindow(WINDOW_MAIN_MENU, {0, 0}, render->GetRenderDimensions()) {
    // MM8 doesn't ship the main menu background/buttons under their MM7 names in icons.lod -
    // it ships them under different names in EnglishD.lod.
    GUIButtonFrameSet framesNew, framesLoad, framesCredits, framesExit;
    std::array<Pointi, 4> buttonPositions;
    if (gameType() == GAME_TYPE_MM8) {
        main_menu_background = assets->getImage_PCXFromEnglishDLOD("title.pcx");
        // title.pcx already bakes in the unpressed ("up") button art, so only
        // the hover ("ht") and pressed ("dn") frames are loaded for the main menu (see
        // GUIButtonFrameSpec's hasUp comment, GUIButtonFrames.h).
        framesNew = loadMM8ButtonFrameSet(MM8ButtonFamily::MainMenuNew);
        framesLoad = loadMM8ButtonFrameSet(MM8ButtonFamily::MainMenuLoad);
        framesCredits = loadMM8ButtonFrameSet(MM8ButtonFamily::MainMenuCredits);
        framesExit = loadMM8ButtonFrameSet(MM8ButtonFamily::MainMenuExit);
        // MM8's title.pcx bakes its buttons into the background art at these coordinates
        // (t_*_up matches title.pcx there), unlike MM7's {495, 172...}.
        buttonPositions = {{{515, 203}, {515, 241}, {515, 279}, {515, 317}}};
    } else {
        main_menu_background = assets->getImage_PCXFromIconsLOD("title.pcx");
        // MM7 ships one texture per button, shown only while hovered - no separate up/down art -
        // so it reuses this mechanism's "hover" slot rather than a whole separate draw path
        // (the draw-time state selection lives in exactly one place, GUIButtonFrameSet::select()).
        framesNew.hover = assets->getImage_ColorKey("title_new");
        framesLoad.hover = assets->getImage_ColorKey("title_load");
        framesCredits.hover = assets->getImage_ColorKey("title_cred");
        framesExit.hover = assets->getImage_ColorKey("title_exit");
        buttonPositions = {{{495, 172}, {495, 227}, {495, 282}, {495, 337}}};
    }

    pBtnNew = CreateButton("MainMenu_NewGame", buttonPositions[0], framesNew.any()->size(), BUTTON_TYPE_NORMAL, 0,
                           UIMSG_MainMenu_ShowPartyCreationWnd, 0, INPUT_ACTION_NEW_GAME, "",
                           {framesNew.down ? framesNew.down : framesNew.hover});
    pBtnLoad = CreateButton("MainMenu_LoadGame", buttonPositions[1], framesLoad.any()->size(), BUTTON_TYPE_NORMAL, 0,
                            UIMSG_MainMenu_ShowLoadWindow, 1, INPUT_ACTION_LOAD_GAME, "",
                            {framesLoad.down ? framesLoad.down : framesLoad.hover});
    pBtnCredits = CreateButton("MainMenu_Credits", buttonPositions[2], framesCredits.any()->size(), BUTTON_TYPE_NORMAL, 0,
                               UIMSG_ShowCredits, 2, INPUT_ACTION_SHOW_CREDITS, "",
                               {framesCredits.down ? framesCredits.down : framesCredits.hover});
    pBtnExit = CreateButton("MainMenu_ExitGame", buttonPositions[3], framesExit.any()->size(), BUTTON_TYPE_NORMAL, 0,
                            UIMSG_ExitToWindows, 3, INPUT_ACTION_EXIT_GAME, "",
                            {framesExit.down ? framesExit.down : framesExit.hover});

    // vTextures[0] above (the "down"/pressed frame, or MM7's single texture) is what
    // OnButtonClick's one-frame click flash draws (processMessage() below) - frames itself
    // additionally carries the hover frame for the persistent per-frame draw in Update().
    pBtnNew->frames = framesNew;
    pBtnLoad->frames = framesLoad;
    pBtnCredits->frames = framesCredits;
    pBtnExit->frames = framesExit;
}

GUIWindow_MainMenu::~GUIWindow_MainMenu() {
    for (GUIButton *button : vButtons)
        button->frames.release();
    assets->evict(std::move(main_menu_background));
}

void GUIWindow_MainMenu::Update() {
    if (main_menu_background)
        render->DrawQuad2D(main_menu_background, {0, 0});

    Pointi pt = mouse->position();
    for (GUIButton *pButton : vButtons) {
        GraphicsImage *pTexture = pButton->frames.select(pButton->Contains(pt.x, pt.y), false);
        if (pTexture)
            render->DrawQuad2D(pTexture, pButton->rect.topLeft());
    }
}

void GUIWindow_MainMenu::processMessage(UIMessageType message) {
    // Play the sound and change visual connected to the related button
    switch (message) {
    case UIMSG_MainMenu_ShowPartyCreationWnd:
        new OnButtonClick(pBtnNew->rect.topLeft(), { 0, 0 }, pBtnNew);
        break;
    case UIMSG_MainMenu_ShowLoadWindow:
        new OnButtonClick(pBtnLoad->rect.topLeft(), { 0, 0 }, pBtnLoad);
        break;
    case UIMSG_ShowCredits:
        new OnButtonClick(pBtnCredits->rect.topLeft(), { 0, 0 }, pBtnCredits);
        break;
    case UIMSG_ExitToWindows:
        new OnButtonClick(pBtnExit->rect.topLeft(), { 0, 0 }, pBtnExit);
        break;
    default:
        break;
    }
}
