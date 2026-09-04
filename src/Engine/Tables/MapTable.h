#pragma once

#include <string_view>

#include "Engine/Data/MapData.h"
#include "Engine/MapEnums.h"

#include "Utility/IndexedArray.h"

class Blob;

void initializeMaps(const Blob &maps);

/**
 * @param fileName                      Map file name, e.g. "out02.odm", case-insensitive.
 * @return                              Id of the map with the given file name, or `MAP_INVALID` if there is no such
 *                                      map.
 */
MapId mapIdByFileName(std::string_view fileName);

extern IndexedArray<MapData, MAP_FIRST, MAP_LAST> mapTable;
