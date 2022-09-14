#pragma once

#include <array>

#include "Engine/Time.h"

/*  169 */
// Originally packed struct
struct MapsLongTimer {
    GameTime NextStartTime {};  // timer will either fire event at this time (type
                             // 2, field_C == 0)
    int16_t timer_evt_ID = 0;
    int16_t timer_evt_seq_num = 0;
    int16_t time_left_to_fire = 0;
    int16_t IntervalHalfMins = 0;  // or fire on these intervals (type 1)
    int16_t YearsInterval = 0;
    int16_t MonthsInterval = 0;
    int16_t WeeksInterval = 0;
    int16_t HoursInterval = 0;
    int16_t MinutesInterval = 0;
    int16_t SecondsInterval = 0;
    EventType timer_evt_type = EVENT_Invalid; // Originally int16_t
    int16_t field_1E = 0;
};
extern std::array<MapsLongTimer, 100> MapsLongTimersList;  // 5B5928
