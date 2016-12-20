#include "Engine/Engine.h"
#include "Engine/Localization.h"
#include "Engine/LOD.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"

#include "GUI/GUIFont.h"
#include "GUI/UI/Books/CalendarBook.h"

#include "Media/Audio/AudioPlayer.h"



Image *ui_book_calendar_background = nullptr;

Image *ui_book_calendar_moon_new = nullptr;
Image *ui_book_calendar_moon_4 = nullptr;
Image *ui_book_calendar_moon_2 = nullptr;
Image *ui_book_calendar_moon_2_2 = nullptr;
Image *ui_book_calendar_moon_full = nullptr;

GUIWindow_CalendarBook::GUIWindow_CalendarBook() :
    GUIWindow_Book()
{
    this->ptr_1C = (void *)WINDOW_CalendarBook; // inherited from GUIWindow::GUIWindow
    BasicBookInitialization();

// --------------------------------
// 004304E7 Game_EventLoop --- part
    pEventTimer->Pause();
    pAudioPlayer->StopChannels(-1, -1);
    pBooksButtonOverlay = new GUIWindow_BooksButtonOverlay(570, 354, 0, 0, (int)pBtn_Calendar, 0);

// ----------------------------------------------
// 00411BFC GUIWindow::InitializeBookView -- part
    ui_book_calendar_background = assets->GetImage_16BitColorKey("sbdate-time", 0x7FF);
    ui_book_calendar_moon_new = assets->GetImage_16BitColorKey("moon_new", 0x7FF);
    ui_book_calendar_moon_4 = assets->GetImage_16BitColorKey("moon_4", 0x7FF);
    ui_book_calendar_moon_2 = assets->GetImage_16BitColorKey("moon_2", 0x7FF);
    ui_book_calendar_moon_2_2 = assets->GetImage_16BitColorKey("moon_2", 0x7FF);
    ui_book_calendar_moon_full = assets->GetImage_16BitColorKey("moon_ful", 0x7FF);
}



//----- (00413D3C) --------------------------------------------------------
static const char *GetDayPart()
{
    if (pParty->uCurrentHour > 5 && pParty->uCurrentHour < 20)
        return localization->GetString(56); // "Day"
    else if (pParty->uCurrentHour == 5)
        return localization->GetString(55); // "Dawn"
    else if (pParty->uCurrentHour == 20)
        return localization->GetString(566); // "Dusk"
    else
        return localization->GetString(567); // "Night"
}



void GUIWindow_CalendarBook::Update()
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
//     BookUI_Calendar_Draw();

// ----- (00413D6F) --------------------------------------------------------
// void BookUI_Calendar_Draw()
// {
    int am; // ecx@5
    char *pMapName; // eax@6
    GUIWindow calendar_window; // [sp+Ch] [bp-60h]@5
    unsigned int pMapID; // [sp+60h] [bp-Ch]@1
    unsigned int pHour;

    static unsigned int pDayMoonPhase[28] = // 4E1B18
    {
        0, 0, 0,
        1, 1, 1, 1,
        2, 2, 2,
        3, 3, 3, 3,
        4, 4, 4,
        3, 3, 3, 3,
        2, 2, 2,
        1, 1, 1, 1
    };

    render->DrawTextureAlphaNew(pViewport->uViewportTL_X/640.0f, pViewport->uViewportTL_Y/480.0f, ui_book_calendar_background);
    pHour = pParty->uCurrentHour;
    if ((signed int)pHour >= 12)
    {
        pHour -= 12;
        if (!pHour)
            pHour = 12;
        am = 1;
    }
    else
        am = 0;

    calendar_window.uFrameX = game_viewport_x;
    calendar_window.uFrameY = game_viewport_y;
    calendar_window.uFrameWidth = game_viewport_width;
    calendar_window.uFrameHeight = game_viewport_height;
    calendar_window.uFrameZ = game_viewport_z;
    calendar_window.uFrameW = game_viewport_w;
    calendar_window.DrawTitleText(pBook2Font, 0, 22, ui_book_calendar_title_color, localization->GetString(186), 3); // "Time in Erathia"

    auto str = StringPrintf(
        "%s\t100:\t110%d:%02d %s - %s",
        localization->GetString(526), // "Time"
        pHour, pParty->uCurrentMinute, localization->GetAmPm(am), GetDayPart()
    );
    calendar_window.DrawText(pBookFont, 70, 55, ui_book_calendar_time_color, str, 0, 0, 0);

    str = StringPrintf(
        "%s\t100:\t110%d - %s",
        localization->GetString(56), // "Day"
        pParty->uCurrentDayOfMonth + 1,
        localization->GetDayName(pParty->uCurrentDayOfMonth % 7)
    );
    calendar_window.DrawText(pBookFont, 70, 2 * LOBYTE(pBookFont->uFontHeight) + 49, ui_book_calendar_day_color, str, 0, 0, 0);

    str = StringPrintf(
        "%s\t100:\t110%d - %s",
        localization->GetString(146), // "Month"
        pParty->uCurrentMonth + 1,
        localization->GetMonthName(pParty->uCurrentMonth)
    );
    calendar_window.DrawText(pBookFont, 70, 4 * LOBYTE(pBookFont->uFontHeight) + 43, ui_book_calendar_month_color, str, 0, 0, 0);

    str = StringPrintf("%s\t100:\t110%d", localization->GetString(245), pParty->uCurrentYear); // "Year"
    calendar_window.DrawText(pBookFont, 70, 6 * LOBYTE(pBookFont->uFontHeight) + 37, ui_book_calendar_year_color, str, 0, 0, 0);

    str = StringPrintf(
        "%s\t100:\t110%s",
        localization->GetString(530), // Moon
        localization->GetMoonPhaseName(pDayMoonPhase[pParty->uCurrentDayOfMonth])
     );
    calendar_window.DrawText(pBookFont, 70, 8 * LOBYTE(pBookFont->uFontHeight) + 31, ui_book_calendar_moon_color, str, 0, 0, 0);

    pMapID = pMapStats->GetMapInfo(pCurrentMapName);
    if (pMapID)
        pMapName = pMapStats->pInfos[pMapID].pName;
    else
        pMapName = "Unknown";

    str = StringPrintf("%s\t100:\t110%s", localization->GetString(531), pMapName); // "Location"
    calendar_window.DrawText(pBookFont, 70, 10 * LOBYTE(pBookFont->uFontHeight) + 25, ui_book_calendar_location_color, str, 0, 0, 0);
}