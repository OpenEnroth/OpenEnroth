cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(OBJECTS_SOURCES Actor.cpp
                    Chest.cpp
                    Items.cpp
                    Monsters.cpp
                    NPC.cpp
                    ObjectList.cpp
                    Player.cpp
                    SpriteObject.cpp)

set(OBJECTS_HEADERS Actor.h
                    Chest.h
                    Items.h
                    Monsters.h
                    NPC.h
                    ObjectList.h
                    Player.h
                    PlayerEnums.h
                    SpriteObject.h)

if(WIN32)
else()
  set(CMAKE_CXX_FLAGS "-fpermissive ${CMAKE_CXX_FLAGS}")
endif()

add_library(objects STATIC ${OBJECTS_SOURCES} ${OBJECTS_HEADERS})
target_link_libraries(objects engine gui)
target_check_style(objects)
