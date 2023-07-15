#pragma once

#include <array>
#include <string>

class Blob;

/**
 * @offset 0x4768A9
 */
void initializeQuests(const Blob &quests);

extern std::array<std::string, 513> pQuestTable;
