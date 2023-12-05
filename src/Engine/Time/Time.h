#pragma once

#include <cstdint>

#include "Duration.h"

const int game_starting_year = 1168;

struct Time {
    Time() = default;
    Time(int seconds, int minutes, int hours = 0, int days = 0, int weeks = 0, int months = 0, int years = 0) {
        value = seconds + 60ll * minutes + 3600ll * hours + 86400ll * days + 604800ll * weeks + 2419200ll * months + 29030400ll * years;
        value = value * Duration::TICKS_PER_REALTIME_SECOND / Duration::GAME_SECONDS_IN_REALTIME_SECOND;
    }

    static Time fromTicks(int64_t ticks) {
        Time result;
        result.value = ticks;
        return result;
    }

    static Time fromSeconds(int seconds) { return Time(seconds, 0, 0, 0, 0, 0, 0); }
    static Time fromMinutes(int minutes) { return Time(0, minutes, 0, 0, 0, 0, 0); }
    static Time fromHours(int hours) { return Time(0, 0, hours, 0, 0, 0, 0); }
    static Time fromDays(int days) { return Time(0, 0, 0, days, 0, 0, 0); }
    static Time fromMonths(int months) { return Time(0, 0, 0, 0, 0, months, 0); }
    static Time fromYears(int years) { return Time(0, 0, 0, 0, 0, 0, years); }

    int64_t ticks() const { return value; }
    int64_t toSeconds() const { return value * Duration::GAME_SECONDS_IN_REALTIME_SECOND / Duration::TICKS_PER_REALTIME_SECOND; }
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

    friend Time operator+(const Time &l, const Time &r) {
        return Time::fromTicks(l.value + r.value);
    }

    // TODO(captainurist): return Duration
    friend Time operator-(const Time &l, const Time &r) {
        return Time::fromTicks(l.value - r.value);
    }

    Time &operator+=(const Time &rhs) {
        value += rhs.value;
        return *this;
    }

    Time &operator+=(const Duration &rhs) {
        value += rhs.ticks();
        return *this;
    }

    // TODO(captainurist): drop
    Time &operator-=(const Time &rhs) {
        value -= rhs.value;
        return *this;
    }

    Time &operator-=(const Duration &rhs) {
        value -= rhs.ticks();
        return *this;
    }

    friend bool operator==(const Time &l, const Time &r) = default;
    friend auto operator<=>(const Time &l, const Time &r) = default;

    explicit operator bool() const {
        return isValid();
    }

    int64_t value = 0;
};

inline Time operator+(const Time &l, const Duration &r) {
    return Time::fromTicks(l.ticks() + r.ticks());
}

// We don't provide operator+(Duration, Time)

inline Time operator-(const Time &l, const Duration &r) {
    return Time::fromTicks(l.ticks() - r.ticks());
}
