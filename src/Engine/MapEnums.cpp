#include "MapEnums.h"

#include "Library/Serialization/EnumSerialization.h"

MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(MapStartPoint, CASE_SENSITIVE, {
    {MAP_START_POINT_PARTY, "Party Start"},
    {MAP_START_POINT_NORTH, "North Start"},
    {MAP_START_POINT_SOUTH, "South Start"},
    {MAP_START_POINT_EAST, "East Start"},
    {MAP_START_POINT_WEST, "West Start"}
})
