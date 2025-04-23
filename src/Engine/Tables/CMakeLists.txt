cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_TABLES_SOURCES
        ChestTable.cpp
        NPCTable.cpp
        ItemTable.cpp
        FactionTable.cpp
        FrameTableInc.cpp
        IconFrameTable.cpp
        PortraitFrameTable.cpp
        HistoryTable.cpp
        TileTable.cpp
        HouseTable.cpp
        AwardTable.cpp
        AutonoteTable.cpp
        QuestTable.cpp
        TransitionTable.cpp
        MerchantTable.cpp
        MessageScrollTable.cpp)

set(ENGINE_TABLES_HEADERS
        ChestTable.h
        NPCTable.h
        ItemTable.h
        FactionTable.h
        FrameTableInc.h
        IconFrameTable.h
        PortraitFrameTable.h
        HistoryTable.h
        TileTable.h
        HouseTable.h
        AwardTable.h
        AutonoteTable.h
        QuestTable.h
        TransitionTable.h
        MerchantTable.h
        MessageScrollTable.h)

add_library(engine_tables STATIC ${ENGINE_TABLES_SOURCES} ${ENGINE_TABLES_HEADERS})
target_link_libraries(engine_tables PUBLIC engine_data)
target_check_style(engine_tables)
