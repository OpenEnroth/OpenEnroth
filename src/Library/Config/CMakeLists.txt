cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

set(LIBRARY_CONFIG_SOURCES
        AnyConfigEntry.cpp
        Config.cpp
        ConfigSection.cpp
        ConfigPatch.cpp)

set(LIBRARY_CONFIG_HEADERS
        AnyConfigEntry.h
        AnyHandler.h
        Config.h
        ConfigFwd.h
        ConfigSection.h
        ConfigEntry.h
        ConfigPatch.h)

add_library(library_config STATIC ${LIBRARY_CONFIG_SOURCES} ${LIBRARY_CONFIG_HEADERS})
target_check_style(library_config)
target_link_libraries(library_config
        PUBLIC
        utility
        library_serialization
        PRIVATE
        inicpp::inicpp)
