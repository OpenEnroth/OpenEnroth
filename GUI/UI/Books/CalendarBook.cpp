#include "GUI/UI/Books/CalendarBook.h"

#include "Engine/Engine.h"
#include "Engine/LOD.h"
#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/Time.h"

#include "Engine/Graphics/IRender.h"
#include "Engine/Graphics/Viewport.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"

#include "Media/Audio/AudioPlayer.h"

Image *ui_book_calendar_background = nullptr;

Image *ui_book_calendar_moon_new = nullptr;
Image *ui_book_calendar_moon_4 = nullptr;
Image *ui_book_calendar_moon_2 = nullptr;
Image *ui_book_calendar_moon_2_2 = nullptr;
Image *ui_book_calendar_moon_full = nullptr;

GUIWindow_CalendarBook::GUIWindow_CalendarBook() : GUIWindow_Book() {
    this->ptr_1C =
        (void *)WINDOW_CalendarBook;  // inherited from GUIWindow::GUIWindow
    BasicBookInitialization();

    // --------------------------------
    // 004304E7 Game_EventLoop --- part
    pEventTimer->Pause();
    pAudioPlayer->StopChannels(-1, -1);
    pBooksButtonOverlay =
        new GUIWindow_BooksButtonOverlay(570, 354, 0, 0, pBtn_Calendar);

    // ----------------------------------------------
    // 00411BFC GUIWindow::InitializeBookView -- part
    ui_book_calendar_background =
        assets->GetImage_ColorKey("sbdate-time", 0x7FF);
    ui_book_calendar_moon_new = assets->GetImage_ColorKey("moon_new", 0x7FF);
    ui_book_calendar_moon_4 = assets->GetImage_ColorKey("moon_4", 0x7FF);
    ui_book_calendar_moon_2 = assets->GetImage_ColorKey("moon_2", 0x7FF);
    ui_book_calendar_moon_2_2 = assets->GetImage_ColorKey("moon_2", 0x7FF);
    ui_book_calendar_moon_full = assets->GetImage_ColorKey("moon_ful", 0x7FF);
}

//----- (00413D3C) --------------------------------------------------------
static const char *GetDayPart() {
    if (pParty->uCurrentHour > 5 && pParty->uCurrentHour < 20)
        return localization->GetString(56);  // "Day"
    else if (pParty->uCurrentHour == 5)
        return localization->GetString(55);  // "Dawn"
    else if (pParty->uCurrentHour == 20)
        return localization->GetString(566);  // "Dusk"
    else
        return localization->GetString(567);  // "Night"
}

void GUIWindow_CalendarBook::Update() {
    render->DrawTextureAlphaNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);

    int am;  // ecx@5
    GUIWindow calendar_window;  // [sp+Ch] [bp-60h]@5
    unsigned int pMapID;  // [sp+60h] [bp-Ch]@1
    unsigned int pHour;

    static unsigned int pDayMoonPhase[28] = {  // 4E1B18
        0, 0, 0,
        1, 1, 1, 1,
        2, 2, 2,
        3, 3, 3, 3,
        4, 4, 4,
        3, 3, 3, 3,
        2, 2, 2,
        1, 1, 1, 1
    };

    render->DrawTextureAlphaNew(pViewport->uViewportTL_X / 640.0f, pViewport->uViewportTL_Y / 480.0f, ui_book_calendar_background);
    pHour = pParty->uCurrentHour;
    if ((int)pHour >= 12) {
        pHour -= 12;
        if (!pHour)
            pHour = 12;
        am = 1;
    } else {
        am = 0;
    }

    calendar_window.uFrameX = game_viewport_x;
    calendar_window.uFrameY = game_viewport_y;
    calendar_window.uFrameWidth = game_viewport_width;
    calendar_window.uFrameHeight = game_viewport_height;
    calendar_window.uFrameZ = game_viewport_z;
    calendar_window.uFrameW = game_viewport_w;
    calendar_window.DrawTitleText(pBook2Font, 0, 22, ui_book_calendar_title_color, localization->GetString(186), 3);  // "Time in Erathia"

    auto str = StringPrintf(
        "%s\t100:\t110%d:%02d %s - %s",
        localization->GetString(526),  // "Time"
        pHour, pParty->uCurrentMinute, localization->GetAmPm(am), GetDayPart());
    calendar_window.DrawText(pBookFont, 70, 55, ui_book_calendar_time_color, str, 0, 0, 0);

    str = StringPrintf(
        "%s\t100:\t110%d - %s",
        localization->GetString(56),  // "Day"
        pParty->uCurrentDayOfMonth + 1,
        localization->GetDayName(pParty->uCurrentDayOfMonth % 7));
    calendar_window.DrawText(pBookFont, 70, 2 * pBookFont->GetHeight() + 49, ui_book_calendar_day_color, str, 0, 0, 0);

    str = StringPrintf(
        "%s\t100:\t110%d - %s",
        localization->GetString(146),  // "Month"
        pParty->uCurrentMonth + 1,
        localization->GetMonthName(pParty->uCurrentMonth));
    calendar_window.DrawText(pBookFont, 70, 4 * pBookFont->GetHeight() + 43, ui_book_calendar_month_color, str, 0, 0, 0);

    str = StringPrintf("%s\t100:\t110%d", localization->GetString(245), pParty->uCurrentYear);  // "Year"
    calendar_window.DrawText(pBookFont, 70, 6 * pBookFont->GetHeight() + 37, ui_book_calendar_year_color, str, 0, 0, 0);

    str = StringPrintf(
        "%s\t100:\t110%s",
        localization->GetString(530),  // Moon
        localization->GetMoonPhaseName(pDayMoonPhase[pParty->uCurrentDayOfMonth]));
    calendar_window.DrawText(pBookFont, 70, 8 * (unsigned char)pBookFont->GetHeight() + 31, ui_book_calendar_moon_color, str, 0, 0, 0);

    pMapID = pMapStats->GetMapInfo(pCurrentMapName);
    String pMapName;
    if (pMapID)
        pMapName = pMapStats->pInfos[pMapID].pName;
    else
        pMapName = "Unknown";

    str = StringPrintf("%s\t100:\t110%s", localization->GetString(531), pMapName.c_str());  // "Location"
    calendar_window.DrawText(pBookFont, 70, 10 * (unsigned char)pBookFont->GetHeight() + 25, ui_book_calendar_location_color, str, 0, 0, 0);
}
