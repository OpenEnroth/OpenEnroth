#pragma once

#include <array>

#include "Engine/Data/HistoryData.h"

class Blob;

struct HistoryTable {
    void Initialize(const Blob &history);
    std::array<HistoryData, 29> historyLines;
    // int field_15C;
    // int field_0;
    // int field_4[87];
};

extern HistoryTable *pHistoryTable;
