#pragma once

#include <array>
#include <string>

/**
 * @offset 0x4768A9
 */
void initializeQuests();

extern std::array<const char *, 513> pQuestTable;
extern std::string pQuestsTXT_Raw;
