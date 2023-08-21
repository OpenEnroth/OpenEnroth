#pragma once

#include <array>

#include "Engine/Objects/CharacterEnums.h"

class Character;
enum class Condition : uint32_t;

/**
 * @param character                     Character that's about to receive a condition.
 * @param condition                     Condition in question.
 * @return                              Whether the condition was successfully blocked, either by protection from magic
 *                                      spell, or by an item. In the former case, this function will decrement the
 *                                      protection from magic counter & expire the buff if the counter reaches zero.
 */
bool blockCondition(Character *character, Condition condition);

const std::array<Condition, 18> &conditionImportancyTable();
