cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_TABLES_SOURCES
        AutonoteTable.cpp
        AwardTable.cpp
        ChestTable.cpp
        DecorationTable.cpp
        HostilityTable.cpp
        HistoryTable.cpp
        HouseTable.cpp
        IconFrameTable.cpp
        ItemTable.cpp
        MapTable.cpp
        MerchantTable.cpp
        MessageScrollTable.cpp
        NPCTable.cpp
        ObjectTable.cpp
        OverlayTable.cpp
        PortraitFrameTable.cpp
        QuestTable.cpp
        RosterTable.cpp
        ResourceMaskTable.cpp
        TextureFrameTable.cpp
        TileTable.cpp
        TransitionTable.cpp
        TransportTable.cpp)

set(ENGINE_TABLES_HEADERS
        AutonoteTable.h
        AwardTable.h
        ChestTable.h
        DecorationTable.h
        HostilityTable.h
        HistoryTable.h
        HouseTable.h
        IconFrameTable.h
        ItemTable.h
        MapTable.h
        MerchantTable.h
        MessageScrollTable.h
        NPCTable.h
        ObjectTable.h
        OverlayTable.h
        PortraitFrameTable.h
        QuestTable.h
        RosterTable.h
        ResourceMaskTable.h
        TextureFrameTable.h
        TileTable.h
        TransitionTable.h
        TransportTable.h)

add_library(engine_tables STATIC ${ENGINE_TABLES_SOURCES} ${ENGINE_TABLES_HEADERS})
target_link_libraries(engine_tables PUBLIC engine_data)
target_check_style(engine_tables)

if(OE_BUILD_TESTS)
    set(TEST_ENGINE_TABLES_UNIT_SOURCES
            Tests/ItemTable_ut.cpp
            Tests/NPCTable_ut.cpp
            Tests/RosterTable_ut.cpp
            Tests/TransitionTable_ut.cpp)

    add_library(test_engine_tables_unit OBJECT ${TEST_ENGINE_TABLES_UNIT_SOURCES})
    target_link_libraries(test_engine_tables_unit PUBLIC testing_unit engine_tables)

    target_check_style(test_engine_tables_unit)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_engine_tables_unit)
endif()
