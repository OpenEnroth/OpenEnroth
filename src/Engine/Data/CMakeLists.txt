cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

# What goes here:
# - Data definitions for data tables (both binary and text).
# - Associated data table-related enumerations.
# TODO(captainurist): Game-related enums go to Engine/XYZ/....??? Engine/Model?
set(ENGINE_DATA_SOURCES
        HouseEnumFunctions.cpp
        TileEnums.cpp
        TileEnumFunctions.cpp)

set(ENGINE_DATA_HEADERS
        AutonoteData.h
        AutonoteEnums.h
        AwardData.h
        AwardEnums.h
        AwardEnumFunctions.h
        ChestData.h
        FrameEnums.h
        HistoryData.h
        HouseData.h
        HouseEnumFunctions.h
        HouseEnums.h
        IconFrameData.h
        ItemData.h
        PortraitFrameData.h
        SpecialEnchantmentData.h
        StandardEnchantmentData.h
        TextureFrameData.h
        TileData.h
        TileEnumFunctions.h
        TileEnums.h)

add_library(engine_data STATIC ${ENGINE_DATA_SOURCES} ${ENGINE_DATA_HEADERS})
target_link_libraries(engine_data PUBLIC library_serialization utility)
target_check_style(engine_data)
