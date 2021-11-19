#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Graphics/IRender.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"

#include "GUI/GUIFont.h"
#include "GUI/UI/Books/TownPortalBook.h"

#include "Io/Mouse.h"

static int pTownPortalBook_xs[6] = {260, 324, 147, 385, 390, 19};
static int pTownPortalBook_ys[6] = {206, 84, 182, 239, 17, 283};
static int pTownPortalBook_ws[6] = {80, 66, 68, 72, 67, 74};
static int pTownPortalBook_hs[6] = {55, 56, 65, 67, 67, 59};

static std::array<Image *, 6>
    ui_book_townportal_icons;  // [0]Harmonale, [1]Pierpont, [2]Nighon,
                               // [3]Evenmorn Island, [4]Celestia, [5]The Pit

Image *ui_book_townportal_background = nullptr;

GUIWindow_TownPortalBook::GUIWindow_TownPortalBook()  // const char *a1)
    : GUIWindow_Book() {
    // this->sHint = a1;  // inherited from GUIWindow::GUIWindow
    this->ptr_1C =
        (void *)WINDOW_TownPortal;  // inherited from GUIWindow::GUIWindow
    BasicBookInitialization();

    // ----------------------------------------------
    // 00411BFC GUIWindow::InitializeBookView -- part
    ui_book_townportal_background = assets->GetImage_Solid("townport");

    ui_book_townportal_icons[0] = assets->GetImage_ColorKey("tpharmndy", 0x7FF);
    ui_book_townportal_icons[1] = assets->GetImage_ColorKey("tpelf", 0x7FF);
    ui_book_townportal_icons[2] = assets->GetImage_ColorKey("tpwarlock", 0x7FF);
    ui_book_townportal_icons[3] = assets->GetImage_ColorKey("tpisland", 0x7FF);
    ui_book_townportal_icons[4] = assets->GetImage_ColorKey("tpheaven", 0x7FF);
    ui_book_townportal_icons[5] = assets->GetImage_ColorKey("tphell", 0x7FF);

    for (uint i = 0; i < 6; ++i)
        CreateButton(pTownPortalBook_xs[i], pTownPortalBook_ys[i],
                     pTownPortalBook_ws[i], pTownPortalBook_hs[i], 1, 182,
                     UIMSG_ClickTownInTP, i);
}

void GUIWindow_TownPortalBook::Update() {
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
    //     BookUI_DrawTownPortalMap();

    // ----- (00411150) --------------------------------------------------------
    // void BookUI_DrawTownPortalMap()
    // {
    int v3;                      // edi@17
    GUIWindow TownPortalWindow;  // [sp+Ch] [bp-64h]@1

    render->ClearZBuffer(0, 479);
    render->DrawTextureNew(8 / 640.0f, 8 / 480.0f,
                           ui_book_townportal_background);
    render->DrawTextureAlphaNew(471 / 640.0f, 445 / 480.0f,
                                ui_exit_cancel_button_background);

    TownPortalWindow.uFrameX = game_viewport_x;
    TownPortalWindow.uFrameY = game_viewport_y;
    TownPortalWindow.uFrameWidth = game_viewport_width;
    TownPortalWindow.uFrameHeight = game_viewport_height;
    TownPortalWindow.uFrameZ = game_viewport_z;
    TownPortalWindow.uFrameW = game_viewport_w;

    const uint fountain_bits_lut[] = {
        QBIT_FOUNTAIN_IN_HARMONDALE_ACTIVATED, QBIT_FOUNTAIN_IN_PIERPONT_ACTIVATED,
        QBIT_FOUNTAIN_IN_MOUNT_NIGHON_ACTIVATED,     QBIT_FOUNTAIN_IN_EVENMORN_ISLE_ACTIVATED,
        QBIT_FOUNTAIN_IN_CELESTIA_ACTIVATED,   QBIT_FOUNTAIN_IN_THE_PIT_ACTIVATED};
    for (uint i = 0; i < 6; ++i) {
        if (_449B57_test_bit(pParty->_quest_bits, fountain_bits_lut[i]) || engine->config->debug_town_portal)
            render->ZDrawTextureAlpha(pTownPortalBook_xs[i] / 640.0f,
                                      pTownPortalBook_ys[i] / 480.0f,
                                      ui_book_townportal_icons[i], i + 1);
    }

    Point pt = mouse->GetCursorPos();
    v3 = render->pActiveZBuffer[pt.x + pSRZBufferLineOffsets[pt.y]] & 0xFFFF;

    if (v3) {
        if (_449B57_test_bit(pParty->_quest_bits, fountain_bits_lut[v3 - 1]) || engine->config->debug_town_portal)
            render->DrawTextureAlphaNew(pTownPortalBook_xs[v3 - 1] / 640.0f,
                                        pTownPortalBook_ys[v3 - 1] / 480.0f,
                                        ui_book_townportal_icons[v3 - 1]);
    }
    TownPortalWindow.DrawTitleText(
        pBook2Font, 0, 22, 0, localization->GetString(LSTR_TOWN_PORTAL), 3);
}
