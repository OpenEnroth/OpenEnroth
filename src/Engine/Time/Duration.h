#pragma once

#include <cstdint>
#include <cassert>
#include <compare>
#include <type_traits>

class RandomEngine;

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
        _ticks = seconds + 60ll * minutes + 3600ll * hours + 86400ll * days + 604800ll * weeks + 2419200ll * months + 29030400ll * years;
        _ticks = _ticks * TICKS_PER_REALTIME_SECOND / GAME_SECONDS_IN_REALTIME_SECOND;
    }

    [[nodiscard]] constexpr static Duration fromTicks(int64_t ticks) {
        Duration result;
        result._ticks = ticks;
        return result;
    }

    [[nodiscard]] constexpr static Duration fromSeconds(int seconds) { return Duration(seconds, 0, 0, 0, 0, 0, 0); }
    [[nodiscard]] constexpr static Duration fromMinutes(int minutes) { return Duration(0, minutes, 0, 0, 0, 0, 0); }
    [[nodiscard]] constexpr static Duration fromHours(int hours) { return Duration(0, 0, hours, 0, 0, 0, 0); }
    [[nodiscard]] constexpr static Duration fromDays(int days) { return Duration(0, 0, 0, days, 0, 0, 0); }
    [[nodiscard]] constexpr static Duration fromMonths(int months) { return Duration(0, 0, 0, 0, 0, months, 0); }
    [[nodiscard]] constexpr static Duration fromYears(int years) { return Duration(0, 0, 0, 0, 0, 0, years); }

    [[nodiscard]] constexpr int64_t ticks() const { return _ticks; }
    [[nodiscard]] constexpr int64_t toSeconds() const { return _ticks * GAME_SECONDS_IN_REALTIME_SECOND / TICKS_PER_REALTIME_SECOND; }
    [[nodiscard]] constexpr int64_t toMinutes() const { return toSeconds() / 60; }
    [[nodiscard]] constexpr int64_t toHours() const { return toMinutes() / 60; }
    [[nodiscard]] constexpr int toDays() const { return toHours() / 24; }
    [[nodiscard]] constexpr int toWeeks() const { return toDays() / 7; }
    [[nodiscard]] constexpr int toMonths() const { return toWeeks() / 4; }
    [[nodiscard]] constexpr int toYears() const { return toMonths() / 12; }

    [[nodiscard]] constexpr static Duration fromRealtimeSeconds(int64_t seconds) { return fromTicks(seconds * TICKS_PER_REALTIME_SECOND); }
    [[nodiscard]] constexpr static Duration fromRealtimeMilliseconds(int64_t msec) { return fromTicks(msec * TICKS_PER_REALTIME_SECOND / 1000); }

    [[nodiscard]] constexpr int64_t toRealtimeSeconds() const { return ticks() / TICKS_PER_REALTIME_SECOND; }
    [[nodiscard]] constexpr int64_t toRealtimeMilliseconds() const { return ticks() * 1000 / TICKS_PER_REALTIME_SECOND; }
    [[nodiscard]] constexpr float toFloatRealtimeSeconds() const { return ticks() / static_cast<float>(TICKS_PER_REALTIME_SECOND); }

    // Unlike with RandomEngine::randomInSegment, two-arg functions below generate a duration in [lo, hi) open interval,
    // not in [lo, hi] segment.

    [[nodiscard]] static Duration random(RandomEngine *rng, Duration hi);
    [[nodiscard]] static Duration randomRealtimeMilliseconds(RandomEngine *rng, int64_t hi);
    [[nodiscard]] static Duration randomRealtimeMilliseconds(RandomEngine *rng, int64_t lo, int64_t hi);
    [[nodiscard]] static Duration randomRealtimeSeconds(RandomEngine *rng, int64_t hi);
    [[nodiscard]] static Duration randomRealtimeSeconds(RandomEngine *rng, int64_t lo, int64_t hi);

    [[nodiscard]] constexpr CivilDuration toCivilDuration() const {
        CivilDuration result;
        result.days = toDays();
        result.hours = toHours() % 24;
        result.minutes = toMinutes() % 60;
        result.seconds = toSeconds() % 60;
        return result;
    }

    [[nodiscard]] constexpr LongCivilDuration toLongCivilDuration() const {
        LongCivilDuration result;
        result.years = toYears();
        result.months = toMonths() % 12;
        result.days = toDays() % 28;
        result.hours = toHours() % 24;
        result.minutes = toMinutes() % 60;
        result.seconds = toSeconds() % 60;
        return result;
    }

    // TODO(captainurist): #time add unit tests.

    [[nodiscard]] constexpr Duration roundedUp(Duration period) const {
        assert(period.ticks() > 0);

        int64_t t = ticks();
        int64_t p = period.ticks();
        return Duration::fromTicks((t + p - 1) / p * p);
    }

    [[nodiscard]] constexpr Duration roundedDown(Duration period) const {
        assert(period.ticks() > 0);

        int64_t t = ticks();
        int64_t p = period.ticks();
        return Duration::fromTicks(t / p * p);
    }

    [[nodiscard]] constexpr friend Duration operator+(const Duration &l, const Duration &r) {
        return Duration::fromTicks(l._ticks + r._ticks);
    }

    [[nodiscard]] constexpr friend Duration operator-(const Duration &l, const Duration &r) {
        return Duration::fromTicks(l._ticks - r._ticks);
    }

    template<class L> requires std::is_arithmetic_v<L>
    [[nodiscard]] constexpr friend Duration operator*(L l, const Duration &r) {
        return Duration::fromTicks(l * r._ticks);
    }

    template<class R> requires std::is_arithmetic_v<R>
    [[nodiscard]] constexpr friend Duration operator*(const Duration &l, R r) {
        return Duration::fromTicks(l._ticks * r);
    }

    template<class R> requires std::is_arithmetic_v<R>
    [[nodiscard]] constexpr friend Duration operator/(const Duration &l, R r) {
        return Duration::fromTicks(l._ticks / r);
    }

    [[nodiscard]] constexpr friend Duration operator%(const Duration &l, const Duration &r) {
        return Duration::fromTicks(l._ticks % r._ticks);
    }

    constexpr Duration &operator+=(const Duration &rhs) {
        _ticks += rhs._ticks;
        return *this;
    }

    constexpr Duration &operator-=(const Duration &rhs) {
        _ticks -= rhs._ticks;
        return *this;
    }

    template<class R> requires std::is_arithmetic_v<R>
    constexpr Duration &operator*=(R r) {
        _ticks *= r;
        return *this;
    }

    template<class R> requires std::is_arithmetic_v<R>
    constexpr Duration &operator/=(R r) {
        _ticks /= r;
        return *this;
    }

    [[nodiscard]] constexpr friend bool operator==(const Duration &l, const Duration &r) = default;
    [[nodiscard]] constexpr friend auto operator<=>(const Duration &l, const Duration &r) = default;

    [[nodiscard]] constexpr explicit operator bool() const {
        return _ticks != 0;
    }

    [[nodiscard]] constexpr static Duration zero() {
        return {};
    }

 private:
    int64_t _ticks = 0;
};

constexpr Duration operator""_ticks(unsigned long long ticks) {
    return Duration::fromTicks(ticks);
}
