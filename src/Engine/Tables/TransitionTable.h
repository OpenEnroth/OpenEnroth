#pragma once

#include <string>
#include <array>

class Blob;

/**
 * @offset 0x476682
 */
void initializeTransitions(const Blob &transitions);

// Sized to fit MM8's trans.txt (real rows up to index 500) rather than MM7's smaller 465-row table - the
// wider table is harmless for MM7, whose rows all fit well within it too.
extern std::array<std::string, 501> pTransitionStrings;
