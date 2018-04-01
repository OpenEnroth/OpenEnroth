#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Localization.h"
#include "Engine/LOD.h"
#include "Engine/Party.h"
#include "Engine/Graphics/IRender.h"

#include "GUI/GUIFont.h"
#include "GUI/UI/Books/TownPortalBook.h"

#include "IO/Mouse.h"

static int pTownPortalBook_xs[6] = { 260, 324, 147, 385, 390,  19 };
static int pTownPortalBook_ys[6] = { 206,  84, 182, 239,  17, 283 };
static int pTownPortalBook_ws[6] = {  80,  66,  68,  72,  67,  74 };
static int pTownPortalBook_hs[6] = {  55,  56,  65,  67,  67,  59 };

static std::array<Image *, 6> ui_book_townportal_icons; // [0]Harmonale, [1]Pierpont, [2]Nighon, [3]Evenmorn Island, [4]Celestia, [5]The Pit

Image *ui_book_townportal_background = nullptr;


GUIWindow_TownPortalBook::GUIWindow_TownPortalBook(const char *a1) :
    GUIWindow_Book()
{
    this->sHint = a1; // inherited from GUIWindow::GUIWindow
    this->ptr_1C = (void *)WINDOW_TownPortal; // inherited from GUIWindow::GUIWindow
    BasicBookInitialization();

// ----------------------------------------------
// 00411BFC GUIWindow::InitializeBookView -- part
    ui_book_townportal_background = assets->GetImage_16Bit("townport");

    ui_book_townportal_icons[0] = assets->GetImage_16BitColorKey("tpharmndy", 0x7FF);
    ui_book_townportal_icons[1] = assets->GetImage_16BitColorKey("tpelf", 0x7FF);
    ui_book_townportal_icons[2] = assets->GetImage_16BitColorKey("tpwarlock", 0x7FF);
    ui_book_townportal_icons[3] = assets->GetImage_16BitColorKey("tpisland", 0x7FF);
    ui_book_townportal_icons[4] = assets->GetImage_16BitColorKey("tpheaven", 0x7FF);
    ui_book_townportal_icons[5] = assets->GetImage_16BitColorKey("tphell", 0x7FF);

    for (uint i = 0; i < 6; ++i)
        CreateButton(
            pTownPortalBook_xs[i],
            pTownPortalBook_ys[i],
            pTownPortalBook_ws[i],
            pTownPortalBook_hs[i],
            1, 182, UIMSG_ClickTownInTP, i, 0, "", nullptr
        );
}


void GUIWindow_TownPortalBook::Update()
{
// -----------------------------------
// 004156F0 GUI_UpdateWindows --- part
// {
//     BookUI_Draw((WindowType)(int)ptr_1C);
// }

// ----- (00413CC6) --------------------------------------------------------
// void BookUI_Draw(WindowType book) --- part
// {
    render->DrawTextureAlphaNew(471/640.0f, 445/480.0f, ui_exit_cancel_button_background);
//     BookUI_DrawTownPortalMap();

// ----- (00411150) --------------------------------------------------------
// void BookUI_DrawTownPortalMap()
// {
    int v3; // edi@17
    GUIWindow TownPortalWindow; // [sp+Ch] [bp-64h]@1

    render->ClearZBuffer(0, 479);
    render->DrawTextureNew(8/640.0f, 8/480.0f, ui_book_townportal_background);
    render->DrawTextureAlphaNew(471/640.0f, 445/480.0f, ui_exit_cancel_button_background);

    TownPortalWindow.uFrameX = game_viewport_x;
    TownPortalWindow.uFrameY = game_viewport_y;
    TownPortalWindow.uFrameWidth = game_viewport_width;
    TownPortalWindow.uFrameHeight = game_viewport_height;
    TownPortalWindow.uFrameZ = game_viewport_z;
    TownPortalWindow.uFrameW = game_viewport_w;

    const uint fountain_bits_lut[] =
    {
        PARTY_QUEST_FOUNTAIN_HARMONDALE,
        PARTY_QUEST_FOUNTAIN_PIERPONT,
        PARTY_QUEST_FOUNTAIN_NIGHON,
        PARTY_QUEST_FOUNTAIN_EVENMORN_ISLE,
        PARTY_QUEST_FOUNTAIN_CELESTIA,
        PARTY_QUEST_FOUNTAIN_THE_PIT
    };
    for (uint i = 0; i < 6; ++i)
    {

        if (_449B57_test_bit(pParty->_quest_bits, fountain_bits_lut[i]))
            render->ZDrawTextureAlpha(
                pTownPortalBook_xs[i]/640.0f,
                pTownPortalBook_ys[i]/480.0f,
                ui_book_townportal_icons[i], i + 1
            );
    }

    Point pt = pMouse->GetCursorPos();
    v3 = render->pActiveZBuffer[pt.x + pSRZBufferLineOffsets[pt.y]] & 0xFFFF;

    if (v3)
    {
        if (_449B57_test_bit(pParty->_quest_bits, fountain_bits_lut[v3 - 1]))
            render->DrawTextureAlphaNew(pTownPortalBook_xs[v3 - 1]/640.0f, pTownPortalBook_ys[v3 - 1]/480.0f, ui_book_townportal_icons[v3 - 1]);
    }
    TownPortalWindow.DrawTitleText(pBook2Font, 0, 22, 0, localization->GetString(10), 3);
}