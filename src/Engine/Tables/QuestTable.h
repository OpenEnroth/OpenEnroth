#pragma once

#include <string>

#include "Engine/PartyEnums.h"

#include "Utility/IndexedArray.h"

class Blob;

/**
 * @offset 0x4768A9
 */
void initializeQuests(std::string_view quests);

extern IndexedArray<std::string, QBIT_FIRST, QBIT_LAST> pQuestTable;
