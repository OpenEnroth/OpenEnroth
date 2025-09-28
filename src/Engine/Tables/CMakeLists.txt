cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_TABLES_SOURCES
        AutonoteTable.cpp
        AwardTable.cpp
        ChestTable.cpp
        FactionTable.cpp
        FrameTableInc.cpp
        HistoryTable.cpp
        HouseTable.cpp
        IconFrameTable.cpp
        ItemTable.cpp
        MerchantTable.cpp
        MessageScrollTable.cpp
        NPCTable.cpp
        PortraitFrameTable.cpp
        QuestTable.cpp
        TextureFrameTable.cpp
        TileTable.cpp
        TransitionTable.cpp)

set(ENGINE_TABLES_HEADERS
        AutonoteTable.h
        AwardTable.h
        ChestTable.h
        FactionTable.h
        FrameTableInc.h
        HistoryTable.h
        HouseTable.h
        IconFrameTable.h
        ItemTable.h
        MerchantTable.h
        MessageScrollTable.h
        NPCTable.h
        PortraitFrameTable.h
        QuestTable.h
        TextureFrameTable.h
        TileTable.h
        TransitionTable.h)

add_library(engine_tables STATIC ${ENGINE_TABLES_SOURCES} ${ENGINE_TABLES_HEADERS})
target_link_libraries(engine_tables PUBLIC engine_data)
target_check_style(engine_tables)
