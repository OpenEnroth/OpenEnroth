cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(ENGINE_OBJECTS_SOURCES
        Actor.cpp
        Chest.cpp
        CombinedSkillValue.cpp
        ItemEnums.cpp
        Items.cpp
        MonsterEnums.cpp
        Monsters.cpp
        NPC.cpp
        ObjectList.cpp
        Character.cpp
        SpriteObject.cpp)

set(ENGINE_OBJECTS_HEADERS
        Actor.h
        ActorEnums.h
        Chest.h
        ChestEnums.h
        CombinedSkillValue.h
        ItemEnchantment.h
        ItemEnums.h
        Items.h
        Monsters.h
        MonsterEnums.h
        NPC.h
        NPCEnums.h
        ObjectList.h
        Character.h
        CharacterEnums.h
        CharacterEnumFunctions.h
        SpriteObject.h)

add_library(engine_objects STATIC ${ENGINE_OBJECTS_SOURCES} ${ENGINE_OBJECTS_HEADERS})
target_check_style(engine_objects)

target_link_libraries(engine_objects PUBLIC engine gui library_random library_color utility)
