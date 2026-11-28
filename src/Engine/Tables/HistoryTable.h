#pragma once

#include <array>
#include <string_view>

#include "Engine/Data/HistoryData.h"

struct HistoryTable {
    void Initialize(std::string_view history);
    std::array<HistoryData, 29> historyLines;
    // int field_15C;
    // int field_0;
    // int field_4[87];
};

extern HistoryTable *pHistoryTable;
