cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(ENGINE_TABLES_SOURCES
        FactionTable.cpp
        FrameTableInc.cpp
        IconFrameTable.cpp
        PlayerFrameTable.cpp
        StorylineTextTable.cpp
        TileTable.cpp
        BuildingTable.cpp)

set(ENGINE_TABLES_HEADERS
        FactionTable.h
        FrameTableInc.h
        IconFrameTable.h
        PlayerFrameTable.h
        StorylineTextTable.h
        TileFrameTable.h
        BuildingTable.h)

add_library(engine_tables STATIC ${ENGINE_TABLES_SOURCES} ${ENGINE_TABLES_HEADERS})
target_link_libraries(engine_tables engine)
target_check_style(engine_tables)
