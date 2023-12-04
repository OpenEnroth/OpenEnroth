#pragma once

#include <cstdint>

const int game_starting_year = 1168;

struct GameTime {
    static constexpr int64_t TICKS_PER_REALTIME_SECOND = 128;
    static constexpr int64_t GAME_SECONDS_IN_REALTIME_SECOND = 30; // Game time runs 30x faster than real time.

    GameTime() = default;
    GameTime(int seconds, int minutes, int hours = 0, int days = 0, int weeks = 0, int months = 0, int years = 0) {
        value = seconds + 60ll * minutes + 3600ll * hours + 86400ll * days + 604800ll * weeks + 2419200ll * months + 29030400ll * years;
        value = value * TICKS_PER_REALTIME_SECOND / GAME_SECONDS_IN_REALTIME_SECOND;
    }

    static GameTime fromTicks(int64_t ticks) {
        GameTime result;
        result.value = ticks;
        return result;
    }

    static GameTime fromSeconds(int seconds) { return GameTime(seconds, 0, 0, 0, 0, 0, 0); }
    static GameTime fromMinutes(int minutes) { return GameTime(0, minutes, 0, 0, 0, 0, 0); }
    static GameTime fromHours(int hours) { return GameTime(0, 0, hours, 0, 0, 0, 0); }
    static GameTime fromDays(int days) { return GameTime(0, 0, 0, days, 0, 0, 0); }
    static GameTime fromMonths(int months) { return GameTime(0, 0, 0, 0, 0, months, 0); }
    static GameTime fromYears(int years) { return GameTime(0, 0, 0, 0, 0, 0, years); }

    int64_t toSeconds() const { return value * GAME_SECONDS_IN_REALTIME_SECOND / TICKS_PER_REALTIME_SECOND; }
    int64_t toMinutes() const { return toSeconds() / 60; }
    int64_t toHours() const { return toMinutes() / 60; }
    int toDays() const { return toHours() / 24; }
    int toWeeks() const { return toDays() / 7; }
    int toMonths() const { return toWeeks() / 4; }
    int toYears() const { return toMonths() / 12; }

    int secondsFraction() const { return toSeconds() % 60; }
    int minutesFraction() const { return toMinutes() % 60; }
    int hoursOfDay() const { return toHours() % 24; }
    int daysOfWeek() const { return toDays() % 7; }
    int daysOfMonth() const { return toDays() % 28; }
    int weeksOfMonth() const { return toWeeks() % 4; }
    int monthsOfYear() const { return toMonths() % 12; }

    // TODO(captainurist): doesn't belong to GameTime.
    void SetExpired() { value = -1;  }
    bool Expired() const { return value < 0; }

    // TODO(captainurist): This is something to look at, we have comparisons with GameTime() in the code, they are not
    //                     the same as Valid().
    bool isValid() const { return value > 0; }

    friend GameTime operator+(const GameTime &l, const GameTime &r) {
        return GameTime::fromTicks(l.value + r.value);
    }

    friend GameTime operator-(const GameTime &l, const GameTime &r) {
        return GameTime::fromTicks(l.value - r.value);
    }

    GameTime &operator+=(const GameTime &rhs) {
        value += rhs.value;
        return *this;
    }

    GameTime &operator-=(const GameTime &rhs) {
        value -= rhs.value;
        return *this;
    }

    friend bool operator==(const GameTime &l, const GameTime &r) = default;
    friend auto operator<=>(const GameTime &l, const GameTime &r) = default;

    explicit operator bool() const {
        return isValid();
    }

    int64_t value = 0;
};
