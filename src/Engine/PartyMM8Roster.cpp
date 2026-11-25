#include "PartyMM8Roster.h"

#include <algorithm>
#include <cassert>
#include <utility>

void PartyMM8Roster::addToInn(Character character) {
    assert(character.rosterId != 0);
    assert(!isInInn(character.rosterId));
    _inn.push_back(std::move(character));
}

Character PartyMM8Roster::takeFromInn(int rosterId) {
    auto it = std::ranges::find(_inn, rosterId, &Character::rosterId);
    assert(it != _inn.end());
    Character result = std::move(*it);
    _inn.erase(it);
    return result;
}

bool PartyMM8Roster::isInInn(int rosterId) const {
    return std::ranges::find(_inn, rosterId, &Character::rosterId) != _inn.end();
}

int PartyMM8Roster::innCount() const {
    return static_cast<int>(_inn.size());
}
