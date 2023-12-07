#pragma once

#include <cstdint>
#include <compare>

#include <chrono>

struct CivilDuration {
    int days = 0;
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
};

struct LongCivilDuration {
    int years = 0;
    int months = 0;
    int days = 0;
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
};

class Duration {
 public:
    static constexpr int64_t TICKS_PER_REALTIME_SECOND = 128;
    static constexpr int64_t GAME_SECONDS_IN_REALTIME_SECOND = 30; // Game time runs 30x faster than real time.

    constexpr Duration() = default;
    constexpr Duration(int seconds, int minutes, int hours, int days, int weeks, int months, int years) {
        value = seconds + 60ll * minutes + 3600ll * hours + 86400ll * days + 604800ll * weeks + 2419200ll * months + 29030400ll * years;
        value = value * TICKS_PER_REALTIME_SECOND / GAME_SECONDS_IN_REALTIME_SECOND;
    }

    static Duration fromTicks(int64_t ticks) {
        Duration result;
        result.value = ticks;
        return result;
    }

    static Duration fromSeconds(int seconds) { return Duration(seconds, 0, 0, 0, 0, 0, 0); }
    static Duration fromMinutes(int minutes) { return Duration(0, minutes, 0, 0, 0, 0, 0); }
    static Duration fromHours(int hours) { return Duration(0, 0, hours, 0, 0, 0, 0); }
    static Duration fromDays(int days) { return Duration(0, 0, 0, days, 0, 0, 0); }
    static Duration fromMonths(int months) { return Duration(0, 0, 0, 0, 0, months, 0); }
    static Duration fromYears(int years) { return Duration(0, 0, 0, 0, 0, 0, years); }

    int64_t ticks() const { return value; }
    int64_t toSeconds() const { return value * GAME_SECONDS_IN_REALTIME_SECOND / TICKS_PER_REALTIME_SECOND; }
    int64_t toMinutes() const { return toSeconds() / 60; }
    int64_t toHours() const { return toMinutes() / 60; }
    int toDays() const { return toHours() / 24; }
    int toWeeks() const { return toDays() / 7; }
    int toMonths() const { return toWeeks() / 4; }
    int toYears() const { return toMonths() / 12; }

    CivilDuration toCivilDuration() const {
        CivilDuration result;
        result.days = toDays();
        result.hours = toHours() % 24;
        result.minutes = toMinutes() % 60;
        result.seconds = toSeconds() % 60;
        return result;
    };

    LongCivilDuration toLongCivilDuration() const {
        LongCivilDuration result;
        result.years = toYears();
        result.months = toMonths() % 12;
        result.days = toDays() % 28;
        result.hours = toHours() % 24;
        result.minutes = toMinutes() % 60;
        result.seconds = toSeconds() % 60;
        return result;
    }

    friend Duration operator+(const Duration &l, const Duration &r) {
        return Duration::fromTicks(l.value + r.value);
    }

    friend Duration operator-(const Duration &l, const Duration &r) {
        return Duration::fromTicks(l.value - r.value);
    }

    Duration &operator+=(const Duration &rhs) {
        value += rhs.value;
        return *this;
    }

    Duration &operator-=(const Duration &rhs) {
        value -= rhs.value;
        return *this;
    }

    friend bool operator==(const Duration &l, const Duration &r) = default;
    friend auto operator<=>(const Duration &l, const Duration &r) = default;

    explicit operator bool() const {
        return value != 0;
    }

    static constexpr Duration zero() {
        return {};
    }

 private:
    int64_t value = 0;
};
