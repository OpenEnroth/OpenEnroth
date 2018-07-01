#pragma once
#include "Engine/Time.h"

/*  169 */
#pragma pack(push, 1)
struct MapsLongTimer {
    GameTime NextStartTime;  // timer will either fire event at this time (type
                             // 2, field_C == 0)
    int16_t timer_evt_ID;
    int16_t timer_evt_seq_num;
    int16_t time_left_to_fire;
    int16_t IntervalHalfMins;  // or fire on these intervals (type 1)
    int16_t YearsInterval;
    int16_t MonthsInterval;
    int16_t WeeksInterval;
    int16_t HoursInterval;
    int16_t MinutesInterval;
    int16_t SecondsInterval;
    int16_t timer_evt_type;
    int16_t field_1E;
};
#pragma pack(pop)
extern MapsLongTimer MapsLongTimersList[100];  // 5B5928
