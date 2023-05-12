#pragma once
#include <array>
#include <string>

/*  351 */
enum class AUTONOTE_TYPE : uint32_t {
    AUTONOTE_POTION_RECEPIE = 0,
    AUTONOTE_STAT_HINT = 1,
    AUTONOTE_OBELISK = 2,
    AUTONOTE_SEER = 3,
    AUTONOTE_MISC = 4,
    AUTONOTE_TEACHER = 5,
};
using enum AUTONOTE_TYPE;

struct Autonote {
    const char *pText;
    AUTONOTE_TYPE eType;
};

/**
 * @offset 0x476750
 */
void initializeAutonotes();

extern std::array<Autonote, 196> pAutonoteTxt;
extern std::string pAutonoteTXT_Raw;
