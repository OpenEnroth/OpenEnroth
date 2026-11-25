#pragma once

#include <vector>

#include "Engine/Objects/Character.h"

/**
 * MM8's Adventurer's Inn bench: companions who have been recruited (from `roster.txt`) but aren't
 * currently occupying one of the party's active slots, keyed by `Character::rosterId`.
 *
 * Only meaningful under `GAME_TYPE_MM8` - MM7 keeps using `Party::pCharacters`'s original fixed-4
 * roster untouched, this class isn't consulted on the MM7 path at all.
 *
 * Deliberately self-contained with no dependency on engine globals, so it can be exercised in unit
 * tests without a running engine.
 */
class PartyMM8Roster {
 public:
    /**
     * Adds `character` to the Inn bench. `character.rosterId` must already be set (nonzero) - it's
     * the key used by `isInInn()`/`takeFromInn()`. Asserts if a member with the same `rosterId` is
     * already on the bench.
     */
    void addToInn(Character character);

    /**
     * Removes and returns the Inn member with the given `rosterId`. Asserts the member exists -
     * check `isInInn()` first.
     */
    Character takeFromInn(int rosterId);

    bool isInInn(int rosterId) const;

    /**
     * @return                          Total number of companions currently on the Inn bench.
     */
    int innCount() const;

    /**
     * The Inn bench, in no particular order - for UI enumeration. Callers must not
     * mutate members through this. Go through `addToInn()`/`takeFromInn()` instead.
     */
    const std::vector<Character> &members() const {
        return _inn;
    }

 private:
    std::vector<Character> _inn;
};
