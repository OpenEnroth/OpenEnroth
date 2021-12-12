cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(TABLES_SOURCES FactionTable.cpp
                   FrameTableInc.cpp
                   IconFrameTable.cpp
                   PlayerFrameTable.cpp
                   StorylineTextTable.cpp
                   TileTable.cpp)

set(TABLES_HEADERS FactionTable.h
                   FrameTableInc.h
                   IconFrameTable.h
                   PlayerFrameTable.h
                   StorylineTextTable.h
                   TileFrameTable.h)

if(WIN32)
else()
endif()

add_library(tables STATIC ${TABLES_SOURCES} ${TABLES_HEADERS})
target_check_style(tables)
