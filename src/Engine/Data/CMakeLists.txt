cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_DATA_SOURCES)

set(ENGINE_DATA_HEADERS
        AutonoteEnums.h
        AutonoteData.h
        AwardData.h
        AwardEnums.h
        HouseData.h
        HouseEnums.h
        HouseEnumFunctions.h)

add_library(engine_data INTERFACE ${ENGINE_DATA_SOURCES} ${ENGINE_DATA_HEADERS})
target_link_libraries(engine_data INTERFACE library_serialization utility)
target_check_style(engine_data)
