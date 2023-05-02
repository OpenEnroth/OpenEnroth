#pragma once

#include <cstdint>

const int game_starting_year = 1168;

// Number of game ticks per 30 game seconds
#define TIME_QUANT                  128
#define TIME_SECONDS_PER_QUANT      30

#define GAME_TIME_TO_SECONDS(VALUE) ((VALUE) * static_cast<uint64_t>(TIME_SECONDS_PER_QUANT) / TIME_QUANT)
#define SECONDS_TO_GAME_TIME(VALUE) ((VALUE) * static_cast<uint64_t>(TIME_QUANT) / TIME_SECONDS_PER_QUANT)

struct GameTime {
    GameTime() : value(0) {}
    explicit GameTime(uint64_t val) : value(val) {}
    GameTime(int seconds, int minutes, int hours = 0, int days = 0, int weeks = 0, int months = 0, int years = 0) {
        this->value = SECONDS_TO_GAME_TIME(
            seconds +
            60ull * minutes +
            3600ull * hours +
            86400ull * days +
            604800ull * weeks +
            2419200ull * months +
            29030400ull * years);
    }

    uint64_t GetSeconds() const {
        return GAME_TIME_TO_SECONDS(this->value);
    }
    uint64_t GetMinutes() const { return this->GetSeconds() / 60; }
    uint64_t GetHours() const { return this->GetMinutes() / 60; }
    int GetDays() const { return (int)(this->GetHours() / 24); }
    int GetWeeks() const { return this->GetDays() / 7; }
    int GetMonths() const { return this->GetWeeks() / 4; }
    int GetYears() const { return this->GetMonths() / 12; }

    int GetSecondsFraction() const { return this->GetSeconds() % 60; }
    int GetMinutesFraction() const { return this->GetMinutes() % 60; }
    int GetHoursOfDay() const { return this->GetHours() % 24; }
    int GetDaysOfWeek() const { return this->GetDays() % 7; }
    int GetDaysOfMonth() const { return this->GetDays() % 28; }
    int GetWeeksOfMonth() const { return this->GetWeeks() % 4; }
    int GetMonthsOfYear() const { return this->GetMonths() % 12; }

    GameTime &AddSeconds(int seconds) {
        this->value += SECONDS_TO_GAME_TIME(seconds);
        return *this;
    }
    GameTime &AddMinutes(int minutes) {
        this->value += SECONDS_TO_GAME_TIME(60ull * minutes);
        return *this;
    }
    GameTime &SubtractHours(int hours) {
        this->value -= SECONDS_TO_GAME_TIME(3600ull * hours);
        return *this;
    }
    GameTime &AddDays(int days) {
        this->value += SECONDS_TO_GAME_TIME(86400ull * days);
        return *this;
    }
    GameTime &AddYears(int years) {
        this->value += SECONDS_TO_GAME_TIME(29030400ull * years);
        return *this;
    }

    void SetExpired() { this->value = -1;  }
    void Reset() { this->value = 0; }
    bool Valid() const { return this->value > 0; }

    friend GameTime operator+(const GameTime &l, const GameTime &r) {
        return GameTime(l.value + r.value);
    }
    friend GameTime operator-(const GameTime &l, const GameTime &r) {
        return GameTime(l.value - r.value);
    }
    GameTime &operator+=(GameTime &rhs) {
        this->value += rhs.value;
        return *this;
    }
    GameTime &operator-=(GameTime &rhs) {
        this->value -= rhs.value;
        return *this;
    }

    bool operator==(const GameTime &rhs) const { return this->value == rhs.value; }
    bool operator>(const GameTime &rhs) const { return this->value > rhs.value; }
    bool operator>=(const GameTime &rhs) const { return this->value >= rhs.value; }
    bool operator<(const GameTime &rhs) const { return this->value < rhs.value; }
    bool operator<=(const GameTime &rhs) const { return this->value <= rhs.value; }

    explicit operator bool() {
        return this->Valid();
    }  // unsafe bool was casuing many problems

    explicit operator int64_t() { return this->value; }  // cast operator conversion require

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

struct Timer {
    static Timer *Create() { return new Timer; }

    Timer() : bReady(false), bPaused(false) {
        bTackGameTime = 0;
        uStartTime = 0;
        uStopTime = 0;
        uGameTimeStart = 0;
        field_18 = 0;
        uTimeElapsed = 0;
        dt_fixpoint = 0;
        uTotalGameTimeElapsed = 0;
    }

    void Initialize();

    /**
     * @return                          Current time in 1/128th of a second.
     */
    uint64_t Time();

    void Update();
    void Pause();
    void Resume();
    void TrackGameTime();
    void StopGameTime();

    unsigned int bReady; // Unused
    unsigned int bPaused;
    int bTackGameTime;
    unsigned int uStartTime; // Last tick (frame) time, in 1/128th of a second.
    unsigned int uStopTime;
    int uGameTimeStart;
    int field_18;
    int uTimeElapsed; // dt in 1/128th of a second (real time, not game time).
    int dt_fixpoint; // dt in seconds in fixpoint format
    unsigned int uTotalGameTimeElapsed; // total time elapsed since the last Initialize() call, in 1/128th of a second.

    // Real-world time intervals in timer quants
    static const unsigned int Second = 128;
    static const unsigned int Minute = 60 * Second;
    static const unsigned int Hour = 60 * Minute;
    static const unsigned int Day = 24 * Hour;
    static const unsigned int Week = 7 * Day;
    static const unsigned int Month = 4 * Week;
    static const unsigned int Year = 12 * Month;
};

extern Timer *pMiscTimer;
extern Timer *pEventTimer;
