cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(ENGINE_OBJECTS_SOURCES
        Actor.cpp
        Chest.cpp
        ItemTable.cpp
        Items.cpp
        Monsters.cpp
        NPC.cpp
        ObjectList.cpp
        Player.cpp
        SpriteObject.cpp)

set(ENGINE_OBJECTS_HEADERS
        Actor.h
        ActorEnums.h
        Chest.h
        ItemEnchantment.h
        ItemEnums.h
        ItemTable.h
        Items.h
        Monsters.h
        NPC.h
        NPCEnums.h
        ObjectList.h
        Player.h
        PlayerEnums.h
        SpriteObject.h)

add_library(engine_objects STATIC ${ENGINE_OBJECTS_SOURCES} ${ENGINE_OBJECTS_HEADERS})
target_check_style(engine_objects)

target_link_libraries(engine_objects engine gui library_random utility)
