cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

set(OBJECTS_SOURCES Actor.cpp
                    Chest.cpp
                    Items.cpp
                    ItemTable.cpp
                    Monsters.cpp
                    NPC.cpp
                    ObjectList.cpp
                    Player.cpp
                    SpriteObject.cpp)

set(OBJECTS_HEADERS Actor.h
                    ActorEnums.h
                    Chest.h
                    ItemEnchantment.h
                    ItemEnums.h
                    Items.h
                    ItemTable.h
                    Monsters.h
                    NPC.h
                    ObjectList.h
                    Player.h
                    PlayerEnums.h
                    SpriteObject.h)

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "-fpermissive ${CMAKE_CXX_FLAGS}")
endif()

add_library(objects STATIC ${OBJECTS_SOURCES} ${OBJECTS_HEADERS})
target_check_style(objects)

target_link_libraries(objects engine gui utility)
