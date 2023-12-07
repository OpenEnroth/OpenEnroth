#pragma once

#include <cstdint>

#include "Duration.h"

const int game_starting_year = 1168;

struct CivilTime {
    int year = 0; // Since the Silence.
    int month = 0; // In [1,12].
    int week = 0; // In [1, 4].
    int day = 0; // In [1, 28].
    int dayOfWeek = 0; // In [1, 7].
    int hour = 0; // In [0, 23].
    int minute = 0;
    int second = 0;
};

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

    CivilTime toCivilTime() const {
        CivilTime result;
        result.year = game_starting_year + toYears();
        result.month = 1 + toMonths() % 12;
        result.week = 1 + toWeeks() % 4;
        result.day = 1 + toDays() % 28;
        result.dayOfWeek = 1 + toDays() % 7;
        result.hour = toHours() % 24;
        result.minute = toMinutes() % 60;
        result.second = toSeconds() % 60;
        return result;
    }

    // TODO(captainurist): doesn't belong to GameTime.
    void SetExpired() { value = -1;  }
    bool Expired() const { return value < 0; }

    // TODO(captainurist): This is something to look at, we have comparisons with GameTime() in the code, they are not
    //                     the same as Valid().
    bool isValid() const { return value > 0; }

    Time &operator+=(const Duration &rhs) {
        value += rhs.ticks();
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

inline Duration operator-(const Time &l, const Time &r) {
    return Duration::fromTicks(l.ticks() - r.ticks());
}
