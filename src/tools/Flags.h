#pragma once

#include <type_traits>

/**
 * `Flags` is basically an analog of `QFlags` from Qt, a type-safe wrapper
 * for flags defined in enumerations.
 *
 * Example usage:
 *      class MonsterType {
 *      public:
 *          enum class Ability {
 *              FlyingAbility = 0x1,
 *              RunningAbility = 0x2,
 *              ShootingAbility = 0x4
 *          };
 *          using enum Ability;
 *          DECLARE_FLAGS(Abilities, Ability)
 *
 *          // ...
 *      };
 *
 *      DECLARE_OPERATORS_FOR_FLAGS(MonsterType::Abilities)
 */
template <class Enum>
class Flags {
    static_assert(std::is_enum<Enum>::value, "Expecting an enumeration");

 public:
    using enumeration_type = Enum;
    using underlying_type = std::underlying_type_t<Enum>;

    constexpr Flags(std::nullptr_t = 0) :
        value_(0)
    {}

    constexpr Flags(enumeration_type value) :
        value_(static_cast<underlying_type>(value))
    {}

    explicit constexpr Flags(underlying_type value) :
        value_(value)
    {}

    explicit operator underlying_type() const {
        return value_;
    }

    constexpr friend Flags operator|(Flags l, Flags r) {
        return Flags(l.value_ | r.value_);
    }

    constexpr friend Flags operator|(enumeration_type l, Flags r) {
        return Flags(static_cast<underlying_type>(l) | r.value_);
    }

    constexpr friend Flags operator|(Flags l, enumeration_type r) {
        return Flags(l.value_ | static_cast<underlying_type>(r));
    }

    constexpr friend Flags operator&(Flags l, Flags r) {
        return Flags(l.value_ & r.value_);
    }

    constexpr friend Flags operator&(enumeration_type l, Flags r) {
        return Flags(static_cast<underlying_type>(l) & r.value_);
    }

    constexpr friend Flags operator&(Flags l, enumeration_type r) {
        return Flags(l.value_ & static_cast<underlying_type>(r));
    }

    constexpr friend bool operator==(Flags l, Flags r) {
        return l.value_ == r.value_;
    }

    constexpr friend bool operator==(enumeration_type l, Flags r) {
        return static_cast<underlying_type>(l) == r.value_;
    }

    constexpr friend bool operator==(Flags l, enumeration_type r) {
        return l.value_ == static_cast<underlying_type>(r);
    }

    constexpr friend bool operator!=(Flags l, Flags r) {
        return l.value_ != r.value_;
    }

    constexpr friend bool operator!=(enumeration_type l, Flags r) {
        return static_cast<underlying_type>(l) != r.value_;
    }

    constexpr friend bool operator!=(Flags l, enumeration_type r) {
        return l.value_ != static_cast<underlying_type>(r);
    }

    constexpr Flags& operator&=(Flags mask) {
        *this = *this & mask;
        return *this;
    }

    constexpr Flags& operator&=(enumeration_type mask) {
        *this = *this & mask;
        return *this;
    }

    constexpr Flags& operator|=(Flags flags) {
        *this = *this | flags;
        return *this;
    }

    constexpr Flags& operator|=(enumeration_type flags) {
        *this = *this | flags;
        return *this;
    }

    constexpr Flags operator~() const {
        return Flags(~value_);
    }

    constexpr bool operator!() const {
        return !value_;
    }

    constexpr operator bool() const {
        return value_;
    }

 private:
    underlying_type value_;
};


#define DECLARE_FLAGS(FLAGS, ENUM)                                              \
    using FLAGS = Flags<ENUM>;


#define DECLARE_OPERATORS_FOR_FLAGS(FLAGS)                                      \
    constexpr inline FLAGS operator|(FLAGS::enumeration_type l, FLAGS::enumeration_type r) { \
        return FLAGS(l) | r;                                                    \
    }                                                                           \
    constexpr inline FLAGS operator~(FLAGS::enumeration_type value) {           \
        return ~FLAGS(value);                                                   \
    }

