#pragma once

#include <string>
#include <array>

class Blob;

/**
 * @offset 0x476682
 */
void initializeTransitions(std::string_view transitions);

extern std::array<std::string, 465> pTransitionStrings;
