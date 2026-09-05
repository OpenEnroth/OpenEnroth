#pragma once

#include <string_view>

#include "Engine/Data/MapData.h"
#include "Engine/MapEnums.h"

#include "Utility/IndexedArray.h"

class Blob;

struct MapTable {
    void Initialize(const Blob &mapStats);
    MapId GetMapInfo(std::string_view fileName);
    IndexedArray<MapData, MAP_FIRST, MAP_LAST> pInfos;
};

extern MapTable *pMapTable;
