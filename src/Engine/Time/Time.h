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
    int hourAmPm = 0; // In [0, 12], note that 12:00 is 12pm.
    bool isPm = false;
    int minute = 0;
    int second = 0;
};

class Time {
 public:
    Time() = default;
    Time(int seconds, int minutes, int hours = 0, int days = 0, int weeks = 0, int months = 0, int years = 0) {
        _ticks = seconds + 60ll * minutes + 3600ll * hours + 86400ll * days + 604800ll * weeks + 2419200ll * months + 29030400ll * years;
        _ticks = _ticks * Duration::TICKS_PER_REALTIME_SECOND / Duration::GAME_SECONDS_IN_REALTIME_SECOND;
    }

    Duration toDurationSinceSilence() const {
        return Duration::fromYears(game_starting_year) + Duration::fromTicks(_ticks);
    }

    static Time fromDurationSinceSilence(Duration duration) {
        return Time::fromTicks((duration - Duration::fromYears(game_starting_year)).ticks());
    }

    static Time fromTicks(int64_t ticks) {
        Time result;
        result._ticks = ticks;
        return result;
    }

    static Time fromSeconds(int seconds) { return Time(seconds, 0, 0, 0, 0, 0, 0); }
    static Time fromMinutes(int minutes) { return Time(0, minutes, 0, 0, 0, 0, 0); }
    static Time fromHours(int hours) { return Time(0, 0, hours, 0, 0, 0, 0); }
    static Time fromDays(int days) { return Time(0, 0, 0, days, 0, 0, 0); }
    static Time fromMonths(int months) { return Time(0, 0, 0, 0, 0, months, 0); }
    static Time fromYears(int years) { return Time(0, 0, 0, 0, 0, 0, years); }

    int64_t ticks() const { return _ticks; }
    int64_t toSeconds() const { return _ticks * Duration::GAME_SECONDS_IN_REALTIME_SECOND / Duration::TICKS_PER_REALTIME_SECOND; }
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
        result.hourAmPm = result.hour > 12 ? result.hour - 12 : result.hour;
        result.isPm = result.hour >= 12; // 12:00 is 12pm.
        result.minute = toMinutes() % 60;
        result.second = toSeconds() % 60;
        return result;
    }

    // TODO(captainurist): #time doesn't belong to GameTime.
    void SetExpired() { _ticks = -1;  }
    bool Expired() const { return _ticks < 0; }

    // TODO(captainurist): #time This is something to look at, we have comparisons with GameTime() in the code, they are not
    //                     the same as Valid().
    bool isValid() const { return _ticks > 0; }

    Time &operator+=(Duration rhs) {
        _ticks += rhs.ticks();
        return *this;
    }

    Time &operator-=(Duration rhs) {
        _ticks -= rhs.ticks();
        return *this;
    }

    friend bool operator==(Time l, Time r) = default;
    friend auto operator<=>(Time l, Time r) = default;

    explicit operator bool() const {
        return isValid();
    }

 private:
    int64_t _ticks = 0;
};

inline Time operator+(Time l, Duration r) {
    return Time::fromTicks(l.ticks() + r.ticks());
}

// We don't provide operator+(Duration, Time)

inline Time operator-(Time l, Duration r) {
    return Time::fromTicks(l.ticks() - r.ticks());
}

inline Duration operator-(Time l, Time r) {
    return Duration::fromTicks(l.ticks() - r.ticks());
}
