#pragma once

#include <array>

#include "Engine/Objects/CharacterEnums.h"

class Character;

// TODO(captainurist): this is not a getter, it decrements protection from magic counter. Rename or redo.
bool IsPlayerAffected(Character *character, Condition condition, bool blockable);

const std::array<Condition, 18> &conditionImportancyTable();
