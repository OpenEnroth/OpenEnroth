cmake_minimum_required(VERSION 3.20.4 FATAL_ERROR)

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

add_library(tables STATIC ${TABLES_SOURCES} ${TABLES_HEADERS})
target_link_libraries(tables engine)
target_check_style(tables)
