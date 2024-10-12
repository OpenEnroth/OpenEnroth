#include "GUI/UI/Books/CalendarBook.h"

#include <string>

#include "Engine/Localization.h"
#include "Engine/Party.h"
#include "Engine/AssetsManager.h"
#include "Engine/MapInfo.h"
#include "Engine/Engine.h"
#include "Engine/mm7_data.h"

#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Graphics/Viewport.h"

#include "GUI/GUIButton.h"

GraphicsImage *ui_book_calendar_background = nullptr;

GraphicsImage *ui_book_calendar_moon_new = nullptr;
GraphicsImage *ui_book_calendar_moon_4 = nullptr;
GraphicsImage *ui_book_calendar_moon_2 = nullptr;
GraphicsImage *ui_book_calendar_moon_2_2 = nullptr;
GraphicsImage *ui_book_calendar_moon_full = nullptr;

// 4E1B18
static std::array<int, 28> pDayMoonPhase = {
        0, 0, 0,
        1, 1, 1, 1,
        2, 2, 2,
        3, 3, 3, 3,
        4, 4, 4,
        3, 3, 3, 3,
        2, 2, 2,
        1, 1, 1, 1
};


GUIWindow_CalendarBook::GUIWindow_CalendarBook() : GUIWindow_Book() {
    this->eWindowType = WindowType::WINDOW_CalendarBook;

    pChildBooksOverlay = new GUIWindow_BooksButtonOverlay({570, 354}, {0, 0}, pBtn_Calendar);

    ui_book_calendar_background = assets->getImage_ColorKey("sbdate-time");
    ui_book_calendar_moon_new = assets->getImage_ColorKey("moon_new");
    ui_book_calendar_moon_4 = assets->getImage_ColorKey("moon_4");
    ui_book_calendar_moon_2 = assets->getImage_ColorKey("moon_2");
    ui_book_calendar_moon_2_2 = assets->getImage_ColorKey("moon_2");
    ui_book_calendar_moon_full = assets->getImage_ColorKey("moon_ful");
}

/**
 * @offset 0x413D3C
 */
static std::string getDayPart(int hour) {
    assert(hour >= 0 && hour < 24);

    if (hour > 5 && hour < 20) {
        return localization->GetString(LSTR_DAY_CAPITALIZED);
    } else if (hour == 5) {
        return localization->GetString(LSTR_DAWN);
    } else if (hour == 20) {
        return localization->GetString(LSTR_DUSK);
    } else {
        return localization->GetString(LSTR_NIGHT);
    }
}

void GUIWindow_CalendarBook::Update() {
    render->DrawTextureNew(471 / 640.0f, 445 / 480.0f, ui_exit_cancel_button_background);

    GUIWindow calendar_window;

    render->DrawTextureNew(pViewport->uViewportTL_X / 640.0f, pViewport->uViewportTL_Y / 480.0f, ui_book_calendar_background);
    CivilTime time = pParty->GetPlayingTime().toCivilTime();

    calendar_window.uFrameX = game_viewport_x;
    calendar_window.uFrameY = game_viewport_y;
    calendar_window.uFrameWidth = game_viewport_width;
    calendar_window.uFrameHeight = game_viewport_height;
    calendar_window.uFrameZ = game_viewport_z;
    calendar_window.uFrameW = game_viewport_w;
    calendar_window.DrawTitleText(assets->pFontBookTitle.get(), 0, 22, ui_book_calendar_title_color, localization->GetString(LSTR_TIME_IN_ERATHIA), 3);

    std::string str = fmt::format("{}\t100:\t110{}:{:02} {} - {}", localization->GetString(LSTR_TIME), time.hourAmPm,
                                  time.minute, localization->GetAmPm(time.isPm), getDayPart(time.hour));
    calendar_window.DrawText(assets->pFontBookCalendar.get(), {70, 55}, ui_book_calendar_time_color, str);

    str = fmt::format("{}\t100:\t110{} - {}", localization->GetString(LSTR_DAY_CAPITALIZED), time.day,
                      localization->GetDayName(time.dayOfWeek - 1));
    calendar_window.DrawText(assets->pFontBookCalendar.get(), {70, 2 * assets->pFontBookCalendar->GetHeight() + 49}, ui_book_calendar_day_color, str);

    str = fmt::format("{}\t100:\t110{} - {}", localization->GetString(LSTR_MONTH), time.month,
                      localization->GetMonthName(time.month - 1));
    calendar_window.DrawText(assets->pFontBookCalendar.get(), {70, 4 * assets->pFontBookCalendar->GetHeight() + 43}, ui_book_calendar_month_color, str);

    str = fmt::format("{}\t100:\t110{}", localization->GetString(LSTR_YEAR), time.year);
    calendar_window.DrawText(assets->pFontBookCalendar.get(), {70, 6 * assets->pFontBookCalendar->GetHeight() + 37}, ui_book_calendar_year_color, str);

    str = fmt::format("{}\t100:\t110{}", localization->GetString(LSTR_MOON), localization->GetMoonPhaseName(pDayMoonPhase[time.day - 1]));
    calendar_window.DrawText(assets->pFontBookCalendar.get(), {70, 8 * assets->pFontBookCalendar->GetHeight() + 31}, ui_book_calendar_moon_color, str);

    std::string pMapName = "Unknown";
    if (engine->_currentLoadedMapId != MAP_INVALID) {
        pMapName = pMapStats->pInfos[engine->_currentLoadedMapId].name;
    }

    str = fmt::format("{}\t100:\t110{}", localization->GetString(LSTR_LOCATION), pMapName);
    calendar_window.DrawText(assets->pFontBookCalendar.get(), {70, 10 * assets->pFontBookCalendar->GetHeight() + 25}, ui_book_calendar_location_color, str);
}
