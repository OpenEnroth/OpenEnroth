cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_DATA_SOURCES
        TileEnumFunctions.cpp)

set(ENGINE_DATA_HEADERS
        AutonoteEnums.h
        AutonoteData.h
        AwardData.h
        AwardEnums.h
        HouseData.h
        HouseEnums.h
        HouseEnumFunctions.h
        HistoryData.h
        FrameEnums.h
        IconFrameData.h
        PortraitFrameData.h
        TileData.h
        TileEnums.h
        TileEnumFunctions.h)

add_library(engine_data STATIC ${ENGINE_DATA_SOURCES} ${ENGINE_DATA_HEADERS})
target_link_libraries(engine_data PUBLIC library_serialization utility)
target_check_style(engine_data)
