#pragma once

#include <cstdint>
#include <compare>

const int game_starting_year = 1168;

// Number of game ticks per 30 game seconds
#define TIME_QUANT                  128
#define TIME_SECONDS_PER_QUANT      30

#define GAME_TIME_TO_SECONDS(VALUE) ((VALUE) * static_cast<uint64_t>(TIME_SECONDS_PER_QUANT) / TIME_QUANT)
#define SECONDS_TO_GAME_TIME(VALUE) ((VALUE) * static_cast<uint64_t>(TIME_QUANT) / TIME_SECONDS_PER_QUANT)

struct GameTime {
    GameTime() = default;
    explicit GameTime(int64_t val) : value(val) {}
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

    int64_t GetSeconds() const {
        return GAME_TIME_TO_SECONDS(this->value);
    }
    int64_t GetMinutes() const { return this->GetSeconds() / 60; }
    int64_t GetHours() const { return this->GetMinutes() / 60; }
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

    [[nodiscard]] GameTime AddSeconds(int seconds) const {
        return *this + GameTime::FromSeconds(seconds);
    }
    [[nodiscard]] GameTime AddMinutes(int minutes) const {
        return *this + GameTime::FromMinutes(minutes);
    }
    [[nodiscard]] GameTime SubtractMinutes(int minutes) const {
        return *this - GameTime::FromMinutes(minutes);
    }
    [[nodiscard]] GameTime AddHours(int hours) const {
        return *this + GameTime::FromHours(hours);
    }
    [[nodiscard]] GameTime SubtractHours(int hours) const {
        return *this - GameTime::FromHours(hours);
    }
    [[nodiscard]] GameTime AddDays(int days) const {
        return *this + GameTime::FromDays(days);
    }
    [[nodiscard]] GameTime SubtractDays(int days) const {
        return *this - GameTime::FromDays(days);
    }
    [[nodiscard]] GameTime AddYears(int years) const {
        return *this + GameTime::FromYears(years);
    }

    void SetExpired() { this->value = -1;  }
    bool Expired() const { return this->value < 0; }
    void Reset() { this->value = 0; }
    bool Valid() const { return this->value > 0; }

    friend GameTime operator+(const GameTime &l, const GameTime &r) {
        return GameTime(l.value + r.value);
    }

    friend GameTime operator-(const GameTime &l, const GameTime &r) {
        return GameTime(l.value - r.value);
    }

    GameTime &operator+=(const GameTime &rhs) {
        this->value += rhs.value;
        return *this;
    }

    GameTime &operator-=(const GameTime &rhs) {
        this->value -= rhs.value;
        return *this;
    }

    friend bool operator==(const GameTime &l, const GameTime &r) = default;
    friend auto operator<=>(const GameTime &l, const GameTime &r) = default;

    explicit operator bool() const {
        return this->Valid();
    }

    explicit operator int64_t() const { return this->value; }  // cast operator conversion require

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
    static GameTime FromMonths(int months) {
        return GameTime(0, 0, 0, 0, 0, months, 0);
    }
    static GameTime FromYears(int years) {
        return GameTime(0, 0, 0, 0, 0, 0, years);
    }

    int64_t value = 0;
};

struct Timer {
    Timer() = default;

    /**
     * @return                          Current real time (not game time!) in timer ticks. One tick is 1/128th of a
     *                                  real time second.
     */
    uint64_t Time();

    void Update();
    void Pause();
    void Resume();
    void TrackGameTime();
    void StopGameTime();

    unsigned int bPaused = false;
    int bTackGameTime = 0;
    unsigned int uStartTime = 0; // Last frame time, in real time ticks (128 ticks is 1 real time second).
    unsigned int uStopTime = 0;
    int uGameTimeStart = 0;
    int uTimeElapsed = 0; // dt since last frame in real time ticks (128 ticks is 1 real time second).
    int dt_fixpoint = 0; // dt since last frame in real time seconds in fixpoint format.
    unsigned int uTotalTimeElapsed = 0; // Total time elapsed since the last Initialize() call, in real time ticks (128 ticks is 1 real time second).

    // Real time intervals in timer ticks.
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
