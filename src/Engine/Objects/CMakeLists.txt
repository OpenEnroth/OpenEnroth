cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(ENGINE_OBJECTS_SOURCES
        Actor.cpp
        Chest.cpp
        CombinedSkillValue.cpp
        Decoration.cpp
        DecorationList.cpp
        ItemEnumFunctions.cpp
        Item.cpp
        MonsterEnumFunctions.cpp
        Monsters.cpp
        NPC.cpp
        ObjectList.cpp
        Character.cpp
        CharacterEnumFunctions.cpp
        SpriteObject.cpp)

set(ENGINE_OBJECTS_HEADERS
        Actor.h
        ActorEnums.h
        ActorEnumFunctions.h
        Chest.h
        ChestEnums.h
        CombinedSkillValue.h
        Decoration.h
        DecorationList.h
        DecorationEnums.h
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
        ObjectList.h
        Character.h
        CharacterEnums.h
        CharacterEnumFunctions.h
        SpriteObject.h
        SpriteEnums.h
        SpriteEnumFunctions.h
        TalkAnimation.h
        TalkAnimation.cpp)

add_library(engine_objects STATIC ${ENGINE_OBJECTS_SOURCES} ${ENGINE_OBJECTS_HEADERS})
target_check_style(engine_objects)

target_link_libraries(engine_objects PUBLIC engine gui library_random library_color utility)
