#pragma once

#include <cstdint>

#define TIME_QUANT                  128
#define TIME_SECONDS_PER_QUANT      30
#define TIME_UNPACK_GAME_SECONDS    (uint64_t)TIME_SECONDS_PER_QUANT / (double)TIME_QUANT
// TIME_UNPACK_GAME_SECONDS = 0.234375
// 30 game seconds per one time quant (128ms)   [128 * 0.234375 = 30]
// seconds = game_time * TIME_UNPACK_GAME_SECONDS
#define TIME_PACK_GAME_SECONDS      (uint64_t)TIME_QUANT / (double)TIME_SECONDS_PER_QUANT
// game_time += seconds * TIME_PACK_GAME_SECONDS

struct GameTime {
    GameTime() : value(0) {}
    explicit GameTime(uint64_t val) : value(val) {}
    GameTime(int seconds, int minutes, int hours = 0, int days = 0,
             int weeks = 0, int months = 0, int years = 0) {
        auto converted = (seconds + (uint64_t)60 * minutes + (uint64_t)3600 * hours + (uint64_t)86400 * days +
            (uint64_t)604800 * weeks + (uint64_t)2419200 * months +
                          (uint64_t)29030400 * years) *
                         TIME_PACK_GAME_SECONDS;

        this->value = (uint64_t)converted;
    }

    uint64_t GetSeconds() const {
        return (uint64_t)(this->value * TIME_UNPACK_GAME_SECONDS);
    }
    uint64_t GetMinutes() const { return this->GetSeconds() / 60; }
    uint64_t GetHours() const { return this->GetMinutes() / 60; }
    int GetDays() const { return (int)(this->GetHours() / 24); }
    int GetWeeks() const { return this->GetDays() / 7; }
    int GetMonths() const { return this->GetWeeks() / 4; }
    int GetYears() const { return this->GetMonths() / 12; }

    int GetSecondsFraction() const { return this->GetSeconds() % 60; }
    int GetMinutesFraction() const { return (this->GetSeconds() / 60) % 60; }
    int GetHoursOfDay() const { return (this->GetSeconds() / 3600) % 24; }
    int GetDaysOfWeek() const { return this->GetDays() % 7; }
    int GetDaysOfMonth() const { return this->GetDays() % 28; }
    int GetWeeksOfMonth() const { return this->GetWeeks() % 4; }
    int GetMonthsOfYear() const { return this->GetMonths() % 12; }

    void AddMinutes(int minutes) {
        this->value += ((uint64_t)60 * minutes * TIME_PACK_GAME_SECONDS);
    }
    void SubtractHours(int hours) {
        this->value -= ((uint64_t)60 * 60 * hours * TIME_PACK_GAME_SECONDS);
    }
    void AddDays(int days) {
        this->value += ((uint64_t)60 * 60 * 24 * days * TIME_PACK_GAME_SECONDS);
    }
    void AddYears(int years) {
        this->value += ((uint64_t)60 * 60 * 24 * 7 * 4 * 12 * years * TIME_PACK_GAME_SECONDS);
    }

    void Reset() { this->value = 0; }
    bool Valid() const { return this->value > 0; }

    GameTime operator+(GameTime &rhs) {
        return GameTime(this->value + rhs.value);
    }
    GameTime operator-(GameTime &rhs) {
        return GameTime(this->value - rhs.value);
    }
    GameTime &operator+=(GameTime &rhs) {
        this->value += rhs.value;
        return *this;
    }

    bool operator>(const GameTime &rhs) const { return this->value > rhs.value; }
    bool operator>=(const GameTime &rhs) const { return this->value >= rhs.value; }
    bool operator<(const GameTime &rhs) const { return this->value < rhs.value; }
    bool operator<=(const GameTime &rhs) const { return this->value <= rhs.value; }

    explicit operator bool() {
        return this->Valid();
    }  // unsafe bool was casuing many problems

    operator int() { return static_cast<int>(this->value); }  // cast operator conversion require

    static GameTime FromSeconds(int seconds) {
        return GameTime(seconds, 0, 0, 0, 0, 0, 0);
    }
    static GameTime FromMinutes(int minutes) {
        return GameTime(0, minutes, 0, 0, 0, 0, 0);
    }
    static GameTime FromHours(int hours) {
        return GameTime(0, 0, hours, 0, 0, 0, 0);
    }
    static GameTime FromDays(int days) {
        return GameTime(0, 0, 0, days, 0, 0, 0);
    }
    static GameTime FromYears(int years) {
        return GameTime(0, 0, 0, 0, 0, 0, years);
    }

    int64_t value;
};

/*   61 */
#pragma pack(push, 1)
struct Timer {
    static Timer *Create() { return new Timer; }

    Timer() : bReady(false), bPaused(false) {
        bTackGameTime = 0;
        uStartTime = 0;
        uStopTime = 0;
        uGameTimeStart = 0;
        field_18 = 0;
        uTimeElapsed = 0;
        dt_in_some_format = 0;
        uTotalGameTimeElapsed = 0;
    }

    void Initialize();
    uint64_t Time();
    void Update();
    void Pause();
    void Resume();
    void TrackGameTime();
    void StopGameTime();

    unsigned int bReady;
    unsigned int bPaused;
    int bTackGameTime;
    unsigned int uStartTime;
    unsigned int uStopTime;
    int uGameTimeStart;
    int field_18;
    unsigned int uTimeElapsed;
    int dt_in_some_format;
    unsigned int uTotalGameTimeElapsed;

    static const unsigned int Minute = 2 * TIME_QUANT;
    static const unsigned int Hour = 60 * Minute;
    static const unsigned int Day = 24 * Hour;
    static const unsigned int Week = 7 * Day;
    static const unsigned int Month = 4 * Week;
    static const unsigned int Year = 12 * Month;
};
#pragma pack(pop)

extern Timer *pMiscTimer;
extern Timer *pEventTimer;
