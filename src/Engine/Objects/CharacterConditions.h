#pragma once

#include <initializer_list>

#include "Engine/Time/Time.h"

#include "Utility/IndexedArray.h"

#include "CharacterEnums.h"

struct CharacterConditions_MM7;

class CharacterConditions {
 public:
    [[nodiscard]] bool has(Condition condition) const {
        return _times[condition].isValid();
    }

    [[nodiscard]] bool hasAny(std::initializer_list<Condition> conditions) const {
        for (Condition condition : conditions)
            if (has(condition))
                return true;
        return false;
    }

    [[nodiscard]] bool hasNone(std::initializer_list<Condition> conditions) const {
        return !hasAny(conditions);
    }

    void reset(Condition condition) {
        _times[condition] = Time();
    }

    void resetAll() {
        for (Time &time : _times)
            time = Time();
    }

    void set(Condition condition, Time time) {
        _times[condition] = time;
    }

    [[nodiscard]] Time get(Condition condition) const {
        return _times[condition];
    }

    friend void snapshot(const CharacterConditions &src, CharacterConditions_MM7 *dst); // In EntitySnapshots.cpp.
    friend void reconstruct(const CharacterConditions_MM7 &src, CharacterConditions *dst); // In EntitySnapshots.cpp.

 private:
    /** Game time when condition has started. Zero means that the character doesn't have a condition. */
    IndexedArray<Time, CONDITION_FIRST, CONDITION_LAST> _times;
};
