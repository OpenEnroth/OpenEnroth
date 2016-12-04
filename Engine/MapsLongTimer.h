#pragma once


/*  169 */
#pragma pack(push, 1)
struct MapsLongTimer
{
  __int64 NextStartTime;      // timer will either fire event at this time (type 2, field_C == 0)
  __int16 timer_evt_ID;
  __int16 timer_evt_seq_num;
  __int16 time_left_to_fire;
  __int16 IntervalHalfMins;       // or fire on these intervals (type 1)
  __int16 YearsInterval;
  __int16 MonthsInterval;
  __int16 WeeksInterval;
  __int16 HoursInterval;
  __int16 MinutesInterval;
  __int16 SecondsInterval;
  __int16 timer_evt_type;
  __int16 field_1E;
};
#pragma pack(pop)
extern MapsLongTimer MapsLongTimersList[100]; //array_5B5928

