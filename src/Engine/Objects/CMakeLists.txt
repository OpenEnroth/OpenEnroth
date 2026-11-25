cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_OBJECTS_SOURCES
        Actor.cpp
        Chest.cpp
        CombinedSkillValue.cpp
        Decoration.cpp
        ItemEnumFunctions.cpp
        Item.cpp
        MonsterEnumFunctions.cpp
        Monsters.cpp
        NPC.cpp
        Character.cpp
        CharacterEnumFunctions.cpp
        CharacterMM8Abilities.cpp
        SpriteObject.cpp
        TalkAnimation.cpp
        Inventory.cpp)

set(ENGINE_OBJECTS_HEADERS
        Actor.h
        ActorEnums.h
        ActorEnumFunctions.h
        Chest.h
        ChestEnums.h
        CombinedSkillValue.h
        Decoration.h
        ItemEnchantment.h
        ItemEnums.h
        ItemEnumFunctions.h
        Item.h
        Monsters.h
        MonsterEnums.h
        MonsterEnumFunctions.h
        NPC.h
        NPCEnums.h
        NPCEnumFunctions.h
        Character.h
        CharacterConditions.h
        CharacterEnums.h
        CharacterEnumFunctions.h
        CharacterMM8Abilities.h
        SpriteObject.h
        SpriteEnums.h
        SpriteEnumFunctions.h
        TalkAnimation.h
        Inventory.h)

add_library(engine_objects STATIC ${ENGINE_OBJECTS_SOURCES} ${ENGINE_OBJECTS_HEADERS})
target_check_style(engine_objects)

target_link_libraries(engine_objects PUBLIC engine gui library_random library_color utility)

if(OE_BUILD_TESTS)
    set(TEST_ENGINE_OBJECTS_SOURCES
            Tests/Inventory_ut.cpp
            Tests/MonsterEnumFunctions_ut.cpp)

    add_library(test_engine_objects OBJECT ${TEST_ENGINE_OBJECTS_SOURCES})
    target_link_libraries(test_engine_objects PUBLIC testing_unit engine_objects)

    target_check_style(test_engine_objects)

    target_link_libraries(OpenEnroth_GameTest PUBLIC test_engine_objects)

    # CharacterEnumFunctions_ut.cpp only tests pure enum/table logic, with no need for a running engine, so it's
    # built as a plain GoogleTest and linked into OpenEnroth_UnitTest instead of OpenEnroth_GameTest.
    set(TEST_ENGINE_OBJECTS_UNIT_SOURCES
            Tests/Character_ut.cpp
            Tests/CharacterEnumFunctions_ut.cpp
            Tests/CharacterMM8Abilities_ut.cpp
            Tests/MonsterEnumFunctionsMm8_ut.cpp
            Tests/Monsters_ut.cpp
            Tests/NPC_ut.cpp)

    add_library(test_engine_objects_unit OBJECT ${TEST_ENGINE_OBJECTS_UNIT_SOURCES})
    target_link_libraries(test_engine_objects_unit PUBLIC testing_unit engine_objects)

    target_check_style(test_engine_objects_unit)

    target_link_libraries(OpenEnroth_UnitTest PUBLIC test_engine_objects_unit)
endif()
