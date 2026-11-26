#pragma once

#include <string>
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

/**
 * @param map                           Map to look up.
 * @return                              File name of the map without the extension, e.g. "d27".
 */
[[nodiscard]] std::string mapFileStem(MapId map);
