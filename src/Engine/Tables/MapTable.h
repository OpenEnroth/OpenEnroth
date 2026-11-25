#pragma once

#include <string_view>

#include "Engine/Data/MapData.h"
#include "Engine/MapEnums.h"

#include "Utility/IndexedArray.h"

class Blob;

struct MapTable {
    void Initialize(const Blob &mapStats);
    MapId GetMapInfo(std::string_view fileName);
    // Spans `MAP_FIRST`..`MAP_LAST_MM8` since this storage must hold whichever game's maps are active.
    IndexedArray<MapData, MAP_FIRST, MAP_LAST_MM8> pInfos;
};

extern MapTable *pMapTable;
